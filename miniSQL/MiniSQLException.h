#pragma once
#include <string>
using std::string;

enum class BPlusTreeException { DuplicateKey, KeyNotExist, IteratorIllegal, IteratorOverBounds };

class MiniSQLException {
public:
    MiniSQLException(const string &message = "Undefined Error!") : message(message) {}
    MiniSQLException(BPlusTreeException e) {
        if(e == BPlusTreeException::DuplicateKey) message = string("Duplicate Value on Unique or Primary Key!");
        else if(e == BPlusTreeException::KeyNotExist) message = string("Value Doesn't Exists!");
        else throw e;
    }
    string getMessage() { return message; }
private:
    string message;
};