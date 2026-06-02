#ifndef LENTUS_TOKEN_HPP
#define LENTUS_TOKEN_HPP

#include <string>
#include <optional>

/**
 * Type of token
 */
enum class token_type : int
{
    NONE, /** none */

    // Data tokens
    IDENTIFIER,
    STRING,
    NUMBER,

    // Non-data tokens

    // Operators
    END_STATEMENT, /** ; */
    ADD,           /** \+ */
    SUBTRACT,      /** \- */
    MULTIPLY,      /** * */
    DIVIDE,        /** / */
    DOT,           /** . */
    COMMA,         /** , */

    // Brackets
    LEFT_BRACKET,  /** ( */
    RIGHT_BRACKET, /** ) */
    LEFT_BRACE,    /** { */
    RIGHT_BRACE,   /** } */
    LEFT_INDEX,    /** [ */
    RIGHT_INDEX,   /** ] */

    // Comparisons
    EQUAL,         /** ==  (equality) */
    NOT_EQUAL,     /** != (inequality) */
    LESS_EQUAL,    /** <= */
    GREATER_EQUAL, /** >= */
    LESS,          /** < */
    GREATER,       /** > */

    // Assigments
    ASSIGN,          /** = (assignment) */
    ADD_ASSIGN,      /** += */
    SUBTRACT_ASSIGN, /** -= */
    MULTIPLY_ASSIGN, /** *= */
    DIVIDE_ASSIGN,   /** /= */
    INCREMENT,       /** ++ */
    DECREMENT,       /** -- */

    // Keywords
    VAR,      /** var */
    EXPORT,   /** export */
    FUNCTION, /** func */
    IF,       /** if */
    ELSE,     /** else */
    WHILE,    /** while */
    DO,       /** do */
    FOR,      /** for */
    RETURN,   /** return */
    BREAK,    /** break */
    CONTINUE, /** continue */

    // Boolean keywords
    TRUE,  /** true */
    FALSE, /** false */
    AND,   /** and (also "&&") */
    OR,    /** or (also "||") */
    NOT,   /** not (also "!") */
};

class token
{
private:
    token_type type;
    int line_number;    /** Line number where token appears */
    int start_position; /** Index where token starts (inclusive) */
    int end_position;   /** Index where token ends (exclusive) */

    union
    {
        std::string *identifier_name; /** for IDENTIFIER tokens */
        std::string *string_constant; /** for STRING tokens */
        double number_constant;       /** for NUMBER tokens */
    };

public:
    token() : type(token_type::NONE), line_number(0), start_position(0), end_position(0) {}
    token(token_type type, int line_number, int start_position, int end_position) : type(type), line_number(line_number), start_position(start_position), end_position(end_position) {};
    token(const token &tok);
    token(token &&tok);

    token &operator=(const token &tok);
    token &operator=(token &&tok);

private:
    token(token_type type, int line_number, int start_position, int end_position, std::string &&string_data);
    token(token_type type, int line_number, int start_position, int end_position, double number_constant);

public:
    static token identifier_token(std::string *identifier, int line_number, int start_position, int end_position);

    static token string_constant_token(std::string *string_constant, int line_number, int start_position, int end_position);
    static token number_constant_token(double number_constant, int line_number, int start_position, int end_position);

    /** Convert the token to a enum name representation for debugging */
    std::string to_string() const;

    /**
     * Uses the position data of this token to extract a substring from the source string
     * @param source The source string
     * @return The substring corresponding to the token's position
     */
    std::string to_string(const std::string &source) const;

    /**
     * Get the type of the token
     * @return the type of the token
     */
    token_type get_type() const { return type; }

    /**
     * Get the line number of the token
     * @return the line number of the token
     */
    int get_line_number() const { return line_number; }

    /**
     * Get the start position of the token in the source string
     * @return the start position of the token in the source string
     */
    int get_start_position() const { return start_position; }

    /**
     * Get the end position of the token in the source string
     * @return the end position of the token in the source string
     */
    int get_end_position() const { return end_position; }

    /**
     * Get the identifier name of the token, if the token is an identifier token
     * @return the identifier name of the token, or nullptr if the token is not an identifier token
     */
    const std::string *get_identifier_name() const
    {
        if (type == token_type::IDENTIFIER)
        {
            return identifier_name;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Get the string constant of the token, if the token is a string constant token
     * @return the string constant of the token, or nullptr if the token is not a string constant token
     */
    const std::string *get_string_constant() const
    {
        if (type == token_type::STRING)
        {
            return string_constant;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Get the number constant of the token, if the token is a number constant token
     * @return the number constant of the token, or 0 if the token is not a number constant token
     */
    double get_number_constant() const
    {
        if (type == token_type::NUMBER)
        {
            return number_constant;
        }
        else
        {
            return 0;
        }
    }

    ~token();
};

std::string token_type_to_string(token_type type);

#endif