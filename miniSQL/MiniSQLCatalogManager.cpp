#include "MiniSQLCatalogManager.h"
#include <fstream>

CatalogManager::CatalogManager(const char *meta_table_file_name, const char *meta_index_file_name)
    : meta_table_file_name(meta_table_file_name), meta_index_file_name(meta_index_file_name)
{
    FILE *fp;
    //读入table信息
    if (fopen_s(&fp, meta_table_file_name, "r")) {
        fopen_s(&fp, meta_table_file_name, "w");
        fclose(fp);
    } else if (fgetc(fp) == EOF) {
        fclose(fp);
    } else {
        fclose(fp);
        std::ifstream inf(meta_table_file_name);
        if (!inf.is_open()) throw MiniSQLException("Cannot Read Meta Table File!");

        string tablename;
        size_t record_length;
        int occupied_record_count;
        int size;
        while (!inf.eof()) {
            inf >> tablename >> record_length >> occupied_record_count >> size;
            vector<Attr> attrs;
            string attr_name;
            Type attr_type;
            bool attr_unique;
            for (int i = 0; i < size; i++) {
                inf >> attr_name >> attr_type >> attr_unique;
                attrs.push_back({ attr_name, attr_type, attr_unique });
            }
            table.insert(make_pair(tablename, Table{ attrs, record_length, occupied_record_count }));
        }
        inf.close();
    }
    //读入index信息
    if (fopen_s(&fp, meta_index_file_name, "r")) {
        fopen_s(&fp, meta_index_file_name, "w");
        fclose(fp);
    } else if (fgetc(fp) == EOF) {
        fclose(fp);
    } else {
        fclose(fp);
        std::ifstream inf(meta_index_file_name);
        if (!inf.is_open()) throw MiniSQLException("Cannot Read Meta Index File!");

        string tablename;
        int size;
        while (!inf.eof()) {
            inf >> tablename >> size;
            vector<Index> indexes;
            string indexname;
            int rank;
            int key_position;
            int key_size;
            set<int> key_positions;
            for (int i = 0; i < size; i++) {
                inf >> indexname >> rank >> key_size;
                for (int j = 0; j < key_size; j++) {
                    inf >> key_position;
                    key_positions.insert(key_position);
                }
                indexes.push_back({ indexname, rank, key_positions });
            }
            index.insert(make_pair(tablename, indexes));
        }

        inf.close();
    }
}

CatalogManager::~CatalogManager() {
    //写入table信息
    std::ofstream outf(meta_table_file_name);
    if (!outf.is_open()) {
        std::cout << "Cannot Open Meta Table File to Write in!" << std::endl;
    } else {
        for (const auto &tab : table) {
            const Table &table_def = tab.second;
            const auto &attr_def = table_def.attrs;
            outf << tab.first << " " << table_def.record_length << " " << table_def.occupied_record_count << " " << attr_def.size() << std::endl;
            for (const auto &attr : attr_def) {
                outf << attr.name << " " << attr.type << " " << attr.unique << std::endl;
            }
        }
        outf.close();
    }

    //写入index信息
    outf = std::ofstream(meta_index_file_name);
    if (!outf.is_open()) {
        std::cout << "Cannot Open Meta Index File to Write in!" << std::endl;
    } else {
        for (const auto &ind : index) {
            const vector<Index> &indexes = ind.second;
            outf << ind.first << " " << indexes.size() << std::endl;
            for (const auto &index : indexes) {
                outf << index.name << " " << index.rank << " " << index.key_positions.size();
                for (const auto &pos : index.key_positions) outf << " " << pos;
                outf << std::endl;
            }
        }
        outf.close();
    }
}

void CatalogManager::increaseRecordCount(const string &tablename) {
    auto t = table.find(tablename);
    if (table.end() == t) throw MiniSQLException("Table Doesn't Exist!");
    table[tablename].occupied_record_count++;
}

const Table &CatalogManager::getTableInfo(const string &tablename) const {
    auto t = table.find(tablename);
    if (table.end() == t) throw MiniSQLException("Table Doesn't Exist!");
    return table.at(tablename);
}

void CatalogManager::addTableInfo(const string &tablename, const vector<Attr> &attrs) {
    if (table.end() != table.find(tablename)) throw MiniSQLException("Duplicate Table Name!");

    size_t length = 1;
    for (auto attr : attrs) length += attr.type.size;
    table[tablename] = { attrs, length, 0 };
    index[tablename];
}

void CatalogManager::deleteTableInfo(const string &tablename) {
    auto t = table.find(tablename);
    if (table.end() == t) throw MiniSQLException("Table Doesn't Exist!");
    table.erase(t);
}

bool CatalogManager::findIndex(const string &tablename, const string &indexname) const {
    if (index.end() == index.find(tablename)) return false;
    for (auto index_data = index.at(tablename).begin(); index_data != index.at(tablename).end(); index_data++) {
        if (index_data->name == indexname) return true;
    }
    return false;
}

const vector<Index> &CatalogManager::getIndexInfo(const string &tablename) const {
    auto t = table.find(tablename);
    if (table.end() == t) throw MiniSQLException("Table Doesn't Exist!");
    return index.at(tablename);
}

void CatalogManager::addIndexInfo(const string &tablename, const string &indexname, int rank, const set<int> &key_positions) {
    if (findIndex(tablename, indexname)) throw MiniSQLException("Duplicate Index Name!");
    index[tablename].push_back({ indexname,rank,key_positions });
}

void CatalogManager::deleteIndexInfo(const string &tablename, const string &indexname) {
    if (index.end() == index.find(tablename)) throw MiniSQLException("Index Doesn't Exist!");
    for (auto index_data = index[tablename].begin(); index_data != index[tablename].end(); index_data++) {
        if (index_data->name == indexname) {
            index[tablename].erase(index_data);
            return;
        }
    }
    throw MiniSQLException("Index Eoesn't Exist!");
}

void CatalogManager::deleteIndexInfo(const string &tablename) {
    auto ind = index.find(tablename);
    if (index.end() == ind) throw MiniSQLException("Index Doesn't Exist!");
    index.erase(ind);
}