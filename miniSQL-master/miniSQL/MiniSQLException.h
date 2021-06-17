#pragma once
#include <string>
using std::string;

class MiniSQLException {
public:
    MiniSQLException(string message = "Undefined Error!"):message(message) {}
    string getMessage() { return message; }
private:
    const string message;
};