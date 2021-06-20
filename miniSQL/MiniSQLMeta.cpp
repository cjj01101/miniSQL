#include "MiniSQLMeta.h"
#include <iostream>

Value::Value(Type type, const void *data) : type(type) {
    if (data == nullptr) throw MiniSQLException("NULL Value!");
    this->data = new char[type.size];
    memcpy_s(this->data, type.size, data, type.size);
}

Value::Value(const Value &rhs) : type(rhs.type) {
    if (data == nullptr) throw MiniSQLException("NULL Value!");
    data = new char[type.size];
    memcpy_s(data, type.size, rhs.data, type.size);
}

template<typename T>
typename std::enable_if<std::is_pointer<T>::value, T>::type Value::translate() const {
    if (std::is_same<T, char*>::value) return reinterpret_cast<T>(data);
    else throw MiniSQLException("Type Unsupported!");
}

template<typename T>
typename std::enable_if<!std::is_pointer<T>::value, T>::type Value::translate() const {
    if (std::is_same<T, int>::value || std::is_same<T, float>::value) return *reinterpret_cast<T*>(data);
    else throw MiniSQLException("Type Unsupported!");
}

void Value::convertTo(const Type &rtype) {
    if (type.btype == rtype.btype) {
        if (type.btype == BaseType::CHAR) {
            if (type.size > rtype.size) throw MiniSQLException("Type Incompatible!");
            char *new_data = new char[rtype.size];
            memcpy_s(new_data, rtype.size, data, type.size);
            delete data;
            data = new_data;
            type.size = rtype.size;
        }
    }
    else if (type.btype == BaseType::INT && rtype.btype == BaseType::FLOAT) {
        float new_data = (float)translate<int>();
        memcpy_s(data, type.size, &new_data, sizeof(float));
        type.btype = BaseType::FLOAT;
    }
    else throw MiniSQLException("Type Incompatible!");
}

bool Value::operator==(const Value &rhs) const {
    if (type.btype == BaseType::CHAR && rhs.type.btype == BaseType::CHAR) return strcmp(this->translate<char*>(), rhs.translate<char*>()) == 0;
    if (type.btype == BaseType::CHAR || rhs.type.btype == BaseType::CHAR) throw MiniSQLException("Comparation unsupported!");
    if (type.btype == BaseType::INT && rhs.type.btype == BaseType::INT) return (translate<int>() == rhs.translate<int>());
    float lvalue = (type.btype == BaseType::INT) ? (float)(translate<int>()) : translate<float>();
    float rvalue = (rhs.type.btype == BaseType::INT) ? (float)(rhs.translate<int>()) : rhs.translate<float>();
    return lvalue == rvalue;
}

bool Value::operator<(const Value &rhs) const {
    if (type.btype == BaseType::CHAR && rhs.type.btype == BaseType::CHAR) return strcmp(this->translate<char*>(), rhs.translate<char*>()) < 0;
    if (type.btype == BaseType::CHAR || rhs.type.btype == BaseType::CHAR) throw MiniSQLException("Comparation unsupported!");
    if (type.btype == BaseType::INT && rhs.type.btype == BaseType::INT) return (translate<int>() < rhs.translate<int>());
    float lvalue = (type.btype == BaseType::INT) ? (float)(translate<int>()) : translate<float>();
    float rvalue = (rhs.type.btype == BaseType::INT) ? (float)(rhs.translate<int>()) : rhs.translate<float>();
    return lvalue < rvalue;
}

std::ostream &operator<<(std::ostream &os, Value value) {
    if (value.type.btype == BaseType::INT) {
        os << value.translate<int>();
    }
    else if (value.type.btype == BaseType::FLOAT) {
        os << value.translate<float>();
    }
    else if (value.type.btype == BaseType::CHAR) {
        os << value.translate<char*>();
    }
    return os;
}

void Meta_test() {
    int a = 3;
    Value v1(Type(BaseType::INT, sizeof(int)), &a);

    float b = 2.4l;
    Value v2(Type(BaseType::FLOAT, sizeof(float)), &b);

    Value v3(v1);

    Value v4(Type(BaseType::CHAR, 5), "abc");

    char* head = v4.translate<char*>();

    std::cout << head << std::endl;
    std::cout << (v1 == v3) << (v1 < v2) << (v1 <= v2) << (v1 >= v2) << (v1 > v2) << (v1 <= v3) << (v1 >= v3) << std::endl;
    /*Value<char, 2> value1("ab");
    Value<char, 2> value2("a");
    std::cout << TypeCompatible<decltype(value1), decltype(value2)>::value << std::endl << value1.value << std::endl << value2.value;*/

    Type t1(BaseType::INT, 4);
    Type t2(BaseType::CHAR, 5);
    std::cout << (t1 == t2) << (t1 != t2) << std::endl;

    Record rec = { v1, v2, v3, v4 };
    for (auto i : rec) {
        std::cout << i << " ";
    }
}