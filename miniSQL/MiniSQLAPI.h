#pragma once
#include<initializer_list>
#include<string>
using std::string;
using std::initializer_list;

class API {
public:
    void createTable();
    void dropTable();
    void createIndex(const string &tablename, const string &indexname, initializer_list<string> keys);
    void dropIndex(const string &indexname);
    void insertIntoTable();
    void selectFromTable();
    void deleteFromTable();
};