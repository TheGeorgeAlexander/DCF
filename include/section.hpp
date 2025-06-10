#ifndef SECTION_HPP
#define SECTION_HPP

#include "value.hpp"
#include <optional>
#include <unordered_map>
#include <iomanip>


namespace dcf {
    class Section {
    public:
        Section();
        Section(const Section& other);
        Section& operator=(const Section& other);

        Value get(const std::string &key) const;
        std::optional<Value> optionalGet(const std::string &key) const;

        void set(const std::string &key, const Value &value);
        void set(const std::string &key, const std::string &value);
        void set(const std::string &key, bool value);
        void set(const std::string &key, int64_t value);
        void set(const std::string &key, double value);
        void set(const std::string &key, const std::vector<Value> &value);
        void set(const std::string &key, const Section &value);

        void remove(const std::string &key);
        std::vector<std::string> keys() const;

        void setHeader(const std::string &key, const std::string &header);
        std::string getHeader(const std::string &key) const;

        std::string toString(int indent = 4) const;

    private:
        std::vector<std::string> keyOrder;
        std::unordered_map<std::string, std::pair<std::string, Value>> map;

        void trim(std::string &text) const;
        void indentWithComments(std::string &text, const std::string &spacePrefix) const;
        std::string toArrayString(const Value &value, int indent, int depth) const;
        std::string valueString(const Value &value, int indent, int depth) const;
        std::string toString(int indent, int depth) const;
    };
} // namespace dcf



inline dcf::Section::Section() { }

inline dcf::Section::Section(const Section& other)
    : keyOrder(other.keyOrder), map(other.map) { }


inline dcf::Section& dcf::Section::operator=(const Section& other) {
    if(this != &other) {
        keyOrder = other.keyOrder;
        map = other.map;
    }
    return *this;
}


inline dcf::Value dcf::Section::get(const std::string &key) const {
    auto it = map.find(key);
    if(it == map.end()) {
        throw std::runtime_error("Key not found: " + key);
    }
    return it->second.second;
}


inline std::optional<dcf::Value> dcf::Section::optionalGet(const std::string &key) const {
    auto it = map.find(key);
    if(it == map.end()) {
        return std::nullopt;
    }
    return it->second.second;
}


inline void dcf::Section::set(const std::string &key, const dcf::Value &value) {
    auto it = map.find(key);
    if(it == map.end()) {
        keyOrder.push_back(key);
        map.insert({key, {"", value}});
    } else {
        it->second.second = value;
    }
}


inline void dcf::Section::set(const std::string &key, const std::string &value) {
    set(key, Value(value));
}


inline void dcf::Section::set(const std::string &key, bool value) {
    set(key, Value(value));
}


inline void dcf::Section::set(const std::string &key, int64_t value) {
    set(key, Value(value));
}


inline void dcf::Section::set(const std::string &key, double value) {
    set(key, Value(value));
}


inline void dcf::Section::set(const std::string &key, const std::vector<dcf::Value> &value) {
    set(key, Value(value));
}


inline void dcf::Section::set(const std::string &key, const dcf::Section &value) {
    set(key, Value(value));
}


inline void dcf::Section::remove(const std::string &key) {
    keyOrder.erase(std::find(keyOrder.begin(), keyOrder.end(), key));
    map.erase(key);
}


inline std::vector<std::string> dcf::Section::keys() const {
    return keyOrder;
}


inline void dcf::Section::setHeader(const std::string &key, const std::string &header) {
    auto it = map.find(key);
    if(it == map.end()) {
        throw std::runtime_error("Key not found: " + key);
    }
    it->second.first = header;
}


inline std::string dcf::Section::getHeader(const std::string &key) const {
    auto it = map.find(key);
    if(it == map.end()) {
        throw std::runtime_error("Key not found: " + key);
    }
    return it->second.first;
}


inline std::string dcf::Section::toString(int indent) const {
    if(indent < 0) {
        throw std::range_error("Indent cannot be smaller than zero");
    }
    return toString(indent, 1);
}


inline void dcf::Section::trim(std::string &text) const {
    auto first = std::find_if_not(text.begin(), text.end(),
        [](unsigned char ch) { return std::isspace(ch); });
    text.erase(text.begin(), first);

    auto last = std::find_if_not(text.rbegin(), text.rend(),
        [](unsigned char ch) { return std::isspace(ch); }).base();
    text.erase(last, text.end());
}


inline void dcf::Section::indentWithComments(std::string &text, const std::string &spacePrefix) const {
    trim(text);

    std::string result;
    size_t start = 0;
    size_t end = text.find('\n');

    while(end != std::string::npos) {
        result.append(text, start, end - start);
        result += "\n" + spacePrefix + "// ";
        start = end + 1;
        while(start < text.size() && (text[start] == ' ' || text[start] == '\t')) {
            start++;
        }
        end = text.find('\n', start);
    }
    if(start < text.size()) {
        result.append(text, start, std::string::npos);
    }

    text = std::move(result);
}


inline std::string dcf::Section::toArrayString(const dcf::Value &value, int indent, int depth) const {
    std::ostringstream output;
    const std::string spacePrefix(indent * depth, ' ');

    output << "[\n";

    const std::vector<dcf::Value> &list = value.asArray();
    for(const dcf::Value &val : list) {
        output << spacePrefix << valueString(val, indent, depth + 1) << ",\n";
    }

    if(!list.empty()) {
        std::string str = output.str();
        str.erase(str.end() - 2);
        return str + std::string(indent * (depth - 1), ' ') + ']';
    }

    output.seekp(-1, std::ios_base::end);
    output << ']';
    return output.str();
}


inline std::string dcf::Section::valueString(const dcf::Value &value, int indent, int depth) const {
    switch(value.getType()) {
        case dcf::ValueType::STRING:
            return '"' + value.asString() + '"';
        case dcf::ValueType::BOOLEAN:
            return value.asBool() ? "true" : "false";
        case dcf::ValueType::INTEGER:
            return std::to_string(value.asInt());
        case dcf::ValueType::DOUBLE: {
            std::ostringstream sstream;
            sstream << std::setprecision(15) << value.asDouble();
            return sstream.str();
        }
        case dcf::ValueType::ARRAY:
            return toArrayString(value, indent, depth);
        case dcf::ValueType::SECTION:
            return value.asSection().toString(indent, depth);
    }
}


inline std::string dcf::Section::toString(int indent, int depth) const {
    std::ostringstream output;
    const std::string spacePrefix(indent * depth, ' ');

    output << "{\n";

    bool firstElement = true;
    for(const std::string &key : keyOrder) {
        const auto &value = map.at(key);
        if(!value.first.empty()) {
            std::string header = value.first;
            indentWithComments(header, spacePrefix);
            if(!firstElement) {
                output << '\n';
            }
            output << spacePrefix << "// " << header << '\n';
        }
        output << spacePrefix << key << ": " << valueString(value.second, indent, depth + 1) << ",\n";
        firstElement = false;
    }

    if(!keyOrder.empty()) {
        std::string str = output.str();
        str.erase(str.end() - 2);
        return str + std::string(indent * (depth - 1), ' ') + '}';
    }

    output.seekp(-1, std::ios_base::end);
    output << '}';
    return output.str();
}

#endif // SECTION_HPP
