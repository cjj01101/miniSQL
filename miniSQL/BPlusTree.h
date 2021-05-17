#pragma once
#include "BPlusTree.cpp"

/*                                          */
/*                                          */
/*                B+Ê÷½áµã                  */
/*                                          */
/*                                          */

template<typename KeyType, typename DataType, int rank>
class BPlusInternalNode;

template<typename KeyType, typename DataType, int rank>
class BPlusNode {
public:
    BPlusNode() : keyNum(0) {}
    ~BPlusNode() = default;
    virtual BPlusNode<KeyType, DataType, rank> *insertData(BPlusInternalNode<KeyType, DataType, rank> *parent, KeyType newKey, DataType &&newData) = 0;
    virtual BPlusNode<KeyType, DataType, rank> *splitNode(BPlusInternalNode<KeyType, DataType, rank> *parent) = 0;
    virtual void print() = 0;

protected:
    int keyNum;
    KeyType key[rank + 1];
};

template<typename KeyType, typename DataType, int rank>
class BPlusLeafNode : public BPlusNode<KeyType, DataType, rank> {
public:
    BPlusLeafNode() : BPlusNode<KeyType, DataType, rank>(), prevLeaf(nullptr), nextLeaf(nullptr) {}
    ~BPlusLeafNode() = default;
    BPlusNode<KeyType, DataType, rank> *insertData(BPlusInternalNode<KeyType, DataType, rank> *parent, KeyType newKey, DataType &&newData) override;
    BPlusNode<KeyType, DataType, rank> *splitNode(BPlusInternalNode<KeyType, DataType, rank> *parent) override;
    void print() override { 
        std::cout << this->keyNum << ":";
        for (int i = 0; i < this->keyNum; i++) std::cout << "[" << this->key[i] << "," << data[i] << "]";
        std::cout << std::endl;
    }

private:
    BPlusLeafNode<KeyType, DataType, rank> *nextLeaf;
    BPlusLeafNode<KeyType, DataType, rank> *prevLeaf;
    DataType data[rank + 1];
};

template<typename KeyType, typename DataType, int rank>
class BPlusInternalNode : public BPlusNode<KeyType, DataType, rank> {
public:
    BPlusInternalNode(BPlusNode<KeyType, DataType, rank> *firstChild = nullptr) : BPlusNode<KeyType, DataType, rank>() { child[0] = firstChild; }
    ~BPlusInternalNode() = default;
    void addKey(KeyType newKey, BPlusNode<KeyType, DataType, rank> *newChild);
    BPlusNode<KeyType, DataType, rank> *insertData(BPlusInternalNode<KeyType, DataType, rank> *parent, KeyType newKey, DataType &&newData) override;
    BPlusNode<KeyType, DataType, rank> *splitNode(BPlusInternalNode<KeyType, DataType, rank> *parent) override;
    void print() override {
        std::cout << this->keyNum << ":";
        for (int i = 0; i < this->keyNum; i++) std::cout << "[" << this->key[i] << "]";
        std::cout << std::endl;
        for (int i = 0; i <= this->keyNum; i++) child[i]->print();
    }

private:
    BPlusNode<KeyType, DataType, rank> *child[rank + 1];
};

template<typename KeyType, typename DataType, int rank>
BPlusNode<KeyType, DataType, rank> *BPlusLeafNode<KeyType, DataType, rank>::insertData(BPlusInternalNode<KeyType, DataType, rank> *parent, KeyType newKey, DataType &&newData) {
    int i;
    for (i = this->keyNum; i > 0 && this->key[i - 1] > newKey; i--) {
        this->key[i] = this->key[i - 1];
        data[i] = data[i - 1];
    }
    this->key[i] = newKey;
    data[i] = newData;

    if (++this->keyNum > rank) return splitNode(parent);
    else return this;
}

template<typename KeyType, typename DataType, int rank>
BPlusNode<KeyType, DataType, rank> *BPlusLeafNode<KeyType, DataType, rank>::splitNode(BPlusInternalNode<KeyType, DataType, rank> *parent) {
    int leftKeyNum = this->keyNum / 2;
    BPlusLeafNode *newNode = new BPlusLeafNode();

    if (this->nextLeaf) this->nextLeaf->prevLeaf = newNode;
    newNode->nextLeaf = this->nextLeaf;
    newNode->prevLeaf = this;
    this->nextLeaf = newNode;

    for (int i = leftKeyNum; i < this->keyNum; i++) newNode->insertData(parent, this->key[i], std::forward<DataType>(data[i]));
    this->keyNum = leftKeyNum;
    if (!parent) parent = new BPlusInternalNode<KeyType, DataType, rank>(this);
    parent->addKey(this->key[leftKeyNum], newNode);
    return parent;
}

template<typename KeyType, typename DataType, int rank>
void BPlusInternalNode<KeyType, DataType, rank>::addKey(KeyType newKey, BPlusNode<KeyType, DataType, rank> *newChild) {
    int i;
    for (i = this->keyNum; i > 0 && this->key[i - 1] > newKey; i--) {
        this->key[i] = this->key[i - 1];
        child[i + 1] = child[i];
    }
    this->key[i] = newKey;
    child[i + 1] = newChild;
    this->keyNum++;
}

template<typename KeyType, typename DataType, int rank>
BPlusNode<KeyType, DataType, rank> *BPlusInternalNode<KeyType, DataType, rank>::insertData(BPlusInternalNode<KeyType, DataType, rank> *parent, KeyType newKey, DataType &&newData) {
    int left = 0, right = this->keyNum - 1;
    while (left != right) {
        int mid = (left + right) / 2;
        if (newKey < this->key[mid]) right = mid;
        else left = mid + 1;
    }
    if (newKey >= this->key[left]) left++;
    child[left]->insertData(this, newKey, std::forward<DataType>(newData));

    if (this->keyNum >= rank) return splitNode(parent);
    else return this;
}

template<typename KeyType, typename DataType, int rank>
BPlusNode<KeyType, DataType, rank> *BPlusInternalNode<KeyType, DataType, rank>::splitNode(BPlusInternalNode<KeyType, DataType, rank> *parent) {
    int leftKeyNum = this->keyNum / 2;
    BPlusInternalNode *newNode = new BPlusInternalNode(child[leftKeyNum + 1]);

    for (int i = leftKeyNum + 1; i < this->keyNum; i++) newNode->addKey(this->key[i], child[i + 1]);
    this->keyNum = leftKeyNum;
    if (!parent) parent = new BPlusInternalNode<KeyType, DataType, rank>(this);
    parent->addKey(this->key[leftKeyNum], newNode);
    return parent;
}

/*                                          */
/*                                          */
/*                  B+Ê÷                    */
/*                                          */
/*                                          */

template<typename KeyType, typename DataType, int rank>
class BPlusTree {
public:
    BPlusTree() : root(new BPlusLeafNode<KeyType, DataType, rank>()) {}
    ~BPlusTree();
    void insertData(KeyType newKey, DataType &&newData) { root = root->insertData(nullptr, newKey, std::forward<DataType>(newData)); }
    //void removeData(KeyType key);
    //void updateData(KeyType key, DataType &data);
    void print() const { root->print(); }
private:

private:
    BPlusNode<KeyType, DataType, rank> *root;
};

template<typename KeyType, typename DataType, int rank>
BPlusTree<KeyType, DataType, rank>::~BPlusTree() {
    
}