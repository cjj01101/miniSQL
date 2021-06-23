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

std::map<Compare, std::set<Value>> API::filterCondition(const std::vector<Condition> &conds) const {
    std::map<Compare, std::set<Value>> newCond;

    //合并范围条件
    for (auto cond : conds) {
        Compare comp = cond.comp;
        Value data = cond.data;
        if (comp == Compare::GE || comp == Compare::GT) {
            auto gCond = newCond.find(Compare::GE);
            if (newCond.end() == gCond) gCond = newCond.find(Compare::GT);
            if (newCond.end() == gCond) newCond[comp].insert(data);
            else {
                Compare old_comp = gCond->first;
                Value old_data = *(gCond->second.begin());
                if (old_data < data || (old_comp == Compare::GE && comp == Compare::GT && old_data == data)) {
                    newCond.erase(gCond);
                    newCond[comp].insert(data);
                }
            }
        }
        else if (comp == Compare::LE || comp == Compare::LT) {
            auto lCond = newCond.find(Compare::LE);
            if (newCond.end() == lCond) lCond = newCond.find(Compare::LT);
            if (newCond.end() == lCond) newCond[comp].insert(data);
            else {
                Compare old_comp = lCond->first;
                Value old_data = *(lCond->second.begin());
                if (old_data > data || (old_comp == Compare::LE && comp == Compare::LT && old_data == data)) {
                    newCond.erase(lCond);
                    newCond[comp].insert(data);
                }
            }
        }
    }

    //两方向范围冲突判断
    auto gCond = newCond.find(Compare::GE);
    if (newCond.end() == gCond) gCond = newCond.find(Compare::GT);
    auto lCond = newCond.find(Compare::LE);
    if (newCond.end() == lCond) lCond = newCond.find(Compare::LT);

    bool hasLCond = (newCond.end() != lCond);
    bool hasGCond = (newCond.end() != gCond);
    if (hasLCond && hasGCond) {
        if (*(lCond->second.begin()) < *(gCond->second.begin()) ||
           (*(lCond->second.begin()) == *(gCond->second.begin()) && !(lCond->first == Compare::LE && gCond->first == Compare::GE))) {
            return std::map<Compare, std::set<Value>>();
        }
    }

    //合并相等条件
    for (auto cond : conds) {
        if (cond.comp == Compare::EQ) {
            Value data = cond.data;
            //判断是否在范围内
            if (hasLCond && (data > *(lCond->second.begin()) || (data == *(lCond->second.begin()) && lCond->first == Compare::LT))) {
                return std::map<Compare, std::set<Value>>();
            }
            if (hasGCond && (data < *(gCond->second.begin()) || (data == *(gCond->second.begin()) && gCond->first == Compare::GT))) {
                return std::map<Compare, std::set<Value>>();
            }
            //判断等号是否冲突
            auto eqCond = newCond.find(Compare::EQ);
            if (newCond.end() != eqCond) {
                if(data != *(eqCond->second.begin())) return std::map<Compare, std::set<Value>>();
            }
            else {
                newCond.clear();
                hasLCond = hasGCond = false;
                newCond[Compare::EQ].insert(data);
            }
        }
    }

    //合并不等条件
    for (auto cond : conds) {
        if (cond.comp == Compare::NE) {
            Value data = cond.data;
            
            //判断等号是否冲突
            auto eqCond = newCond.find(Compare::EQ);
            if (newCond.end() != eqCond) {
                if (data == *(eqCond->second.begin())) return std::map<Compare, std::set<Value>>();
            }
            //判断是否在范围内
            else if ((!hasLCond || (hasLCond && (data < *(lCond->second.begin()) || (data == *(lCond->second.begin()) && lCond->first == Compare::LE)))) &&
                (!hasGCond || (hasGCond && (data > *(gCond->second.begin()) || (data == *(gCond->second.begin()) && gCond->first == Compare::GE))))) {
                newCond[Compare::NE].insert(data);
            }
        }
    }

    return newCond;
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
    int attr_pos = 0;
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
            attr_pos++;
        }
        if(!key_exists) throw MiniSQLException("Invalid Index Key Identifier!");
    }
    size_t size = (PAGESIZE - basic_length) / (sizeof(int) + sizeof(Position) + primary_key_type.size) - 1;

    CM->addIndexInfo(tablename, indexname, keys);
    switch (primary_key_type.btype) {
    case BaseType::CHAR:    IM->createIndex<FLString>(tablename, indexname, size); break;
    case BaseType::INT:    IM->createIndex<int>(tablename, indexname, size); break;
    case BaseType::FLOAT:    IM->createIndex<float>(tablename, indexname, size); break;
    }

    ReturnTable T = selectFromTable(tablename, Predicate()).ret;
    for (const auto &record : T) {
        switch (primary_key_type.btype) {
        case BaseType::CHAR:    IM->insertIntoIndex<FLString>(tablename, indexname, size, FLString((record.content)[attr_pos].translate<char*>()), record.pos); break;
        case BaseType::INT:    IM->insertIntoIndex<int>(tablename, indexname, size, (record.content)[attr_pos].translate<int>(), record.pos); break;
        case BaseType::FLOAT:    IM->insertIntoIndex<float>(tablename, indexname, size, (record.content)[attr_pos].translate<float>(), record.pos); break;
        }
    }
}

