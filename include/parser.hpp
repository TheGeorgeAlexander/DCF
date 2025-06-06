#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "value.hpp"
#include "section.hpp"
#include <cstdint>


namespace dcf::internal {

    class Parser {
    public:
        Parser(const std::vector<Token> &tokens);
        dcf::Section parse();

    private:
        const std::vector<Token> &tokens;
        size_t index = -1;

        [[noreturn]] void error(const std::string &expected) const;

        Token matchNextNoComments(const Token::Type expected);
        bool nextWillMatch(const Token::Type expected);
        bool nextWillMatchNoComments(const Token::Type expected);

        bool isAtEnd() const;

        Token next();
        Token nextNoComments();
        Token peekNext() const;
        Token peekNextNoComments() const;

        dcf::Section parseSection();
        void parsePairList(dcf::Section &section);
        void parsePair(dcf::Section &section);
        dcf::Value parseValue();
        dcf::Value parseArray();
        void parseValueList(std::vector<dcf::Value> &array);
        void parseFunction();

        std::string cleanCommentTokenValue(const Token &token);
        std::string cleanStringTokenValue(const Token &token);
        bool cleanBooleanTokenValue(const Token &token);
        int64_t cleanIntegerTokenValue(const Token &token);
        double cleanDoubleTokenValue(const Token &token);
    };
} // namespace dcf


inline dcf::internal::Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens) { }

inline dcf::Section dcf::internal::Parser::parse() {
    dcf::Section root = parseSection();
    matchNextNoComments(Token::Type::END_OF_INPUT);
    return root;
}

[[noreturn]] inline void dcf::internal::Parser::error(const std::string &expected) const {
    Token curr = tokens[index];
    throw dcf::parse_error("Expected " + expected + " but got " + typeToString(curr.type), curr.line, curr.column);
}

inline dcf::internal::Token dcf::internal::Parser::matchNextNoComments(const Token::Type expected) {
    Token curr = nextNoComments();
    if(curr.type != expected) {
        error(typeToString(expected));
    }
    return curr;
}

inline bool dcf::internal::Parser::nextWillMatch(const Token::Type expected) {
    return peekNext().type == expected;
}

inline bool dcf::internal::Parser::nextWillMatchNoComments(const Token::Type expected) {
    return peekNextNoComments().type == expected;
}

inline bool dcf::internal::Parser::isAtEnd() const {
    return tokens[index].type == Token::Type::END_OF_INPUT;
}

inline dcf::internal::Token dcf::internal::Parser::next() {
    if(isAtEnd()) {
        return tokens[index];
    }
    index++;
    return tokens[index];
}

inline dcf::internal::Token dcf::internal::Parser::nextNoComments() {
    if(isAtEnd()) {
        return tokens[index];
    }
    do {
        index++;
    } while(tokens[index].type == Token::Type::COMMENT);
    return tokens[index];
}

inline dcf::internal::Token dcf::internal::Parser::peekNext() const {
    if(isAtEnd()) {
        return tokens[index];
    }
    return tokens[index + 1];
}

inline dcf::internal::Token dcf::internal::Parser::peekNextNoComments() const {
    if(isAtEnd()) {
        return tokens[index];
    }
    size_t nextIndex = index;
    do {
        nextIndex++;
    } while(tokens[nextIndex].type == Token::Type::COMMENT);
    return tokens[nextIndex];
}

inline dcf::Section dcf::internal::Parser::parseSection() {
    matchNextNoComments(Token::Type::L_BRACE);
    dcf::Section section;
    parsePairList(section);
    matchNextNoComments(Token::Type::R_BRACE);
    return section;
}

inline void dcf::internal::Parser::parsePairList(dcf::Section &section) {
    if(!nextWillMatchNoComments(Token::Type::KEY)) {
        return;
    }
    parsePair(section);
    if(nextWillMatchNoComments(Token::Type::COMMA)) {
        nextNoComments();
        parsePairList(section);
    }
}

inline void dcf::internal::Parser::parsePair(dcf::Section &section) {
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

inline dcf::Value dcf::internal::Parser::parseValue() {
    switch(peekNextNoComments().type) {
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
            return dcf::Value("");
        
        default:
            nextNoComments();
            error("value");
            break;
    }
}

inline dcf::Value dcf::internal::Parser::parseArray() {
    matchNextNoComments(Token::Type::L_BRACKET);
    std::vector<dcf::Value> array;
    parseValueList(array);
    matchNextNoComments(Token::Type::R_BRACKET);
    return dcf::Value(array);
}

inline void dcf::internal::Parser::parseValueList(std::vector<dcf::Value> &array) {
    switch(peekNextNoComments().type) {
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

inline void dcf::internal::Parser::parseFunction() {
    matchNextNoComments(Token::Type::FUNCTION);
    matchNextNoComments(Token::Type::L_PAREN);
    std::vector<dcf::Value> arguments;
    parseValueList(arguments);
    matchNextNoComments(Token::Type::R_PAREN);
}

inline std::string dcf::internal::Parser::cleanCommentTokenValue(const Token &token) {
    std::string value = token.value;
    if(value[1] == '/') {
        return value.substr(2);
    }
    return value.substr(2, value.length() - 4);
}

inline std::string dcf::internal::Parser::cleanStringTokenValue(const Token &token) {
    const std::string &value = token.value;
    return value.substr(1, value.length() - 2);
}

inline bool dcf::internal::Parser::cleanBooleanTokenValue(const Token &token) {
    return token.value[0] == 't';
}

inline int64_t dcf::internal::Parser::cleanIntegerTokenValue(const Token &token) {
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

inline double dcf::internal::Parser::cleanDoubleTokenValue(const Token &token) {
    return std::stod(token.value);
}

#endif //PARSER_HPP
