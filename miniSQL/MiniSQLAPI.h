#pragma once
#include "MiniSQLCatalogManager.h"
#include "MiniSQLIndexManager.h"
#include "MiniSQLException.h"
using std::string;

class API {
public:
    API(CatalogManager *CM, IndexManager *IM) : CM(CM), IM(IM) {}
    void createTable(const string &tablename, const std::vector<Attr> &attrs, const set<string> &primary_key);
    void dropTable(const string &tablename);
    void createIndex(const string &tablename, const string &indexname, const set<string> &keys);
    void dropIndex(const string &tablename, const string &indexname);
    void insertIntoTable(const string &tablename, std::vector<Value> record);
    void selectFromTable();
    void deleteFromTable();
private:
    CatalogManager *CM;
    IndexManager *IM;
};