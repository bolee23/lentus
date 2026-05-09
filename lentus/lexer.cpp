#include "loader.hpp"
#include "lexer.hpp"

bool expect(lexer_data &data, char expected)
{
    if (data.source[data.position] == expected)
    {
        data.position++;
        return true;
    }
    else
    {
        return false;
    }
}

bool peek(lexer_data &data, char expected)
{
    return data.source[data.position] == expected;
}

void handle_number(lexer_data &data)
{
    // i will eventually contain the index of the last character + 1
    int i;

    // Loop breaks if end of file reached or non alphanumeric character reached
    for (i = data.position; i < data.source.length(); i++)
    {
        char current = data.source[data.position];
        if (current < '0' || current > '9')
        {
            break;
        }
    }
    // Update position to be after the number
    data.position = i;

    std::string substring = data.source.substr(data.position, i);

    try
    {
        double d = std::stod(substring);
        data.tokens.push_back(token::number_constant_token(d, data.line_number));
    }
    catch (const std::invalid_argument &e)
    {
        data.error_stream << "Invalid number format at line " << data.line_number << std::endl;
    }
}

void handle_string_constant(lexer_data &data)
{
}

void handle_identifier(lexer_data &data)
{
}

std::optional<lexed_code_unit> tokenize(const raw_code_unit &raw_unit, std::ostream &error_stream)
{
    std::vector<token> tokens;

    lexer_data data = {1, 0, raw_unit.source, tokens, error_stream, false};

    while (true)
    {
        char c = data.source[data.position];

        // Reached end of string (std::string is always null terminated)
        if (c == '\0')
        {
            break;
        }

        // Number constant
        if (c >= '0' && c <= '9')
        {
            handle_number(data);
            continue;
        }

        // Identifiers (can contain, but must not start with numbers)
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
        {
            handle_identifier(data);
            continue;
        }

        switch (c)
        {
        case '(':
            tokens.push_back(token(token_type::LEFT_BRACKET, data.line_number));
            data.position++;
            break;

        case ')':
            tokens.push_back(token(token_type::RIGHT_BRACKET, data.line_number));
            data.position++;
            break;

        case '{':
            tokens.push_back(token(token_type::LEFT_BRACE, data.line_number));
            data.position++;
            break;

        case '}':
            tokens.push_back(token(token_type::RIGHT_BRACE, data.line_number));
            data.position++;
            break;

            // Whitespace
        case ' ':
        case '\t':
            data.position++;
            break;

        case '\n':
            data.line_number++;
            data.position++;
            break;

            // Unexpected character
        default:
            data.error_found = true;
            error_stream << "Unexpected character '" << c << "' at line " << data.line_number << std::endl;
            data.position++;
            break;
        }
    }

    if (data.error_found)
    {
        return std::nullopt;
    }

    return lexed_code_unit{std::move(tokens), raw_unit.path};
}