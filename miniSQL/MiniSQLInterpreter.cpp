#include "MiniSQLInterpreter.h"
#include <sstream>

Type Interpreter::getType(const string &type_string) {
    Type type(BaseType::INT, 4);
    if (type_string == "float")  type.btype = BaseType::FLOAT;
    else if (type_string != "int") {
        type.btype = BaseType::CHAR;
        size_t left = type_string.find_first_of('(') + 1;
        size_t right = type_string.find_first_of(')');
        type.size = stoi(type_string.substr(left, right - left));
    }

    return type;
}

void Interpreter::parse_table_definition(const string &tablename, string &content, smatch &result) {
    std::set<string> primary_keys;
    std::set<string> attr_names;
    std::vector<Attr> attr_def;

    bool has_primary_key = false;
    bool has_attr = false;

    //解析主键定义
    while (regex_search(content, result, primary_key_definition_pattern)) {
        if (has_primary_key) throw MiniSQLException("Duplicate Primary Key!");
        has_primary_key = true;

        string primary_key_name = result[1];
        trim(primary_key_name);
        primary_keys.insert(primary_key_name);

        cout << "[Primary Key Definition] " << result[1] << endl;
        content.erase(result.position(0), result.length(0));
    }

    //解析属性定义
    istringstream split_by_comma(content);
    string attr;
    while (getline(split_by_comma, attr, ',')) {
        if (regex_match(attr, result, attr_definition_pattern)) {
            string name = result[1];
            Type type = getType(result[2]);
            bool unique = (result.length(3) != 0 || (primary_keys.find(name) != primary_keys.end()));

            if (attr_names.end() != attr_names.find(name)) throw MiniSQLException("Duplicate Attribute Name!");
            attr_names.insert(name);
            attr_def.push_back({ name, type, unique });
            
            cout << "[Attribute Definition] " << name << ": " << result[2] << result[3] << endl;
            has_attr = true;
        }
        else throw MiniSQLException("Illegal Attribute Definition!");
    }

    if (!has_attr) throw MiniSQLException("Illegal Table Definition!");
    if (!std::includes(attr_names.begin(), attr_names.end(), primary_keys.begin(), primary_keys.end())) throw MiniSQLException("Illegal Primary Key Definition!");

    core->createTable(tablename, attr_def, primary_keys);
}

void Interpreter::parse_insert_value(const string &tablename, string &content, smatch &result) {
    std::vector<Value> record;

    istringstream split_by_comma(content);
    string attr_value;
    while (getline(split_by_comma, attr_value, ',')) {
        trim(attr_value);
        if (regex_match(attr_value, result, integer_pattern)) {
            int value = stoi(result[1]);
            record.push_back(Value(Type(BaseType::INT, 4), &value));
            cout << "[Integer Value] " << record.back() << endl;
        }
        else if (regex_match(attr_value, result, float_pattern)) {
            float value = stof(result[1]);
            record.push_back(Value(Type(BaseType::FLOAT, 4), &value));
            cout << "[Float Value] " << record.back() << endl;
        }
        else if (regex_match(attr_value, result, string_pattern)) {
            string value = result[1];
            record.push_back(Value(Type(BaseType::CHAR, value.size() + 1), value.data()));
            cout << "[String Value] " << record.back() << endl;
        }
        else throw MiniSQLException("Illegal Inserted Value!");
    }
    core->insertIntoTable(tablename, record);
}

