#include "MiniSQLMeta.h"
#include <iostream>

Value::Value(Type type, void *data) : type(type) {
    this->data = new char[type.size];
    memcpy_s(this->data, type.size, data, type.size);
}

Value::Value(const Value &rhs) : type(rhs.type) {
    data = new char[type.size];
    memcpy_s(data, type.size, rhs.data, type.size);
}

Value::~Value() {
    delete[](char*)data;
}

void Meta_test() {
    int a = 3;
    Value v1(Type(BaseType::INT, sizeof(int)), &a);

    float b = 2.4l;
    Value v2(Type(BaseType::FLOAT, sizeof(float)), &b);

    Value v3(v1);
    /*Value<char, 2> value1("ab");
    Value<char, 2> value2("a");
    std::cout << TypeCompatible<decltype(value1), decltype(value2)>::value << std::endl << value1.value << std::endl << value2.value;*/
}