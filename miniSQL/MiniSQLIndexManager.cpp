#include <iostream>
#include "MiniSQLIndexManager.h"

void IndexManager::createIndex(const string &tablename, const string &indexname, const Type &type, int size) {
    string filename = INDEX_FILE_PATH(tablename, indexname);
    BPlusTree<Position> newTree(buffer, filename, type, size);
}

void IndexManager::dropIndex(const string &tablename, const string &indexname) {
    string filename = INDEX_FILE_PATH(tablename, indexname);
    buffer->setEmpty(filename);
    remove(filename.data());
}

void IndexManager::insertIntoIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key, const Position &pos) {
    string filename = INDEX_FILE_PATH(tablename, indexname);
    BPlusTree<Position> tree(buffer, filename, type, size);
    try { tree.insertData(key, pos); }
    catch (BPlusTreeException &e) { throw MiniSQLException(e); }
}

Position IndexManager::findOneFromIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key) {
    string filename = INDEX_FILE_PATH(tablename, indexname);
    BPlusTree<Position> tree(buffer, filename, type, size);
    auto iter = tree.getStart(key, true);
    if (iter.valid() && (*iter).first == key) {
        return (*iter).second;
    }
    else return Position({ -1, 0 });
}

void IndexManager::findRangeFromIndex(const string &tablename, const string &indexname, const Type &type, int size, const std::pair<Compare, Value> &startKey, const std::pair<Compare, Value> &endKey, const std::set<Value> &neKeys, std::vector<Position> &pos) {
    string filename = INDEX_FILE_PATH(tablename, indexname);
    BPlusTree<Position> tree(buffer, filename, type, size);
    auto start = (startKey.first == Compare::EQ) ? tree.begin() : (startKey.first == Compare::GE) ? tree.getStart(startKey.second, true) : tree.getStart(startKey.second, false);
    auto end = (endKey.first == Compare::EQ) ? tree.end() : (endKey.first == Compare::LE) ? tree.getStart(endKey.second, false) : tree.getStart(endKey.second, true);
    auto neKey_ptr = neKeys.begin();
    auto neEnd = neKeys.end();
    while (start != end) {
        if (neEnd != neKey_ptr && (*start).first == *neKey_ptr) {
            neKey_ptr++;
        }
        else pos.push_back((*start).second);
        start.next();
    }
}

void IndexManager::removeFromIndex(const string &tablename, const string &indexname, const Type &type, int size, const Value &key) {
    string filename = INDEX_FILE_PATH(tablename, indexname);
    BPlusTree<Position> tree(buffer, filename, type, size);
    try { tree.removeData(key); }
    catch (BPlusTreeException &e) { throw MiniSQLException(e); }
}

void IndexManager_test() {
    BufferManager BM;
    CatalogManager CM(META_TABLE_FILE_PATH, META_INDEX_FILE_PATH);
    IndexManager IM(&BM);
    /*IM.createIndex("table1", "index1", 200, { "a" });
    cout << IM.createIndex("table1", "index1", 200, { "a" }) << endl;
    cout << IM.createIndex("another table", "index1", 200, { "a" }) << endl;

    cout << IM.findIndex("table1", "index1", 200, { "a" }) << endl;
    cout << IM.findIndex("table1", "index1", 200, { "b" }) << endl;
    cout << IM.findIndex("abc", { "a" }) << endl;
    cout << IM.findIndex("another table", { "key" }) << endl;

    cout << IM.createIndex("abc", { "b" }) << endl;
    cout << IM.findIndex("abc", { "b" }) << endl;

    cout << IM.dropIndex("abc", { "a" }) << endl;
    cout << IM.dropIndex("abc", { "a" }) << endl;
    cout << IM.dropIndex("abc", { "c" }) << endl;

    cout << IM.findIndex("abc", { "a" }) << endl;
    cout << IM.findIndex("abc", { "b" }) << endl;

    cout << IM.dropIndex("abc", { "b" }) << endl;
    cout << IM.findIndex"abc", { "b" }) << endl;*/
}