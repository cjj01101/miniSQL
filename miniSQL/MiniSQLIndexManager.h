#pragma once

#include "MiniSQLCatalogManager.h"
#include <tuple>
using namespace std;

class IndexManager {
public:
    IndexManager(BufferManager *buffer, CatalogManager *metadata) : buffer(buffer), metadata(metadata) {}

    template<typename KeyType>
    BPlusTreeInterface* findIndex(const string &database, initializer_list<string> keys) const;

    template<typename KeyType>
    bool createIndex(const string &database, initializer_list<string> keys);

    template<typename KeyType>
    bool dropIndex(const string &database, initializer_list<string> keys);
private:
    BufferManager *buffer;
    CatalogManager *metadata;
};

template<typename KeyType>
BPlusTreeInterface* IndexManager::findIndex(const string &database, initializer_list<string> keys) const {
    index_file &index = metadata->getIndexFile();
    if(index.end() == index.find(database)) return nullptr;
    for (auto index_data = index[database].begin(); index_data != index[database].end(); index_data++) {
        auto index_tree = index_data->tree;
        auto index_keys = index_data->keys;
        if (dynamic_cast<BPlusTree<KeyType, int, 200>*>(index_tree) && std::equal(index_keys.begin(), index_keys.end(), keys.begin(), keys.end()))
            return index_tree;
    }
    return nullptr;
}

template<typename KeyType>
bool IndexManager::createIndex(const string &database, initializer_list<string> keys) {
    index_file &index = metadata->getIndexFile();
    if (findIndex<KeyType>(database, keys)) return false;
    BPlusTreeInterface *newIndex = new BPlusTree<KeyType, int, 200>(buffer, "../IMtest.txt");
    index[database].push_back({ keys, newIndex });
    return true;
}

template<typename KeyType>
bool IndexManager::dropIndex(const string &database, initializer_list<string> keys) {
    index_file &index = metadata->getIndexFile();
    if (index.end() == index.find(database)) return false;
    for (auto index_data = index[database].begin(); index_data != index[database].end(); index_data++) {
        auto index_tree = index_data->tree;
        auto index_keys = index_data->keys;
        if (dynamic_cast<BPlusTree<KeyType, int, 200>*>(index_tree) && std::equal(index_keys.begin(), index_keys.end(), keys.begin(), keys.end())) {
            index[database].erase(index_data);
            return true;
        }
    }
    return false;
}