#pragma once

#include "MiniSQLAPI.h"
#include <iostream>
#include <regex>
using namespace std;

#define IOBUFFER_CAPACITY 1000

class Interpreter {
public:
    Interpreter(istream &in, ostream &out, API *core = nullptr) : in(in), out(out), core(core) {};
    void start();
private:
    istream &in;
    ostream &out;
    API *core;
};