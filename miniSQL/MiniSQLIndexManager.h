#pragma once

#include "BPlusTree.h"
#include <initializer_list>
#include <vector>
#include <set>
#include <map>
#include <tuple>
using namespace std;

struct index_info {
    struct index_info(initializer_list<string> keys, BPlusTreeInterface *file): index_key(keys), index_tree(file) {}
    set<string> index_key;
    BPlusTreeInterface *index_tree;
};

class IndexManager {
public:
    template<typename... KeyTypes>
    BPlusTreeInterface* findIndex(string database, initializer_list<string> keys);

    template<typename... KeyTypes>
    bool createIndex(string database, initializer_list<string> keys);

    template<typename... KeyTypes>
    bool dropIndex(string database, initializer_list<string> keys);
private:
    map<string, vector<struct index_info>> index;
};

template<typename... KeyTypes>
BPlusTreeInterface* IndexManager::findIndex(string database, initializer_list<string> keys) {
    if (index.end() == index.find(database)) return nullptr;
    for (auto index_file = index[database].begin(); index_file != index[database].end(); index_file++) {
        auto tree = (*index_file).index_tree;
        auto key = (*index_file).index_key;
        if (dynamic_cast<BPlusTree<tuple<KeyTypes...>, int, 200>*>(tree) && std::equal(key.begin(), key.end(), keys.begin(), keys.end()))
            return tree;
    }
    return nullptr;
}

template<typename... KeyTypes>
bool IndexManager::createIndex(string database, initializer_list<string> keys) {
    if (findIndex<KeyTypes...>(database, keys)) return false;
    BPlusTreeInterface *newIndex = new BPlusTree<tuple<KeyTypes...>, int, 200>();
    index[database].push_back({ keys, newIndex });
    return true;
}

template<typename... KeyTypes>
bool IndexManager::dropIndex(string database, initializer_list<string> keys) {
    if (index.end() == index.find(database)) return false;
    for (auto index_file = index[database].begin(); index_file != index[database].end(); index_file++) {
        auto tree = (*index_file).index_tree;
        auto key = (*index_file).index_key;
        if (dynamic_cast<BPlusTree<tuple<KeyTypes...>, int, 200>*>(tree) && std::equal(key.begin(), key.end(), keys.begin(), keys.end())) {
            index[database].erase(index_file);
            return true;
        }
    }
    return false;
}