#ifndef LEXER_HPP
#define LEXER_HPP

#include "dcf.hpp"
#include <regex>


namespace dcf {
    class Section;

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
    };
} // namespace dcf


namespace dcf::internal {

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
    };


    std::string typeToString(Token::Type type) {
        switch(type) {
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
    };


    const TokenDefinition TOKEN_DEFINITIONS[] = {
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


    void tokenize(const std::string &text, std::vector<Token> &tokens) {
        std::size_t line = 1, column = 1, pos = 0;
        std::string_view input = text;

        while(pos < input.size()) {
            bool matched = false;
            std::string_view current = input.substr(pos);

            for(const TokenDefinition &def : TOKEN_DEFINITIONS) {
                std::cmatch match;
                if(std::regex_search(current.begin(), current.end(), match, def.regex)) {
                    matched = true;
                    const std::string matchStr = match.str(0);
                    const Token::Type type = def.type;

                    if(type != Token::Type::WHITESPACE) {
                        tokens.emplace_back(type, matchStr, line, column);
                    }

                    // Whitespace or comment might span multiple lines
                    if(type == Token::Type::WHITESPACE || type == Token::Type::COMMENT) {
                        for(char c : matchStr) {
                            if(c == '\n') {
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

            if(!matched) {
                throw dcf::parse_error("Unknown token", line, column);
            }
        }
    }
} // namespace dcf::internal

#endif
