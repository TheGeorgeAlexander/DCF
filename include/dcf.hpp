// VERSION 0.0.1

#ifndef DCF_HPP
#define DCF_HPP

#include <regex>
#include <unordered_map>
#include <variant>
#include <optional>
#include <cstdint>
#include <iostream>



static_assert(sizeof(double) == 8,
    "This library requires 'double' to be 8 bytes (64-bit IEEE 754). "
    "Your platform does not meet this requirement.");


namespace dcf {
    enum class ValueType {
        STRING,
        BOOLEAN,
        INTEGER,
        DOUBLE,
        ARRAY,
        SECTION
    };


    class Section;

    class Value {
    public:
        Value(const Value&) = default;

        Value(const std::string &text)
            : type(ValueType::STRING), data(text) { }

        Value(bool boolean)
            : type(ValueType::BOOLEAN), data(boolean) { }

        Value(int64_t num_integer)
            : type(ValueType::INTEGER), data(num_integer) { }

        Value(double num_double)
            : type(ValueType::DOUBLE), data(num_double) { }

        Value(const std::vector<Value> &array)
            : type(ValueType::ARRAY), data(array) { }

        Value(const Section &section);


        Value& operator=(const Value&) = default;


        ValueType getType() const {
            return type;
        }


        const std::string& asString() const {
            checkType(ValueType::STRING);
            return std::get<std::string>(data);
        }


        bool asBool() const {
            checkType(ValueType::BOOLEAN);
            return std::get<bool>(data);
        }


        int64_t asInt() const {
            checkType(ValueType::INTEGER);
            return std::get<int64_t>(data);
        }


        double asDouble() const {
            checkType(ValueType::DOUBLE);
            return std::get<double>(data);
        }


        const std::vector<Value>& asArray() const {
            checkType(ValueType::ARRAY);
            return std::get<std::vector<Value>>(data);
        }


        const Section& asSection() const;



    private:
        ValueType type;
        std::variant<
            std::string,
            bool,
            int64_t,
            double,
            std::vector<Value>,
            std::shared_ptr<Section>  // store Section indirectly to break the cycle
        > data;


        void checkType(ValueType expected) const {
            if (type != expected) {
                throw std::runtime_error("Type mismatch");
            }
        }
    }; // class Value


    class Section {
    public:
        Value get(const std::string &key) const {
            auto it = map.find(key);
            if (it == map.end()) {
                throw std::runtime_error("Key not found: " + key);
            }
            return it->second.second;
        }


        std::optional<Value> optionalGet(const std::string &key) const {
            auto it = map.find(key);
            if (it == map.end()) {
                return std::nullopt;
            }
            return it->second.second;
        }


        void set(const std::string &key, const Value &value) {
            auto it = map.find(key);
            if (it == map.end()) {
                keyOrder.push_back(key);
                map.insert({key, {"", value}});
            } else {
                it->second.second = value;
            }
        }


        void set(const std::string &key, const std::string &value) {
            set(key, Value(value));
        }


        void set(const std::string &key, bool value) {
            set(key, Value(value));
        }


        void set(const std::string &key, int64_t value) {
            set(key, Value(value));
        }


        void set(const std::string &key, double value) {
            set(key, Value(value));
        }


        void set(const std::string &key, const std::vector<Value> &value) {
            set(key, Value(value));
        }


        void set(const std::string &key, const Section &value) {
            set(key, Value(value));
        }


        void remove(const std::string &key) {
            keyOrder.erase(std::find(keyOrder.begin(), keyOrder.end(), key));
            map.erase(key);
        }


        std::vector<std::string> keys() const {
            return keyOrder;
        }


        void setHeader(const std::string &key, const std::string &header) {
            auto it = map.find(key);
            if (it == map.end()) {
                throw std::runtime_error("Key not found: " + key);
            }
            it->second.first = header;
        }


        std::string getHeader(const std::string &key) const {
            auto it = map.find(key);
            if (it == map.end()) {
                throw std::runtime_error("Key not found: " + key);
            }
            return it->second.first;
        }


        std::string toString(int indent = 4) const {
            if(indent < 0) {
                throw std::range_error("Indent cannot be smaller than zero");
            }
            return toString(indent, 1);
        }


    
    private:
        std::vector<std::string> keyOrder;
        std::unordered_map<std::string, std::pair<std::string, Value>> map;


        void trim(std::string &text) const {
            auto first = std::find_if_not(text.begin(), text.end(), 
                [](unsigned char ch) { return std::isspace(ch); });
            text.erase(text.begin(), first);
            
            auto last = std::find_if_not(text.rbegin(), text.rend(), 
                [](unsigned char ch) { return std::isspace(ch); }).base();
            text.erase(last, text.end());
        }


