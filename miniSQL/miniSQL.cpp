#include <iostream>
#include "BPlusTree.h"
#include "MiniSQLIndexManager.h"
using namespace std;

extern void BufferManager_test();
extern void BPlusTree_test();
extern void IndexManager_test();
extern void Meta_test();

int main()
{
    BPlusTree_test();
    //IndexManager_test();
    //BufferManager_test();
}