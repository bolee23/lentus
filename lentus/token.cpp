#include "token.hpp"
#include <string>

token::token(token_type type, int line_number) : type(type), line_number(line_number) {}

token token::identifier_token(std::string *identifier, int line_number)
{
    token tok(token_type::IDENTIFIER, line_number);
    tok.identifier_name = identifier;
    return tok;
}

token token::string_constant_token(std::string *string_constant, int line_number)
{
    token tok(token_type::STRING_CONSTANT, line_number);
    tok.string_constant = string_constant;
    return tok;
}

token token::number_constant_token(double number_constant, int line_number)
{
    token tok(token_type::NUMBER_CONSTANT, line_number);
    tok.number_constant = number_constant;
    return tok;
}

token::~token()
{
    switch (type)
    {
    case token_type::IDENTIFIER:
        delete identifier_name;
        break;
    case token_type::STRING_CONSTANT:
        delete string_constant;
        break;
    case token_type::NUMBER_CONSTANT:
        break;
    default:
        break;
    }
}