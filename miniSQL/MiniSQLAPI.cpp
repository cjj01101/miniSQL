#include "MiniSQLAPI.h"
#include <iostream>

void API::createTable(const string &tablename, const std::vector<Attr> &attrs, const set<string> &primary_key) {
    CM->addTableInfo(tablename, attrs);
    if(primary_key.size() > 0) createIndex(tablename, "PRIMARY_KEY", primary_key);
}

void API::dropTable(const string &tablename) {
    auto indexes = CM->getIndexInfo(tablename);
    for (auto index : indexes) IM->dropIndex(tablename, index.name);

    CM->deleteTableInfo(tablename);
    CM->deleteIndexInfo(tablename);
}

void API::createIndex(const string &tablename, const string &indexname, const set<string> &keys) {
    Table table = CM->getTableInfo(tablename);
    Type primary_key_type;
    size_t basic_length = sizeof(bool) + sizeof(int) * 3;
    for (const auto &key : keys) {
        bool key_exists = false;
        for (const auto &it : table.attrs) {
            if (key == it.name) {
                if (false == it.unique) throw MiniSQLException("Index Key Is Not Unique!");
                primary_key_type = it.type;
                key_exists = true;
                break;
            }
        }
        if(!key_exists) throw MiniSQLException("Invalid Index Key Identifier!");
    }
    size_t size = (PAGESIZE - basic_length) / (sizeof(int) * 2 + primary_key_type.size) - 1;

    CM->addIndexInfo(tablename, indexname, keys);
    IM->createIndex<int>(tablename, indexname, size);
}

void API::dropIndex(const string &tablename, const string &indexname) {
    CM->deleteIndexInfo(tablename, indexname);
    IM->dropIndex(tablename, indexname);
}

void API_test() {
    BufferManager BM;
    CatalogManager CM;
    IndexManager IM(&BM);
    API core(&CM, &IM);
    try {
        core.createIndex("table1", "index1", { "a" });
        core.dropIndex("table1", "index1");
    } catch (MiniSQLException &e) {
        std::cout << e.getMessage() << std::endl;
    }
}