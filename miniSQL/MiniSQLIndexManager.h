#pragma once

#include "MiniSQLCatalogManager.h"
using namespace std;

class IndexManager {
public:
    IndexManager(BufferManager *buffer, CatalogManager *metadata) : buffer(buffer), metadata(metadata) {}

    bool findIndex(const string &database, const string &name) const;

    template<typename KeyType>
    bool createIndex(const string &database, const string &name, initializer_list<string> keys);

    bool dropIndex(const string &database, const string &name);
private:
    BufferManager *buffer;
    CatalogManager *metadata;
};

template<typename KeyType>
bool IndexManager::createIndex(const string &database, const string &name, initializer_list<string> keys) {
    index_file &index = metadata->getIndexFile();
    if (findIndex(database, name)) return false;
    string index_filename = "../" + database;
    for (auto key : keys) index_filename += "_" + key;
    index_filename += ".index";
    BPlusTree<KeyType, int, 200> newTree(buffer, index_filename);
    index[database].push_back({ name,keys });
    return true;
}