#pragma once
#include "MiniSQLCatalogManager.h"
#include "MiniSQLRecordManager.h"
#include "MiniSQLIndexManager.h"
#include "MiniSQLException.h"
using std::string;

struct SQLResult {
    Table table;
    ReturnTable ret;
};

class API {
public:
    API(CatalogManager *CM, RecordManager *RM, IndexManager *IM) : CM(CM), RM(RM), IM(IM) {}

    void createTable(const string &tablename, const std::vector<Attr> &attrs, const set<string> &primary_key);
    void dropTable(const string &tablename);
    void createIndex(const string &tablename, const string &indexname, const set<string> &keys);
    void dropIndex(const string &tablename, const string &indexname);
    void insertIntoTable(const string &tablename, Record &record);
    SQLResult selectFromTable(const string &tablename, const Predicate &pred);
    int deleteFromTable(const string &tablename, const Predicate &pred);

private:
    CatalogManager *CM;
    RecordManager *RM;
    IndexManager *IM;

    void checkPredicate(const string &tablename, const Predicate &pred) const;
    std::map<Compare, std::set<Value>> filterCondition(const std::vector<Condition> &conds) const;
};