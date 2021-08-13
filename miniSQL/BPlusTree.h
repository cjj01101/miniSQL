#pragma once

#include "MiniSQLBufferManager.h"
#include "MiniSQLMeta.h"
#include "MiniSQLException.h"

#define META_PAGE_ID 0

/*                                          */
/*                  异常                    */
/*                                          */

template<typename DataType>
class BPlusTree;

/*                                          */
/*                                          */
/*              B+树叶子结点                */
/*                                          */
/*                                          */

/*                                          */
/*                  定义                    */
/*                                          */

template<typename DataType>
class BPlusNode {
public:
    using NodeType = BPlusNode<DataType>;

    /*                                          */
    /*                                          */
    /*             叶子结点迭代器               */
    /*                                          */
    /*                                          */

    class iter {
    public:
        iter(const NodeType *node, int offset) {
            if (node == nullptr) {
                self = 0;
                this->offset = 0;
                key = nullptr;
                data = nullptr;
                return;
            }
            if (!(node->isLeaf)) throw BPlusTreeException::IteratorIllegal;

            buffer = node->buffer;
            filename = node->filename;
            self = node->self;
            type = node->type;
            rank = node->rank;
            keyNum = node->keyNum;
            nextLeaf = node->nextLeaf;

            key = new Value[rank + 1];
            data = new DataType[rank + 1];
            for (int i = 0; i < keyNum; i++) {
                key[i] = node->key[i];
                data[i] = node->data[i];
            }
            this->offset = offset;
        }
        iter(const iter &rhs)
            : buffer(rhs.buffer), filename(rhs.filename), self(rhs.self), type(rhs.type), rank(rhs.rank), keyNum(rhs.keyNum), nextLeaf(rhs.nextLeaf), offset(rhs.offset)
        {
            if (self == 0) {
                key = nullptr;
                data = nullptr;
                return;
            }
            key = new Value[rank + 1];
            data = new DataType[rank + 1];
            for (int i = 0; i < keyNum; i++) {
                key[i] = rhs.key[i];
                data[i] = rhs.data[i];
            }
        };
        ~iter() { if (key != nullptr) delete[] key; if (data != nullptr) delete[] data; }

        bool valid() const { return (self != 0); }
        void next() {
            if (!valid()) throw BPlusTreeException::IteratorOverBounds;
            if (offset < keyNum - 1) offset++;
            else if (nextLeaf) {
                NodeType nextNode(buffer, filename, nextLeaf, type, rank);
                self = nextNode.self;
                keyNum = nextNode.keyNum;
                nextLeaf = nextNode.nextLeaf;
                for (int i = 0; i < keyNum; i++) {
                    key[i] = nextNode.key[i];
                    data[i] = nextNode.data[i];
                }
                offset = 0;
            } else {
                self = 0;
                offset = 0;
            }
        }
        std::pair<Value, DataType> operator*() const {
            if (!valid()) throw BPlusTreeException::IteratorOverBounds;
            return std::make_pair(key[offset], data[offset]);
        }

        bool operator==(const iter &rhs) const {
            return (self == rhs.self && offset == rhs.offset);
        }
        bool operator!=(const iter &rhs) const { return !(*this == rhs); }

    private:
        BufferManager *buffer;
        string filename;
        int self;
        Type type;
        int rank;

        int keyNum;
        Value *key;
        DataType *data;
        int nextLeaf;
        int offset;
    };

    BPlusNode(BufferManager *buffer, const string& filename, int self, const Type &type, int rank, bool isLeaf);
    BPlusNode(BufferManager *buffer, const string& filename, int self, const Type &type, int rank);
    BPlusNode(const BPlusNode &) = delete;
    ~BPlusNode() { delete[] key; delete[] child; delete[] data; }

    void writeBackToBuffer();

    int findNextPath(const Value &guideKey) const;
    int splitNode(NodeType *parentNode);

    bool checkData(const Value &guideKey) const;
    int insertData(NodeType *parentNode, const Value &newKey, const DataType &newData);
    void removeData(NodeType *parentNode, const Value &guideKey);

