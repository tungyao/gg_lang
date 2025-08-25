#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <memory>
#include <variant>

using Value = std::variant<int, std::string>;
// ------------------ Tokenizer ------------------
enum class TokenType {
	LET, PRINT, IF, ELSE, FOR,

	IDENT, NUMBER, STRING,

	PLUS, MINUS, STAR, SLASH, PERCENT,
	ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, STAR_ASSIGN, SLASH_ASSIGN,
	EQ, NEQ, LT, GT, LE, GE,
	LPAREN, RPAREN, LBRACE, RBRACE,
	SEMICOLON,
	END
};

struct Token {
	TokenType type;
	std::string text;
};

class Lexer {
	std::string src;
	size_t pos = 0;

public:
	Lexer(const std::string& s) : src(s) {}

	Token next() {
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
		}
		return { TokenType::END, "" };
	}
};

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

struct Stmt { virtual ~Stmt() {} };
struct BlockStmt : Stmt { std::vector<std::unique_ptr<Stmt>> stmts; };
struct PrintStmt : Stmt { std::unique_ptr<Expr> expr; explicit PrintStmt(std::unique_ptr<Expr> e) :expr(std::move(e)) {} };
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


class Parser {
	Lexer& lexer;
	Token cur;

	void advance() { cur = lexer.next(); }
	bool match(TokenType t) { if (cur.type == t) { advance(); return true; } return false; }

	std::unique_ptr<Expr> parsePrimary() {
		if (cur.type == TokenType::NUMBER) {
			int val = std::stoi(cur.text); advance();
			return std::make_unique<NumberExpr>(val);
		}
		if (cur.type == TokenType::IDENT) {
			std::string name = cur.text; advance();
			return std::make_unique<VarExpr>(name);
		}
		if (cur.type == TokenType::STRING) {
			std::string s = cur.text; advance();
			return std::make_unique<StringExpr>(s);
		}
		if (match(TokenType::LPAREN)) {
			auto e = parseExpr();
			if (!match(TokenType::RPAREN)) throw std::runtime_error("expected )");
			return e;
		}
		throw std::runtime_error("unexpected token in primary");
	}

	std::unique_ptr<Expr> parseTerm() {
		auto left = parsePrimary();
		while (cur.type == TokenType::STAR || cur.type == TokenType::SLASH || cur.type == TokenType::PERCENT) {
			std::string op = cur.text; advance();
			auto right = parsePrimary();
			left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
		}
		return left;
	}

	std::unique_ptr<Expr> parseAdd() {
		auto left = parseTerm();
		while (cur.type == TokenType::PLUS || cur.type == TokenType::MINUS) {
			std::string op = cur.text; advance();
			auto right = parseTerm();
			left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
		}
		return left;
	}

	std::unique_ptr<Expr> parseCmp() {
		auto left = parseAdd();
		while (cur.type == TokenType::EQ || cur.type == TokenType::NEQ ||
			cur.type == TokenType::LT || cur.type == TokenType::GT ||
			cur.type == TokenType::LE || cur.type == TokenType::GE) {
			std::string op = cur.text; advance();
			auto right = parseAdd();
			left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
		}
		return left;
	}

	std::unique_ptr<Expr> parseAssign() {
		auto left = parseCmp();
		if (cur.type == TokenType::ASSIGN ||
			cur.type == TokenType::PLUS_ASSIGN ||
			cur.type == TokenType::MINUS_ASSIGN ||
			cur.type == TokenType::STAR_ASSIGN ||
			cur.type == TokenType::SLASH_ASSIGN) {

			auto* v = dynamic_cast<VarExpr*>(left.get());
			if (!v) throw std::runtime_error("left of assignment must be variable");
			std::string name = v->name;
			TokenType opType = cur.type;
			advance();

			auto right = parseAssign();

			if (opType == TokenType::ASSIGN)
				return std::make_unique<AssignExpr>(name, std::move(right));

			// 转换成普通二元表达式再赋值
			std::string op;
			if (opType == TokenType::PLUS_ASSIGN) op = "+";
			if (opType == TokenType::MINUS_ASSIGN) op = "-";
			if (opType == TokenType::STAR_ASSIGN) op = "*";
			if (opType == TokenType::SLASH_ASSIGN) op = "/";

			auto varExpr = std::make_unique<VarExpr>(name);
			auto bin = std::make_unique<BinaryExpr>(op, std::move(varExpr), std::move(right));
			return std::make_unique<AssignExpr>(name, std::move(bin));
		}
		return left;
	}

