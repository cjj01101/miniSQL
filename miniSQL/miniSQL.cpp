#include <iostream>
#include "BPlusTree.h"

int main()
{
    std::cout << "Hello World!\n";
    BPlusTree<int, int, 4> BPT;
    BPT.insertData(2, 2);
    BPT.insertData(4, 4);
    BPT.insertData(3, 3);
    BPT.insertData(1, 1);
    BPT.insertData(5, 5);
    BPT.insertData(0, 0);
    BPT.insertData(6, 6);
    BPT.insertData(-1, -1);
    BPT.insertData(7, 7);
    BPT.insertData(8, 8);
    BPT.insertData(6, 6);
    BPT.insertData(6, 6);
    BPT.insertData(9, 9);
    /*BPT.insertData(10, 10);*/
    BPT.print();
}
