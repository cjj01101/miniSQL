#include <iostream>
#include "BPlusTree.h"
using namespace std;

void BPlusTree_test() {
    BufferManager BM;
    BPlusTree<int, int, 5> BPT(&BM, "../test.index");
    const int insert_list[] = { 2,4,3,1,5,0,6,-1,7,8,6,2,30,40,25,24,15,16,14,13,11,12,35,9,10 };
    for (auto i : insert_list) {
        try {
            BPT.insertData(i, i);
            BPT.print();
        }
        catch (BPlusTreeException &e) {
            if (e == BPlusTreeException::DuplicateKey) cout << i << " is Duplicate Key!\n";
        }
        catch (...) {
            cout << "Error";
        }
        cout << "----------------\n";
    }

    const int remove_list[] = { 3,2,1,7,8,9,5,6,4,10,12,11,13,60,25,30,24,16,35,-1,40,14,15,0 };

    for (auto i : remove_list) {
        if (BPT.findData(i)) cout << i << "exists." << endl;
        else cout << i << "doesn't exist." << endl;
    }

    for (auto i : remove_list) {
        try {
            BPT.removeData(i);
            BPT.print();
        }
        catch (BPlusTreeException &e) {
            if (e == BPlusTreeException::KeyNotExist) cout << i << " Not Found!\n";
        }
        cout << "----------------\n";
    }
}