	std::unique_ptr<Expr> parseExpr() { return parseAssign(); }

public:
	Parser(Lexer& l) :lexer(l) { advance(); }

	std::unique_ptr<Stmt> parseStmt() {
		if (match(TokenType::PRINT)) {
			auto e = parseExpr();
			if (!match(TokenType::SEMICOLON)) throw std::runtime_error("expected ; after print");
			return std::make_unique<PrintStmt>(std::move(e));
		}
		if (match(TokenType::LET)) {
			if (cur.type != TokenType::IDENT) throw std::runtime_error("expected identifier");
			std::string name = cur.text; advance();
			if (!match(TokenType::ASSIGN)) throw std::runtime_error("expected =");
			auto e = parseExpr();
			if (!match(TokenType::SEMICOLON)) throw std::runtime_error("expected ; after let");
			return std::make_unique<LetStmt>(name, std::move(e));
		}
		if (match(TokenType::IF)) {
			if (!match(TokenType::LPAREN)) throw std::runtime_error("expected (");
			auto cond = parseExpr();
			if (!match(TokenType::RPAREN)) throw std::runtime_error("expected )");
			auto thenStmt = parseStmt();
			std::unique_ptr<Stmt> elseStmt;
			if (match(TokenType::ELSE)) elseStmt = parseStmt();
			auto s = std::make_unique<IfStmt>();
			s->cond = std::move(cond);
			s->thenStmt = std::move(thenStmt);
			s->elseStmt = std::move(elseStmt);
			return s;
		}
		if (match(TokenType::FOR)) {
			if (!match(TokenType::LPAREN)) throw std::runtime_error("expected ( after for");
			auto init = parseStmt(); // let ... ;
			auto cond = parseExpr();
			if (!match(TokenType::SEMICOLON)) throw std::runtime_error("expected ; in for");
			auto step = parseExpr(); // i = i + 1
			if (!match(TokenType::RPAREN)) throw std::runtime_error("expected ) in for");
			auto body = parseStmt();
			auto s = std::make_unique<ForStmt>();
			s->init = std::move(init);
			s->cond = std::move(cond);
			s->step = std::move(step);
			s->body = std::move(body);
			return s;
		}
		if (match(TokenType::LBRACE)) {
			auto block = std::make_unique<BlockStmt>();
			while (cur.type != TokenType::RBRACE && cur.type != TokenType::END) {
				auto st = parseStmt();
				if (!st) throw std::runtime_error("invalid statement in block");
				block->stmts.push_back(std::move(st));
			}
			if (!match(TokenType::RBRACE)) throw std::runtime_error("expected }");
			return block;
		}
		if (cur.type == TokenType::IDENT) {
			auto expr = parseExpr();   // parseAssign 会处理 = / += / -= / *= / /=

			// 确保这是赋值表达式
			if (auto a = dynamic_cast<AssignExpr*>(expr.get())) {
				if (!match(TokenType::SEMICOLON)) throw std::runtime_error("expected ; after assignment");
				return std::make_unique<AssignStmt>(std::unique_ptr<AssignExpr>(static_cast<AssignExpr*>(expr.release())));
			}

			throw std::runtime_error("expression statements not supported except assignment");
		}

		// 允许空语句（在某些场景下容错）
		if (cur.type == TokenType::SEMICOLON) { advance(); return std::make_unique<BlockStmt>(); }
		return nullptr;
	}
};

