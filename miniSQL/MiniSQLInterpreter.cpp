#include "MiniSQLInterpreter.h"
#include <sstream>

void Interpreter::parse_table_definition(string &content, smatch &result) {
    bool has_primary_key = false;
    bool has_attr = false;

    //解析主键定义
    while (regex_search(content, result, primary_key_definition_pattern)) {
        if (has_primary_key) throw MiniSQLException("Duplicate Primary Key!");
        has_primary_key = true;
        cout << "[Primary Key Definition] " << result[1] << endl;
        content.erase(result.position(0), result.length(0));
    }

    //解析属性定义
    istringstream split_by_comma(content);
    string attr_def;
    while (getline(split_by_comma, attr_def, ',')) {
        if (regex_match(attr_def, result, attr_definition_pattern)) {
            cout << "[Attribute Definition] " << result[1] << ": " << result[2] << result[3] << endl;
            has_attr = true;
        }
        else throw MiniSQLException("Illegal Attribute Definition!");
    }
    if (!has_attr) throw MiniSQLException("Illegal Table Definition!");
}

void Interpreter::parse_insert_value(string &content, smatch &result) {
    istringstream split_by_comma(content);
    string attr_value;
    while (getline(split_by_comma, attr_value, ',')) {
        trim(attr_value);
        if (regex_match(attr_value, result, integer_pattern)) {
            cout << "[Integer Value] " << result[1] << endl;
        }
        else if (regex_match(attr_value, result, float_pattern)) {
            cout << "[Float Value] " << result[1] << endl;
        }
        else if (regex_match(attr_value, result, string_pattern)) {
            cout << "[String Value] " << result[1] << endl;
        }
        else throw MiniSQLException("Illegal Inserted Value!");
    }
}

void Interpreter::parse_condition(string &content, smatch &result) {
    while (regex_match(content, result, condition_pattern)) {
        string name = result[1];
        string op = result[2];
        string value = result[3];
        content = result[4];
        cout << "[Condition] " << name <<  op << value << endl;

        if (regex_match(value, result, integer_pattern)) {
            cout << "[Integer Value] " << result[1] << endl;
        }
        else if (regex_match(value, result, float_pattern)) {
            cout << "[Float Value] " << result[1] << endl;
        }
        else if (regex_match(value, result, string_pattern)) {
            cout << "[String Value] " << result[1] << endl;
        }
        else throw MiniSQLException("Illegal Compared Value!");
    }
}

void Interpreter::parse_input(const string &input) {
    string tablename;
    string indexname;
    string content;
    smatch result;

    if (regex_match(input, result, create_table_pattern)) {
        tablename = result[1];
        content = result[2];
        cout << "Match CREATE TABLE!" << endl << "[table name] " << tablename << endl << "[content] " << content << endl;
        parse_table_definition(content, result);
    }
    else if (regex_match(input, result, drop_table_pattern)) {
        tablename = result[1];
        cout << "Match DROP TABLE!" << endl << "[table name] " << tablename << endl;
    }
    else if (regex_match(input, result, create_index_pattern)) {
        tablename = result[1];
        indexname = result[2];
        content = result[3];
        cout << "Match CREATE INDEX!" << endl << "[table name] " << tablename << endl << "[index name] " << indexname << endl << "[content] " << content << endl;
    }
    else if (regex_match(input, result, drop_index_pattern)) {
        tablename = result[1];
        indexname = result[2];
        cout << "Match DROP INDEX!" << endl << "[table name] " << tablename << endl << "[index name] " << indexname << endl;
    }
    else if (regex_match(input, result, insert_pattern)) {
        tablename = result[1];
        content = result[2];
        cout << "Match INSERT!" << endl << "[table name] " << tablename << endl << "[content] " << content << endl;
        parse_insert_value(content, result);
    }
    else if (regex_match(input, result, select_pattern)) {
        tablename = result[2];
        content = result[3];
        trim(content);
        cout << "Match SELECT!" << endl << "[selected attrs] " << result[1] << endl << "[table name] " << tablename << endl << "[condition] " << content << endl;
        parse_condition(content, result);
    }
    else if (regex_match(input, result, delete_pattern)) {
        tablename = result[1];
        content = result[2];
        trim(content);
        cout << "Match DELETE!" << endl << "[table name] " << tablename << endl << "[condition] " << content << endl;
        parse_condition(content, result);
    }
    else if (regex_match(input, result, execfile_pattern)) {
        cout << "Match EXECFILE!" << endl << "[filename] " << result[1] << endl;
    }
    else if (regex_match(input, result, quit_pattern)) {
        throw InterpreterQuit();
    }
    else {
        throw MiniSQLException("Syntax Error!");
    }
}

void Interpreter::start() {
    char buffer[IOBUFFER_CAPACITY];
    string input;
    
    while (true) {
        in.getline(buffer, IOBUFFER_CAPACITY, ';');
        in.ignore(1);
        input = regex_replace(buffer, regex("\\s+"), " ");
        trim(input);
        out << input << endl;
        try {
            parse_input(input);
        } catch (InterpreterQuit) {
            break;
        } catch (MiniSQLException &e) {
            cout << e.getMessage() << endl;
        }
    }
}

void Interpreter_test() {
    Interpreter IO(cin, cout);
    IO.start();
}