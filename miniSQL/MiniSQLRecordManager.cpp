#include "MiniSQLRecordManager.h"
#include <cmath>
#include <cstdio>
#include <cstring>
//计算表所在文件有多少块
int RecordManager::getBlockNum(Table table) {
	float n = 1.0* table.record_count / table.record_count;
	return ceil(n);
}

//计算一条记录有多少位（包括valid bit）
int RecordManager::getRecordLength(Table table) {
	int length = 1;//算上valid bit
	for (auto iter = table.attrs.begin(); iter != table.attrs.end(); iter++) {
		if (iter->type.btype==BaseType::CHAR) {
			length += iter->type.size;
		}
		else {//int和float都是4个字节
			length += 4;
		}
	}
	return length;
}
//判断记录是否符合条件
bool RecordManager::isFit(Value v, Predicate pred, const string &attr) {
	int flag = 0;
	for (auto iter = pred[attr].begin(); iter != pred[attr].end(); iter++) {
		switch (iter->comp)
		{
		case Compare::EQ:
			if (v==iter->data)flag = 1;
			else flag = 0;
			break;
		case Compare::LE:
			if (v <= iter->data)flag = 1;
			else flag = 0;
			break;
		case Compare::GE:
			if (v >= iter->data)flag = 1;
			else flag = 0;
			break;
		case Compare::NE:
			if (v != iter->data)flag = 1;
			else flag = 0;
			break;
		case Compare::LT:
			if (v < iter->data)flag = 1;
			else flag = 0;
			break;
		case Compare::GT:
			if (v > iter->data)flag = 1;
			else flag = 0;
			break;
		default:
			break;
		}
		if (flag == 0) return false;//只要有一个条件不符合就不选
	}
	return true;
}

//集成符合条件的记录
Record RecordManager::AddRecord(char *q, Table table) {
	Record rec;
	for (auto iter = table.attrs.begin(); iter != table.attrs.end(); iter++) {
		int attr_length = iter->type.size;
		char *data = nullptr;
		data = new char[30];
		strncpy(data, q, attr_length);
		Value v = Value::Value(iter->type, data);
		rec.push_back(v);
		q += attr_length;
	}
	return rec;
}

//判断记录是否冲突？
bool RecordManager::isConflict(Table table, Record record, const string &tablename, int i) {
	int searched_record = 0;
	int block_num = getBlockNum(table);
	int record_length = getRecordLength(table);
	for (int k = 0; k < block_num; k++) {
		char* head = buffer->getBlockContent(tablename, k);//返回该页的头指针
		char* p = head;
		if (k == block_num - 1) { //到最后一块
			do {
				if (*(p + 1) == '1') {
					p++;
					int j = 0;
					auto iter = table.attrs.begin();
					for (; j < i; j++, iter++) {
						p += iter->type.size;
					}
					//p指向第i个属性的值,iter指向第i个Attr结构
					char *data = nullptr;
					data = new char[30];
					strncpy(data, p, iter->type.size);
					Value v = Value::Value(iter->type, data);
					if (v==record[i])return true;
					for (; iter != table.attrs.end(); iter++) {
						p += iter->type.size;
					}
				}
				else {
					p += record_length;
				}
				searched_record++;
				
			} while (searched_record != table.record_count);
		}
		else {
			do {
				if (*(p + 1) == '1') {
					p++;
					int j = 0;
					auto iter = table.attrs.begin();
					for (; j < i; j++, iter++) {
						p += iter->type.size;
					}
					//p指向第i个属性的值,iter指向第i个Attr结构
					char *data = nullptr;
					data = new char[30];
					strncpy(data, p, iter->type.size);
					Value v = Value::Value(iter->type, data);
					if (v == record[i])return true;
					for (; iter != table.attrs.end(); iter++) {
						p += iter->type.size;
					}
				}
				else {
					p += record_length;
				}
				searched_record++;
			} while (searched_record % (table.record_per_block) != 0);
		}
	}
	return false;
}


void RecordManager::createTable(const string &tablename) {
	FILE* fp;
	fopen_s(&fp, tablename.c_str(), "rb+");
	if (fp == nullptr) throw MiniSQLException("Fail to create table file!"); //创建文件失败
	fclose(fp);
}

