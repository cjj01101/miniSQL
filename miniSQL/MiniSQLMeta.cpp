#include "MiniSQLMeta.h"
#include <iostream>

Value::Value(Type type, const void *data) : type(type) {
    this->data = new char[type.size];
    memcpy_s(this->data, type.size, data, type.size);
}

Value::Value(const Value &rhs) : type(rhs.type) {
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

bool Value::operator==(const Value &rhs) const {
    if (type.btype == BaseType::CHAR && rhs.type.btype == BaseType::CHAR) return strcmp(this->translate<char*>(), rhs.translate<char*>()) == 0;
    if (type.btype == BaseType::CHAR || rhs.type.btype == BaseType::CHAR) throw MiniSQLException("Comparation unsupported!");
    float lvalue = (type.btype == BaseType::INT) ? (float)(this->translate<int>()) : this->translate<float>();
    float rvalue = (rhs.type.btype == BaseType::INT) ? (float)(rhs.translate<int>()) : rhs.translate<float>();
    return lvalue == rvalue;
}

bool Value::operator<(const Value &rhs) const {
    if (type.btype == BaseType::CHAR && rhs.type.btype == BaseType::CHAR) return strcmp(this->translate<char*>(), rhs.translate<char*>()) < 0;
    if (type.btype == BaseType::CHAR || rhs.type.btype == BaseType::CHAR) throw MiniSQLException("Comparation unsupported!");
    float lvalue = (type.btype == BaseType::INT) ? (float)(this->translate<int>()) : this->translate<float>();
    float rvalue = (rhs.type.btype == BaseType::INT) ? (float)(rhs.translate<int>()) : rhs.translate<float>();
    return lvalue < rvalue;
}

void Meta_test() {
    int a = 3;
    Value v1(Type(BaseType::INT, sizeof(int)), &a);

    float b = 2.4l;
    Value v2(Type(BaseType::FLOAT, sizeof(float)), &b);

    Value v3(v1);

    Value v4(Type(BaseType::CHAR, 4), "abc");

    char* head = v4.translate<char*>();

    std::cout << head << std::endl;
    std::cout << (v1 == v3) << (v1 < v2) << (v1 <= v2) << (v1 >= v2) << (v1 > v2) << (v1 <= v3) << (v1 >= v3);
    /*Value<char, 2> value1("ab");
    Value<char, 2> value2("a");
    std::cout << TypeCompatible<decltype(value1), decltype(value2)>::value << std::endl << value1.value << std::endl << value2.value;*/
}