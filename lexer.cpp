#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <memory>
#include <variant>
#include "lexer.h"

Lexer::Lexer(const std::string& s) : src(s) {
	// 获取全部的start对于的行号
	size_t i = 0;
	size_t index = 1;
	while (i < src.size()) {
		if (src[i] == '\n') {
			str2line.push_back(i);
			//str2line[i] = index;
			//index++;
		}
		i++;
	}
}
size_t Lexer::getline(size_t start) {

	for (size_t i = 0; i < str2line.size(); i++)
	{
		if (start <= str2line[i]) {
			return i + 1;
		}
	}
	return 0;
}
Token Lexer::next() {
	// skip whitespace
	while (pos < src.size() && std::isspace((unsigned char)src[pos])) pos++;
	if (pos >= src.size()) return { TokenType::END, "", getline(pos) };

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
		return { TokenType::STRING, str , getline(start) };
	}
	// 关键字
	if (std::isalpha((unsigned char)c) || c == '_') {
		size_t start = pos;
		while (pos < src.size() && (std::isalnum((unsigned char)src[pos]) || src[pos] == '_')) pos++;
		std::string word = src.substr(start, pos - start);
		if (word == "let")   return { TokenType::LET, word , getline(start) };
		if (word == "print") return { TokenType::PRINT, word , getline(start) };
		if (word == "if")    return { TokenType::IF, word , getline(start) };
		if (word == "else")  return { TokenType::ELSE, word , getline(start) };
		if (word == "for")   return { TokenType::FOR, word, getline(start) };
		if (word == "func")  return { TokenType::FUNC, word, getline(start) };
		if (word == "return") return { TokenType::RETURN, word , getline(start) };
		return { TokenType::IDENT, word, getline(start) };
	}

	// 数字
	if (std::isdigit((unsigned char)c)) {
		size_t start = pos;
		while (pos < src.size() && std::isdigit((unsigned char)src[pos])) pos++;
		return { TokenType::NUMBER, src.substr(start, pos - start), getline(start) };
	}

	// 两个字符的操作符
	if (c == '=' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::EQ,"==" , getline(pos) }; }
	if (c == '!' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::NEQ,"!=" , getline(pos) }; }
	if (c == '<' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::LE,"<=" , getline(pos) }; }
	if (c == '>' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::GE,">=", getline(pos) }; }

	if (c == '+' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::PLUS_ASSIGN,"+=" , getline(pos) }; }
	if (c == '-' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::MINUS_ASSIGN,"-=" , getline(pos) }; }
	if (c == '*' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::STAR_ASSIGN,"*=" , getline(pos) }; }
	if (c == '/' && pos + 1 < src.size() && src[pos + 1] == '=') { pos += 2; return { TokenType::SLASH_ASSIGN,"/=" , getline(pos) }; }

	// 单个token
	pos++;
	switch (c) {
	case '+': return { TokenType::PLUS, "+" , getline(pos) };
	case '-': return { TokenType::MINUS, "-", getline(pos) };
	case '*': return { TokenType::STAR, "*" , getline(pos) };
	case '/': return { TokenType::SLASH, "/", getline(pos) };
	case '%': return { TokenType::PERCENT, "%", getline(pos) };
	case '=': return { TokenType::ASSIGN, "=", getline(pos) };
	case '<': return { TokenType::LT, "<", getline(pos) };
	case '>': return { TokenType::GT, ">", getline(pos) };
	case '(': return { TokenType::LPAREN, "(" , getline(pos) };
	case ')': return { TokenType::RPAREN, ")" , getline(pos) };
	case '{': return { TokenType::LBRACE, "{", getline(pos) };
	case '}': return { TokenType::RBRACE, "}", getline(pos) };
	case ';': return { TokenType::SEMICOLON, ";", getline(pos) };
	case ',': return { TokenType::COMMA, ",", getline(pos) };
	}
	throw std::runtime_error("unknown character: " + std::string(1, c));
}