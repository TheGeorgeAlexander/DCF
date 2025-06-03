// VERSION 0.0.1

#ifndef DCF_HPP
#define DCF_HPP

#include <regex>




namespace dcf {
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
        static void parse(const std::string &text) {
            std::vector<Token> tokens;
            tokenize(text, tokens);

            if(tokens.size() == 0) {
                throw dcf::parse_error("Expected content in input but got nothing");
            }

            tokens.emplace_back(Token::Type::END_OF_INPUT, "", tokens.back().line, tokens.back().column + tokens.back().value.length());

            Parser(tokens).parse();
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
                : regex("^(?:" + pattern + ")", std::regex_constants::icase), type(type) {}

            const std::regex regex;
            const Token::Type type;
        }; // private class TokenDefinition


        inline static const TokenDefinition TOKEN_DEFINITIONS[] = {
            {R"(\s+)",                      Token::Type::WHITESPACE},
            {R"(//[^\n]*|/\*(?:.|\n)*?\*/)",Token::Type::COMMENT},
            {R"("[^"\n]*"|'[^'\n]*')",      Token::Type::STRING},
            {R"(true|false)",               Token::Type::BOOLEAN},
            {R"(-?(?:\d*\.\d+|\d+\.\d*))",  Token::Type::NUM_DECIMAL},
            {R"(-?0x[\da-f]+)",             Token::Type::NUM_HEX},
            {R"(-?0b[01]+)",                Token::Type::NUM_BINARY},
            {R"(-?\d+)",                    Token::Type::NUM_INT},
            {R"([a-z](?:[\w-]*[a-z0-9])?)", Token::Type::KEY},
            {R"(@[a-z]+)",                  Token::Type::FUNCTION},
            {R"(\()",                       Token::Type::L_PAREN},
            {R"(\))",                       Token::Type::R_PAREN},
            {R"(\{)",                       Token::Type::L_BRACE},
            {R"(})",                        Token::Type::R_BRACE},
            {R"(\[)",                       Token::Type::L_BRACKET},
            {R"(])",                        Token::Type::R_BRACKET},
            {R"(:)",                        Token::Type::COLON},
            {R"(,)",                        Token::Type::COMMA}
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


            void parse() {
                parseSection();
                matchNextNoComments(Token::Type::END_OF_INPUT);
            }



        private:
            const std::vector<Token> &tokens;

            size_t index = -1;


            [[noreturn]] void error(const std::string &expected) const {
                Token curr = tokens[index];
                throw dcf::parse_error("Expected " + expected + " but got " + typeToString(curr.type), curr.line, curr.column);
            }


            void matchNextNoComments(const Token::Type expected) {
                Token curr = nextNoComments();
                if(curr.type != expected) {
                    error(typeToString(expected));
                }
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


            void parseSection() {
                matchNextNoComments(Token::Type::L_BRACE);
                parsePairList();
                matchNextNoComments(Token::Type::R_BRACE);
            }


            void parsePairList() {
                if(!nextWillMatchNoComments(Token::Type::KEY)) {
                    return;
                }
                parsePair();
                if(nextWillMatchNoComments(Token::Type::COMMA)) {
                    nextNoComments();
                    parsePairList();
                }
            }


            void parsePair() {
                parsePairHeader();
                matchNextNoComments(Token::Type::KEY);
                matchNextNoComments(Token::Type::COLON);
                parseValue();
            }


            void parsePairHeader() {
                while(nextWillMatch(Token::Type::COMMENT)) {
                    next();
                }
            }


            void parseValue() {
                switch (peekNextNoComments().type) {
                    case Token::Type::STRING:
                    case Token::Type::BOOLEAN:
                        nextNoComments();
                        break;

                    case Token::Type::NUM_INT:
                    case Token::Type::NUM_DECIMAL:
                    case Token::Type::NUM_HEX:
                    case Token::Type::NUM_BINARY:
                        nextNoComments();
                        break;

                    case Token::Type::L_BRACKET:
                        parseArray();
                        break;

                    case Token::Type::L_BRACE:
                        parseSection();
                        break;

                    case Token::Type::FUNCTION:
                        parseFunction();
                        break;
                    
                    default:
                        nextNoComments();
                        error("value");
                        break;
                }
            }


            void parseArray() {
                matchNextNoComments(Token::Type::L_BRACKET);
                parseValueList();
                matchNextNoComments(Token::Type::R_BRACKET);
            }


            void parseValueList() {
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
                parseValue();
                if(nextWillMatchNoComments(Token::Type::COMMA)) {
                    nextNoComments();
                    parseValueList();
                }
            }


            void parseFunction() {
                matchNextNoComments(Token::Type::FUNCTION);
                matchNextNoComments(Token::Type::L_PAREN);
                parseValueList();
                matchNextNoComments(Token::Type::R_PAREN);

            }
        }; // private class Parser
    }; // class DCF
} // namespace dcf

#endif // DCF_HPP
