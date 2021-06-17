#include <iostream>
#include "BPlusTree.h"
#include "MiniSQLIndexManager.h"
using namespace std;

extern void Meta_test();
extern void BufferManager_test();
extern void BPlusTree_test();
extern void IndexManager_test();
extern void API_test();

int main()
{
    //Meta_test();
    //BPlusTree_test();
    //IndexManager_test();
    //BufferManager_test();
    API_test();
}