#include "MiniSQLAPI.h"
#include <iostream>

void API::checkPredicate(const string &tablename, const Predicate &pred) const {
    const Table &table = CM->getTableInfo(tablename);
    for (const auto &pred : pred) {
        bool attr_exists = false;
        for (const auto &attr : table.attrs) {
            if (pred.first == attr.name) {
                attr_exists = true;
                break;
            }
        }
        if (!attr_exists) throw MiniSQLException("Invalid Attribute Identifier!");
    }
}

void API::createTable(const string &tablename, const std::vector<Attr> &attrs, const set<string> &primary_key) {
    CM->addTableInfo(tablename, attrs);
    RM->createTable(tablename);
    if(primary_key.size() > 0) createIndex(tablename, "PRIMARY_KEY", primary_key);
}

void API::dropTable(const string &tablename) {
    const auto &indexes = CM->getIndexInfo(tablename);
    for (const auto &index : indexes) IM->dropIndex(tablename, index.name);

    CM->deleteTableInfo(tablename);
    CM->deleteIndexInfo(tablename);
    RM->dropTable(tablename);
}

void API::createIndex(const string &tablename, const string &indexname, const set<string> &keys) {
    const Table &table = CM->getTableInfo(tablename);
    Type primary_key_type;
    size_t basic_length = sizeof(bool) + sizeof(int) * 3;
    for (const auto &key : keys) {
        bool key_exists = false;
        for (const auto &attr : table.attrs) {
            if (key == attr.name) {
                if (false == attr.unique) throw MiniSQLException("Index Key Is Not Unique!");
                primary_key_type = attr.type;
                key_exists = true;
                break;
            }
        }
        if(!key_exists) throw MiniSQLException("Invalid Index Key Identifier!");
    }
    size_t size = (PAGESIZE - basic_length) / (sizeof(int) * 2 + primary_key_type.size) - 1;

    CM->addIndexInfo(tablename, indexname, keys);
    switch (primary_key_type.btype) {
    case BaseType::CHAR:    IM->createIndex<FLString>(tablename, indexname, size); break;
    case BaseType::INT:    IM->createIndex<int>(tablename, indexname, size); break;
    case BaseType::FLOAT:    IM->createIndex<float>(tablename, indexname, size); break;
    }
}

void API::dropIndex(const string &tablename, const string &indexname) {
    CM->deleteIndexInfo(tablename, indexname);
    IM->dropIndex(tablename, indexname);
}

/*
create table table1 (
 id int,
 name char(20),
 money float unique,
 primary key (id)
);

insert into table1 values (3, "abc", 4.5);
insert into table1 values (4, "abc", 5.5);
insert into table1 values (7, "abc", 5.5);
select * from table1 where id = 4 and money > 600 and name <> "amy";
*/

void  API::insertIntoTable(const string &tablename, Record &record) {
    const Table &table = CM->getTableInfo(tablename);
    if (table.attrs.size() != record.size()) throw MiniSQLException("Wrong Number of Inserted Values!");

    //Value×ª»»
    auto value_ptr = record.begin();
    for (const auto &attr : table.attrs) {
        value_ptr->convertTo(attr.type);
        value_ptr++;
    }

    string filename = "../" + tablename + ".table";
    RM->insertRecord(filename, table, record);
    CM->modifyRecordCount(tablename, 1);

    const auto &indexes = CM->getIndexInfo(tablename);
    for (const auto &index : indexes) {
        size_t basic_length = sizeof(bool) + sizeof(int) * 3;
        Type index_key_type;
        value_ptr = record.begin();
        for (const auto &key : index.keys) {
            for (const auto &attr : table.attrs) {
                if (attr.name == key) {
                    index_key_type = attr.type;
                    break;
                }
                value_ptr++;
            }
        }
        size_t size = (PAGESIZE - basic_length) / (sizeof(int) * 2 + index_key_type.size) - 1;
        switch (index_key_type.btype) {
        case BaseType::CHAR:    IM->insertIntoIndex<FLString>(tablename, index.name, size, FLString(value_ptr->translate<char*>()),0); break;
        case BaseType::INT:    IM->insertIntoIndex<int>(tablename, index.name, size, value_ptr->translate<int>(),0); break;
        case BaseType::FLOAT:    IM->insertIntoIndex<float>(tablename, index.name, size, value_ptr->translate<float>(),0); break;
        }
    }
}

void API::selectFromTable(const string &tablename, const Predicate &pred) {
    checkPredicate(tablename, pred);

    const auto &indexes = CM->getIndexInfo(tablename);
    for (const auto &pred : pred) {
        for (const auto &index : indexes) {
            if (index.keys.end() == index.keys.find(pred.first)) continue;
            size_t basic_length = sizeof(bool) + sizeof(int) * 3;
            Type index_key_type;
            const Table &table = CM->getTableInfo(tablename);
            for (const auto &key : index.keys) {
                for (const auto &attr : table.attrs) {
                    if (attr.name == key) {
                        index_key_type = attr.type;
                        break;
                    }
                }
            }
            size_t size = (PAGESIZE - basic_length) / (sizeof(int) * 2 + index_key_type.size) - 1;
            switch (index_key_type.btype) {
            case BaseType::CHAR:    IM->createIndex<FLString>(tablename, index.name, size); break;
            case BaseType::INT:    IM->createIndex<int>(tablename, index.name, size); break;
            case BaseType::FLOAT:    IM->createIndex<float>(tablename, index.name, size); break;
            }
        }
    }
}

void API::deleteFromTable(const string &tablename, const Predicate &pred) {
    checkPredicate(tablename, pred);

    const auto &indexes = CM->getIndexInfo(tablename);
}

void API_test() {
    BufferManager BM;
    CatalogManager CM;
    RecordManager RM(&BM);
    IndexManager IM(&BM);
    API core(&CM, &RM, &IM);
    try {
        core.createIndex("table1", "index1", { "a" });
        core.dropIndex("table1", "index1");
    } catch (MiniSQLException &e) {
        std::cout << e.getMessage() << std::endl;
    }
}