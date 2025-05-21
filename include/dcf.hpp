// VERSION 0.0.1

#ifndef DCF_HPP
#define DCF_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <stdexcept>




namespace dcf {
    class DCF {
    public:
        static void parseFile(const std::string &path) {
            std::fstream fileStream(path);
            if(!fileStream.is_open()) {
                throw std::runtime_error("Unable to open file");
            }

            // Read the full source file
            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            std::string fullFile = buffer.str();
            fileStream.close();

            parse(fullFile);
        }



        static void parse(const std::string &text) {
            std::vector<Token> tokens = tokenize(text);

            for(const Token &token : tokens) {
                std::cout << std::endl << typeToString(token.type) << std::endl;
                std::cout << "||" << token.value << "||\t" << std::endl;
            }

            // TODO: parse the tokens
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
                COMMA
            } type;
            const std::string value;
            size_t line;
            size_t column;

            Token(Type type, const std::string &value, size_t line, size_t column)
                : type(type), value(value), line(line), column(column) {}
        }; // class Token


        static std::string typeToString(Token::Type type) {
            switch (type) {
                case Token::Type::WHITESPACE:   return "WHITESPACE";
                case Token::Type::COMMENT:      return "COMMENT";
                case Token::Type::STRING:       return "STRING";
                case Token::Type::BOOLEAN:      return "BOOLEAN";
                case Token::Type::NUM_DECIMAL:  return "NUM_DECIMAL";
                case Token::Type::NUM_HEX:      return "NUM_HEX";
                case Token::Type::NUM_BINARY:   return "NUM_BINARY";
                case Token::Type::NUM_INT:      return "NUM_INT";
                case Token::Type::KEY:          return "KEY";
                case Token::Type::FUNCTION:     return "FUNCTION";
                case Token::Type::L_PAREN:      return "L_PAREN";
                case Token::Type::R_PAREN:      return "R_PAREN";
                case Token::Type::L_BRACE:      return "L_BRACE";
                case Token::Type::R_BRACE:      return "R_BRACE";
                case Token::Type::L_BRACKET:    return "L_BRACKET";
                case Token::Type::R_BRACKET:    return "R_BRACKET";
                case Token::Type::COLON:        return "COLON";
                case Token::Type::COMMA:        return "COMMA";
                default:                        return "__UNKNOWN__";
            }
        }


        class TokenDefinition {
        public:
            TokenDefinition(const std::string &pattern, Token::Type type)
                : regex("^(" + pattern + ")", std::regex_constants::icase), type(type) {}

            const std::regex regex;
            const Token::Type type;
        }; // class TokenDefinition


        static std::vector<Token> tokenize(const std::string &text) {
            static const std::vector<TokenDefinition> TOKEN_DEFINITIONS = {
                {R"(\s+)",                      Token::Type::WHITESPACE},
                {R"(//[^\n]*|/\*(.|\n)*?\*/)",  Token::Type::COMMENT},
                {R"("[^"\n]*"|'[^'\n]*')",      Token::Type::STRING},
                {R"(true|false)",               Token::Type::BOOLEAN},
                {R"(\d*\.\d+|\d+\.\d*)",        Token::Type::NUM_DECIMAL},
                {R"(0x[\da-f]+)",               Token::Type::NUM_HEX},
                {R"(0b[01]+)",                  Token::Type::NUM_BINARY},
                {R"(\d+)",                      Token::Type::NUM_INT},
                {R"([a-z]([\w-]*[a-z0-9])?)",   Token::Type::KEY},
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

            std::vector<Token> tokens;
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

                        if (type == Token::Type::WHITESPACE || type == Token::Type::COMMENT) {
                            for (char c : matchStr) {
                                if (c == '\n') {
                                    line++;
                                    column = 1;
                                } else {
                                    column++;
                                }
                            }
                        } else {
                            column += match.length();
                        }

                        pos += match.length();
                        break;
                    }
                }

                if (!matched) {
                    throw std::invalid_argument("Unknown token at line " + std::to_string(line) + ", column " + std::to_string(column));
                }
            }

            return tokens;
        }
    }; // class DCF
} // namespace dcf

#endif // DCF_HPP
