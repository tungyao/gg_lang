#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

class Parser {
	Lexer& lexer;
	Token cur;

	void advance();
	bool match(TokenType t);

	std::unique_ptr<Expr> parsePrimary();
	std::unique_ptr<Expr> parseTerm();
	std::unique_ptr<Expr> parseAdd();
	std::unique_ptr<Expr> parseCmp();
	std::unique_ptr<Expr> parseAssign();
	std::unique_ptr<Expr> parseExpr();

public:
	Parser(Lexer& l);

	std::unique_ptr<Stmt> parseStmt();
};

#endif // PARSER_H