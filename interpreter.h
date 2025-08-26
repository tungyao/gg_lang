c++
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <unordered_map>
#include <vector>
#include <variant>
#include <iostream>
#include <string>
#include <memory>

using Value = std::variant<int, std::string>;

struct Function {
	std::vector<std::string> params;
	std::unique_ptr<Stmt> body;
};

struct ReturnException {
	Value val;
	explicit ReturnException(Value v) : val(v) {}
};

class Interpreter {
	std::vector<std::unordered_map<std::string, Value>> scopes;
	std::unordered_map<std::string, std::unique_ptr<Function>> funcs;

	void push_scope();
	void pop_scope();

	Value eval(Expr* e);

public:
	Interpreter(); // global scope

	void exec(Stmt* s);
};

#endif // INTERPRETER_H