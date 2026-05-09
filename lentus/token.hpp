#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <optional>

enum class token_type
{
    IDENTIFIER,
    STRING_CONSTANT,
    NUMBER_CONSTANT,

    PLUS,
    MINUS,
    ASTERISK,
    SLASH,

    LEFT_BRACKET,
    RIGHT_BRACKET,
    LEFT_BRACE,
    RIGHT_BRACE,
};

class token
{
private:
    token_type type;
    int line_number;
    union
    {
        std::string *identifier_name;
        std::string *string_constant;
        double number_constant;
    };

public:
    token(token_type type, int line_number);

private:
    token(token_type type, int line_number, std::string &&string_data);
    token(token_type type, int line_number, double number_constant);

public:
    static token identifier_token(std::string *identifier, int line_number);

    static token string_constant_token(std::string *string_constant, int line_number);
    static token number_constant_token(double number_constant, int line_number);

    ~token();
};

#endif