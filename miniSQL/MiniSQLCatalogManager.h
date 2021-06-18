#pragma once

#include "MiniSQLMeta.h"
#include "MiniSQLBufferManager.h"
#include <initializer_list>
#include <vector>
#include <set>
#include <map>
using std::initializer_list;
using std::vector;
using std::string;
using std::map;
using std::set;

struct Attr {
    string name;
    Type type;
    bool unique;
};
struct Table {
    vector<Attr> attrs;
    size_t record_per_block;
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

    const Table &getTableInfo(const string &tablename);
    void addTableInfo(const string &tablename, const vector<Attr> &attrs);
    void deleteTableInfo(const string &tablename);

    bool findIndex(const string &tablename, const string &indexname) const;
    void addIndexInfo(const string &tablename ,const string &indexname, initializer_list<string> keys);
    void deleteIndexInfo(const string &tablename, const string &indexname);
    void deleteIndexInfo(const string &tablename);

private:
    table_file table;
    index_file index;
};