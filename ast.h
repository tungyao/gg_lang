#ifndef AST_H
#define AST_H

#include <vector>
#include <string>
#include <memory>
#include <variant>

using Value = std::variant<int, std::string>;

// ------------------ AST ------------------
struct Expr { virtual ~Expr() {} };
struct NumberExpr : Expr { int value; explicit NumberExpr(int v) :value(v) {} };
struct VarExpr : Expr { std::string name; explicit VarExpr(std::string n) :name(std::move(n)) {} };
struct BinaryExpr : Expr {
	std::string op; std::unique_ptr<Expr> left, right;
	BinaryExpr(std::string o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
		: op(std::move(o)), left(std::move(l)), right(std::move(r)) {
	}
};
struct AssignExpr : Expr {
	std::string name; std::unique_ptr<Expr> value;
	AssignExpr(std::string n, std::unique_ptr<Expr> v)
		: name(std::move(n)), value(std::move(v)) {
	}
};
struct StringExpr : Expr {
	std::string value;
	explicit StringExpr(std::string v) : value(std::move(v)) {}
};
struct CallExpr : Expr {
	std::string name;
	std::vector<std::unique_ptr<Expr>> args;
	CallExpr(std::string n, std::vector<std::unique_ptr<Expr>> a) : name(std::move(n)), args(std::move(a)) {}
};

struct Stmt { virtual ~Stmt() {} };
struct BlockStmt : Stmt { std::vector<std::unique_ptr<Stmt>> stmts; };
struct PrintStmt : Stmt { std::vector<std::unique_ptr<Expr>> exprs; explicit PrintStmt(std::vector<std::unique_ptr<Expr>> e) :exprs(std::move(e)) {} };
struct LetStmt : Stmt {
	std::string name; std::unique_ptr<Expr> expr;
	LetStmt(std::string n, std::unique_ptr<Expr> e) :name(std::move(n)), expr(std::move(e)) {}
};
struct IfStmt : Stmt { std::unique_ptr<Expr> cond; std::unique_ptr<Stmt> thenStmt, elseStmt; };
struct ForStmt : Stmt {
	std::unique_ptr<Stmt> init;      // let 语句
	std::unique_ptr<Expr> cond;      // 表达式
	std::unique_ptr<Expr> step;      // 赋值表达式
	std::unique_ptr<Stmt> body;      // 语句或块
};
struct AssignStmt : Stmt {
	std::unique_ptr<AssignExpr> assign;
	explicit AssignStmt(std::unique_ptr<AssignExpr> a) : assign(std::move(a)) {}
};
struct FunctionDefStmt : Stmt {
	std::string name;
	std::vector<std::string> params;
	std::unique_ptr<Stmt> body;
	FunctionDefStmt(std::string n, std::vector<std::string> p, std::unique_ptr<Stmt> b)
		: name(std::move(n)), params(std::move(p)), body(std::move(b)) {
	}
};
struct ReturnStmt : Stmt {
	std::unique_ptr<Expr> expr;
	explicit ReturnStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
};
struct ExprStmt : Stmt {
	std::unique_ptr<Expr> expr;
	explicit ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
};

#endif // AST_H