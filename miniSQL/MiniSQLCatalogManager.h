#pragma once

#include "BPlusTree.h"
#include "MiniSQLMeta.h"
#include "MiniSQLException.h"
#include <initializer_list>
#include <vector>
#include <set>
#include <map>
using namespace std;

struct attr_info {
    string name;
    Type type;
};
struct table_info {
    vector<attr_info> attrs;
    int total_length;
    int relation_count;
};
using table_file = map<string, table_info>;

struct index_info {
    index_info(initializer_list<string> keys, BPlusTreeInterface *tree) : keys(keys), tree(tree) {}
    set<string> keys;
    BPlusTreeInterface *tree;
};
using index_file = map<string, vector<index_info>>;

class CatalogManager {
public:
    index_file &getIndexFile() { return index; }
    table_file &getTableFile() { return table; }

private:
    table_file table;
    index_file index;
};