#pragma once

#include "MiniSQLMeta.h"
#include "MiniSQLException.h"
#include <initializer_list>
#include <vector>
#include <set>
#include <map>
using namespace std;

struct Attr {
    string name;
    Type type;
};
struct Table {
    vector<Attr> attrs;
    int record_per_block;
    int record_count;
};
using table_file = map<string, Table>;

struct Index {
    Index(string name, initializer_list<string> keys) : name(name), keys(keys) {}
    string name;
    set<string> keys;
};
using index_file = map<string, vector<Index>>;

class CatalogManager {
public:
    index_file &getIndexFile() { return index; }
    table_file &getTableFile() { return table; }

private:
    table_file table;
    index_file index;
};