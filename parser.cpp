#include "parser.h"
#include <stdexcept>
#include <utility>

Parser::Parser(Lexer& l) :lexer(l) { advance(); }

void Parser::advance() { cur = lexer.next(); }

bool Parser::match(TokenType t) {
    if (cur.type == t) {
        advance();
        return true;
    }
    return false;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (cur.type == TokenType::NUMBER) {
        int val = std::stoi(cur.text); advance();
        return std::make_unique<NumberExpr>(val);
    }
    if (cur.type == TokenType::STRING) {
        std::string s = cur.text; advance();
        return std::make_unique<StringExpr>(s);
    }
    if (cur.type == TokenType::IDENT) {
        std::string name = cur.text; advance();
        if (match(TokenType::LPAREN)) {
            std::vector<std::unique_ptr<Expr>> args;
            if (!match(TokenType::RPAREN)) {
                do {
                    args.push_back(parseExpr());
                } while (match(TokenType::COMMA));
                if (!match(TokenType::RPAREN)) throw std::runtime_error("expected )");
            }
            return std::make_unique<CallExpr>(name, std::move(args));
        }
        return std::make_unique<VarExpr>(name);
    }
    if (match(TokenType::LPAREN)) {
        auto e = parseExpr();
        if (!match(TokenType::RPAREN)) throw std::runtime_error("expected )");
        return e;
    }
    throw std::runtime_error("unexpected token in primary");
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto left = parsePrimary();
    while (cur.type == TokenType::STAR || cur.type == TokenType::SLASH || cur.type == TokenType::PERCENT) {
        std::string op = cur.text; advance();
        auto right = parsePrimary();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseAdd() {
    auto left = parseTerm();
    while (cur.type == TokenType::PLUS || cur.type == TokenType::MINUS) {
        std::string op = cur.text; advance();
        auto right = parseTerm();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseCmp() {
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

std::unique_ptr<Expr> Parser::parseAssign() {
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

std::unique_ptr<Expr> Parser::parseExpr() { return parseAssign(); }

std::unique_ptr<Stmt> Parser::parseStmt() {
    if (match(TokenType::PRINT)) {
        std::vector<std::unique_ptr<Expr>> exprs;
        exprs.push_back(parseExpr());
        while (match(TokenType::COMMA)) {
            exprs.push_back(parseExpr());
        }
        if (!match(TokenType::SEMICOLON)) throw std::runtime_error("expected ; after print");
        return std::make_unique<PrintStmt>(std::move(exprs));
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
    if (match(TokenType::FUNC)) {
        if (cur.type != TokenType::IDENT) throw std::runtime_error("expected function name");
        std::string name = cur.text; advance();
        if (!match(TokenType::LPAREN)) throw std::runtime_error("expected ( after function name");
        std::vector<std::string> params;
        if (cur.type != TokenType::RPAREN) {
            do {
                if (cur.type != TokenType::IDENT) throw std::runtime_error("expected parameter name");
                params.push_back(cur.text);
                advance();
            } while (match(TokenType::COMMA));
        }
        if (!match(TokenType::RPAREN)) throw std::runtime_error("expected ) after parameters");
        auto body = parseStmt();
        return std::make_unique<FunctionDefStmt>(name, std::move(params), std::move(body));
    }
    if (match(TokenType::RETURN)) {
        auto e = parseExpr();
        if (!match(TokenType::SEMICOLON)) throw std::runtime_error("expected ; after return");
        return std::make_unique<ReturnStmt>(std::move(e));
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

        // 确保这是赋值表达式或调用表达式
        if (auto a = dynamic_cast<AssignExpr*>(expr.get())) {
            if (!match(TokenType::SEMICOLON)) throw std::runtime_error("expected ; after assignment");
            return std::make_unique<AssignStmt>(std::unique_ptr<AssignExpr>(static_cast<AssignExpr*>(expr.release())));
        }
        else if (auto c = dynamic_cast<CallExpr*>(expr.get())) {
             if (!match(TokenType::SEMICOLON)) throw std::runtime_error("expected ; after call");
            return std::make_unique<ExprStmt>(std::unique_ptr<Expr>(expr.release()));
        }
        else {
            throw std::runtime_error("expression statements not supported except assignment or call");
        }
    }

    // 允许空语句（在某些场景下容错）
    if (cur.type == TokenType::SEMICOLON) { advance(); return std::make_unique<BlockStmt>(); }
    return nullptr;
}