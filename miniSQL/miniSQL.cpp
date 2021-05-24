#include <iostream>
#include "BPlusTree.h"
#include "MiniSQLIndexManager.h"
using namespace std;

extern void BPlusTree_test();
extern void IndexManager_test();

int main()
{
    //BPlusTree_test();

    IndexManager_test();
}