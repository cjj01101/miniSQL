#pragma once

#include "MiniSQLBufferManager.h"
#include "MiniSQLException.h"

#define META_PAGE_ID 0

/*                                          */
/*                  异常                    */
/*                                          */

template<typename KeyType, typename DataType>
class BPlusTree;

enum class BPlusTreeException { DuplicateKey, KeyNotExist, IteratorIllegal, IteratorOverBounds };

/*                                          */
/*                                          */
/*              B+树叶子结点                */
/*                                          */
/*                                          */

/*                                          */
/*                  定义                    */
/*                                          */

template<typename KeyType, typename DataType>
class BPlusNode {
public:
    using NodeType = BPlusNode<DataType, KeyType>;

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
                data = nullptr;
                return;
            }
            if (!(node->isLeaf)) throw BPlusTreeException::IteratorIllegal;

            buffer = node->buffer;
            filename = node->filename;
            self = node->self;
            rank = node->rank;
            keyNum = node->keyNum;
            nextLeaf = node->nextLeaf;

            data = new DataType[rank + 1];
            for (int i = 0; i < keyNum; i++) data[i] = node->data[i];
            this->offset = offset;
        }
        iter(const iter &rhs)
            : buffer(rhs.buffer), filename(rhs.filename), self(rhs.self), rank(rhs.rank), keyNum(rhs.keyNum), nextLeaf(rhs.nextLeaf), offset(rhs.offset)
        {
            data = new DataType[rank + 1];
            for (int i = 0; i < keyNum; i++) data[i] = rhs.data[i];
        };
        ~iter() { if(data != nullptr) delete[] data; }

        bool valid() const { return (self != 0); }
        void next() {
            if (!valid()) throw BPlusTreeException::IteratorOverBounds;
            if (offset < keyNum - 1) offset++;
            else if (nextLeaf) {
                NodeType nextNode(buffer, filename, nextLeaf, rank);
                self = nextNode.self;
                keyNum = nextNode.keyNum;
                nextLeaf = nextNode.nextLeaf;
                for (int i = 0; i < keyNum; i++) data[i] = nextNode.data[i];
                offset = 0;
            }
            else {
                self = 0;
                offset = 0;
            }
        }
        DataType operator*() const {
            if (!valid()) throw BPlusTreeException::IteratorOverBounds;
            return data[offset];
        }

        bool operator==(const iter &rhs) const {
            return (self == rhs.self && offset == rhs.offset);
        }
        bool operator!=(const iter &rhs) const { return !(*this == rhs); }

    private:
        BufferManager *buffer;
        string filename;
        int self;
        int rank;

        int keyNum;
        DataType *data;
        int nextLeaf;
        int offset;
    };

    BPlusNode(BufferManager *buffer, const string& filename, int self, int rank, bool isLeaf)
        : buffer(buffer), filename(filename), self(self), rank(rank), isLeaf(isLeaf), keyNum(0), prevLeaf(0), nextLeaf(0)
        , key(new KeyType[rank + 1]), child(new int[rank + 1]), data(new DataType[rank + 1]) {}
    BPlusNode(BufferManager *buffer, const string& filename, int rank, int self);
    BPlusNode(const BPlusNode &) = delete;
    ~BPlusNode() { delete[] key; delete[] child; delete[] data; }

    void writeBackToBuffer();

    int findNextPath(const KeyType &guideKey) const;
    iter getFirst() const;
    int splitNode(NodeType *parentNode);
    void print() const {
        if (isLeaf) {
            std::cout << this->keyNum << "-Leaf:";
            for (int i = 0; i < this->keyNum; i++) std::cout << "[" << this->key[i] << "," << data[i] << "]";
            std::cout << std::endl;
        } else {
            std::cout << this->keyNum << "-Internal:";
            for (int i = 0; i < this->keyNum; i++) std::cout << "[" << this->key[i] << "]";
            std::cout << std::endl;
            for (int i = 0; i <= this->keyNum; i++) {
                const NodeType childNode(buffer, filename, child[i], rank);
                childNode.print();
            }
        }
    }

    bool checkData(const KeyType &guideKey) const;
    iter findData(const KeyType &guideKey) const;
    int insertData(NodeType *parentNode, const KeyType &newKey, const DataType &newData);
    void removeData(NodeType *parentNode, const KeyType &guideKey);

    void addKey(const KeyType &newKey, int newChild, bool childAtRight = true);
    void changeKey(const KeyType &oldKey, const KeyType &newKey);
    void deleteKey(const KeyType &guideKey, bool childAtRight = true);

