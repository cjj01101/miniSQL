#include "MiniSQLRecordManager.h"

#define TABLE_FILE_PATH(tablename) ("../" + (tablename) + ".table")

//计算表所在文件有多少块
int RecordManager::getBlockNum(const Table &table) const {
    int record_per_block = PAGESIZE / table.record_length;
	return table.occupied_record_count / record_per_block + 1;
}

//判断记录是否符合条件
bool RecordManager::isFit(const Value &v, const std::vector<Condition> &cond) const {
	for (const auto &iter : cond) {
        switch (iter.comp)
        {
        case Compare::EQ:
            if (v != iter.data) return false;
            break;
        case Compare::LE:
            if (v > iter.data) return false;
            break;
        case Compare::GE:
            if (v < iter.data) return false;
            break;
        case Compare::NE:
            if (v == iter.data) return false;
            break;
        case Compare::LT:
            if (v >= iter.data) return false;
            break;
        case Compare::GT:
            if (v <= iter.data) return false;
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
	for (const auto &attr : table.attrs) {
		record.push_back(Value(attr.type, p));
		p += attr.type.size;
	}
	return record;
}

void RecordManager::createTable(const string &tablename) {
    string filename = TABLE_FILE_PATH(tablename);

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
input:tablename,Table,Predicate
output:返回记录（集成）
找文件头，一块一块载入buffer，每载入一块就一条一条查，在valid bit为1的记录中比较Predicate
然后加入一个set
*/
ReturnTable RecordManager::selectRecord(const string &tablename, const Table &table, const Predicate &pred){
    string filename = TABLE_FILE_PATH(tablename);

	int searched_record = 0;
	int block_num = getBlockNum(table);
    int record_length = table.record_length;
    int record_per_block = PAGESIZE / record_length;
	ReturnTable T;
    for (int k = 0; k < block_num; k++) {
        char* curRecord = buffer->getBlockContent(filename, k);//返回该页的头指针
        while (true) {
            if (k == block_num - 1 && searched_record == table.occupied_record_count) break;
            char *p = curRecord;
            if (*reinterpret_cast<bool*>(p) == true) { //valid bit为1
                p++;//移到第一个属性
                //一个个属性和pred比对
                bool satisfied = true;
                for (const auto &attr : table.attrs) {
                    string attr_name = attr.name;
                    if (pred.end() != pred.find(attr_name)) {//找到了这个属性上的条件
                        //提取data进行比较
                        Value v(attr.type, p);
                        if (!isFit(v, pred.at(attr_name))) {//不合条件
                            satisfied = false;
                            break;
                        }
                    }
                    p += attr.type.size;
                }
                if (satisfied) {//循环之后satisfied仍为1 or 没有where条件
                    //加入set
                    RecordInfo rec;
                    rec.pos = { k, searched_record * record_length };
                    rec.content = addRecord(curRecord + sizeof(bool), table);
                    T.push_back(rec);
                }
            }

            curRecord += record_length;
            if (++searched_record % record_per_block == 0) break;
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
void RecordManager::deleteRecord(const string &tablename, const Position &pos) {
    string filename = TABLE_FILE_PATH(tablename);
    bool valid = false;
    buffer->setBlockContent(filename, pos.block_id, pos.offset, reinterpret_cast<char*>(&valid), sizeof(valid));
}
/*
insert
input:tablename,Table，Record
output:none
插入需要检查unique属性还有主键属性是否重复，throw异常
然后找到文件最后一块的最末尾，插记录，valid bit置为1
*/
Position RecordManager::insertRecord(const string &tablename, const Table &table, const Record &record) {
    string filename = TABLE_FILE_PATH(tablename);

	//检测冲突
    auto value_ptr = record.begin();
    for (auto attr = table.attrs.begin(); attr != table.attrs.end(); attr++, value_ptr++) {
        if (attr->unique) {
            Predicate pred;
            pred[attr->name].push_back({ Compare::EQ, *value_ptr });
            ReturnTable result = selectRecord(tablename, table, pred);
            if(result.size() > 0) throw MiniSQLException("Duplicate Value on Unique Attribute!");
        }
    }
    //插入
    int inserted_block_num = getBlockNum(table) - 1;
    int record_per_block = PAGESIZE / table.record_length;
    int offset = (table.occupied_record_count - record_per_block*inserted_block_num)*table.record_length;
    Position pos = { inserted_block_num, offset };
    //设置valid
    bool valid = true;
    buffer->setBlockContent(filename, inserted_block_num, offset, reinterpret_cast<char*>(&valid), sizeof(valid));
    //写数据
    offset += sizeof(valid);
    for (const auto &value : record) {
        char *data = value.translate<char*>();
        buffer->setBlockContent(filename, inserted_block_num, offset, data, value.type.size);
        offset += value.type.size;
    }

    return pos;
}