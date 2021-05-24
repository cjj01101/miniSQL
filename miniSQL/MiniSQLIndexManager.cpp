#include <iostream>
#include "MiniSQLIndexManager.h"
using namespace std;

void IndexManager_test() {
    IndexManager IM;
    cout << IM.findIndex<int, char>("abc", { "a" }) << endl;

    cout << IM.createIndex<int, char>("abc", { "a" }) << endl;
    cout << IM.createIndex<int, char>("abc", { "a" }) << endl;
    cout << IM.createIndex<int, char>("another database", { "key" }) << endl;

    cout << IM.findIndex<int, char>("abc", { "a" }) << endl;
    cout << IM.findIndex<int, char>("abc", { "b" }) << endl;
    cout << IM.findIndex<int, int>("abc", { "a" }) << endl;
    cout << IM.findIndex<int, char>("another database", { "key" }) << endl;
    cout << IM.findIndex<int, char>("another database", { "key" }) << endl;

    cout << IM.createIndex<int, char>("abc", { "b" }) << endl;
    cout << IM.findIndex<int, char>("abc", { "b" }) << endl;

    cout << IM.dropIndex<int, int>("abc", { "a" }) << endl;
    cout << IM.dropIndex<int, char>("abc", { "a" }) << endl;
    cout << IM.dropIndex<int, char>("abc", { "c" }) << endl;

    cout << IM.findIndex<int, char>("abc", { "a" }) << endl;
    cout << IM.findIndex<int, char>("abc", { "b" }) << endl;

    cout << IM.dropIndex<int, char>("abc", { "b" }) << endl;
    cout << IM.findIndex<int, char>("abc", { "b" }) << endl;
}