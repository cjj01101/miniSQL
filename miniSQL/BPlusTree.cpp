#include <iostream>
#include <sstream>
#include "BPlusTree.h"
using namespace std;

void BPlusTree_test() {
    BufferManager BM;
    BPlusTree<int, int> BPT(&BM, "../test.index", 3);
    const int insert_list[] = { 2,4,3,1,5,0,6,-1,7,8,6,2,30,40,25,24,15,16,14,13,11,12,35,9,10,37,31,27 };
    //const FLString insert_list[] = { "a","adf","poe","geofiur","asdf","tind","mank","qe","dfo","grad","excl","qpeoi","dflkjq","uit" };
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

    const int remove_list[] = { 3,2,1,7,8,9,5,6,4,10,27,12,11,37,13,60,25,30,31,24,16,35,-1,40,14,15,0 };
    //const FLString remove_list[] = { "qe","dfo","grad","a","adf","mank","excl","qpeoi","poe","geofiur","asdf","tind","red","adf","mank","dflkjq","uit" };

    for (auto i : remove_list) {
        if (BPT.checkData(i)) cout << i << " exists." << endl;
        else cout << i << " doesn't exist." << endl;
    }

    BPT.print();
    auto it_2 = BPT.getStart(20, false);
    if (it_2 != BPT.end()) cout << (*it_2).first << endl;

    for (auto it = BPT.begin(); it != BPT.end(); it.next()) {
        cout << (*it).first << "->";
    }

    /*for (auto i : remove_list) {
        try {
            BPT.removeData(i);
            BPT.print();
        }
        catch (BPlusTreeException &e) {
            if (e == BPlusTreeException::KeyNotExist) cout << i << " Not Found!\n";
        }
        cout << "----------------\n";
    }*/
}