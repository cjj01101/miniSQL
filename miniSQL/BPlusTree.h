#pragma once
#include "BPlusTree.cpp"

template<typename KeyType, int rank>
class BPlusNode {
public:
    BPlusNode() { keyNum = 0; }
    ~BPlusNode() = default;
protected:
    int keyNum;
    KeyType key[rank];
};

template<typename KeyType, typename DataType, int rank>
class BPlusLeafNode : public BPlusNode<KeyType, rank> {
public:
    BPlusLeafNode() { prevLeaf = nextLeaf = nullptr; }
    ~BPlusLeafNode() = default;
private:
    BPlusLeafNode<KeyType, DataType, rank> *nextLeaf;
    BPlusLeafNode<KeyType, DataType, rank> *prevLeaf;
    DataType data[rank];
};

template<typename KeyType, int rank>
class BPlusInternalNode : public BPlusNode<KeyType, rank> {
public:
    BPlusInternalNode() = default;
    ~BPlusInternalNode() = default;
private:
    BPlusNode<KeyType, rank> *child[rank];
};

template<typename KeyType, typename DataType, int rank>
class BPlusTree {
public:
    BPlusTree();
    ~BPlusTree();
    bool insertData(KeyType key, DataType &data);
    //void removeData(KeyType key);
    //void updateData(KeyType key, DataType &data);
private:
    BPlusNode<KeyType, rank> *root;
};

template<typename KeyType, typename DataType, int rank>
BPlusTree<KeyType, DataType, rank>::BPlusTree() {
    root = new BPlusLeafNode();
}

template<typename KeyType, typename DataType, int rank>
BPlusTree<KeyType, DataType, rank>::~BPlusTree() {
    
}

template<typename KeyType, typename DataType, int rank>
bool BPlusTree<KeyType, DataType, rank>::insertData(KeyType key, DataType &data) {

}