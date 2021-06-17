#include <iostream>
#include "MiniSQLIndexManager.h"
using namespace std;

void IndexManager_test() {
    BufferManager BM;
    CatalogManager CM;
    IndexManager IM(&BM, &CM);
    cout << IM.findIndex<int>("abc", { "a" }) << endl;

    cout << IM.createIndex<int>("abc", { "a" }) << endl;
    /*cout << IM.createIndex<int>("abc", { "a" }) << endl;
    cout << IM.createIndex<int>("another database", { "key" }) << endl;

    cout << IM.findIndex<int>("abc", { "a" }) << endl;
    cout << IM.findIndex<int>("abc", { "b" }) << endl;
    cout << IM.findIndex<char>("abc", { "a" }) << endl;
    cout << IM.findIndex<int>("another database", { "key" }) << endl;

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