#include <iostream>
#include "MiniSQLIndexManager.h"

bool IndexManager::findIndex(const string &table, const string &name) const {
    index_file &index = metadata->getIndexFile();
    if (index.end() == index.find(table)) return false;
    for (auto index_data = index[table].begin(); index_data != index[table].end(); index_data++) {
        if (index_data->name == name) return true;
    }
    return false;
}

bool IndexManager::dropIndex(const string &name) {
    index_file &index = metadata->getIndexFile();
    for (auto &table_indexes : index) {
        auto &index_vector = table_indexes.second;
        for (auto index_data = index_vector.begin(); index_data != index_vector.end(); index_data++) {
            if (index_data->name == name) {
                table_indexes.second.erase(index_data);
                return true;
            }
        }
    }
    return false;
}

void IndexManager_test() {
    BufferManager BM;
    CatalogManager CM;
    IndexManager IM(&BM, &CM);
    cout << IM.findIndex("table1", "index1") << endl;

    cout << IM.createIndex<int>("table1", "index1", { "attr1" }) << endl;
    cout << IM.dropIndex("index1");
    /*cout << IM.createIndex<int>("abc", { "a" }) << endl;
    cout << IM.createIndex<int>("another table", { "key" }) << endl;

    cout << IM.findIndex<int>("abc", { "a" }) << endl;
    cout << IM.findIndex<int>("abc", { "b" }) << endl;
    cout << IM.findIndex<char>("abc", { "a" }) << endl;
    cout << IM.findIndex<int>("another table", { "key" }) << endl;

    cout << IM.createIndex<int>("abc", { "b" }) << endl;
    cout << IM.findIndex<int>("abc", { "b" }) << endl;

    cout << IM.dropIndex<int>("abc", { "a" }) << endl;
    cout << IM.dropIndex<char>("abc", { "a" }) << endl;
    cout << IM.dropIndex<int>("abc", { "c" }) << endl;

    cout << IM.findIndex<int>("abc", { "a" }) << endl;
    cout << IM.findIndex<int>("abc", { "b" }) << endl;

    cout << IM.dropIndex<int>("abc", { "b" }) << endl;
    cout << IM.findIndex<int>("abc", { "b" }) << endl;*/
}