#include "MiniSQLRecordManager.h"

//计算表所在文件有多少块
int RecordManager::getBlockNum(const Table &table) const {
	return table.record_count / table.record_per_block + 1;
}

//计算一条记录有多少位（包括valid bit）
int RecordManager::getRecordLength(const Table &table) const {
	size_t length = 1;//算上valid bit
	for (auto attr : table.attrs) length += attr.type.size;
	return length;
}

//判断记录是否符合条件
bool RecordManager::isFit(const Value &v, const std::vector<Condition> &cond) const {
	for (auto iter = cond.begin(); iter != cond.end(); iter++) {
        switch (iter->comp)
        {
        case Compare::EQ:
            if (v != iter->data) return false;
            break;
        case Compare::LE:
            if (v > iter->data) return false;
            break;
        case Compare::GE:
            if (v < iter->data) return false;
            break;
        case Compare::NE:
            if (v == iter->data) return false;
            break;
        case Compare::LT:
            if (v >= iter->data) return false;
            break;
        case Compare::GT:
            if (v <= iter->data) return false;
            break;
        default: throw MiniSQLException("Unsupported Comparation!");
        }
	}
	return true;
}

//集成符合条件的记录
Record RecordManager::addRecord(const char *const data, const Table &table) {
	Record record;
    const char *p = data;
	for (auto attr : table.attrs) {
		record.push_back(Value(attr.type, p));
		p += attr.type.size;
	}
	return record;
}

void RecordManager::createTable(const string &tablename) {
    string filename = "../" + tablename + ".table";

	FILE* fp;
	fopen_s(&fp, filename.data(), "w");
	if (fp == nullptr) throw MiniSQLException("Fail to create table file!"); //创建文件失败
	fclose(fp);
}

void RecordManager::dropTable(const string &tablename) {
    string filename = "../" + tablename + ".table";

	buffer->setEmpty(filename);
	//清除文件内容,remove成功返回0
    if (remove(filename.data()) != 0) throw MiniSQLException("Fail to drop table file!");
}
/*
select
input:filename,Table,Predicate
output:返回记录（集成）
找文件头，一块一块载入buffer，每载入一块就一条一条查，在valid bit为1的记录中比较Predicate
然后加入一个set
*/
ReturnTable RecordManager::selectRecord(const string &filename, const Table &table, const Predicate &pred){
	int searched_record = 0;
	int block_num = getBlockNum(table);
	int record_length = getRecordLength(table);
	ReturnTable T;
    for (int k = 0; k < block_num; k++) {
        char* curRecord = buffer->getBlockContent(filename, k);//返回该页的头指针
        while (true) {
            if (k == block_num - 1 && searched_record == table.record_count) break;
            char *p = curRecord;
            if (*reinterpret_cast<bool*>(p) == true) { //valid bit为1
                p++;//移到第一个属性
                //一个个属性和pred比对
                bool satisfied = true;
                for (auto attr : table.attrs) {
                    string attr_name = attr.name;
                    size_t attr_length = attr.type.size;
                    if (pred.end() != pred.find(attr_name)) {//找到了这个属性上的条件
                        //提取data进行比较
                        Value v(attr.type, p);
                        if (!isFit(v, pred.at(attr_name))) {//不合条件
                            satisfied = false;
                            break;
                        }
                    }
                    p += attr_length;
                }
                if (satisfied) {//循环之后satisfied仍为1 or 没有where条件
                    //加入set
                    RecordInfo rec;
                    rec.pos = { filename, k, searched_record * record_length };
                    rec.content = addRecord(curRecord, table);
                    T.push_back(rec);
                }
            }

            curRecord += record_length;
            if (++searched_record % (table.record_per_block) == 0) break;
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
void RecordManager::deleteRecord(const Position &pos) {
    string tablename = pos.filename;
    int block_id = pos.block_id;
    int offset = pos.offset;
    char *data = "0";
    buffer->setBlockContent(tablename, block_id, offset, data, 1);
}
/*
insert
input:filename,Table，Record
output:none
插入需要检查unique属性还有主键属性是否重复，throw异常
然后找到文件最后一块的最末尾，插记录，valid bit置为1
*/
void RecordManager::insertRecord(const string &filename, const Table &table, const Record &record) {
	//检测冲突
    auto value_ptr = record.begin();
    for (auto attr = table.attrs.begin(); attr != table.attrs.end(); attr++, value_ptr++) {
        if (attr->unique) {
            Predicate pred;
            pred[attr->name].push_back({ Compare::EQ, *value_ptr });
            ReturnTable result = selectRecord(filename, table, pred);
            if(result.size() > 0) throw MiniSQLException("Duplicate Value on Unique Attribute!");
        }
    }
    //插入
    int block_num = getBlockNum(table);
    int record_length = getRecordLength(table);
    int offset = (table.record_count - table.record_per_block*(block_num - 1))*record_length;
    //设置valid
    bool valid = true;
    buffer->setBlockContent(filename, block_num - 1, offset, reinterpret_cast<char*>(&valid), sizeof(valid));
    //写数据
    offset += sizeof(valid);
    for (auto value : record) {
        char *data = value.translate<char*>();
        buffer->setBlockContent(filename, block_num - 1, offset, data, value.type.size);
        offset += value.type.size;
    }
}