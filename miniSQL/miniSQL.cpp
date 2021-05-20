#include <iostream>
#include "BPlusTree.h"

using namespace std;

int main()
{
    //const int insert_list[] = { 2,4,3,1,5,0,6,-1,7,8,30,40,25,24,15,16,14,13,9,12,23,22,21,20 };
    const int insert_list[] = { 2,4,3,1,5,0,6,-1,7,8,6,7,8,3,3,3,30,40,9,25,40,9,2 };
    BPlusTree<int, int, 3> BPT;
    for (auto i : insert_list) {
        try {
            BPT.insertData(i, i);
            BPT.print();
        } catch (BPlusTreeException &e) {
            if (e == BPlusTreeException::DuplicateKey) cout << i << "is Duplicate Key!\n";
        }
        cout << "----------------\n";
    }
    for (int i = -2; i < 50; i++) cout << BPT.findData(i);
}
