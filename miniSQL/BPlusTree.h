#pragma once

#include "MiniSQLBufferManager.h"
#include "MiniSQLException.h"

#define META_PAGE_ID 0

/*                                          */
/*                  异常                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
class BPlusTree;

enum class BPlusTreeException { DuplicateKey, KeyNotExist, IteratorOverBounds };

/*                                          */
/*                                          */
/*              B+树叶子结点                */
/*                                          */
/*                                          */

/*                                          */
/*                  定义                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
class BPlusNode {
public:
    struct storage{
        storage() {}
        storage(const BPlusNode<KeyType, DataType, rank> &node) {
            isLeaf = node.isLeaf;
            keyNum = node.keyNum;
            nextLeaf = node.nextLeaf;
            prevLeaf = node.prevLeaf;
            for (int i = 0; i < rank + 1; i++) {
                key[i] = node.key[i];
                data[i] = node.data[i];
                child[i] = node.child[i];
            }
        }
        bool isLeaf;
        int keyNum;
        KeyType key[rank + 1];
        int parent;
        int nextLeaf;
        int prevLeaf;
        int child[rank + 1];
        DataType data[rank + 1];
    };

    using NodeType = BPlusNode<DataType, KeyType, rank>;
    BPlusNode(BufferManager *buffer, const string& filename, int self, bool isLeaf)
        : buffer(buffer), filename(filename), self(self), isLeaf(isLeaf), keyNum(0), prevLeaf(0), nextLeaf(0) {}
    BPlusNode(BufferManager *buffer, const string& filename, int self);
    BPlusNode(const BPlusNode &) = delete;
    ~BPlusNode() = default;

    void writeBackToBuffer() {
        if (self == 0) return;
        auto st = storage(*this);
        buffer->setBlockContent(filename, self, 0, reinterpret_cast<char*>(&st), sizeof(st));
    }

    int splitNode(NodeType *parentNode);
    int findNextPath(const KeyType &guideKey) const;
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
                const NodeType childNode(buffer, filename, child[i]);
                childNode.print();
            }
        }
    }

    bool findData(const KeyType &guideKey) const;
    int insertData(NodeType *parentNode, const KeyType &newKey, const DataType &newData);
    void removeData(NodeType *parentNode, const KeyType &guideKey);

    void addKey(const KeyType &newKey, int newChild, bool childAtRight = true);
    void changeKey(const KeyType &oldKey, const KeyType &newKey);
    void deleteKey(const KeyType &guideKey, bool childAtRight = true);

private:
    int splitNode_leaf(NodeType *parentNode);
    int splitNode_intern(NodeType *parentNode);
    bool findData_leaf(const KeyType &guideKey) const;
    bool findData_intern(const KeyType &guideKey) const;
    int insertData_leaf(NodeType *parentNode, const KeyType &newKey, const DataType &newData);
    int insertData_intern(NodeType *parentNode, const KeyType &newKey, const DataType &newData);
    void removeData_leaf(NodeType *parentNode, const KeyType &guideKey);
    void removeData_intern(NodeType *parentNode, const KeyType &guideKey);

    BufferManager *buffer;
    const string filename;
    int self;

    bool isLeaf;
    int keyNum;
    KeyType key[rank + 1];
    int child[rank + 1];
    DataType data[rank + 1];
    int nextLeaf;
    int prevLeaf;

    friend class BPlusTree<KeyType, DataType, rank>;
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
BPlusNode<KeyType, DataType, rank>::BPlusNode(BufferManager *buffer, const string& filename, int self) : buffer(buffer), filename(filename), self(self)
{
    char *nodeBuffer = buffer->getBlockContent(filename, self);
    storage st;
    memcpy_s(&st, sizeof(st), nodeBuffer, sizeof(st));
    isLeaf = st.isLeaf;
    keyNum = st.keyNum;
    nextLeaf = st.nextLeaf;
    prevLeaf = st.prevLeaf;
    for (int i = 0; i < rank + 1; i++) {
        key[i] = st.key[i];
        data[i] = st.data[i];
        child[i] = st.child[i];
    }
}

template<typename KeyType, typename DataType, int rank>
int BPlusNode<KeyType, DataType, rank>::splitNode(NodeType *parentNode) {
    if (isLeaf) return splitNode_leaf(parentNode);
    else return splitNode_intern(parentNode);
}

template<typename KeyType, typename DataType, int rank>
int BPlusNode<KeyType, DataType, rank>::splitNode_leaf(NodeType *parentNode) {
    int retval = 0;
    int leftKeyNum = keyNum / 2;

    int newBlock = buffer->allocNewBlock(filename);
    NodeType newNode(buffer, filename, newBlock, true);

    if (this->nextLeaf) {
        NodeType nextNode(buffer, filename, nextLeaf);
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
        NodeType parentNode(buffer, filename, parentBlock, false);
        parentNode.child[0] = self;
        parentNode.addKey(key[leftKeyNum], newBlock);
        parentNode.writeBackToBuffer();
        retval = parentBlock;
    } else {
        parentNode->addKey(key[leftKeyNum], newBlock);
    }

    return retval;
}

template<typename KeyType, typename DataType, int rank>
int BPlusNode<KeyType, DataType, rank>::splitNode_intern(NodeType *parentNode) {
    int retval = 0;
    int leftKeyNum = keyNum / 2;

    int newBlock = buffer->allocNewBlock(filename);
    NodeType newNode(buffer, filename, newBlock, false);
    newNode.child[0] = child[leftKeyNum + 1];
    for (int i = leftKeyNum + 1; i < keyNum; i++) newNode.addKey(key[i], child[i + 1]);
    newNode.writeBackToBuffer();

    keyNum = leftKeyNum;
    if (!parentNode) {
        int parentBlock = buffer->allocNewBlock(filename);
        NodeType parentNode(buffer, filename, parentBlock, false);
        parentNode.child[0] = self;
        parentNode.addKey(key[leftKeyNum], newBlock);
        parentNode.writeBackToBuffer();
        retval = parentBlock;
    } else {
        parentNode->addKey(key[leftKeyNum], newBlock);
    }

    return retval;
}

template<typename KeyType, typename DataType, int rank>
int BPlusNode<KeyType, DataType, rank>::findNextPath(const KeyType &guideKey) const {
    int left = 0, right = keyNum - 1;
    while (left != right) {
        int mid = (left + right) / 2;
        if (guideKey < key[mid]) right = mid;
        else left = mid + 1;
    }
    if (guideKey >= key[right]) right++;
    return right;
}

template<typename KeyType, typename DataType, int rank>
bool BPlusNode<KeyType, DataType, rank>::findData(const KeyType &guideKey) const {
    if (isLeaf) return findData_leaf(guideKey);
    else return findData_intern(guideKey);
}

template<typename KeyType, typename DataType, int rank>
int BPlusNode<KeyType, DataType, rank>::insertData(NodeType *parentNode, const KeyType &newKey, const DataType &newData) {
    if (isLeaf) return insertData_leaf(parentNode, newKey, newData);
    else return insertData_intern(parentNode, newKey, newData);
}

template<typename KeyType, typename DataType, int rank>
void BPlusNode<KeyType, DataType, rank>::removeData(NodeType *parentNode, const KeyType &guideKey) {
    if (isLeaf) removeData_leaf(parentNode, guideKey);
    else removeData_intern(parentNode, guideKey);
}

template<typename KeyType, typename DataType, int rank>
bool BPlusNode<KeyType, DataType, rank>::findData_leaf(const KeyType &guideKey) const {
    int left = 0, right = keyNum - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (guideKey == key[mid]) return true;
        else if (guideKey < key[mid]) right = mid - 1;
        else left = mid + 1;
    }
    return false;
}

template<typename KeyType, typename DataType, int rank>
bool BPlusNode<KeyType, DataType, rank>::findData_intern(const KeyType &guideKey) const {
    int next = findNextPath(guideKey);

    const NodeType childNode(buffer, filename, child[next]);
    return childNode.findData(guideKey);
}

template<typename KeyType, typename DataType, int rank>
int BPlusNode<KeyType, DataType, rank>::insertData_leaf(NodeType *parentNode, const KeyType &newKey, const DataType &newData) {
    if (findData(newKey)) throw BPlusTreeException::DuplicateKey;

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

template<typename KeyType, typename DataType, int rank>
int BPlusNode<KeyType, DataType, rank>::insertData_intern(NodeType *parentNode, const KeyType &newKey, const DataType &newData) {
    int next = findNextPath(newKey);

    NodeType childNode(buffer, filename, child[next]);
    childNode.insertData(this, newKey, newData);
    childNode.writeBackToBuffer();

    if (keyNum >= rank) return splitNode(parentNode);
    else return 0;
}

template<typename KeyType, typename DataType, int rank>
void BPlusNode<KeyType, DataType, rank>::removeData_leaf(NodeType *parentNode, const KeyType &guideKey) {
    if (!findData(guideKey)) throw BPlusTreeException::KeyNotExist;

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

        NodeType prevNode(buffer, filename, prev);
        NodeType nextNode(buffer, filename, next);

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
                NodeType newNextNode(buffer, filename, nextLeaf);
                newNextNode.prevLeaf = self;
                newNextNode.writeBackToBuffer();
            }
        }
    }
}

template<typename KeyType, typename DataType, int rank>
void BPlusNode<KeyType, DataType, rank>::removeData_intern(NodeType *parentNode, const KeyType &guideKey) {
    int next = findNextPath(guideKey);
    NodeType childNode(buffer, filename, child[next]);
    childNode.removeData(this, guideKey);
    childNode.writeBackToBuffer();

    if (keyNum < (rank - 1) / 2 && parentNode) {
        int ind = parentNode->findNextPath(key[0]);
        int prev = (ind > 0) ? (parentNode->child[ind - 1]) : 0;
        int next = (ind < parentNode->keyNum) ? (parentNode->child[ind + 1]) : 0;

        NodeType prevNode(buffer, filename, prev);
        NodeType nextNode(buffer, filename, next);

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

template<typename KeyType, typename DataType, int rank>
void BPlusNode<KeyType, DataType, rank>::addKey(const KeyType &newKey, int newChild, bool childAtRight) {
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

template<typename KeyType, typename DataType, int rank>
void BPlusNode<KeyType, DataType, rank>::changeKey(const KeyType &oldKey, const KeyType &newKey) {
    int i = findNextPath(oldKey) - 1;
    if (0 > i) throw BPlusTreeException::KeyNotExist;
    key[i] = newKey;
}

template<typename KeyType, typename DataType, int rank>
void BPlusNode<KeyType, DataType, rank>::deleteKey(const KeyType &guideKey, bool childAtRight) {
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

template<typename KeyType, typename DataType, int rank>
class BPlusTree: public BPlusTreeInterface {
public:
    using NodeType = BPlusNode<KeyType, DataType, rank>;
    BPlusTree(BufferManager *buffer = nullptr, const string &filename = "");
    ~BPlusTree() = default;

    bool findData(const KeyType &key) const;
    void insertData(const KeyType &key, const DataType &data);
    void removeData(const KeyType &key);
    void print() const {
        const NodeType rootNode(buffer, filename, root);
        rootNode.print();
    }

private:
    BufferManager *buffer;
    const string filename;
    int root;
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
BPlusTree<KeyType, DataType, rank>::BPlusTree(BufferManager *buffer, const string &filename) : buffer(buffer), filename(filename) {
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
        NodeType rootNode(buffer, filename, root, true);
        rootNode.writeBackToBuffer();
    }
}

template<typename KeyType, typename DataType, int rank>
bool BPlusTree<KeyType, DataType, rank>::findData(const KeyType &key) const {
    const NodeType rootNode(buffer, filename, root);
    return rootNode.findData(key);
}

template<typename KeyType, typename DataType, int rank>
void BPlusTree<KeyType, DataType, rank>::insertData(const KeyType &key, const DataType &data){
    NodeType rootNode(buffer, filename, root);
    int newRoot = rootNode.insertData(nullptr, key, data);
    rootNode.writeBackToBuffer();
    if (newRoot) {
        root = newRoot;
        buffer->setBlockContent(filename, META_PAGE_ID, 0, reinterpret_cast<char*>(&root), sizeof(root));
    }
}

template<typename KeyType, typename DataType, int rank>
void BPlusTree<KeyType, DataType, rank>::removeData(const KeyType &key) {
    NodeType rootNode(buffer, filename, root);
    rootNode.removeData(nullptr, key);
    rootNode.writeBackToBuffer();
    if (0 == rootNode.keyNum && false == rootNode.isLeaf) {
        root = rootNode.child[0];
        NodeType newRootNode(buffer, filename, root);
        buffer->setBlockContent(filename, META_PAGE_ID, 0, reinterpret_cast<char*>(&root), sizeof(root));
    }
}