        void indentWithComments(std::string &text, const std::string &spacePrefix) const {
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


        std::string toArrayString(const Value &value, int indent, int depth) const {
            std::ostringstream output;
            const std::string spacePrefix(indent * depth, ' ');
            
            output << "[\n";
            
            const std::vector<Value> &list = value.asArray();
            for(const Value &val : list) {
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


        std::string valueString(const Value &value, int indent, int depth) const {
            switch(value.getType()) {
                case ValueType::STRING:
                    return '"' + value.asString() + '"';

                case ValueType::BOOLEAN:
                    return value.asBool() ? "true" : "false";

                case ValueType::INTEGER:
                    return std::to_string(value.asInt());

                case ValueType::DOUBLE: {
                    std::ostringstream sstream;
                    sstream << value.asDouble();
                    return sstream.str();
                }

                case ValueType::ARRAY:
                    return toArrayString(value, indent, depth);

                case ValueType::SECTION:
                    return value.asSection().toString(indent, depth);
            }
        }


        std::string toString(int indent, int depth) const {
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
    }; // class Section


    inline Value::Value(const Section &section)
        : type(ValueType::SECTION), data(std::make_shared<Section>(section)) {}


    inline const Section& Value::asSection() const {
        checkType(ValueType::SECTION);
        return *std::get<std::shared_ptr<Section>>(data);
    }


    class parse_error : public std::exception {
    public:
        parse_error(const std::string &message)
            : errorMessage(message),
            whatMessage(message + ", line " + std::to_string(linePos) + ", column " + std::to_string(columnPos)) { }

        parse_error(const std::string &message, const size_t line, const size_t column)
            : errorMessage(message),
            linePos(line),
            columnPos(column),
            whatMessage(message + ", line " + std::to_string(line) + ", column " + std::to_string(column)) { }


        const char* what() const noexcept {
            return whatMessage.c_str();
        }

        const char* message() const noexcept {
            return errorMessage.c_str();
        }

        size_t line() const noexcept {
            return linePos;
        }

        size_t column() const noexcept {
            return columnPos;
        }



    private:
        const std::string errorMessage;
        const size_t linePos = 1;
        const size_t columnPos = 1;
        const std::string whatMessage;
    }; // class parse_error


    class DCF {
    public:
        static dcf::Section parse(const std::string &text) {
            std::vector<Token> tokens;
            tokenize(text, tokens);

            if(tokens.size() == 0) {
                throw dcf::parse_error("Expected content in input but got nothing");
            }

            tokens.emplace_back(Token::Type::END_OF_INPUT, "", tokens.back().line, tokens.back().column + tokens.back().value.length());

            return Parser(tokens).parse();
        }



    private:
        class Token {
        public:
            const enum class Type {
                WHITESPACE,
                COMMENT,
                STRING,
                BOOLEAN,
                NUM_DECIMAL,
                NUM_HEX,
                NUM_BINARY,
                NUM_INT,
                KEY,
                FUNCTION,
                L_PAREN,
                R_PAREN,
                L_BRACE,
                R_BRACE,
                L_BRACKET,
                R_BRACKET,
                COLON,
                COMMA,
                END_OF_INPUT
            } type;
            const std::string value;
            const size_t line;
            const size_t column;

            Token(Type type, const std::string &value, size_t line, size_t column)
                : type(type), value(value), line(line), column(column) {}
        }; // private class Token


        static std::string typeToString(Token::Type type) {
            switch (type) {
                case Token::Type::WHITESPACE:   return "whitespace";
                case Token::Type::COMMENT:      return "comment";
                case Token::Type::STRING:       return "string";
                case Token::Type::BOOLEAN:      return "boolean";
                case Token::Type::NUM_DECIMAL:  return "decimal number";
                case Token::Type::NUM_HEX:      return "hexadecimal number";
                case Token::Type::NUM_BINARY:   return "binary number";
                case Token::Type::NUM_INT:      return "integer number";
                case Token::Type::KEY:          return "key";
                case Token::Type::FUNCTION:     return "function";
                case Token::Type::L_PAREN:      return "'('";
                case Token::Type::R_PAREN:      return "')'";
                case Token::Type::L_BRACE:      return "'{'";
                case Token::Type::R_BRACE:      return "'}'";
                case Token::Type::L_BRACKET:    return "'['";
                case Token::Type::R_BRACKET:    return "']'";
                case Token::Type::COLON:        return "':'";
                case Token::Type::COMMA:        return "','";
                case Token::Type::END_OF_INPUT: return "end of input";
            }
        }


        class TokenDefinition {
        public:
            TokenDefinition(const std::string &pattern, Token::Type type)
                : regex("^(?:" + pattern + ")"), type(type) {}

            const std::regex regex;
            const Token::Type type;
        }; // private class TokenDefinition


        inline static const TokenDefinition TOKEN_DEFINITIONS[] = {
            {R"(\s+)",                                                      Token::Type::WHITESPACE},
            {R"(//[^\n]*|/\*(?:.|\n)*?\*/)",                                Token::Type::COMMENT},
            {R"("[^"\n]*"|'[^'\n]*')",                                      Token::Type::STRING},
            {R"(true|false)",                                               Token::Type::BOOLEAN},
            {R"(-?(?:\d+\.\d*|\.\d+)(?:[eE][+-]?\d+)?|-?\d+[eE][+-]?\d+)",  Token::Type::NUM_DECIMAL},
            {R"(-?0x[\da-fA-F]+)",                                          Token::Type::NUM_HEX},
            {R"(-?0b[01]+)",                                                Token::Type::NUM_BINARY},
            {R"(-?\d+)",                                                    Token::Type::NUM_INT},
            {R"([a-zA-Z](?:[\w-]*[a-zA-Z0-9])?)",                           Token::Type::KEY},
            {R"(@[a-zA-Z]+)",                                               Token::Type::FUNCTION},
            {R"(\()",                                                       Token::Type::L_PAREN},
            {R"(\))",                                                       Token::Type::R_PAREN},
            {R"(\{)",                                                       Token::Type::L_BRACE},
            {R"(})",                                                        Token::Type::R_BRACE},
            {R"(\[)",                                                       Token::Type::L_BRACKET},
            {R"(])",                                                        Token::Type::R_BRACKET},
            {R"(:)",                                                        Token::Type::COLON},
            {R"(,)",                                                        Token::Type::COMMA}
        };


        static void tokenize(const std::string &text, std::vector<Token> &tokens) {
            std::size_t line = 1, column = 1, pos = 0;
            std::string_view input = text;

            while (pos < input.size()) {
                bool matched = false;
                std::string_view current = input.substr(pos);

                for (const TokenDefinition &def : TOKEN_DEFINITIONS) {
                    std::cmatch match;
                    if (std::regex_search(current.begin(), current.end(), match, def.regex)) {
                        matched = true;
                        const std::string matchStr = match.str(0);
                        const Token::Type type = def.type;

                        if(type != Token::Type::WHITESPACE) {
                            tokens.emplace_back(type, matchStr, line, column);
                        }

                        // Whitespace or comment might span multiple lines
                        if (type == Token::Type::WHITESPACE || type == Token::Type::COMMENT) {
                            for (char c : matchStr) {
                                if (c == '\n') {
                                    line++;
                                    column = 1;
                                } else {
                                    column++;
                                }
                            }

                        // the rest doesn't
                        } else {
                            column += match.length();
                        }

                        pos += match.length();
                        break;
                    }
                }

                if (!matched) {
                    throw dcf::parse_error("Unknown token", line, column);
                }
            }
        }


        class Parser {
        public:
            Parser(const std::vector<Token> &tokens)
                : tokens(tokens) { }


            dcf::Section parse() {
                dcf::Section root = parseSection();
                matchNextNoComments(Token::Type::END_OF_INPUT);
                return root;
            }



        private:
            const std::vector<Token> &tokens;

            size_t index = -1;


            [[noreturn]] void error(const std::string &expected) const {
                Token curr = tokens[index];
                throw dcf::parse_error("Expected " + expected + " but got " + typeToString(curr.type), curr.line, curr.column);
            }


            Token matchNextNoComments(const Token::Type expected) {
                Token curr = nextNoComments();
                if(curr.type != expected) {
                    error(typeToString(expected));
                }
                return curr;
            }


            bool nextWillMatch(const Token::Type expected) {
                return peekNext().type == expected;
            }


            bool nextWillMatchNoComments(const Token::Type expected) {
                return peekNextNoComments().type == expected;
            }

            
            bool isAtEnd() const {
                return tokens[index].type == Token::Type::END_OF_INPUT;
            }


            Token next() {
                if(isAtEnd()) {
                    return tokens[index];
                }
                index++;
                return tokens[index];
            }


            Token nextNoComments() {
                if(isAtEnd()) {
                    return tokens[index];
                }
                do {
                    index++;
                } while(tokens[index].type == Token::Type::COMMENT);
                return tokens[index];
            }
            
            
            Token peekNext() const {
                if(isAtEnd()) {
                    return tokens[index];
                }
                return tokens[index + 1];
            }


            Token peekNextNoComments() const {
                if(isAtEnd()) {
                    return tokens[index];
                }
                size_t nextIndex = index;
                do {
                    nextIndex++;
                } while(tokens[nextIndex].type == Token::Type::COMMENT);
                return tokens[nextIndex];
            }


            dcf::Section parseSection() {
                matchNextNoComments(Token::Type::L_BRACE);
                dcf::Section section;
                parsePairList(section);
                matchNextNoComments(Token::Type::R_BRACE);
                return section;
            }


            void parsePairList(dcf::Section &section) {
                if(!nextWillMatchNoComments(Token::Type::KEY)) {
                    return;
                }
                parsePair(section);
                if(nextWillMatchNoComments(Token::Type::COMMA)) {
                    nextNoComments();
                    parsePairList(section);
                }
            }


            void parsePair(dcf::Section &section) {
                std::string header;
                while(nextWillMatch(Token::Type::COMMENT)) {
                    header += '\n' + cleanCommentTokenValue(next());
                }
                Token keyToken = matchNextNoComments(Token::Type::KEY);
                matchNextNoComments(Token::Type::COLON);
                dcf::Value value = parseValue();
                section.set(keyToken.value, value);
                section.setHeader(keyToken.value, header);
            }


            dcf::Value parseValue() {
                switch (peekNextNoComments().type) {
                    case Token::Type::STRING:
                        return dcf::Value(cleanStringTokenValue(nextNoComments()));

                    case Token::Type::BOOLEAN:
                        return dcf::Value(cleanBooleanTokenValue(nextNoComments()));
                        break;

                    case Token::Type::NUM_INT:
                    case Token::Type::NUM_HEX:
                    case Token::Type::NUM_BINARY:
                        return dcf::Value(cleanIntegerTokenValue(nextNoComments()));
                        break;

                    case Token::Type::NUM_DECIMAL:
                        return dcf::Value(cleanDoubleTokenValue(nextNoComments()));
                        break;

                    case Token::Type::L_BRACKET:
                        return parseArray();
                        break;

                    case Token::Type::L_BRACE:
                        return dcf::Value(parseSection());
                        break;

                    case Token::Type::FUNCTION:
                        parseFunction();
                        return Value("");
                    
                    default:
                        nextNoComments();
                        error("value");
                        break;
                }
            }


            dcf::Value parseArray() {
                matchNextNoComments(Token::Type::L_BRACKET);
                std::vector<dcf::Value> array;
                parseValueList(array);
                matchNextNoComments(Token::Type::R_BRACKET);
                return dcf::Value(array);
            }


            void parseValueList(std::vector<dcf::Value> &array) {
                switch (peekNextNoComments().type) {
                    case Token::Type::STRING:
                    case Token::Type::BOOLEAN:
                    case Token::Type::NUM_INT:
                    case Token::Type::NUM_DECIMAL:
                    case Token::Type::NUM_HEX:
                    case Token::Type::NUM_BINARY:
                    case Token::Type::L_BRACKET:
                    case Token::Type::L_BRACE:
                    case Token::Type::FUNCTION:
                        break;
                    default:
                        return;
                }
                array.push_back(parseValue());
                if(nextWillMatchNoComments(Token::Type::COMMA)) {
                    nextNoComments();
                    parseValueList(array);
                }
            }


            void parseFunction() {
                matchNextNoComments(Token::Type::FUNCTION);
                matchNextNoComments(Token::Type::L_PAREN);
                std::vector<dcf::Value> arguments;
                parseValueList(arguments);
                matchNextNoComments(Token::Type::R_PAREN);
            }


            std::string cleanCommentTokenValue(const Token &token) {
                std::string value = token.value;
                if(value[1] == '/') {
                    return value.substr(2);
                }
                return value.substr(2, value.length() - 4);
            }


            std::string cleanStringTokenValue(const Token &token) {
                const std::string &value = token.value;
                return value.substr(1, value.length() - 2);
            }


            bool cleanBooleanTokenValue(const Token &token) {
                return token.value[0] == 't';
            }


            int64_t cleanIntegerTokenValue(const Token &token) {
                const std::string &value = token.value;
                Token::Type type = token.type;

                bool negative = value[0] == '-';

                size_t start = negative ? 3 : 2;
                if(type == Token::Type::NUM_INT) {
                    start -= 2;
                }

                int base = 10;
                if(type == Token::Type::NUM_HEX) {
                    base = 16;
                } else if(type == Token::Type::NUM_BINARY) {
                    base = 2;
                }

                uint64_t num = std::stoull(value.substr(start), nullptr, base);

                if(negative) {
                    return -static_cast<int64_t>(num);
                }
                return static_cast<int64_t>(num);
            }


            double cleanDoubleTokenValue(const Token &token) {
                return std::stod(token.value);
            }
        }; // private class Parser
    }; // class DCF
} // namespace dcf

#endif // DCF_HPP
