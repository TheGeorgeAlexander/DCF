#ifndef DCF_HPP
#define DCF_HPP

#include "parser.hpp"
#include "section.hpp"
#include "lexer.hpp"
#include "value.hpp"


static_assert(sizeof(double) == 8,
    "This library requires 'double' to be 8 bytes (64-bit IEEE 754). "
    "Your platform does not meet this requirement.");


namespace dcf {
    dcf::Section parse(const std::string &text) {
        std::vector<internal::Token> tokens;
        tokenize(text, tokens);

        if(tokens.size() == 0) {
            throw parse_error("Expected content in input but got nothing");
        }

        internal::Token lastToken = tokens.back();
        tokens.emplace_back(internal::Token::Type::END_OF_INPUT, "", lastToken.line, lastToken.column + lastToken.value.length());

        return internal::Parser(tokens).parse();
    }
} // namespace dcf

#endif // DCF_HPP
