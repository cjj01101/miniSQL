#include "MiniSQLAPI.h"
#include <iostream>

void API::createIndex(const string &tablename, const string &indexname, initializer_list<string> keys) {
    try {
        CM->addIndexInfo(tablename, indexname, keys);
        IM->createIndex<int>(tablename, indexname, 200);
    } catch (MiniSQLException &e) {
        std::cout << e.getMessage() << std::endl;
    }
}

void API::dropIndex(const string &tablename, const string &indexname) {
    try {
        CM->deleteIndexInfo(tablename, indexname);
        IM->dropIndex(tablename, indexname);
    }
    catch (MiniSQLException &e) {
        std::cout << e.getMessage() << std::endl;
    }
}

void API_test() {
    BufferManager BM;
    CatalogManager CM;
    IndexManager IM(&BM);
    API core(&CM, &IM);
    core.createIndex("table1", "index1", { "a" });
    core.dropIndex("table1", "index1");
}