    void addKey(const Value &newKey, int newChild, bool childAtRight = true);
    void changeKey(const Value &oldKey, const Value &newKey);
    void deleteKey(const Value &guideKey, bool childAtRight = true);

    iter getFirst() const;
    iter getStart(const Value &guideKey, bool canEqual) const;

private:
    int splitNode_leaf(NodeType *parentNode);
    int splitNode_intern(NodeType *parentNode);
    bool checkData_leaf(const Value &guideKey) const;
    bool checkData_intern(const Value &guideKey) const;
    int insertData_leaf(NodeType *parentNode, const Value &newKey, const DataType &newData);
    int insertData_intern(NodeType *parentNode, const Value &newKey, const DataType &newData);
    void removeData_leaf(NodeType *parentNode, const Value &guideKey);
    void removeData_intern(NodeType *parentNode, const Value &guideKey);
    iter getStart_leaf(const Value &guideKey, bool canEqual) const;
    iter getStart_intern(const Value &guideKey, bool canEqual) const;

    BufferManager *buffer;
    const string filename;
    int self;
    const Type type;
    const int rank;

    bool isLeaf;
    int keyNum;
    Value *key;
    int *child;
    DataType *data;
    int nextLeaf;
    int prevLeaf;

    friend class BPlusTree<DataType>;
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename DataType>
BPlusNode<DataType>::BPlusNode(BufferManager *buffer, const string& filename, int self, const Type &type, int rank, bool isLeaf)
    : buffer(buffer), filename(filename), self(self), type(type), rank(rank), isLeaf(isLeaf), keyNum(0), prevLeaf(0), nextLeaf(0)
    , key(new Value[rank + 1]), child(new int[rank + 1]), data(new DataType[rank + 1])
{
    Value value(type, nullptr);
    for (int i = 0; i < rank + 1; i++)
        key[i] = value;
}

template<typename DataType>
BPlusNode<DataType>::BPlusNode(BufferManager *buffer, const string& filename, int self, const Type &type, int rank)
    : buffer(buffer), filename(filename), type(type), self(self), rank(rank), key(new Value[rank + 1]), child(new int[rank + 1]), data(new DataType[rank + 1])
{
    Value value(type, nullptr);
    for (int i = 0; i < rank + 1; i++) key[i] = value;

    char *nodeBuffer = buffer->getBlockContent(filename, self);

    int p = 0;
    memcpy_s(&isLeaf, sizeof(isLeaf), nodeBuffer + p, sizeof(isLeaf));
    p += sizeof(isLeaf);
    memcpy_s(&keyNum, sizeof(keyNum), nodeBuffer + p, sizeof(keyNum));
    p += sizeof(keyNum);
    memcpy_s(&prevLeaf, sizeof(prevLeaf), nodeBuffer + p, sizeof(prevLeaf));
    p += sizeof(prevLeaf);
    memcpy_s(&nextLeaf, sizeof(nextLeaf), nodeBuffer + p, sizeof(nextLeaf));
    p += sizeof(nextLeaf);
    for (int i = 0; i < rank + 1; i++) {
        memcpy_s(key[i].data, type.size, nodeBuffer + p, type.size);
        p += type.size;
    }
    memcpy_s(child, sizeof(child[0]) * (rank + 1), nodeBuffer + p, sizeof(child[0]) * (rank + 1));
    p += sizeof(child[0]) * (rank + 1);
    memcpy_s(data, sizeof(data[0]) * (rank + 1), nodeBuffer + p, sizeof(data[0]) * (rank + 1));
    p += sizeof(data[0]) * (rank + 1);
}

template<typename DataType>
void BPlusNode<DataType>::writeBackToBuffer() {
    if (self == 0) return;

    int p = 0;
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));
    p += sizeof(isLeaf);
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(&keyNum), sizeof(keyNum));
    p += sizeof(keyNum);
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(&prevLeaf), sizeof(prevLeaf));
    p += sizeof(prevLeaf);
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(&nextLeaf), sizeof(nextLeaf));
    p += sizeof(nextLeaf);
    for (int i = 0; i < rank + 1; i++) {
        buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(key[i].data), type.size);
        p += type.size;
    }
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(child), sizeof(child[0]) * (rank + 1));
    p += sizeof(child[0]) * (rank + 1);
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(data), sizeof(data[0]) * (rank + 1));
    p += sizeof(data[0]) * (rank + 1);
}

