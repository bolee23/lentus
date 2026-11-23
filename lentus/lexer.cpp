#include "lexer.hpp"
#include <iostream>
#include <unordered_map>

namespace lentus
{

const token_trie_node token_trie = {
    std::nullopt,
    {
        {';', token_trie_node(token_type::END_STATEMENT)},
        {'+', token_trie_node(token_type::ADD, {{'=', token_trie_node(token_type::ADD_ASSIGN)}, {'+', token_trie_node(token_type::INCREMENT)}})},
        {'-', token_trie_node(token_type::SUBTRACT, {{'=', token_trie_node(token_type::SUBTRACT_ASSIGN)}, {'-', token_trie_node(token_type::DECREMENT)}})},
        {'*', token_trie_node(token_type::MULTIPLY, {{'=', token_trie_node(token_type::MULTIPLY_ASSIGN)}})},
        {'/', token_trie_node(token_type::DIVIDE, {{'=', token_trie_node(token_type::DIVIDE_ASSIGN)}})},
        {'.', token_trie_node(token_type::DOT)},
        {',', token_trie_node(token_type::COMMA)},

        {'(', token_trie_node(token_type::LEFT_BRACKET)},
        {')', token_trie_node(token_type::RIGHT_BRACKET)},
        {'{', token_trie_node(token_type::LEFT_BRACE)},
        {'}', token_trie_node(token_type::RIGHT_BRACE)},
        {'[', token_trie_node(token_type::LEFT_INDEX)},
        {']', token_trie_node(token_type::RIGHT_INDEX)},

        {'<', token_trie_node(token_type::LESS, {{'=', token_trie_node(token_type::LESS_EQUAL)}})},
        {'>', token_trie_node(token_type::GREATER, {{'=', token_trie_node(token_type::GREATER_EQUAL)}})},

        {'=', token_trie_node(token_type::ASSIGN, {{'=', token_trie_node(token_type::EQUAL)}})},

        {'!', token_trie_node(token_type::NOT, {{'=', token_trie_node(token_type::NOT_EQUAL)}})},
        {'&', token_trie_node(std::nullopt, {{'&', token_trie_node(token_type::AND)}})},
        {'|', token_trie_node(std::nullopt, {{'|', token_trie_node(token_type::OR)}})},
    }};

// Map of keywords to their token types
const std::unordered_map<std::string, token_type> keywords = {
    {"var", token_type::VAR},
    {"export", token_type::EXPORT},
    {"func", token_type::FUNCTION},
    {"if", token_type::IF},
    {"else", token_type::ELSE},
    {"while", token_type::WHILE},
    {"do", token_type::DO},
    {"for", token_type::FOR},
    {"return", token_type::RETURN},
    {"break", token_type::BREAK},
    {"continue", token_type::CONTINUE},
    {"none", token_type::NONE},

    {"true", token_type::TRUE},
    {"false", token_type::FALSE},
    {"and", token_type::AND},
    {"or", token_type::OR},
    {"not", token_type::NOT},
};

void handle_number(lexer_data &data)
{
    // i will eventually contain the index of the last character + 1
    int i;
    char last_char = '\0';
    int start_position = data.position;

    // Loop breaks if end of file reached or non alphanumeric character reached
    for (i = data.position; i < (int)data.source.length(); i++)
    {
        char current = data.source[i];
        if ((current < '0' || current > '9') && current != '.' && current != 'e' && current != 'E')
        {
            break;
        }
        last_char = current;
    }

    if (last_char == 'e' || last_char == 'E')
    {
        write_error_message(data, "Number cannot end with 'e'");
        return;
    }

    if (last_char == '.')
    {
        write_error_message(data, "Number cannot end with '.'");
        return;
    }

    std::string substring = data.source.substr(data.position, i - data.position);

    // Update position to be after the number
    data.position = i;

    try
    {
        double d = std::stod(substring);
        data.tokens.push_back(token::number_constant_token(d, data.line_number, start_position, data.position));
    }
    catch (const std::invalid_argument &e)
    {
        write_error_message(data, "Invalid number constant '" + substring + "'");
    }
    catch (const std::out_of_range &e)
    {
        write_error_message(data, "Number constant out of range");
    }
}

void handle_string_constant(lexer_data &data)
{
    int i;
    int start_position = data.position;
    bool escape = false;
    std::string substring;

    // Loop breaks if end of file reached or closing quote reached
    for (i = data.position + 1; i < (int)data.source.length(); i++)
    {
        char current = data.source[i];
        if (current == '\r' || current == '\n' || current == '\0')
        {
            data.position = i;
            write_error_message(data, "Unterminated string constant");
            return;
        }

        // End of string
        if (current == '"' && !escape)
        {
            break;
        }

        // Escape handling
        if (current == '\\')
        {
            if (escape)
            {
                substring += '\\';
                escape = false;
            }
            else
            {
                escape = true;
            }
        }
        else if (current == 'n' && escape)
        {
            substring += '\n';
            escape = false;
        }
        else if (current == 't' && escape)
        {
            substring += '\t';
            escape = false;
        }
        else if (current == 'r' && escape)
        {
            substring += '\r';
            escape = false;
        }
        else if (current == '"' && escape)
        {
            substring += '"';
            escape = false;
        }
        else
        {
            // If the current character is not an escape character [n, t, r, "] then the backslash needs to be added
            if (escape)
            {
                substring += '\\';
            }
            substring += current;
            escape = false;
        }
    }

    if (i == (int)data.source.length())
    {
        data.position = i;
        write_error_message(data, "Unterminated string constant");
        return;
    }

    // Update position to be after the closing quote
    data.position = i + 1;

    data.tokens.push_back(token::string_constant_token(new std::string(std::move(substring)), data.line_number, start_position, data.position));
}

void handle_identifier_or_keyword(lexer_data &data)
{
    int i;
    int start_position = data.position;

    // Loop breaks if end of file reached or non alphanumeric character reached
    for (i = data.position; i < (int)data.source.length(); i++)
    {
        char current = data.source[i];
        if ((current < 'a' || current > 'z') && (current < 'A' || current > 'Z') && (current < '0' || current > '9') && current != '_')
        {
            break;
        }
    }
    std::string substring = data.source.substr(data.position, i - data.position);
    data.position = i;

    if (keywords.contains(substring)) // Keyword
    {
        data.tokens.push_back(token(keywords.at(substring), data.line_number, start_position, data.position));
    }
    else // Identifier
    {
        data.tokens.push_back(token::identifier_token(new std::string(std::move(substring)), data.line_number, start_position, data.position));
    }
}

bool handle_symbolic_token(lexer_data &data)
{
    // Pointer to current trie node
    const token_trie_node *current_node = &token_trie;
    int start_position = data.position;

    int i;

    for (i = data.position; i < (int)data.source.length(); i++)
    {
        char current_char = data.source[i];
        if (current_node->matches.contains(current_char)) // Match success (check next charater in next iteration)
        {
            current_node = &current_node->matches.at(current_char);
        }
        else
        {
            break;
        }
    }

    // Regardless of end of string reached or non matching character found:
    if (current_node->default_token.has_value())
    {
        token_type type = current_node->default_token.value();
        data.tokens.push_back(token(type, data.line_number, start_position, i));
        data.position = i;
        return true;
    }
    else // Token failed to match
    {
        return false;
    }
}

bool handle_comment(lexer_data &data)
{
    char c = data.source[data.position];
    if (c == '/')
    {
        if (data.position + 1 < (int)data.source.length())
        {
            char second = data.source[data.position + 1];
            if (second == '/') // Single line comment
            {

                // Skip characters until \n reached or end of file.
                for (int i = data.position + 2; i < (int)data.source.length(); i++)
                {
                    if (data.source[i] == '\n')
                    {
                        data.line_number++;
                        data.line_start_position = i + 1;
                        data.position = i + 1; // Update position to be after the comment
                        return true;
                    }
                }
                data.position = data.source.length();
                return true;
            }
            else if (second == '*') // Multi line comment
            {
                // Skip characters until */ reached.
                bool star = false; // True if previous character was a "*"
                int i;
                for (i = data.position + 2; i < (int)data.source.length(); i++)
                {
                    if (star && data.source[i] == '/') // Previous was * and this character is /
                    {
                        break;
                    }
                    if (data.source[i] == '\n')
                    {
                        data.line_number++;
                        data.line_start_position = i + 1;
                    }
                    star = data.source[i] == '*';
                }
                data.position = i + 1;
                if (i >= (int)data.source.length())
                {
                    write_error_message(data, "Unclosed /* comment");
                }
                return true;
            }
        }
    }
    return false;
}

std::optional<std::vector<token>> tokenize(const std::string &source, std::ostream &error_stream)
{
    std::vector<token> tokens;

    lexer_data data = {1, 0, 0, source, tokens, error_stream, false};

    while (data.position < (int)data.source.length())
    {

        char c = data.source[data.position];

        // Number constant
        if (c >= '0' && c <= '9')
        {
            handle_number(data);
            continue;
        }

        // Identifiers (can contain, but must not start with a number)
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
        {
            handle_identifier_or_keyword(data);
            continue;
        }

        if (handle_comment(data))
        {
            continue;
        }

        if (handle_symbolic_token(data))
        {
            continue;
        }

        // The rest of the tokens
        switch (c)
        {

        case '"':
            handle_string_constant(data);
            break;

            // Whitespace
        case ' ':
        case '\t':
        case '\r':
            data.position++;
            break;

        case '\n':
            data.line_number++;
            data.position++;
            data.line_start_position = data.position;
            break;

            // Unexpected character
        default:
            write_error_message(data, std::string("Unexpected character '") + c + "'");
            data.position++;
            break;
        }
    }

    if (data.error_found)
    {
        return std::nullopt;
    }

    return std::make_optional(std::move(data.tokens));
}

void write_error_message(lexer_data &data, const std::string &message)
{
    int line_start = data.line_start_position;
    int line_end = data.source.find('\n', line_start);
    std::string line_text = data.source.substr(line_start, line_end - line_start);

    data.error_stream << "Error at line " << data.line_number << ": " << message << std::endl;
    data.error_stream << line_text << std::endl;
    data.error_stream << std::string(data.position - line_start, ' ') << "^" << std::endl;
    data.error_found = true;
}

void write_tokens(const std::vector<token> &tokens, std::ostream &out)
{
    for (const token &tok : tokens)
    {
        out << tok.to_string() << ' ';
        if (tok.get_type() == token_type::END_STATEMENT || tok.get_type() == token_type::LEFT_BRACE || tok.get_type() == token_type::RIGHT_BRACE)
        {
            out << std::endl;
        }
    }
    out << std::endl;
}

} // namespace lentus