void Interpreter::parse_condition(string &content, smatch &result, Predicate &pred) {
    while (regex_match(content, result, condition_pattern)) {
        string name = result[1];
        string op = result[2];
        string compared_value = result[3];
        content = result[4];

        Compare comp;
        if (op == "<=") comp = Compare::LE;
        else if (op == ">=") comp = Compare::GE;
        else if (op == "<>") comp = Compare::NE;
        else if (op == "=") comp = Compare::EQ;
        else if (op == "<") comp = Compare::LT;
        else if (op == ">") comp = Compare::GT;
        else throw MiniSQLException("Illegal Comparation Identifier!");

        if (regex_match(compared_value, result, integer_pattern)) {
            int value = stoi(result[1]);
            pred[name].push_back({ comp, Value(Type(BaseType::INT, 4), &value) });
        }
        else if (regex_match(compared_value, result, float_pattern)) {
            float value = stof(result[1]);
            pred[name].push_back({ comp, Value(Type(BaseType::FLOAT, 4), &value) });
        }
        else if (regex_match(compared_value, result, string_pattern)) {
            string value = result[1];
            pred[name].push_back({ comp, Value(Type(BaseType::CHAR, value.size() + 1), value.data()) });
        }
        else throw MiniSQLException("Illegal Compared Value!");

        cout << "[Condition] " << name << op << pred[name].back().data << endl;
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
        parse_table_definition(tablename, content, result);
    }
    else if (regex_match(input, result, drop_table_pattern)) {
        tablename = result[1];
        cout << "Match DROP TABLE!" << endl << "[table name] " << tablename << endl;
        core->dropTable(tablename);
    }
    else if (regex_match(input, result, create_index_pattern)) {
        indexname = result[1];
        tablename = result[2];
        content = result[3];
        cout << "Match CREATE INDEX!" << endl << "[table name] " << tablename << endl << "[index name] " << indexname << endl << "[content] " << content << endl;
        core->createIndex(tablename, indexname, { content });
    }
    else if (regex_match(input, result, drop_index_pattern)) {
        indexname = result[1];
        tablename = result[2];
        cout << "Match DROP INDEX!" << endl << "[table name] " << tablename << endl << "[index name] " << indexname << endl;
        core->dropIndex(tablename, indexname);
    }
    else if (regex_match(input, result, insert_pattern)) {
        tablename = result[1];
        content = result[2];
        cout << "Match INSERT!" << endl << "[table name] " << tablename << endl << "[content] " << content << endl;
        parse_insert_value(tablename, content, result);
    }
    else if (regex_match(input, result, select_pattern)) {
        tablename = result[1];
        content = result[2];
        trim(content);
        cout << "Match SELECT!" << endl << "[table name] " << tablename << endl << "[conditions] " << content << endl;
        Predicate pred;
        parse_condition(content, result, pred);
        core->selectFromTable(tablename, pred);
    }
    else if (regex_match(input, result, delete_pattern)) {
        tablename = result[1];
        content = result[2];
        trim(content);
        cout << "Match DELETE!" << endl << "[table name] " << tablename << endl << "[conditions] " << content << endl;
        Predicate pred;
        parse_condition(content, result, pred);
        core->deleteFromTable(tablename, pred);
    }
    else if (regex_match(input, result, execfile_pattern)) {
        string filename = result[1];
        cout << "Match EXECFILE!" << endl << "[filename] " << result[1] << endl;
        std::ifstream inf(filename);
        if (!inf.is_open()) throw MiniSQLException("File Doesn't Exist!");
        Interpreter interp(core, inf, out);
        interp.start();
    }
    else if (regex_match(input, result, quit_pattern)) {
        throw InterpreterQuit();
    }
    else throw MiniSQLException("Syntax Error!");
}

void Interpreter::start() {
    char buffer[IOBUFFER_CAPACITY];
    string input;
    
    while (true) {
        if (in.eof()) break;
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
            cout << "--------------------------------" << endl;
            cout << "Error: " << e.getMessage() << endl;
            cout << "--------------------------------" << endl;
        }
    }
}

void Interpreter_test() {
    BufferManager BM;
    CatalogManager CM(META_TABLE_FILE_PATH, META_INDEX_FILE_PATH);
    RecordManager RM(&BM);
    IndexManager IM(&BM);
    API core(&CM, &RM, &IM);

    Interpreter IO(&core, cin, cout);
    IO.start();
}