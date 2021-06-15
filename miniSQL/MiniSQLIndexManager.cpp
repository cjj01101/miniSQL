#include <iostream>
#include "MiniSQLIndexManager.h"
using namespace std;

void IndexManager_test() {
    BufferManager BM;
    CatalogManager CM;
    IndexManager IM(&BM, &CM);
    /*cout << IM.findIndex<tuple<int, char>>("abc", { "a" }) << endl;

    cout << IM.createIndex<tuple<int, char>>("abc", { "a" }) << endl;
    cout << IM.createIndex<tuple<int, char>>("abc", { "a" }) << endl;
    cout << IM.createIndex<tuple<int, char>>("another database", { "key" }) << endl;

    cout << IM.findIndex<tuple<int, char>>("abc", { "a" }) << endl;
    cout << IM.findIndex<tuple<int, char>>("abc", { "b" }) << endl;
    cout << IM.findIndex<tuple<int, int>>("abc", { "a" }) << endl;
    cout << IM.findIndex<tuple<int, char>>("another database", { "key" }) << endl;

    cout << IM.createIndex<tuple<int, char>>("abc", { "b" }) << endl;
    cout << IM.findIndex<tuple<int, char>>("abc", { "b" }) << endl;

    cout << IM.dropIndex<tuple<int, int>>("abc", { "a" }) << endl;
    cout << IM.dropIndex<tuple<int, char>>("abc", { "a" }) << endl;
    cout << IM.dropIndex<tuple<int, char>>("abc", { "c" }) << endl;

    cout << IM.findIndex<tuple<int, char>>("abc", { "a" }) << endl;
    cout << IM.findIndex<tuple<int, char>>("abc", { "b" }) << endl;

    cout << IM.dropIndex<tuple<int, char>>("abc", { "b" }) << endl;
    cout << IM.findIndex<tuple<int, char>>("abc", { "b" }) << endl;*/
}