#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <optional>

enum class token_type : int
{
    // Data tokens
    IDENTIFIER,
    STRING,
    NUMBER,

    END_STATEMENT, /** ; */
    ADD,           /** \+ */
    SUBTRACT,      /** \- */
    MULTIPLY,      /** * */
    DIVIDE,        /** / */
    DOT,           /** . */

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
    int line_number;
    union
    {
        std::string *identifier_name;
        std::string *string_constant;
        double number_constant;
    };

public:
    token(token_type type, int line_number);
    token(const token &tok);
    token(token &&tok);

    token &operator=(const token &tok);
    token &operator=(token &&tok);

private:
    token(token_type type, int line_number, std::string &&string_data);
    token(token_type type, int line_number, double number_constant);

public:
    static token identifier_token(std::string *identifier, int line_number);

    static token string_constant_token(std::string *string_constant, int line_number);
    static token number_constant_token(double number_constant, int line_number);

    std::string to_string() const;

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

#endif