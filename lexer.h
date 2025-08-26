<CODE_BLOCK>
#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <cctype>

enum class TokenType {
    // Operators and Punctuation
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQUALS, BANG, LESS, GREATER, AMPERSAND, PIPE, CARET,
    TILDE, QUESTION, COLON, SEMICOLON, COMMA, DOT,
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,

    // Keywords
    LET, RETURN, IF, ELSE, WHILE, FOR, TRUE, FALSE, NIL,

    // Literals
    IDENTIFIER, INTEGER_LITERAL, FLOAT_LITERAL, STRING_LITERAL,

    // Other
    END_OF_FILE, ILLEGAL
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

class Lexer {
public:
    Lexer(const std::string& source);

    Token nextToken();

private:
    std::string source;
    int start;
    int current;
    int line;
    int column;

    char peek();
    char peekNext();
    bool isAtEnd();
    void advance();
    Token makeToken(TokenType type);
    Token errorToken(const std::string& message);
    void skipWhitespace();
    Token identifier();
    Token number();
    Token string();
};

#endif // LEXER_H
</CODE_BLOCK>