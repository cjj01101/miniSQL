#pragma once

#include "BPlusTree.h"
#include "MiniSQLCatalogManager.h"
using namespace std;

class IndexManager {
public:
    IndexManager(BufferManager *buffer, CatalogManager *metadata) : buffer(buffer), metadata(metadata) {}

    bool findIndex(const string &table, const string &name) const;

    template<typename KeyType>
    bool createIndex(const string &table, const string &name, initializer_list<string> keys);

    bool dropIndex(const string &name);
private:
    BufferManager *buffer;
    CatalogManager *metadata;
};

template<typename KeyType>
bool IndexManager::createIndex(const string &table, const string &name, initializer_list<string> keys) {
    index_file &index = metadata->getIndexFile();
    if (findIndex(table, name)) return false;
    string index_filename = "../" + name + ".index";
    BPlusTree<KeyType, int, 200> newTree(buffer, index_filename);
    index[table].push_back({ name,keys });
    return true;
}