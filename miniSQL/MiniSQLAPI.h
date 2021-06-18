#pragma once
#include "MiniSQLCatalogManager.h"
#include "MiniSQLIndexManager.h"
#include "MiniSQLException.h"
#include <initializer_list>
#include <string>
using std::initializer_list;
using std::string;

class API {
public:
    API(CatalogManager *CM, IndexManager *IM) : CM(CM), IM(IM) {}
    void createTable(const string &tablename, const std::vector<Attr> &attrs, initializer_list<string> primary_key);
    void dropTable(const string &tablename);
    void createIndex(const string &tablename, const string &indexname, initializer_list<string> keys);
    void dropIndex(const string &tablename, const string &indexname);
    void insertIntoTable();
    void selectFromTable();
    void deleteFromTable();
private:
    CatalogManager *CM;
    IndexManager *IM;
};