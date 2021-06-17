#include "MiniSQLCatalogManager.h"

bool CatalogManager::findIndex(const string &tablename, const string &indexname) const {
    if (index.end() == index.find(tablename)) return false;
    for (auto index_data = index.at(tablename).begin(); index_data != index.at(tablename).end(); index_data++) {
        if (index_data->name == indexname) return true;
    }
    return false;
}

void CatalogManager::addIndexInfo(const string &tablename, const string &indexname, initializer_list<string> keys) {
    if (findIndex(tablename, indexname)) throw MiniSQLException("Index already exists!");
    index[tablename].push_back({ indexname,keys });
}

void CatalogManager::deleteIndexInfo(const string &tablename, const string &indexname) {
    if (index.end() == index.find(tablename)) throw MiniSQLException("Index doesn't exist!");
    for (auto index_data = index[tablename].begin(); index_data != index[tablename].end(); index_data++) {
        if (index_data->name == indexname) index[tablename].erase(index_data);
        return;
    }
}