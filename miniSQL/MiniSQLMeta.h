#pragma once
#include <string>

enum class BaseType {
    INT, CHAR, FLOAT
};

struct Type {
    BaseType type;
    size_t size;
};

class data_wrapper {
public:
    template <typename T>
    data_wrapper(T data) : content(new data_holder<T>(data)) {}
    data_wrapper(const data_wrapper &rhs) : content(rhs.content ? rhs.content->clone() : nullptr) {}

    ~data_wrapper() { delete content; }
private:
    class data_holder_interface {
    public:
        virtual const std::string &getType() const = 0;
        virtual data_holder_interface *clone() const = 0;
    };

    template <typename T>
    class data_holder : public data_holder_interface {
    public:
        using type = T;

        data_holder(T data) : data(data) {};
        virtual const std::string &getType() const override {
            return typeid(T).name();
        };
        virtual data_holder_interface *clone() const override {
            return new data_holder<T>(*content);
        };
    private:
        T data;
    };

    data_holder_interface *content;
};
