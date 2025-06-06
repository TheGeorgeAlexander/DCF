#ifndef VALUE_HPP
#define VALUE_HPP

#include <variant>


namespace dcf {

    enum class ValueType {
        STRING,
        BOOLEAN,
        INTEGER,
        DOUBLE,
        ARRAY,
        SECTION
    };

    class Value {
    public:
        Value(const Value& other);
        Value(const std::string &text);
        Value(bool boolean);
        Value(int64_t num_integer);
        Value(double num_double);
        Value(const std::vector<Value> &array);
        Value(const Section &section);

        Value& operator=(const Value& other);

        ValueType getType() const;

        const std::string& asString() const;
        bool asBool() const;
        int64_t asInt() const;
        double asDouble() const;
        const std::vector<Value>& asArray() const;
        const Section& asSection() const;

    private:
        ValueType type;
        std::variant<
            std::string,
            bool,
            int64_t,
            double,
            std::vector<Value>,
            std::shared_ptr<Section>
        > data;

        void checkType(ValueType expected) const;
    };
} // namespace dcf


inline dcf::Value::Value(const Value& other) 
    : type(other.type), data(other.data) { }

inline dcf::Value::Value(const std::string &text)
    : type(ValueType::STRING), data(text) { }

inline dcf::Value::Value(bool boolean)
    : type(ValueType::BOOLEAN), data(boolean) { }

inline dcf::Value::Value(int64_t num_integer)
    : type(ValueType::INTEGER), data(num_integer) { }

inline dcf::Value::Value(double num_double)
    : type(ValueType::DOUBLE), data(num_double) { }

inline dcf::Value::Value(const std::vector<Value> &array)
    : type(ValueType::ARRAY), data(array) { }

inline dcf::Value::Value(const dcf::Section &section)
    : type(ValueType::SECTION), data(std::make_shared<dcf::Section>(section)) { }


inline dcf::Value& dcf::Value::operator=(const Value& other) {
    if(this != &other) {
        type = other.type;
        data = other.data;
    }
    return *this;
}


inline dcf::ValueType dcf::Value::getType() const {
    return type;
}


inline const std::string& dcf::Value::asString() const {
    checkType(ValueType::STRING);
    return std::get<std::string>(data);
}


inline bool dcf::Value::asBool() const {
    checkType(ValueType::BOOLEAN);
    return std::get<bool>(data);
}


inline int64_t dcf::Value::asInt() const {
    checkType(ValueType::INTEGER);
    return std::get<int64_t>(data);
}


inline double dcf::Value::asDouble() const {
    checkType(ValueType::DOUBLE);
    return std::get<double>(data);
}


inline const std::vector<dcf::Value>& dcf::Value::asArray() const {
    checkType(ValueType::ARRAY);
    return std::get<std::vector<Value>>(data);
}


inline const dcf::Section& dcf::Value::asSection() const {
    checkType(ValueType::SECTION);
    return *std::get<std::shared_ptr<Section>>(data);
}


inline void dcf::Value::checkType(ValueType expected) const {
    if(type != expected) {
        throw std::runtime_error("Type mismatch");
    }
}

#endif

