#pragma once

#include "BPlusTree.h"
#include "MiniSQLCatalogManager.h"
using std::string;

#define MAXCHARSIZE 255
#define INDEX_FILE_PATH(tablename, indexname) ("../" + (tablename) + "_" + (indexname) + ".index")

struct FLString
{
    char content[MAXCHARSIZE];
    FLString() = default;
    FLString(const FLString &rhs) { memcpy_s(content, MAXCHARSIZE, rhs.content, sizeof(rhs.content)); }
    FLString(const Value& value) { memcpy_s(content, MAXCHARSIZE, value.translate<char*>(), value.type.size); }
    FLString(const string &content) { memcpy_s(this->content, MAXCHARSIZE, content.data(), MAXCHARSIZE); }
    FLString(const char *str) { strncpy_s(content, str, MAXCHARSIZE); }

    FLString& operator =(const FLString& rhs) {
        memcpy_s(content, MAXCHARSIZE, rhs.content, sizeof(rhs.content));
        return *this;
    }

    bool operator ==(const FLString& rhs) const { return strcmp(content, rhs.content) == 0; }
    bool operator !=(const FLString& rhs) const { return !(*this == rhs); }
    bool operator <(const FLString& rhs) const { return strcmp(content, rhs.content) < 0; }
    bool operator >(const FLString& rhs) const { return (rhs < *this); }
    bool operator <=(const FLString& rhs) const { return !(*this > rhs); }
    bool operator >=(const FLString& rhs) const { return !(*this < rhs); }

    friend std::ostream & operator<<(std::ostream & os, const FLString &str) {
        os << str.content;
        return os;
    }
};

class IndexManager {
public:
    IndexManager(BufferManager *buffer) : buffer(buffer) {}

    template<typename KeyType>
    void createIndex(const string &tablename, const string &indexname, int size) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<KeyType, Position> newTree(buffer, filename, size);
    }

    void dropIndex(const string &tablename, const string &indexname) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        buffer->setEmpty(filename);
        remove(filename.data());
    }

    template<typename KeyType>
    void insertIntoIndex(const string &tablename, const string &indexname, int size, const KeyType &key, const Position &pos) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<KeyType, Position> tree(buffer, filename, size);
        try { tree.insertData(key, pos); }
        catch (BPlusTreeException &e) { throw MiniSQLException(e); }
    }

    template<typename KeyType>
    Position findOneFromIndex(const string &tablename, const string &indexname, int size, const KeyType &key) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<KeyType, Position> tree(buffer, filename, size);
        auto iter = tree.getStart(key, true);
        if ((*iter).first == key) {
            return (*iter).second;
        }
        else return Position({ -1, 0 });
    }

    template<typename KeyType>
    void findRangeFromIndex(const string &tablename, const string &indexname, int size, const std::pair<Compare, KeyType> &startKey, const std::pair<Compare, KeyType> &endKey, const std::set<KeyType> &neKeys, std::vector<Position> &pos) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<KeyType, Position> tree(buffer, filename, size);
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

    template<typename KeyType>
    void removeFromIndex(const string &tablename, const string &indexname, int size, const KeyType &key) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<KeyType, Position> tree(buffer, filename, size);
        try { tree.removeData(key); }
        catch (BPlusTreeException &e) { throw MiniSQLException(e); }
    }
private:
    BufferManager *buffer;
};