// ------------------ Interpreter ------------------
class Interpreter {
	std::unordered_map<std::string, Value> vars;

	Value  eval(Expr* e) {
		if (auto n = dynamic_cast<NumberExpr*>(e)) return n->value;
		if (auto v = dynamic_cast<VarExpr*>(e)) {
			if (vars.count(v->name)) return vars[v->name];
			throw std::runtime_error("undefined variable: " + v->name);
		}
		if (auto s = dynamic_cast<StringExpr*>(e)) {
			return s->value;  // 直接返回字符串，不打印
		}
		if (auto a = dynamic_cast<AssignExpr*>(e)) {
			Value  val = eval(a->value.get());
			vars[a->name] = val;
			return val;
		}
		if (auto b = dynamic_cast<BinaryExpr*>(e)) {
			Value  l = eval(b->left.get()), r = eval(b->right.get());
			if (b->op == "+") {
				if (std::holds_alternative<int>(l) && std::holds_alternative<int>(r))
					return std::get<int>(l) + std::get<int>(r);
				return std::get<std::string>(l) + std::get<std::string>(r); // 简单转 string
			}
			if (b->op == "-")  return std::get<int>(l) - std::get<int>(r);
			if (b->op == "*")  return std::get<int>(l) * std::get<int>(r);
			if (b->op == "/")  return std::get<int>(l) / std::get<int>(r);
			if (b->op == "%")  return std::get<int>(l) % std::get<int>(r);
			if (b->op == "==") return std::get<int>(l) == std::get<int>(r);
			if (b->op != "!=" && b->op != "<" && b->op != ">" && b->op != "<=" && b->op != ">=") {
				throw std::runtime_error("unknown operator: " + b->op);
			}
			if (b->op == "!=") return std::get<int>(l) != std::get<int>(r);
			if (b->op == "<")  return std::get<int>(l) < std::get<int>(r);
			if (b->op == ">")  return std::get<int>(l) > std::get<int>(r);
			if (b->op == "<=") return std::get<int>(l) <= std::get<int>(r);
			if (b->op == ">=") return std::get<int>(l) >= std::get<int>(r);
		}
		return 0;
	}

public:
	void exec(Stmt* s) {
		if (auto p = dynamic_cast<PrintStmt*>(s)) {
			Value val = eval(p->expr.get());
			if (std::holds_alternative<int>(val))
				std::cout << std::get<int>(val) << "\n";
			else
				std::cout << std::get<std::string>(val) << "\n";
		}
		else if (auto l = dynamic_cast<LetStmt*>(s)) {
			vars[l->name] = eval(l->expr.get());
		}
		else if (auto b = dynamic_cast<BlockStmt*>(s)) {
			for (auto& stmt : b->stmts) exec(stmt.get());
		}
		else if (auto i = dynamic_cast<IfStmt*>(s)) {
			Value val = eval(i->cond.get());

			if (std::get<int>(val)) exec(i->thenStmt.get());
			else if (i->elseStmt)    exec(i->elseStmt.get());
		}
		else if (auto f = dynamic_cast<ForStmt*>(s)) {
			exec(f->init.get());
			while (std::get<int>(eval(f->cond.get()))) {
				exec(f->body.get());
				(void)eval(f->step.get()); // step 是表达式（如 i = i + 1）
			}
		}
		else if (auto a = dynamic_cast<AssignStmt*>(s)) {
			eval(a->assign.get());
		}
	}
};

#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
	std::string filename;

	if (argc < 2) {
		filename = "script.gg";
	}
	else {
		filename = argv[1];
	}

	std::ifstream file(filename);
	if (!file) { std::cerr << "Cannot open file: " << filename << "\n"; return 1; }

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string code = buffer.str();
	try {
		Lexer lexer(code);
		Parser parser(lexer);
		Interpreter interp;

		while (true) {
			auto stmt = parser.parseStmt();
			if (!stmt) break;
			interp.exec(stmt.get());
		}
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
	}
}