template<typename DataType>
int BPlusNode<DataType>::findNextPath(const Value &guideKey) const {
    int left = 0, right = keyNum - 1;
    while (left != right) {
        int mid = (left + right) / 2;
        if (guideKey < key[mid]) right = mid;
        else left = mid + 1;
    }
    if (guideKey >= key[right]) right++;
    return right;
}

template<typename DataType>
int BPlusNode<DataType>::splitNode(NodeType *parentNode) {
    if (isLeaf) return splitNode_leaf(parentNode);
    else return splitNode_intern(parentNode);
}

template<typename DataType>
int BPlusNode<DataType>::splitNode_leaf(NodeType *parentNode) {
    int retval = 0;
    int leftKeyNum = keyNum / 2;

    int newBlock = buffer->allocNewBlock(filename);
    NodeType newNode(buffer, filename, newBlock, type, rank, true);

    if (this->nextLeaf) {
        NodeType nextNode(buffer, filename, nextLeaf, type, rank);
        nextNode.prevLeaf = newBlock;
        nextNode.writeBackToBuffer();
    }

    newNode.nextLeaf = nextLeaf;
    newNode.prevLeaf = self;
    this->nextLeaf = newBlock;
    for (int i = leftKeyNum; i < keyNum; i++) newNode.insertData(parentNode, key[i], data[i]);
    newNode.writeBackToBuffer();

    this->keyNum = leftKeyNum;
    if (!parentNode) {
        int parentBlock = buffer->allocNewBlock(filename);
        NodeType parentNode(buffer, filename, parentBlock, type, rank, false);
        parentNode.child[0] = self;
        parentNode.addKey(key[leftKeyNum], newBlock);
        parentNode.writeBackToBuffer();
        retval = parentBlock;
    }
    else {
        parentNode->addKey(key[leftKeyNum], newBlock);
    }

    return retval;
}

template<typename DataType>
int BPlusNode<DataType>::splitNode_intern(NodeType *parentNode) {
    int retval = 0;
    int leftKeyNum = keyNum / 2;

    int newBlock = buffer->allocNewBlock(filename);
    NodeType newNode(buffer, filename, newBlock, type, rank, false);
    newNode.child[0] = child[leftKeyNum + 1];
    for (int i = leftKeyNum + 1; i < keyNum; i++) newNode.addKey(key[i], child[i + 1]);
    newNode.writeBackToBuffer();

    keyNum = leftKeyNum;
    if (!parentNode) {
        int parentBlock = buffer->allocNewBlock(filename);
        NodeType parentNode(buffer, filename, parentBlock, type, rank, false);
        parentNode.child[0] = self;
        parentNode.addKey(key[leftKeyNum], newBlock);
        parentNode.writeBackToBuffer();
        retval = parentBlock;
    }
    else {
        parentNode->addKey(key[leftKeyNum], newBlock);
    }

    return retval;
}

template<typename DataType>
bool BPlusNode<DataType>::checkData(const Value &guideKey) const {
    if (isLeaf) return checkData_leaf(guideKey);
    else return checkData_intern(guideKey);
}

template<typename DataType>
int BPlusNode<DataType>::insertData(NodeType *parentNode, const Value &newKey, const DataType &newData) {
    if (isLeaf) return insertData_leaf(parentNode, newKey, newData);
    else return insertData_intern(parentNode, newKey, newData);
}

template<typename DataType>
void BPlusNode<DataType>::removeData(NodeType *parentNode, const Value &guideKey) {
    if (isLeaf) removeData_leaf(parentNode, guideKey);
    else removeData_intern(parentNode, guideKey);
}

template<typename DataType>
bool BPlusNode<DataType>::checkData_leaf(const Value &guideKey) const {
    int left = 0, right = keyNum - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (guideKey == key[mid]) return true;
        else if (guideKey < key[mid]) right = mid - 1;
        else left = mid + 1;
    }
    return false;
}

