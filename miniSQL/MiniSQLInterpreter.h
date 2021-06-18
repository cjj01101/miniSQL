#pragma once

#include "MiniSQLAPI.h"
#include <iostream>
#include <regex>
using namespace std;

#define IOBUFFER_CAPACITY 1000

string &trim(string &str) {
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
};

class InterpreterQuit {};

class Interpreter {
public:
    Interpreter(istream &in, ostream &out, API *core = nullptr) : in(in), out(out), core(core) {};
    void parse_input(const string &input);
    void parse_table_definition(string &content, smatch &result);
    void parse_insert_value(string &content, smatch &result);
    void parse_condition(string &content, smatch &result);
    void parse_attribute_sequence(string &content, smatch &result);

    void start();
private:
    istream &in;
    ostream &out;
    API *core;

    const regex create_table_pattern = regex("create table (\\w+)\\s?\\(([\\s\\S]+)\\)");
    const regex drop_table_pattern = regex("drop table (\\w+)\\s?");
    const regex create_index_pattern = regex("create index (\\w+) on (\\w+)\\s?\\(\\s?([^\\)]*)\\s?\\)");
    const regex drop_index_pattern = regex("drop index (\\w+) on (\\w+)");
    const regex insert_pattern = regex("insert into (\\w+) values\\s?\\(([^\\)]*)\\)");
    const regex select_pattern = regex("select * from (\\w+)(?: where ([\\s\\S]+))?");
    const regex delete_pattern = regex("delete from (\\w+)(?: where ([\\s\\S]+))?");
    const regex execfile_pattern = regex("execfile ([\\s\\S]+)");
    const regex quit_pattern = regex("quit");

    const regex attr_definition_pattern = regex("\\s?(\\w+) (int|float|char\\([0-9]+\\))( unique)?\\s?");
    const regex primary_key_definition_pattern = regex(",\\s?primary key\\s?\\(\\s?([^\\)]+)\\s?\\)\\s?");

    const regex integer_pattern = regex("([[:digit:]])");
    const regex float_pattern = regex("(\\d+(\\.\\d+)?)");
    const regex string_pattern = regex("(?:\"|')([\\s\\S]+)(?:\"|')");
    //const regex condition_pattern = regex("(\\w+)\\s?(=|<|>|<=|>=|<>)\\s?([[:digit:]]|\\d+(?:\\.\\d+)?|(?:\"|')[\\s\\S]+?(?:\"|'))(?: and ([\\s\\S]+))?");
    const regex condition_pattern = regex("(\\w+)\\s?(<=|>=|<>|=|<|>)\\s?([\\s\\S]+?)(?: and ([\\s\\S]+))?");
    //select * from t where a = 3 and b > 2.4 and d<>'adh' and f = "abd";
};