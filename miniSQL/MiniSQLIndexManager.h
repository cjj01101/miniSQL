#pragma once

#include "BPlusTree.h"
#include "MiniSQLCatalogManager.h"
using namespace std;

class IndexManager {
public:
    IndexManager(BufferManager *buffer) : buffer(buffer) {}

    template<typename KeyType>
    void createIndex(const string &tablename, const string &indexname, int size) {
        string filename = "../" + tablename + "_" + indexname + ".index";
        BPlusTree<KeyType, int> newTree(buffer, filename, size);
    }

    void dropIndex(const string &tablename, const string &indexname) {
        string filename = "../" + tablename + "_" + indexname + ".index";
        buffer->setEmpty(filename);
        remove(filename.data());
    }
private:
    BufferManager *buffer;
};