template<typename DataType>
bool BPlusNode<DataType>::checkData_intern(const Value &guideKey) const {
    int next = findNextPath(guideKey);

    const NodeType childNode(buffer, filename, child[next], type, rank);
    return childNode.checkData(guideKey);
}

template<typename DataType>
int BPlusNode<DataType>::insertData_leaf(NodeType *parentNode, const Value &newKey, const DataType &newData) {
    if (checkData(newKey)) throw BPlusTreeException::DuplicateKey;

    int i;
    for (i = keyNum; i > 0 && key[i - 1] > newKey; i--) {
        key[i] = key[i - 1];
        data[i] = data[i - 1];
    }
    key[i] = newKey;
    data[i] = newData;

    if (++keyNum > rank) return splitNode(parentNode);
    else return 0;
}

template<typename DataType>
int BPlusNode<DataType>::insertData_intern(NodeType *parentNode, const Value &newKey, const DataType &newData) {
    int next = findNextPath(newKey);

    NodeType childNode(buffer, filename, child[next], type, rank);
    childNode.insertData(this, newKey, newData);
    childNode.writeBackToBuffer();

    if (keyNum >= rank) return splitNode(parentNode);
    else return 0;
}

template<typename DataType>
void BPlusNode<DataType>::removeData_leaf(NodeType *parentNode, const Value &guideKey) {
    if (!checkData(guideKey)) throw BPlusTreeException::KeyNotExist;

    int i = 0;
    while (key[i] != guideKey) i++;
    for (; i < keyNum - 1; i++) {
        key[i] = key[i + 1];
        data[i] = data[i + 1];
    }

    if (--keyNum < (rank + 1) / 2 && parentNode) {
        int ind = parentNode->findNextPath(key[0]);
        int prev = (ind > 0) ? (parentNode->child[ind - 1]) : 0;
        int next = (ind < parentNode->keyNum) ? (parentNode->child[ind + 1]) : 0;

        NodeType prevNode(buffer, filename, prev, type, rank);
        NodeType nextNode(buffer, filename, next, type, rank);

        if (prev && prevNode.keyNum > (rank + 1) / 2) {
            Value xKey = prevNode.key[prevNode.keyNum - 1];
            DataType xData = prevNode.data[prevNode.keyNum - 1];
            prevNode.removeData(parentNode, xKey);
            insertData(parentNode, xKey, xData);
            parentNode->changeKey(guideKey, xKey);

            prevNode.writeBackToBuffer();
        }
        else if (next && nextNode.keyNum > (rank + 1) / 2) {
            Value xKey = nextNode.key[0];
            Value parentNewKey = nextNode.key[1];
            DataType xData = nextNode.data[0];
            nextNode.removeData(parentNode, xKey);
            insertData(parentNode, xKey, xData);
            parentNode->changeKey(xKey, parentNewKey);

            nextNode.writeBackToBuffer();
        }
        else if (prev) {
            for (int i = 0; i < keyNum; i++) prevNode.insertData(nullptr, key[i], data[i]);
            parentNode->deleteKey(key[0]);
            prevNode.nextLeaf = nextLeaf;
            if (nextLeaf) nextNode.prevLeaf = prevLeaf;

            prevNode.writeBackToBuffer();
        }
        else if (next) {
            for (int i = 0; i < nextNode.keyNum; i++) insertData(nullptr, nextNode.key[i], nextNode.data[i]);
            parentNode->deleteKey(nextNode.key[0]);
            nextLeaf = nextNode.nextLeaf;
            if (nextLeaf) {
                NodeType newNextNode(buffer, filename, nextLeaf, type, rank);
                newNextNode.prevLeaf = self;
                newNextNode.writeBackToBuffer();
            }
        }
    }
}

