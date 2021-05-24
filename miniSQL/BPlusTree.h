#pragma once

/*                                          */
/*                前向声明                  */
/*                                          */

template<typename KeyType, typename DataType, int rank>
class BPlusInternalNode;

template<typename KeyType, typename DataType, int rank>
class BPlusTree;

/*                                          */
/*                  异常                    */
/*                                          */

enum class BPlusTreeException { DuplicateKey, KeyNotExist };

/*                                          */
/*                                          */
/*              B+树结点基类                */
/*                  虚类                    */
/*                                          */

/*                                          */
/*                  定义                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
class BPlusNode {
public:
    BPlusNode() : keyNum(0), parent(nullptr) {}
    ~BPlusNode() = default;

    BPlusInternalNode<KeyType, DataType, rank> *getParent() { return parent; }
    void setParent(BPlusInternalNode<KeyType, DataType, rank> *parent) { this->parent = parent; }

    virtual void splitNode() = 0;
    //virtual void print() const = 0;
    virtual void clearDescendent() = 0;

    virtual bool findData(const KeyType &guideKey) const = 0;
    virtual void insertData(const KeyType &newKey, const DataType &newData) = 0;
    virtual void removeData(const KeyType &guideKey) = 0;

protected:
    int keyNum;
    KeyType key[rank + 1];
    BPlusInternalNode<KeyType, DataType, rank> *parent;

    friend class BPlusTree<KeyType, DataType, rank>;
};

/*                                          */
/*                                          */
/*              B+树叶子结点                */
/*                                          */
/*                                          */

/*                                          */
/*                  定义                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
class BPlusLeafNode : public BPlusNode<KeyType, DataType, rank> {
public:
    BPlusLeafNode() : BPlusNode<KeyType, DataType, rank>(), prevLeaf(nullptr), nextLeaf(nullptr) {}
    ~BPlusLeafNode() = default;

    void splitNode() override;
    void mergeRight();
    void clearDescendent() override {};
    //void print() const override {
    //    std::cout << this->keyNum << "-Leaf:";
    //    for (int i = 0; i < this->keyNum; i++) std::cout << "[" << this->key[i] << "," << data[i] << "]";
    //    std::cout << std::endl;
    //}

    bool findData(const KeyType &guideKey) const override;
    void insertData(const KeyType &newKey, const DataType &newData) override;
    void removeData(const KeyType &guideKey) override;

private:
    BPlusLeafNode<KeyType, DataType, rank> *nextLeaf;
    BPlusLeafNode<KeyType, DataType, rank> *prevLeaf;
    DataType data[rank + 1];
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
void BPlusLeafNode<KeyType, DataType, rank>::splitNode() {
    int leftKeyNum = this->keyNum / 2;
    BPlusLeafNode *newNode = new BPlusLeafNode();

    if (this->nextLeaf) this->nextLeaf->prevLeaf = newNode;
    newNode->nextLeaf = this->nextLeaf;
    newNode->prevLeaf = this;
    this->nextLeaf = newNode;

    for (int i = leftKeyNum; i < this->keyNum; i++) newNode->insertData(this->key[i], data[i]);
    this->keyNum = leftKeyNum;
    if (!this->parent) this->parent = new BPlusInternalNode<KeyType, DataType, rank>(this);
    newNode->parent = this->parent;
    this->parent->addKey(this->key[leftKeyNum], newNode);
}

template<typename KeyType, typename DataType, int rank>
void BPlusLeafNode<KeyType, DataType, rank>::mergeRight() {
    if (!nextLeaf) return;

    auto next = nextLeaf;
    for (int i = 0; i < next->keyNum; i++) this->insertData(next->key[i], next->data[i]);
    this->parent->deleteKey(next->key[0]);
    nextLeaf = next->nextLeaf;
    if (nextLeaf) nextLeaf->prevLeaf = this;
    delete next;
}

template<typename KeyType, typename DataType, int rank>
bool BPlusLeafNode<KeyType, DataType, rank>::findData(const KeyType &guideKey) const {
    int left = 0, right = this->keyNum - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (guideKey == this->key[mid]) return true;
        else if (guideKey < this->key[mid]) right = mid - 1;
        else left = mid + 1;
    }
    return false;
}

template<typename KeyType, typename DataType, int rank>
void BPlusLeafNode<KeyType, DataType, rank>::insertData(const KeyType &newKey, const DataType &newData) {
    if (findData(newKey)) throw BPlusTreeException::DuplicateKey;

    int i;
    for (i = this->keyNum; i > 0 && this->key[i - 1] > newKey; i--) {
        this->key[i] = this->key[i - 1];
        data[i] = data[i - 1];
    }
    this->key[i] = newKey;
    data[i] = newData;

    if (++this->keyNum > rank) splitNode();
}

template<typename KeyType, typename DataType, int rank>
void BPlusLeafNode<KeyType, DataType, rank>::removeData(const KeyType &guideKey) {
    if (!findData(guideKey)) throw BPlusTreeException::KeyNotExist;

    int i = 0;
    while (this->key[i] != guideKey) i++;
    for (; i < this->keyNum - 1; i++) {
        this->key[i] = this->key[i + 1];
        data[i] = data[i + 1];
    }

    if (--this->keyNum < (rank + 1) / 2) {
        if (prevLeaf && prevLeaf->parent == this->parent && prevLeaf->keyNum > (rank + 1) / 2) {
            KeyType xKey = prevLeaf->key[prevLeaf->keyNum - 1];
            DataType xData = prevLeaf->data[prevLeaf->keyNum - 1];
            prevLeaf->removeData(xKey);
            this->insertData(xKey, xData);
            this->parent->changeKey(guideKey, xKey);
        }
        else if (nextLeaf && nextLeaf->parent == this->parent && nextLeaf->keyNum > (rank + 1) / 2) {
            KeyType xKey = nextLeaf->key[0];
            KeyType parentNewKey = nextLeaf->key[1];
            DataType xData = nextLeaf->data[0];
            nextLeaf->removeData(xKey);
            this->insertData(xKey, xData);
            this->parent->changeKey(xKey, parentNewKey);
        }
        else if (prevLeaf && prevLeaf->parent == this->parent) prevLeaf->mergeRight();
        else if (nextLeaf && nextLeaf->parent == this->parent) mergeRight();
    }
}

/*                                          */
/*                                          */
/*              B+树内部结点                */
/*                                          */
/*                                          */

