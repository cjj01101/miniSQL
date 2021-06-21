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
        BPlusTree<KeyType, int> newTree(buffer, filename, size);
    }

    void dropIndex(const string &tablename, const string &indexname) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        buffer->setEmpty(filename);
        remove(filename.data());
    }

    template<typename KeyType>
    void insertIntoIndex(const string &tablename, const string &indexname, int size, const KeyType &key, const int pos) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<KeyType, int> tree(buffer, filename, size);
        try { tree.insertData(key, pos); }
        catch (BPlusTreeException &e) { throw MiniSQLException(e); }
    }

    template<typename KeyType>
    void removeFromIndex(const string &tablename, const string &indexname, int size, const KeyType &key) {
        string filename = INDEX_FILE_PATH(tablename, indexname);
        BPlusTree<KeyType, int> tree(buffer, filename, size);
        try { tree.removeData(key); }
        catch (BPlusTreeException &e) { throw MiniSQLException(e); }
    }
private:
    BufferManager *buffer;
};