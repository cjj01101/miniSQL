#include <iostream>
#include "MiniSQLIndexManager.h"

void IndexManager_test() {
    BufferManager BM;
    CatalogManager CM(META_TABLE_FILE_PATH, META_INDEX_FILE_PATH);
    IndexManager IM(&BM);
    /*IM.createIndex<int>("table1", "index1", 200);
    cout << IM.createIndex<int>("abc", { "a" }) << endl;
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