/*                                          */
/*                  定义                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
class BPlusInternalNode : public BPlusNode<KeyType, DataType, rank> {
public:
    BPlusInternalNode(BPlusNode<KeyType, DataType, rank> *firstChild) : BPlusNode<KeyType, DataType, rank>() { child[0] = firstChild; firstChild->setParent(this); }
    ~BPlusInternalNode() = default;

    int findNextPath(const KeyType &guideKey) const;
    void splitNode() override;
    void clearDescendent() override;
    //void print() const override {
    //    std::cout << this->keyNum << "-Internal:";
    //    for (int i = 0; i < this->keyNum; i++) std::cout << "[" << this->key[i] << "]";
    //    std::cout << std::endl;
    //    for (int i = 0; i <= this->keyNum; i++) child[i]->print();
    //}

    void addKey(const KeyType &newKey, BPlusNode<KeyType, DataType, rank> *newChild, bool childAtRight = true);
    void changeKey(const KeyType &oldKey, const KeyType &newKey);
    void deleteKey(const KeyType &guideKey, bool childAtRight = true);

    bool findData(const KeyType &guideKey) const override;
    void insertData(const KeyType &newKey, const DataType &newData) override;
    void removeData(const KeyType &guideKey) override;

    friend class BPlusTree<KeyType, DataType, rank>;

private:
    BPlusNode<KeyType, DataType, rank> *child[rank + 1];
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
int BPlusInternalNode<KeyType, DataType, rank>::findNextPath(const KeyType &guideKey) const {
    int left = 0, right = this->keyNum - 1;
    while (left != right) {
        int mid = (left + right) / 2;
        if (guideKey < this->key[mid]) right = mid;
        else left = mid + 1;
    }
    if (guideKey >= this->key[right]) right++;
    return right;
}

template<typename KeyType, typename DataType, int rank>
void BPlusInternalNode<KeyType, DataType, rank>::splitNode() {
    int leftKeyNum = this->keyNum / 2;
    BPlusInternalNode *newNode = new BPlusInternalNode(child[leftKeyNum + 1]);

    for (int i = leftKeyNum + 1; i < this->keyNum; i++) newNode->addKey(this->key[i], child[i + 1]);
    this->keyNum = leftKeyNum;
    if (!this->parent) this->parent = new BPlusInternalNode<KeyType, DataType, rank>(this);
    newNode->parent = this->parent;
    this->parent->addKey(this->key[leftKeyNum], newNode);
}

template<typename KeyType, typename DataType, int rank>
void BPlusInternalNode<KeyType, DataType, rank>::clearDescendent() {
    for (int i = 0; i <= this->keyNum; i++) {
        child[i]->clearDescendent();
        delete child[i];
    }
}

template<typename KeyType, typename DataType, int rank>
void BPlusInternalNode<KeyType, DataType, rank>::addKey(const KeyType &newKey, BPlusNode<KeyType, DataType, rank> *newChild, bool childAtRight) {
    int i;
    if (!childAtRight) child[this->keyNum + 1] = child[this->keyNum];
    for (i = this->keyNum; i > 0 && this->key[i - 1] > newKey; i--) {
        this->key[i] = this->key[i - 1];
        child[i + childAtRight] = child[i - 1 + childAtRight];
    }
    this->key[i] = newKey;
    child[i + childAtRight] = newChild;
    newChild->setParent(this);
    this->keyNum++;
}

template<typename KeyType, typename DataType, int rank>
void BPlusInternalNode<KeyType, DataType, rank>::changeKey(const KeyType &oldKey, const KeyType &newKey) {
    int i = findNextPath(oldKey) - 1;
    if (0 > i) throw BPlusTreeException::KeyNotExist;
    this->key[i] = newKey;
}

template<typename KeyType, typename DataType, int rank>
void BPlusInternalNode<KeyType, DataType, rank>::deleteKey(const KeyType &guideKey, bool childAtRight) {
    int i = findNextPath(guideKey) - 1;
    if (0 > i) throw BPlusTreeException::KeyNotExist;
    for (; i < this->keyNum - 1; i++) {
        this->key[i] = this->key[i + 1];
        child[i + childAtRight] = child[i + 1 + childAtRight];
    }
    if (!childAtRight) child[i] = child[i + 1];
    this->keyNum--;
}

template<typename KeyType, typename DataType, int rank>
bool BPlusInternalNode<KeyType, DataType, rank>::findData(const KeyType &guideKey) const {
    int next = findNextPath(guideKey);
    return child[next]->findData(guideKey);
}

template<typename KeyType, typename DataType, int rank>
void BPlusInternalNode<KeyType, DataType, rank>::insertData(const KeyType &newKey, const DataType &newData) {
    int next = findNextPath(newKey);
    child[next]->insertData(newKey, newData);

    if (this->keyNum >= rank) splitNode();
}

template<typename KeyType, typename DataType, int rank>
void BPlusInternalNode<KeyType, DataType, rank>::removeData(const KeyType &guideKey) {
    int next = findNextPath(guideKey);
    child[next]->removeData(guideKey);

    if (this->keyNum < (rank - 1) / 2 && this->parent) {
        auto parent = this->parent;
        int ind = parent->findNextPath(this->key[0]);
        auto prev = (ind > 0) ? (static_cast<decltype(this)>(parent->child[ind - 1])) : nullptr;
        auto next = (ind < parent->keyNum) ? (static_cast<decltype(this)>(parent->child[ind + 1])) : nullptr;
        if (prev && prev->keyNum > (rank - 1) / 2) {
            KeyType pKey = parent->key[ind - 1];
            KeyType sKey = prev->key[prev->keyNum - 1];
            auto sChild = prev->child[prev->keyNum];
            addKey(pKey, sChild, false);
            parent->changeKey(pKey, sKey);
            prev->deleteKey(sKey);
        }
        else if (next && next->keyNum > (rank - 1) / 2) {
            KeyType pKey = parent->key[ind];
            KeyType sKey = next->key[0];
            auto sChild = next->child[0];
            addKey(pKey, sChild);
            parent->changeKey(pKey, sKey);
            next->deleteKey(sKey, false);
        }
        else if (prev) {
            KeyType pKey = parent->key[ind - 1];
            prev->addKey(pKey, this->child[0]);
            for (int i = 0; i < this->keyNum; i++) prev->addKey(this->key[i], child[i + 1]);
            this->parent->deleteKey(pKey);
            delete this;
        }
        else if (next) {
            KeyType pKey = parent->key[ind];
            this->addKey(pKey, next->child[0]);
            for (int i = 0; i < next->keyNum; i++) addKey(next->key[i], next->child[i + 1]);
            this->parent->deleteKey(pKey);
            delete next;
        }
    }
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
}; // 供IndexManager用

template<typename KeyType, typename DataType, int rank>
class BPlusTree: public BPlusTreeInterface {
public:
    BPlusTree() : root(new BPlusLeafNode<KeyType, DataType, rank>()) {}
    ~BPlusTree();

    bool findData(const KeyType &key) const;
    void insertData(const KeyType &key, const DataType &data);
    void removeData(const KeyType &key);
    //void print() const { root->print(); }
private:

private:
    BPlusNode<KeyType, DataType, rank> *root;
};

/*                                          */
/*                  实现                    */
/*                                          */

template<typename KeyType, typename DataType, int rank>
BPlusTree<KeyType, DataType, rank>::~BPlusTree() {
    root->clearDescendent();
    delete root;
}

template<typename KeyType, typename DataType, int rank>
bool BPlusTree<KeyType, DataType, rank>::findData(const KeyType &key) const {
    return root->findData(key);
}

template<typename KeyType, typename DataType, int rank>
void BPlusTree<KeyType, DataType, rank>::insertData(const KeyType &key, const DataType &data){
    root->insertData(key, data);
    if (root->parent) root = root->parent;
}

template<typename KeyType, typename DataType, int rank>
void BPlusTree<KeyType, DataType, rank>::removeData(const KeyType &key) {
    root->removeData(key);
    if (0 == root->keyNum && dynamic_cast<BPlusInternalNode<KeyType, DataType, rank>*>(root)) {
        auto temp = root;
        root = dynamic_cast<BPlusInternalNode<KeyType, DataType, rank>*>(root)->child[0];
        root->parent = nullptr;
        delete temp;
    }
}