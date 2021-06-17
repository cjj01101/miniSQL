#include "MiniSQLInterpreter.h"

void Interpreter::start() {
    const regex create_table_pattern("\\s?create table (\\w+)\\s?\\{([^\\}]*)\\}\\s?");
    const regex drop_table_pattern("\\s?drop table (\\w+)\\s?");
    const regex create_index_pattern("\\s?create index (\\w+) on (\\w+)\\s?\\(([^\\)]*)\\)\\s?");
    const regex drop_index_pattern("\\s?drop index (\\w+) on (\\w+)\\s?");
    const regex insert_pattern("\\s?insert into (\\w+) values\\s?\\(([^\\)]*)\\)\\s?");
    const regex select_pattern("\\s?select ([\\s\\S]+) from (\\w+) (?:where ([\\s\\S]+))?\\s?");
    const regex delete_pattern("\\s?delete from (\\w+) (?:where ([\\s\\S]+))?\\s?");
    const regex execfile_pattern("\\s?execfile ([\\s\\S]+)\\s?");
    const regex quit_pattern("\\s?quit\\s?");
    smatch result;
    char buffer[IOBUFFER_CAPACITY];
    string input;
    
    while (true) {
        in.getline(buffer, IOBUFFER_CAPACITY, ';');
        in.ignore(1);
        input = regex_replace(buffer, regex("\\s+"), " ");
        out << input << endl;
        if (regex_match(input, result, create_table_pattern)) {
            cout << "Match CREATE TABLE!" << endl << "table name: " << result[1] << endl << "content: " << result[2] << endl;
        }
        else if (regex_match(input, result, drop_table_pattern)) {
            cout << "Match DROP TABLE!" << endl << "table name: " << result[1] << endl;
        }
        else if (regex_match(input, result, create_index_pattern)) {
            cout << "Match CREATE INDEX!" << endl << "table name: " << result[1] << endl << "index name: " << result[2] <<endl << "content: " << result[3] << endl;
        }
        else if (regex_match(input, result, drop_index_pattern)) {
            cout << "Match DROP INDEX!" << endl << "table name: " << result[1] << endl << "index name: " << result[2] << endl;
        }
        else if (regex_match(input, result, insert_pattern)) {
            cout << "Match INSERT!" << endl << "table name: " << result[1] << endl << "content: " << result[2] << endl;
        }
        else if (regex_match(input, result, select_pattern)) {
            cout << "Match SELECT!" << endl << "select attr: " << result[1] << endl << "table name: " << result[2] << endl << "condition: " << result[3] << endl;
        }
        else if (regex_match(input, result, delete_pattern)) {
            cout << "Match DELETE!" << endl << "table name: " << result[1] << endl << "condition: " << result[2] << endl;
        }
        else if (regex_match(input, result, execfile_pattern)) {
            cout << "Match EXECFILE!" << endl << "filename: " << result[1] << endl;
        }
        else if (regex_match(input, result, quit_pattern)) {
            cout << "Match QUIT!" << endl;
            break;
        }
    }
}

void Interpreter_test() {
    Interpreter IO(cin, cout);
    IO.start();
}