void API::dropIndex(const string &tablename, const string &indexname) {
    CM->deleteIndexInfo(tablename, indexname);
    IM->dropIndex(tablename, indexname);
}

void API::insertIntoTable(const string &tablename, Record &record) {
    const Table &table = CM->getTableInfo(tablename);
    if (table.attrs.size() != record.size()) throw MiniSQLException("Wrong Number of Inserted Values!");

    //Value转换
    auto value_ptr = record.begin();
    for (const auto &attr : table.attrs) {
        value_ptr->convertTo(attr.type);
        value_ptr++;
    }

    Position insertPos = RM->insertRecord(tablename, table, record);
    CM->increaseRecordCount(tablename);

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
        if (index_key_type.btype == BaseType::CHAR) index_key_type.size = MAXCHARSIZE;
        size_t size = (PAGESIZE - basic_length) / (sizeof(int) + sizeof(Position) + index_key_type.size) - 1;
        switch (index_key_type.btype) {
        case BaseType::CHAR:    IM->insertIntoIndex<FLString>(tablename, index.name, size, FLString(value_ptr->translate<char*>()),insertPos); break;
        case BaseType::INT:    IM->insertIntoIndex<int>(tablename, index.name, size, value_ptr->translate<int>(), insertPos); break;
        case BaseType::FLOAT:    IM->insertIntoIndex<float>(tablename, index.name, size, value_ptr->translate<float>(), insertPos); break;
        }
    }
}

/*
create table t (
 id int,
 name char(20),
 money float unique,
 primary key (id)
);

insert into t values (3, "abc", 4.5);
insert into t values (4, "abc", 5.5);
insert into t values (7, "abc", 10.5);
delete from t where id = 7;
insert into t values (7, "abc", 10.5);
select * from t where id = 4 and money > 600 and name <> "amy";

create table table2 (
 no int,
 name char(20) unique,
 salary float
);
*/

