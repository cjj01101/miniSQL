#include <iostream>
#include "BPlusTree.h"

using namespace std;

int main()
{
    BPlusTree<int, int, 4> BPT;

    const int insert_list[] = { 2,4,3,1,5,0,6,-1,7,8,6,2,30,40,25,24,15,16,14,13,11,12,35,9,10 };
    for (auto i : insert_list) {
        try {
            BPT.insertData(i, i);
            BPT.print();
        } catch (BPlusTreeException &e) {
            if (e == BPlusTreeException::DuplicateKey) cout << i << " is Duplicate Key!\n";
        }
        cout << "----------------\n";
    }
    
    const int remove_list[] = { 3,2,1,7,8,9,5,6,4,10,12,11,13,25,30,24,16,35,-1,40,14,15,0 };
    for (auto i : remove_list) {
        try {
            BPT.removeData(i);
            BPT.print();
        } catch (BPlusTreeException &e) {
            if (e == BPlusTreeException::KeyNotExist) cout << i << " Not Found!\n";
        }
        cout << "----------------\n";
    }

    for (auto i : insert_list) {
        try {
            BPT.insertData(i, i);
            BPT.print();
        }
        catch (BPlusTreeException &e) {
            if (e == BPlusTreeException::DuplicateKey) cout << i << " is Duplicate Key!\n";
        }
        cout << "----------------\n";
    }
}
