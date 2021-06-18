#include "MiniSQLAPI.h"
#include <iostream>

void API::createTable(const string &tablename, const std::vector<Attr> &attrs, initializer_list<string> primary_key) {
    CM->addTableInfo(tablename, attrs);
    CM->getTableInfo(tablename);
}

void API::dropTable(const string &tablename) {
    CM->deleteTableInfo(tablename);
    CM->deleteIndexInfo(tablename);
}

void API::createIndex(const string &tablename, const string &indexname, initializer_list<string> keys) {
    CM->addIndexInfo(tablename, indexname, keys);
    IM->createIndex<int>(tablename, indexname, 200);
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
        cout << e.getMessage() << endl;
    }
}