private:
    int splitNode_leaf(NodeType *parentNode);
    int splitNode_intern(NodeType *parentNode);
    bool checkData_leaf(const KeyType &guideKey) const;
    bool checkData_intern(const KeyType &guideKey) const;
    iter findData_leaf(const KeyType &guideKey) const;
    iter findData_intern(const KeyType &guideKey) const;
    int insertData_leaf(NodeType *parentNode, const KeyType &newKey, const DataType &newData);
    int insertData_intern(NodeType *parentNode, const KeyType &newKey, const DataType &newData);
    void removeData_leaf(NodeType *parentNode, const KeyType &guideKey);
    void removeData_intern(NodeType *parentNode, const KeyType &guideKey);

    BufferManager *buffer;
    const string filename;
    int self;
    const int rank;

    bool isLeaf;
    int keyNum;
    KeyType *key;
    int *child;
    DataType *data;
    int nextLeaf;
    int prevLeaf;

    friend class BPlusTree<KeyType, DataType>;
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename KeyType, typename DataType>
BPlusNode<KeyType, DataType>::BPlusNode(BufferManager *buffer, const string& filename, int self, int rank)
    : buffer(buffer), filename(filename), self(self), rank(rank), key(new KeyType[rank + 1]), child(new int[rank + 1]), data(new DataType[rank + 1])
{
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
    memcpy_s(key, sizeof(key[0]) * (rank + 1), nodeBuffer + p, sizeof(key[0]) * (rank + 1));
    p += sizeof(key[0]) * (rank + 1);
    memcpy_s(child, sizeof(child[0]) * (rank + 1), nodeBuffer + p, sizeof(child[0]) * (rank + 1));
    p += sizeof(child[0]) * (rank + 1);
    memcpy_s(data, sizeof(data[0]) * (rank + 1), nodeBuffer + p, sizeof(data[0]) * (rank + 1));
    p += sizeof(data[0]) * (rank + 1);
}

template<typename KeyType, typename DataType>
void BPlusNode<KeyType, DataType>::writeBackToBuffer() {
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
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(key), sizeof(key[0]) * (rank + 1));
    p += sizeof(key[0]) * (rank + 1);
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(child), sizeof(child[0]) * (rank + 1));
    p += sizeof(child[0]) * (rank + 1);
    buffer->setBlockContent(filename, self, p, reinterpret_cast<char*>(data), sizeof(data[0]) * (rank + 1));
    p += sizeof(data[0]) * (rank + 1);
}

template<typename KeyType, typename DataType>
int BPlusNode<KeyType, DataType>::findNextPath(const KeyType &guideKey) const {
    int left = 0, right = keyNum - 1;
    while (left != right) {
        int mid = (left + right) / 2;
        if (guideKey < key[mid]) right = mid;
        else left = mid + 1;
    }
    if (guideKey >= key[right]) right++;
    return right;
}

template<typename KeyType, typename DataType>
typename BPlusNode<KeyType, DataType>::iter BPlusNode<KeyType, DataType>::getFirst() const {
    if (isLeaf) {
        if (keyNum > 0) return iter(this, 0);
        else return iter(nullptr, 0);
    } else {
        const NodeType childNode(buffer, filename, child[0], rank);
        return childNode.getFirst();
    }
}

template<typename KeyType, typename DataType>
int BPlusNode<KeyType, DataType>::splitNode(NodeType *parentNode) {
    if (isLeaf) return splitNode_leaf(parentNode);
    else return splitNode_intern(parentNode);
}

