#pragma once

#include "MiniSQLException.h"
#include <map>
#include <string>

/*                                          */
/*                                          */
/*                基本类型                  */
/*                                          */
/*                                          */

enum class BaseType {
    CHAR, INT, FLOAT
};

struct Type {
    Type(BaseType btype, size_t size) : btype(btype), size(size) {}
    BaseType btype;
    size_t size;
};

struct Value {
    Value(Type type, void *data);
    Value(const Value &rhs);
    ~Value();

    Type type;
    void *data;
};

/*                                          */
/*                                          */
/*                操作类型                  */
/*                                          */
/*                                          */

enum class Compare {
    EQ, LE, GE, NE, LT, GT
};

struct Condition {
    Compare comp;
    Value data;
};
using Predicate = std::map<string, Condition>;

enum class Op {
    CREATE_TABLE, DROP_TABLE, CREATE_INDEX, DROP_INDEX, SELECT, REMOVE, INSERT
};

struct operation {
    Op op;
    string table;
    Predicate pred;
};

/*class data_wrapper {
public:
    template<typename T>
    data_wrapper(T data, size_t countOfElements = 1);
    data_wrapper(const data_wrapper &rhs) : leaf(rhs.leaf ? rhs.leaf->clone() : nullptr) {}
    ~data_wrapper() { delete leaf; }
private:
    class data_holder_interface {
    public:
        virtual data_holder_interface *clone() const = 0;
    };

    template <typename T>
    class data_holder : public data_holder_interface {
    public:
        data_holder(T data, int size) : type(size), data(data) {};
        virtual data_holder_interface *clone() const override {
            return new data_holder<T, countOfElements>(data));
        };
    private:
        Type type;
        T data;
    };

    data_holder_interface *leaf;
};

template<typename T>
data_wrapper::data_wrapper(T data, size_t countOfElements) {
    if (std::is_same<T, int>::value) leaf = new data_holder<int>(data, 1);
    else if (std::is_same<T, float>::value) leaf = new data_holder<float>(data, 1);
    else if (std::is_same<T, char*>::value) leaf = new data_holder<char>(data, countOfElements);
}

template<typename T1, typename T2>
struct TypeCompatible {};

template<typename T1, size_t count1, typename T2, size_t count2>
struct TypeCompatible<Value<T1, count1>, Value<T2, count2>> {
    const static bool value = std::is_same<T1, T2>::value && (count1 == count2 || (std::is_same<T1, char>::value && count1 < count2));
};*/
