#include "lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& s) : src(s) {}

Token Lexer::next() {
	// skip whitespace
	while (pos < src.size() && std::isspace((unsigned char)src[pos])) pos++;
	if (pos >= src.size()) return { TokenType::END, "" };

	char c = src[pos];
	if (c == '"') {
		pos++;
		size_t start = pos;
		while (pos < src.size() && src[pos] != '"') {
			if (src[pos] == '\\' && pos + 1 < src.size()) pos++; // 支持转义
			pos++;
		}
		if (pos >= src.size()) throw std::runtime_error("unterminated string literal");
		std::string str = src.substr(start, pos - start);
		pos++; // consume closing "
		return { TokenType::STRING, str };
	}
	// 关键字
	if (std::isalpha((unsigned char)c) || c == '_') {
		size_t start = pos;
		while (pos < src.size() && (std::isalnum((unsigned char)src[pos]) || src[pos] == '_')) pos++;
		std::string word = src.substr(start, pos - start);
		if (word == "let")   return { TokenType::LET, word };
		if (word == "print") return { TokenType::PRINT, word };
		if (word == "if")    return { TokenType::IF, word };
		if (word == "else")  return { TokenType::ELSE, word };
		if (word == "for")   return { TokenType::FOR, word };
		if (word == "func")  return { TokenType::FUNC, word };
		if (word == "return") return { TokenType::RETURN, word };
		return { TokenType::IDENT, word };
	}

	// 数字
	if (std::isdigit((unsigned char)c)) {
		size_t start = pos;
		while (pos < src.size() && std::isdigit((unsigned char)src[pos])) pos++;
		return { TokenType::NUMBER, src.substr(start, pos - start) };
	}

	// 两个字符的操作符
	if (c == '=' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::EQ,"==" }; }
	if (c == '!' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::NEQ,"!=" }; }
	if (c == '<' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::LE,"<=" }; }
	if (c == '>' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::GE,">=" }; }

	if (c == '+' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::PLUS_ASSIGN,"+=" }; }
	if (c == '-' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::MINUS_ASSIGN,"-=" }; }
	if (c == '*' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::STAR_ASSIGN,"*=" }; }
	if (c == '/' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::SLASH_ASSIGN,"/=" }; }

	// 单个token
	pos++;
	switch (c) {
	case '+': return { TokenType::PLUS, "+" };
	case '-': return { TokenType::MINUS, "-" };
	case '*': return { TokenType::STAR, "*" };
	case '/': return { TokenType::SLASH, "/" };
	case '%': return { TokenType::PERCENT, "%" };
	case '=': return { TokenType::ASSIGN, "=" };
	case '<': return { TokenType::LT, "<" };
	case '>': return { TokenType::GT, ">" };
	case '(': return { TokenType::LPAREN, "(" };
	case ')': return { TokenType::RPAREN, ")" };
	case '{': return { TokenType::LBRACE, "{" };
	case '}': return { TokenType::RBRACE, "}" };
	case ';': return { TokenType::SEMICOLON, ";" };
	case ',': return { TokenType::COMMA, "," };
	}
	throw std::runtime_error("unknown character: " + std::string(1, c));
}