template<typename DataType>
void BPlusNode<DataType>::removeData_intern(NodeType *parentNode, const Value &guideKey) {
    int next = findNextPath(guideKey);
    NodeType childNode(buffer, filename, child[next], type, rank);
    childNode.removeData(this, guideKey);
    childNode.writeBackToBuffer();

    if (keyNum < (rank - 1) / 2 && parentNode) {
        int ind = parentNode->findNextPath(key[0]);
        int prev = (ind > 0) ? (parentNode->child[ind - 1]) : 0;
        int next = (ind < parentNode->keyNum) ? (parentNode->child[ind + 1]) : 0;

        NodeType prevNode(buffer, filename, prev, type, rank);
        NodeType nextNode(buffer, filename, next, type, rank);

        if (prev && prevNode.keyNum > (rank - 1) / 2) {
            Value pKey = parentNode->key[ind - 1];
            Value sKey = prevNode.key[prevNode.keyNum - 1];
            int sChild = prevNode.child[prevNode.keyNum];
            addKey(pKey, sChild, false);
            parentNode->changeKey(pKey, sKey);
            prevNode.deleteKey(sKey);

            prevNode.writeBackToBuffer();
        }
        else if (next && nextNode.keyNum > (rank - 1) / 2) {
            Value pKey = parentNode->key[ind];
            Value sKey = nextNode.key[0];
            int sChild = nextNode.child[0];
            addKey(pKey, sChild);
            parentNode->changeKey(pKey, sKey);
            nextNode.deleteKey(sKey, false);

            nextNode.writeBackToBuffer();
        }
        else if (prev) {
            Value pKey = parentNode->key[ind - 1];
            prevNode.addKey(pKey, child[0]);
            for (int i = 0; i < keyNum; i++) prevNode.addKey(key[i], child[i + 1]);
            parentNode->deleteKey(pKey);

            prevNode.writeBackToBuffer();
        }
        else if (next) {
            Value pKey = parentNode->key[ind];
            addKey(pKey, nextNode.child[0]);
            for (int i = 0; i < nextNode.keyNum; i++) addKey(nextNode.key[i], nextNode.child[i + 1]);
            parentNode->deleteKey(pKey);
        }
    }
}

template<typename DataType>
void BPlusNode<DataType>::addKey(const Value &newKey, int newChild, bool childAtRight) {
    int i;
    if (!childAtRight) child[keyNum + 1] = child[keyNum];
    for (i = keyNum; i > 0 && key[i - 1] > newKey; i--) {
        key[i] = key[i - 1];
        child[i + childAtRight] = child[i - 1 + childAtRight];
    }
    key[i] = newKey;
    child[i + childAtRight] = newChild;

    keyNum++;
}

template<typename DataType>
void BPlusNode<DataType>::changeKey(const Value &oldKey, const Value &newKey) {
    int i = findNextPath(oldKey) - 1;
    if (0 > i) throw BPlusTreeException::KeyNotExist;
    key[i] = newKey;
}

template<typename DataType>
void BPlusNode<DataType>::deleteKey(const Value &guideKey, bool childAtRight) {
    int i = findNextPath(guideKey) - 1;
    if (0 > i) throw BPlusTreeException::KeyNotExist;
    for (; i < this->keyNum - 1; i++) {
        key[i] = key[i + 1];
        child[i + childAtRight] = child[i + 1 + childAtRight];
    }
    if (!childAtRight) child[i] = child[i + 1];
    keyNum--;
}

template<typename DataType>
typename BPlusNode<DataType>::iter BPlusNode<DataType>::getFirst() const {
    if (isLeaf) {
        if (keyNum > 0) return iter(this, 0);
        else return iter(nullptr, 0);
    }
    else {
        const NodeType childNode(buffer, filename, child[0], type, rank);
        return childNode.getFirst();
    }
}

template<typename DataType>
typename BPlusNode<DataType>::iter BPlusNode<DataType>::getStart(const Value &guideKey, bool canEqual) const {
    if (isLeaf) return getStart_leaf(guideKey, canEqual);
    else return getStart_intern(guideKey, canEqual);
}

