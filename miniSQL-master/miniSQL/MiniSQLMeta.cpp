#include "MiniSQLMeta.h"
#include <iostream>

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