SQLResult API::selectFromTable(const string &tablename, Predicate &pred) {
    checkPredicate(tablename, pred);

    const Table &table = CM->getTableInfo(tablename);
    ReturnTable result;

    //尝试索引
    const auto &indexes = CM->getIndexInfo(tablename);
    for (auto pred_ptr = pred.begin(); pred_ptr != pred.end(); pred_ptr++) {
        for (const auto &index : indexes) {
            if (index.keys.end() == index.keys.find(pred_ptr->first)) continue;

            //计算索引size
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
            if (index_key_type.btype == BaseType::CHAR) index_key_type.size = MAXCHARSIZE;
            size_t size = (PAGESIZE - basic_length) / (sizeof(int) + sizeof(Position) + index_key_type.size) - 1;

            vector<Position> possible_poses;

            //条件合并
            auto newCond = filterCondition(pred_ptr->second);
            if (newCond.size() == 0) return SQLResult();
            if(newCond.end() != newCond.find(Compare::EQ)) {
                const Value &eqValue = *(newCond.find(Compare::EQ)->second.begin());
                Position pos;
                switch (index_key_type.btype) {
                case BaseType::CHAR:    pos = IM->findOneFromIndex<FLString>(tablename, index.name, size, eqValue.translate<char*>()); break;
                case BaseType::INT:    pos = IM->findOneFromIndex<int>(tablename, index.name, size, eqValue.translate<int>()); break;
                case BaseType::FLOAT:    pos = IM->findOneFromIndex<float>(tablename, index.name, size, eqValue.translate<float>()); break;
                }
                possible_poses.push_back(pos);
            } else {
                switch (index_key_type.btype) {
                case BaseType::CHAR: {
                    std::pair<Compare, FLString> startKey = make_pair(Compare::EQ, FLString(""));
                    std::pair<Compare, FLString> endKey = make_pair(Compare::EQ, FLString(""));
                    std::set<FLString> neKeys;
                    if (newCond.end() != newCond.find(Compare::GE)) startKey = make_pair(Compare::GE, FLString(newCond.find(Compare::GE)->second.begin()->translate<char*>()));
                    else if (newCond.end() != newCond.find(Compare::GT)) startKey = make_pair(Compare::GT, FLString(newCond.find(Compare::GT)->second.begin()->translate<char*>()));
                    if (newCond.end() != newCond.find(Compare::LE)) endKey = make_pair(Compare::LE, FLString(newCond.find(Compare::LE)->second.begin()->translate<char*>()));
                    else if (newCond.end() != newCond.find(Compare::LT)) endKey = make_pair(Compare::LT, FLString(newCond.find(Compare::LT)->second.begin()->translate<char*>()));
                    if (newCond.end() != newCond.find(Compare::NE)) {
                        for (auto neKey : newCond.find(Compare::NE)->second) neKeys.insert(FLString(neKey.translate<char*>()));
                    }
                    IM->findRangeFromIndex<FLString>(tablename, index.name, size, startKey, endKey, neKeys, possible_poses); break;
                }
                case BaseType::INT: {
                    std::pair<Compare, int> startKey = make_pair(Compare::EQ, 0);
                    std::pair<Compare, int> endKey = make_pair(Compare::EQ, 0);
                    std::set<int> neKeys;
                    if (newCond.end() != newCond.find(Compare::GE)) startKey = make_pair(Compare::GE, newCond.find(Compare::GE)->second.begin()->translate<int>());
                    else if (newCond.end() != newCond.find(Compare::GT)) startKey = make_pair(Compare::GT, newCond.find(Compare::GT)->second.begin()->translate<int>());
                    if (newCond.end() != newCond.find(Compare::LE)) endKey = make_pair(Compare::LE, newCond.find(Compare::LE)->second.begin()->translate<int>());
                    else if (newCond.end() != newCond.find(Compare::LT)) endKey = make_pair(Compare::LT, newCond.find(Compare::LT)->second.begin()->translate<int>());
                    if (newCond.end() != newCond.find(Compare::NE)) {
                        for (auto neKey : newCond.find(Compare::NE)->second) neKeys.insert(neKey.translate<int>());
                    }
                    IM->findRangeFromIndex<int>(tablename, index.name, size, startKey, endKey, neKeys, possible_poses); break;
                }
                case BaseType::FLOAT: {
                    std::pair<Compare, float> startKey = make_pair(Compare::EQ, 0);
                    std::pair<Compare, float> endKey = make_pair(Compare::EQ, 0);
                    std::set<float> neKeys;
                    if (newCond.end() != newCond.find(Compare::GE)) startKey = make_pair(Compare::GE, newCond.find(Compare::GE)->second.begin()->translate<float>());
                    else if (newCond.end() != newCond.find(Compare::GT)) startKey = make_pair(Compare::GT, newCond.find(Compare::GT)->second.begin()->translate<float>());
                    if (newCond.end() != newCond.find(Compare::LE)) endKey = make_pair(Compare::LE, newCond.find(Compare::LE)->second.begin()->translate<float>());
                    else if (newCond.end() != newCond.find(Compare::LT)) endKey = make_pair(Compare::LT, newCond.find(Compare::LT)->second.begin()->translate<float>());
                    if (newCond.end() != newCond.find(Compare::NE)) {
                        for (auto neKey : newCond.find(Compare::NE)->second) neKeys.insert(neKey.translate<float>());
                    }
                    IM->findRangeFromIndex<float>(tablename, index.name, size, startKey, endKey, neKeys, possible_poses); break;
                }
                }
            }

            pred.erase(pred_ptr);
            result = RM->selectRecord(tablename, table, pred, possible_poses);
            return SQLResult{ table, result };
        }
    }

    result = RM->selectRecord(tablename, table, pred);
    return SQLResult{ table, result };
}

int API::deleteFromTable(const string &tablename, Predicate &pred) {
    checkPredicate(tablename, pred);

    const Table &table = CM->getTableInfo(tablename);
    SQLResult records = selectFromTable(tablename, pred);
    const ReturnTable &result = records.ret;
    for (const auto &record : result) RM->deleteRecord(tablename, record.pos);

    const auto &indexes = CM->getIndexInfo(tablename);
    for (const auto &index : indexes) {
        size_t basic_length = sizeof(bool) + sizeof(int) * 3;
        Type index_key_type;
        for (const auto &record : result) {
            auto value_ptr = record.content.begin();
            for (const auto &key : index.keys) {
                for (const auto &attr : table.attrs) {
                    if (attr.name == key) {
                        index_key_type = attr.type;
                        break;
                    }
                    value_ptr++;
                }
            }
            if (index_key_type.btype == BaseType::CHAR) index_key_type.size = MAXCHARSIZE;
            size_t size = (PAGESIZE - basic_length) / (sizeof(int) + sizeof(Position) + index_key_type.size) - 1;
            switch (index_key_type.btype) {
            case BaseType::CHAR:    IM->removeFromIndex<FLString>(tablename, index.name, size, FLString(value_ptr->translate<char*>())); break;
            case BaseType::INT:    IM->removeFromIndex<int>(tablename, index.name, size, value_ptr->translate<int>()); break;
            case BaseType::FLOAT:    IM->removeFromIndex<float>(tablename, index.name, size, value_ptr->translate<float>()); break;
            }
        }
    }

    return result.size();
}

void API_test() {
    BufferManager BM;
    CatalogManager CM(META_TABLE_FILE_PATH, META_INDEX_FILE_PATH);
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