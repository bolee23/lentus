#include "token.hpp"
#include <string>
#include "magic_enum.hpp"

token::token(token_type type, int line_number) : type(type), line_number(line_number) {}

token::token(const token &tok)
{
    type = tok.type;
    line_number = tok.line_number;
    switch (type)
    {
    case token_type::IDENTIFIER:
        identifier_name = new std::string(*tok.identifier_name);
        break;
    case token_type::STRING:
        string_constant = new std::string(*tok.string_constant);
        break;
    case token_type::NUMBER:
        number_constant = tok.number_constant;
        break;
    default:
        break;
    }
}

token::token(token &&tok) : type(tok.type), line_number(tok.line_number)
{
    switch (type)
    {
    case token_type::IDENTIFIER:
        identifier_name = tok.identifier_name;
        tok.identifier_name = nullptr;
        break;
    case token_type::STRING:
        string_constant = tok.string_constant;
        tok.string_constant = nullptr;
        break;
    case token_type::NUMBER:
        number_constant = tok.number_constant;
        break;
    default:
        break;
    }
}

token &token::operator=(const token &tok)
{
    if (this == &tok)
        return *this;

    this->~token();
    type = tok.type;
    line_number = tok.line_number;
    switch (type)
    {
    case token_type::IDENTIFIER:
        identifier_name = new std::string(*tok.identifier_name);
        break;
    case token_type::STRING:
        string_constant = new std::string(*tok.string_constant);
        break;
    case token_type::NUMBER:
        number_constant = tok.number_constant;
        break;
    default:
        break;
    }
    return *this;
}

token &token::operator=(token &&tok)
{
    if (this == &tok)
        return *this;

    this->~token();
    type = tok.type;
    line_number = tok.line_number;
    switch (type)
    {
    case token_type::IDENTIFIER:
        identifier_name = tok.identifier_name;
        tok.identifier_name = nullptr;
        break;
    case token_type::STRING:
        string_constant = tok.string_constant;
        tok.string_constant = nullptr;
        break;
    case token_type::NUMBER:
        number_constant = tok.number_constant;
        break;
    default:
        break;
    }
    return *this;
}

token token::identifier_token(std::string *identifier, int line_number)
{
    token tok(token_type::IDENTIFIER, line_number);
    tok.identifier_name = identifier;
    return tok;
}

token token::string_constant_token(std::string *string_constant, int line_number)
{
    token tok(token_type::STRING, line_number);
    tok.string_constant = string_constant;
    return tok;
}

token token::number_constant_token(double number_constant, int line_number)
{
    token tok(token_type::NUMBER, line_number);
    tok.number_constant = number_constant;
    return tok;
}

std::string token::to_string() const
{
    switch (type)
    {
    case token_type::IDENTIFIER:
        return "IDENTIFIER(" + *identifier_name + ")";
    case token_type::STRING:
        return "STRING(\"" + *string_constant + "\")";
    case token_type::NUMBER:
        return "NUMBER(" + std::to_string(number_constant) + ")";
    default:
        return std::string(magic_enum::enum_name(type));
    }
}

token::~token()
{
    switch (type)
    {
    case token_type::IDENTIFIER:
        delete identifier_name;
        break;
    case token_type::STRING:
        delete string_constant;
        break;
    case token_type::NUMBER:
        break;
    default:
        break;
    }
}