template<typename DataType>
typename BPlusNode<DataType>::iter BPlusNode<DataType>::getStart_leaf(const Value &guideKey, bool canEqual) const {
    if (keyNum > 0 && (key[keyNum - 1] < guideKey || (key[keyNum - 1] == guideKey && !canEqual))) {
        iter it(this, keyNum - 1);
        it.next();
        return it;
    }
    for (int i = 0; i < keyNum; i++) {
        if (key[i] > guideKey || (key[i] == guideKey && canEqual)) return iter(this, i);
    }
    return iter(nullptr, 0);
}

template<typename DataType>
typename BPlusNode<DataType>::iter BPlusNode<DataType>::getStart_intern(const Value &guideKey, bool canEqual) const {
    int next = findNextPath(guideKey);

    const NodeType childNode(buffer, filename, child[next], type, rank);
    return childNode.getStart(guideKey, canEqual);
}

/*                                          */
/*                                          */
/*                  B+树                    */
/*                                          */
/*                                          */

/*                                          */
/*                  定义                    */
/*                                          */

template<typename DataType>
class BPlusTree {
public:
    using NodeType = BPlusNode<DataType>;
    BPlusTree(BufferManager *buffer, const string &filename, const Type &type, int rank);
    ~BPlusTree() = default;

    bool checkData(const Value &key) const;
    void insertData(const Value &key, const DataType &data);
    void removeData(const Value &key);

    const typename NodeType::iter begin();
    const typename NodeType::iter end() { return NodeType::iter::iter(nullptr, 0); }
    typename NodeType::iter getStart(const Value &key, bool canEqual) const;

    /*void print() const {
        const NodeType rootNode(buffer, filename, root, rank);
        rootNode.print();
    }*/

private:
    BufferManager *buffer;
    const string filename;
    int root;
    const Type type;
    const int rank;
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename DataType>
BPlusTree<DataType>::BPlusTree(BufferManager *buffer, const string &filename, const Type &type, int rank)
    : buffer(buffer), filename(filename), type(type), rank(rank) {
    try {
        char *meta = buffer->getBlockContent(filename, META_PAGE_ID);
        root = reinterpret_cast<int*>(meta)[0];
    }
    catch (MiniSQLException) {
        FILE *fp;
        fopen_s(&fp, filename.data(), "w");
        fclose(fp);
        buffer->allocNewBlock(filename);
        root = buffer->allocNewBlock(filename);
        buffer->setBlockContent(filename, META_PAGE_ID, 0, reinterpret_cast<char*>(&root), sizeof(root));
        NodeType rootNode(buffer, filename, root, type, rank, true);
        rootNode.writeBackToBuffer();
    }
}

template<typename DataType>
bool BPlusTree<DataType>::checkData(const Value &key) const {
    const NodeType rootNode(buffer, filename, root, type, rank);
    return rootNode.checkData(key);
}

template<typename DataType>
void BPlusTree<DataType>::insertData(const Value &key, const DataType &data){
    NodeType rootNode(buffer, filename, root, type, rank);
    int newRoot = rootNode.insertData(nullptr, key, data);
    rootNode.writeBackToBuffer();
    if (newRoot) {
        root = newRoot;
        buffer->setBlockContent(filename, META_PAGE_ID, 0, reinterpret_cast<char*>(&root), sizeof(root));
    }
}

template<typename DataType>
void BPlusTree<DataType>::removeData(const Value &key) {
    NodeType rootNode(buffer, filename, root, type, rank);
    rootNode.removeData(nullptr, key);
    rootNode.writeBackToBuffer();
    if (0 == rootNode.keyNum && false == rootNode.isLeaf) {
        root = rootNode.child[0];
        NodeType newRootNode(buffer, filename, root, type, rank);
        buffer->setBlockContent(filename, META_PAGE_ID, 0, reinterpret_cast<char*>(&root), sizeof(root));
    }
}

template<typename DataType>
const typename BPlusNode<DataType>::iter BPlusTree<DataType>::begin() {
    const NodeType rootNode(buffer, filename, root, type, rank);
    return rootNode.getFirst();
}

template<typename DataType>
typename BPlusNode<DataType>::iter BPlusTree<DataType>::getStart(const Value &key, bool canEqual) const {
    const NodeType rootNode(buffer, filename, root, type, rank);
    return rootNode.getStart(key, canEqual);
}