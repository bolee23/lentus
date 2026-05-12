#ifndef LENTUS_LEXER_HPP
#define LENTUS_LEXER_HPP

#include <string>
#include <vector>
#include <ostream>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include "token.hpp"

/**
 * Trie node for tokenizing multi-character tokens (e.g. ==, <=, >=, etc.)
 * The default_token is the token type to return if no match is found for the next character
 */
struct token_trie_node
{
    std::optional<token_type> default_token;           /** Token corresponding to this node (if there is one) */
    std::unordered_map<char, token_trie_node> matches; /** Map of characters to their corresponding trie nodes */

    token_trie_node(std::optional<token_type> default_token) : default_token(default_token) {}
    token_trie_node(std::optional<token_type> default_token, std::unordered_map<char, token_trie_node> matches) : default_token(default_token), matches(matches) {}
};

/**
 * Struct containing the state of the lexer while tokenizing
 */
struct lexer_data
{
    // Line tracking for error reporting
    int line_number;         /** Starts at 1 */
    int line_start_position; /** Position of the first character of the current line */
    int position;            /** Current position of the lexer into source */

    const std::string &source; /** Lentus source string */
    std::vector<token> &tokens;
    std::ostream &error_stream; /** Error stream */
    bool error_found = false;   /** True if at least one error found */
};

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
 * Handles an identifier or keyword (sequence of letters, digits, and underscores, not starting with a digit)
 * @param data current state of the lexer
 */
void handle_identifier_or_keyword(lexer_data &data);

/**
 * Handles a symbolic token (e.g. ==, <=, >=, +, -, etc.)
 * @param data current state of the lexer
 * @return true if a symbolic token was found and handled, false otherwise
 */
bool handle_symbolic_token(lexer_data &data);

/**
 * Handles single and multi line
 * @param data current state of the lexer
 * @return true if a comment was found and handled, false otherwise
 */
bool handle_comment(lexer_data &data);

/**
 * Tokenizes a raw code unit into a lexed code unit
 * @param source the source code to tokenize
 * @param error_stream the stream to write error messages to
 * @return optional containing a vector of tokens if tokenization succeeded, or nullopt if an error was found
 */
std::optional<std::vector<token>> tokenize(const std::string &source, std::ostream &error_stream);

/**
 * Writes an error message to the error stream of the lexer data
 * including the line text and line number, then sets the error_found flag to true
 * @param data current state of the lexer
 * @param message error message to write
 */
void write_error_message(lexer_data &data, const std::string &message);

/**
 * Writes all tokens to the specified output stream (for debugging purposes)
 * @param tokens the tokens to write
 * @param out the output stream to write to
 */
void write_tokens(const std::vector<token> &tokens, std::ostream &out);

#endif