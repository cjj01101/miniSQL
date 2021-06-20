#pragma once

#include "MiniSQLException.h"
#include <set>
#include <map>
#include <vector>
#include <string>
#include <iostream>

#define META_TABLE_FILE_PATH "../META_TABLE.table"
#define META_INDEX_FILE_PATH "../META_INDEX.table"

/*                                          */
/*                                          */
/*                基本类型                  */
/*                                          */
/*                                          */

enum class BaseType {
    CHAR = 0, INT, FLOAT
};

struct Type {
    Type() : btype(BaseType::CHAR), size(0) {}
    Type(BaseType btype, size_t size) : btype(btype), size(btype == BaseType::CHAR ? size : 4) {}
    BaseType btype;
    size_t size;

    bool operator==(const Type &rhs) { return (btype == rhs.btype && size == rhs.size); }
    bool operator!=(const Type &rhs) { return !(*this == rhs); }
    friend std::ostream &operator<<(std::ostream &os, Type type) {
        os << (int)type.btype << " " << type.size;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, Type &type) {
        int type_id;
        is >> type_id >> type.size;
        type.btype = static_cast<BaseType>(type_id);
        return is;
    }
};

struct Value {
    Value(Type type, const void *data);
    Value(const Value &rhs);
    ~Value() { delete[](char*)data; };

    template<typename T>
    typename std::enable_if<std::is_pointer<T>::value, T>::type translate() const;

    template<typename T>
    typename std::enable_if<!std::is_pointer<T>::value, T>::type translate() const;

    void convertTo(const Type &rtype);

    bool operator==(const Value &rhs) const;
    bool operator!=(const Value &rhs) const { return !(*this == rhs); }
    bool operator<(const Value &rhs) const;
    bool operator>(const Value &rhs) const { return (rhs < *this); }
    bool operator<=(const Value &rhs) const { return !(*this > rhs); }
    bool operator>=(const Value &rhs) const { return !(*this < rhs); }

    Type type;
    void *data;

    friend std::ostream &operator<<(std::ostream &os, Value value);
};

using Record = std::vector<Value>;

typedef struct {
    string filename;
    int block_id;
    int offset;
} Position;

typedef struct {
    Position pos;
    Record content;
} RecordInfo;

using ReturnTable = std::vector<RecordInfo>;

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
using Predicate = std::map<std::string, std::vector<Condition>>;

/*
enum class Op {
    CREATE_TABLE, DROP_TABLE, CREATE_INDEX, DROP_INDEX, SELECT, DELETE, INSERT, EXECFILE, QUIT
};

struct Operation {
    Op op;
    string table;
    string index;
    Predicate pred;
};

class data_wrapper {
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