void RecordManager::dropTable(const string &tablename, Table table) {
	buffer->setEmpty(tablename);
	//清除文件内容,remove成功返回0
	if(remove(tablename.c_str())!=0)throw MiniSQLException("Fail to drop table file!");
}
/*
select
input:table_name,Table,Predicate
output:返回记录（集成）
找文件头，一块一块载入buffer，每载入一块就一条一条查，在valid bit为1的记录中比较Predicate
然后加入一个set
*/
ReturnTable RecordManager::selectRecord(const string &tablename, Table table, Predicate pred){
	int searched_record = 0;
	int block_num = getBlockNum(table);
	int record_length = getRecordLength(table);
	ReturnTable T;
	for (int i = 0; i < block_num; i++) {
		char* head=buffer->getBlockContent(tablename, i);//返回该页的头指针
		char* p = head;
		if (i == block_num - 1) { //到最后一块
			do {
				if (*(p + 1) == '1') { //valid bit为1
					p++;//移到第一个属性
					//一个个属性和pred比对
					int flag = 1;
					for (auto iter = table.attrs.begin(); iter != table.attrs.end(); iter++) {
						string attr = iter->name;
						int attr_length = iter->type.size;
						if (pred.end() != pred.find(attr)) {//找到了这个属性上的条件
							//提取data进行比较
							char *data = nullptr;
							data = new char[30];
							strncpy(data, p, attr_length);
							Value v = Value::Value(iter->type, data);
							if (!isFit(v, pred, attr)) {//不合条件
								flag = 0;
								break;
							}
						}
						p += attr_length;
					}
					if (flag == 1) {//循环之后flag仍为1 or 没有where条件
						//pos
						Position pos;
						pos.filename = tablename;
						pos.block_id = i;
						pos.offset = searched_record * record_length;
						//加入set
						RecordInfo rec;
						rec.pos = pos;
						char *q = p - record_length + 1;//指向第一个属性
						rec.content = AddRecord(q, table);
						T.insert(rec);
					}
				}
				else {
					p += record_length;
				}
				searched_record++;//读完一条记录
			} while (searched_record != table.record_count);
		}
		else {//之前的块都是满的，记录数为record_per_block
			do {
				if (*(p+1)=='1') { //valid bit为1
					p++;//移到第一个属性
					//一个个属性和pred比对
					int flag = 1;
					for (auto iter = table.attrs.begin(); iter != table.attrs.end(); iter++) {
						string attr = iter->name;
						int attr_length = iter->type.size;
						if (pred.end() != pred.find(attr)) {//找到了这个属性上的条件
							//提取data进行比较
							char *data = nullptr;
							data = new char[30];
							strncpy(data, p, attr_length);
							Value v = Value::Value(iter->type,data);
							if (!isFit(v, pred, attr)) {//不合条件
								flag = 0;
								break;
							}
						}
						p += attr_length;
					}
					if (flag == 1) {//循环之后flag仍为1 or 没有where条件
						//pos
						Position pos;
						pos.filename = tablename;
						pos.block_id = i;
						pos.offset = searched_record * record_length;
						//加入set
						RecordInfo rec;
						rec.pos = pos;
						char *q = p - record_length + 1;//指向第一个属性
						rec.content = AddRecord(q,table);
						T.insert(rec);
					}
				}
				else {
					p += record_length;
				}
				searched_record++;//读完一条记录
			} while (searched_record % (table.record_per_block) != 0);//searched_block为整数倍说明这一块的记录读完了
			
		}
	}
	return T;
}

/*
delete
input:table_name,Table,Predicate
output:none
传入position，把buffer中相应块dirty=true，该记录的valid bit置为false
*/
void RecordManager::deleteRecord(Position pos) {
	string tablename = pos.filename;
	int block_id = pos.block_id;
	int offset = pos.offset;
	char *data = "0";
	buffer->setBlockContent(tablename, block_id, offset, data, 1);
}
/*
insert
input:table_name,Table，Record
output:none
插入需要检查unique属性还有主键属性是否重复，throw异常
然后找到文件最后一块的最末尾，插记录，valid bit置为1
*/
void RecordManager::insertRecord(const string &tablename, Table table, Record record) {
	//检测冲突
	int i = 0;//第i个属性
	for (auto iter = table.attrs.begin(); iter != table.attrs.end(); iter++,i++) {
		string attr = iter->name;
		if (iter->unique) {
			if (isConflict(table, record, tablename, i)) throw MiniSQLException("Duplicate value on unique attribute!");
		}
	}
	//插入
	int block_num = getBlockNum(table);
	int record_length = getRecordLength(table);
	int offset = (table.record_count - table.record_per_block*(block_num-1))*record_length;
	//设置valid
	char *valid = "1";
	buffer->setBlockContent(tablename, block_num-1, offset, valid, 1);
	//写数据
	offset++;
	for (auto iter = record.begin(); iter != record.end(); iter++) {
		char *data = iter->translate<char*>();
		buffer->setBlockContent(tablename, block_num - 1, offset, data, iter->type.size);
		offset += iter->type.size;
	}
}
