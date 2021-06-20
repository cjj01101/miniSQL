#pragma once

#include "MiniSQLMeta.h"
#include "MiniSQLBufferManager.h"
#include <vector>
#include <set>
#include <map>
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
    size_t record_length;
    int occupied_record_count;
};
using table_file = map<string, Table>;

struct Index {
    Index(string name, set<string> keys) : name(name), keys(keys) {}
    string name;
    set<string> keys;
};
using index_file = map<string, vector<Index>>;

class CatalogManager {
public:
    CatalogManager(const char *meta_table_file_name, const char *meta_index_file_name);
    ~CatalogManager();

    void increaseRecordCount(const string &tablename);

    const Table &getTableInfo(const string &tablename) const;
    void addTableInfo(const string &tablename, const vector<Attr> &attrs);
    void deleteTableInfo(const string &tablename);

    bool findIndex(const string &tablename, const string &indexname) const;
    const vector<Index> &getIndexInfo(const string &tablename) const;
    void addIndexInfo(const string &tablename ,const string &indexname, const set<string> &keys);
    void deleteIndexInfo(const string &tablename, const string &indexname);
    void deleteIndexInfo(const string &tablename);

private:
    string meta_table_file_name;
    string meta_index_file_name;
    table_file table;
    index_file index;
};