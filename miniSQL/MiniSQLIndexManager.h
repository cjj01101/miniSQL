#pragma once

#include "BPlusTree.h"
#include "MiniSQLCatalogManager.h"
using std::string;

#define MAXCHARSIZE 255
#define INDEX_FILE_PATH(tablename, indexname) ("../" + (tablename) + "_" + (indexname) + ".index")

class IndexManager {
public:
    IndexManager(BufferManager *buffer) : buffer(buffer) {}

    void createIndex(const string &tablename, const string &indexname, const Type &type, int size) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<Position> newTree(buffer, filename, type, size);
    }

    void dropIndex(const string &tablename, const string &indexname) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        buffer->setEmpty(filename);
        remove(filename.data());
    }

    template<typename Type>
    void insertIntoIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key, const Position &pos) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<Position> tree(buffer, filename, type, size);
        try { tree.insertData(key, pos); }
        catch (BPlusTreeException &e) { throw MiniSQLException(e); }
    }

    template<typename Type>
    Position findOneFromIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<Position> tree(buffer, filename, type, size);
        auto iter = tree.getStart(key, true);
        if (iter.valid() && (*iter).first == key) {
            return (*iter).second;
        }
        else return Position({ -1, 0 });
    }

    template<typename Type>
    void findRangeFromIndex(const string &tablename, const string &indexname, const Type &type, int size, const std::pair<Compare, Value> &startKey, const std::pair<Compare, Value> &endKey, const std::set<Value> &neKeys, std::vector<Position> &pos) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<Position> tree(buffer, filename, type, size);
        auto start = (startKey.first == Compare::EQ) ? tree.begin() : (startKey.first == Compare::GE) ? tree.getStart(startKey.second, true) : tree.getStart(startKey.second, false);
        auto end = (endKey.first == Compare::EQ) ? tree.end() : (endKey.first == Compare::LE) ? tree.getStart(endKey.second, false) : tree.getStart(endKey.second, true);
        auto neKey_ptr = neKeys.begin();
        auto neEnd = neKeys.end();
        while (start != end) {
            if (neEnd != neKey_ptr && (*start).first == *neKey_ptr) {
                neKey_ptr++;
            } else pos.push_back((*start).second);
            start.next();
        }
    }

    template<typename Type>
    void removeFromIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<Position> tree(buffer, filename, type, size);
        try { tree.removeData(key); }
        catch (BPlusTreeException &e) { throw MiniSQLException(e); }
    }
private:
    BufferManager *buffer;
};