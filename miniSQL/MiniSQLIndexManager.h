#pragma once

#include "BPlusTree.h"
#include "MiniSQLCatalogManager.h"
using std::string;

#define MAXCHARSIZE 255
#define INDEX_FILE_PATH(tablename, indexname) ("../" + (tablename) + "_" + (indexname) + ".index")

class IndexManager {
public:
    IndexManager(BufferManager *buffer) : buffer(buffer) {}

    void createIndex(const string &tablename, const string &indexname, const Type &type, int size);
    void dropIndex(const string &tablename, const string &indexname);
    void insertIntoIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key, const Position &pos);
    Position findOneFromIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key);
    void findRangeFromIndex(const string &tablename, const string &indexname, const Type &type, int size,
        const std::pair<Compare, Value> &startKey, const std::pair<Compare, Value> &endKey, const std::set<Value> &neKeys, std::vector<Position> &pos);
    void removeFromIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key);

private:
    BufferManager *buffer;
};