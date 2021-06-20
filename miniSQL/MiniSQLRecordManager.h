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
	void deleteRecord(const Position &pos);
	void insertRecord(const string &tablename, const Table &table, const Record &record);
private:
	//计算表所在文件有多少块
	int getBlockNum(Table table);
	//计算一条记录有多少位（包括valid bit）
	int getRecordLength(Table table);
	//判断记录是否符合条件
	bool isFit(Value v, Predicate pred, const string &attr);
	//集成符合条件的记录
	Record AddRecord(char *q, Table table);
	//判断记录是否冲突？
	bool isConflict(Table table, Record record, const string &tablename, int i);
	
	BufferManager *buffer;

};
