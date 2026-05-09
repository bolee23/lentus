#ifndef LENTUS_LEXER_HPP
#define LENTUS_LEXER_HPP

#include <string>
#include <vector>
#include <ostream>
#include <filesystem>
#include <optional>
#include "token.hpp"
#include "loader.hpp"

struct lexed_code_unit
{
    std::vector<token> tokens;
    std::filesystem::path path;
};

struct lexer_data
{
    int line_number;
    int position;              /** Current position of the lexer into source */
    const std::string &source; /** Lentus source string */
    std::vector<token> &tokens;
    std::ostream &error_stream; /** Error stream */
    bool error_found = false;   /** True if at least one error found */
};

/**
 * Tests if the current character matches an expected character, and advances
 * the position of the character
 * @param data current state of the lexer
 * @param expected character to check for
 * @return true if the character at the current position matches
 */
bool expect(lexer_data &data, char expected);

/**
 * Tests if the current character matches an expected character
 * @param data current state of the lexer
 * @param expected character to check for
 * @return true if the character at the current position matches
 */
bool peek(lexer_data &data, char expected);

/**
 * Handles an alphanumeric sequence of numbers
 * @param data current state of the lexer
 */
void handle_number(lexer_data &data);

/**
 * Handles a string constant (enclosed in double quotes)
 * @param data current state of the lexer
 */
void handle_string_constant(lexer_data &data);

/**
 * Handles an identifier (sequence of letters, digits, and underscores, not starting with a digit)
 * @param data current state of the lexer
 */
void handle_identifier(lexer_data &data);

/**
 * Tokenizes a raw code unit into a lexed code unit
 * @param raw_unit the raw code unit to tokenize
 * @param error_stream stream to write errors to
 * @return the lexed code unit, or nullopt if an error was found
 */
std::optional<lexed_code_unit> tokenize(const raw_code_unit &raw_unit, std::ostream &error_stream);

#endif