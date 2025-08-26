#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <cctype>

enum class TokenType {
    // Operators and Punctuation
	LET, PRINT, IF, ELSE, FOR, FUNC, RETURN,

	IDENT, NUMBER, STRING,

	PLUS, MINUS, STAR, SLASH, PERCENT,
	ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, STAR_ASSIGN, SLASH_ASSIGN,
	EQ, NEQ, LT, GT, LE, GE,
	LPAREN, RPAREN, LBRACE, RBRACE,
	SEMICOLON, COMMA,
	END
};

struct Token {
    TokenType type;
    std::string text;
    int line;
    int column;
};

class Lexer {
public:
    Lexer(const std::string& source);
    Token next();

private:
    std::string src;
    int start;
    int current;
    int line;
    int column;
	size_t pos = 0;


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