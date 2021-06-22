#pragma once
#include "MiniSQLBufferManager.h"
#include "MiniSQLCatalogManager.h"
#include "MiniSQLException.h"
#include <vector>
#include <set>
#include <map>
#include <string>
using namespace std;

class RecordManager {
public:
    RecordManager(BufferManager *buffer) : buffer(buffer) {}

	void createTable(const string &tablename);
	void dropTable(const string &tablename);
	ReturnTable selectRecord(const string &tablename, const Table &table, const Predicate &pred);
	void deleteRecord(const string &tablename, const Position &pos);
	Position insertRecord(const string &tablename, const Table &table, const Record &record);
private:
	//计算表所在文件有多少块
	int getBlockNum(const Table &table) const;
	//判断记录是否符合条件
	bool isFit(const Value &v, const std::vector<Condition> &cond) const;
	//集成符合条件的记录
	Record addRecord(const char *const data, const Table &table);
	
	BufferManager *buffer;

};