template<typename KeyType, typename DataType>
int BPlusNode<KeyType, DataType>::splitNode_leaf(NodeType *parentNode) {
    int retval = 0;
    int leftKeyNum = keyNum / 2;

    int newBlock = buffer->allocNewBlock(filename);
    NodeType newNode(buffer, filename, newBlock, rank, true);

    if (this->nextLeaf) {
        NodeType nextNode(buffer, filename, nextLeaf, rank);
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
        NodeType parentNode(buffer, filename, parentBlock, rank, false);
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

template<typename KeyType, typename DataType>
int BPlusNode<KeyType, DataType>::splitNode_intern(NodeType *parentNode) {
    int retval = 0;
    int leftKeyNum = keyNum / 2;

    int newBlock = buffer->allocNewBlock(filename);
    NodeType newNode(buffer, filename, newBlock, rank, false);
    newNode.child[0] = child[leftKeyNum + 1];
    for (int i = leftKeyNum + 1; i < keyNum; i++) newNode.addKey(key[i], child[i + 1]);
    newNode.writeBackToBuffer();

    keyNum = leftKeyNum;
    if (!parentNode) {
        int parentBlock = buffer->allocNewBlock(filename);
        NodeType parentNode(buffer, filename, parentBlock, rank, false);
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

template<typename KeyType, typename DataType>
bool BPlusNode<KeyType, DataType>::checkData(const KeyType &guideKey) const {
    if (isLeaf) return checkData_leaf(guideKey);
    else return checkData_intern(guideKey);
}

template<typename KeyType, typename DataType>
typename BPlusNode<KeyType, DataType>::iter BPlusNode<KeyType, DataType>::findData(const KeyType &guideKey) const {
    if (isLeaf) return findData_leaf(guideKey);
    else return findData_intern(guideKey);
}

template<typename KeyType, typename DataType>
int BPlusNode<KeyType, DataType>::insertData(NodeType *parentNode, const KeyType &newKey, const DataType &newData) {
    if (isLeaf) return insertData_leaf(parentNode, newKey, newData);
    else return insertData_intern(parentNode, newKey, newData);
}

template<typename KeyType, typename DataType>
void BPlusNode<KeyType, DataType>::removeData(NodeType *parentNode, const KeyType &guideKey) {
    if (isLeaf) removeData_leaf(parentNode, guideKey);
    else removeData_intern(parentNode, guideKey);
}

template<typename KeyType, typename DataType>
bool BPlusNode<KeyType, DataType>::checkData_leaf(const KeyType &guideKey) const {
    int left = 0, right = keyNum - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (guideKey == key[mid]) return true;
        else if (guideKey < key[mid]) right = mid - 1;
        else left = mid + 1;
    }
    return false;
}

template<typename KeyType, typename DataType>
bool BPlusNode<KeyType, DataType>::checkData_intern(const KeyType &guideKey) const {
    int next = findNextPath(guideKey);

    const NodeType childNode(buffer, filename, child[next], rank);
    return childNode.checkData(guideKey);
}

template<typename KeyType, typename DataType>
typename BPlusNode<KeyType, DataType>::iter BPlusNode<KeyType, DataType>::findData_leaf(const KeyType &guideKey) const {
    int left = 0, right = keyNum - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (guideKey == key[mid]) return iter::iter(this, mid);
        else if (guideKey < key[mid]) right = mid - 1;
        else left = mid + 1;
    }
    return iter(nullptr, 0);
}

template<typename KeyType, typename DataType>
typename BPlusNode<KeyType, DataType>::iter BPlusNode<KeyType, DataType>::findData_intern(const KeyType &guideKey) const {
    int next = findNextPath(guideKey);

    const NodeType childNode(buffer, filename, child[next], rank);
    return childNode.findData(guideKey);
}

template<typename KeyType, typename DataType>
int BPlusNode<KeyType, DataType>::insertData_leaf(NodeType *parentNode, const KeyType &newKey, const DataType &newData) {
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

template<typename KeyType, typename DataType>
int BPlusNode<KeyType, DataType>::insertData_intern(NodeType *parentNode, const KeyType &newKey, const DataType &newData) {
    int next = findNextPath(newKey);

    NodeType childNode(buffer, filename, child[next], rank);
    childNode.insertData(this, newKey, newData);
    childNode.writeBackToBuffer();

    if (keyNum >= rank) return splitNode(parentNode);
    else return 0;
}

template<typename KeyType, typename DataType>
void BPlusNode<KeyType, DataType>::removeData_leaf(NodeType *parentNode, const KeyType &guideKey) {
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

        NodeType prevNode(buffer, filename, prev, rank);
        NodeType nextNode(buffer, filename, next, rank);

        if (prev && prevNode.keyNum > (rank + 1) / 2) {
            KeyType xKey = prevNode.key[prevNode.keyNum - 1];
            DataType xData = prevNode.data[prevNode.keyNum - 1];
            prevNode.removeData(parentNode, xKey);
            insertData(parentNode, xKey, xData);
            parentNode->changeKey(guideKey, xKey);

            prevNode.writeBackToBuffer();
        }
        else if (next && nextNode.keyNum > (rank + 1) / 2) {
            KeyType xKey = nextNode.key[0];
            KeyType parentNewKey = nextNode.key[1];
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
                NodeType newNextNode(buffer, filename, nextLeaf, rank);
                newNextNode.prevLeaf = self;
                newNextNode.writeBackToBuffer();
            }
        }
    }
}

template<typename KeyType, typename DataType>
void BPlusNode<KeyType, DataType>::removeData_intern(NodeType *parentNode, const KeyType &guideKey) {
    int next = findNextPath(guideKey);
    NodeType childNode(buffer, filename, child[next], rank);
    childNode.removeData(this, guideKey);
    childNode.writeBackToBuffer();

    if (keyNum < (rank - 1) / 2 && parentNode) {
        int ind = parentNode->findNextPath(key[0]);
        int prev = (ind > 0) ? (parentNode->child[ind - 1]) : 0;
        int next = (ind < parentNode->keyNum) ? (parentNode->child[ind + 1]) : 0;

        NodeType prevNode(buffer, filename, prev, rank);
        NodeType nextNode(buffer, filename, next, rank);

        if (prev && prevNode.keyNum > (rank - 1) / 2) {
            KeyType pKey = parentNode->key[ind - 1];
            KeyType sKey = prevNode.key[prevNode.keyNum - 1];
            int sChild = prevNode.child[prevNode.keyNum];
            addKey(pKey, sChild, false);
            parentNode->changeKey(pKey, sKey);
            prevNode.deleteKey(sKey);

            prevNode.writeBackToBuffer();
        }
        else if (next && nextNode.keyNum > (rank - 1) / 2) {
            KeyType pKey = parentNode->key[ind];
            KeyType sKey = nextNode.key[0];
            int sChild = nextNode.child[0];
            addKey(pKey, sChild);
            parentNode->changeKey(pKey, sKey);
            nextNode.deleteKey(sKey, false);

            nextNode.writeBackToBuffer();
        }
        else if (prev) {
            KeyType pKey = parentNode->key[ind - 1];
            prevNode.addKey(pKey, child[0]);
            for (int i = 0; i < keyNum; i++) prevNode.addKey(key[i], child[i + 1]);
            parentNode->deleteKey(pKey);

            prevNode.writeBackToBuffer();
        }
        else if (next) {
            KeyType pKey = parentNode->key[ind];
            addKey(pKey, nextNode.child[0]);
            for (int i = 0; i < nextNode.keyNum; i++) addKey(nextNode.key[i], nextNode.child[i + 1]);
            parentNode->deleteKey(pKey);
        }
    }
}

template<typename KeyType, typename DataType>
void BPlusNode<KeyType, DataType>::addKey(const KeyType &newKey, int newChild, bool childAtRight) {
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

template<typename KeyType, typename DataType>
void BPlusNode<KeyType, DataType>::changeKey(const KeyType &oldKey, const KeyType &newKey) {
    int i = findNextPath(oldKey) - 1;
    if (0 > i) throw BPlusTreeException::KeyNotExist;
    key[i] = newKey;
}

template<typename KeyType, typename DataType>
void BPlusNode<KeyType, DataType>::deleteKey(const KeyType &guideKey, bool childAtRight) {
    int i = findNextPath(guideKey) - 1;
    if (0 > i) throw BPlusTreeException::KeyNotExist;
    for (; i < this->keyNum - 1; i++) {
        key[i] = key[i + 1];
        child[i + childAtRight] = child[i + 1 + childAtRight];
    }
    if (!childAtRight) child[i] = child[i + 1];
    keyNum--;
}

/*                                          */
/*                                          */
/*                  B+树                    */
/*                                          */
/*                                          */

/*                                          */
/*                  定义                    */
/*                                          */

class BPlusTreeInterface {
    virtual void print() const {};
}; //供IndexManager用

template<typename KeyType, typename DataType>
class BPlusTree: public BPlusTreeInterface {
public:
    using NodeType = BPlusNode<KeyType, DataType>;
    BPlusTree(BufferManager *buffer, const string &filename, int rank);
    ~BPlusTree() = default;

    bool checkData(const KeyType &key) const;
    typename NodeType::iter findData(const KeyType &key) const;
    void insertData(const KeyType &key, const DataType &data);
    void removeData(const KeyType &key);

    const typename NodeType::iter begin();
    const typename NodeType::iter end() { return NodeType::iter::iter(nullptr, 0); }

    void print() const {
        const NodeType rootNode(buffer, filename, root, rank);
        rootNode.print();
    }

private:
    BufferManager *buffer;
    const string filename;
    int root;
    const int rank;
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename KeyType, typename DataType>
BPlusTree<KeyType, DataType>::BPlusTree(BufferManager *buffer, const string &filename, int rank) : buffer(buffer), filename(filename), rank(rank) {
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
        NodeType rootNode(buffer, filename, root, rank, true);
        rootNode.writeBackToBuffer();
    }
}

template<typename KeyType, typename DataType>
bool BPlusTree<KeyType, DataType>::checkData(const KeyType &key) const {
    const NodeType rootNode(buffer, filename, root, rank);
    return rootNode.checkData(key);
}

template<typename KeyType, typename DataType>
typename BPlusNode<KeyType, DataType>::iter BPlusTree<KeyType, DataType>::findData(const KeyType &key) const {
    const NodeType rootNode(buffer, filename, root, rank);
    return rootNode.findData(key);
}

template<typename KeyType, typename DataType>
void BPlusTree<KeyType, DataType>::insertData(const KeyType &key, const DataType &data){
    NodeType rootNode(buffer, filename, root, rank);
    int newRoot = rootNode.insertData(nullptr, key, data);
    rootNode.writeBackToBuffer();
    if (newRoot) {
        root = newRoot;
        buffer->setBlockContent(filename, META_PAGE_ID, 0, reinterpret_cast<char*>(&root), sizeof(root));
    }
}

template<typename KeyType, typename DataType>
void BPlusTree<KeyType, DataType>::removeData(const KeyType &key) {
    NodeType rootNode(buffer, filename, root, rank);
    rootNode.removeData(nullptr, key);
    rootNode.writeBackToBuffer();
    if (0 == rootNode.keyNum && false == rootNode.isLeaf) {
        root = rootNode.child[0];
        NodeType newRootNode(buffer, filename, root, rank);
        buffer->setBlockContent(filename, META_PAGE_ID, 0, reinterpret_cast<char*>(&root), sizeof(root));
    }
}

template<typename KeyType, typename DataType>
const typename BPlusNode<KeyType, DataType>::iter BPlusTree<KeyType, DataType>::begin() {
    const NodeType rootNode(buffer, filename, root, rank);
    return rootNode.getFirst();
}