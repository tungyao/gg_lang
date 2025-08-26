#include "interpreter.h"
#include <iostream>
#include <stdexcept>
#include <string>

Value Interpreter::eval(Expr* e) {
    if (auto n = dynamic_cast<NumberExpr*>(e)) return n->value;
    if (auto v = dynamic_cast<VarExpr*>(e)) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto f = it->find(v->name);
            if (f != it->end()) return f->second;
        }
        throw std::runtime_error("undefined variable: " + v->name);
    }
    if (auto s = dynamic_cast<StringExpr*>(e)) {
        return s->value;
    }
    if (auto a = dynamic_cast<AssignExpr*>(e)) {
        Value val = eval(a->value.get());
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(a->name)) {
                (*it)[a->name] = val;
                return val;
            }
        }
        scopes.back()[a->name] = val;
        return val;
    }
    if (auto b = dynamic_cast<BinaryExpr*>(e)) {
        Value l = eval(b->left.get()), r = eval(b->right.get());
        if (b->op == "+") {
            if (std::holds_alternative<std::string>(l) || std::holds_alternative<std::string>(r)) {
                std::string ls = std::holds_alternative<int>(l) ? std::to_string(std::get<int>(l)) : std::get<std::string>(l);
                std::string rs = std::holds_alternative<int>(r) ? std::to_string(std::get<int>(r)) : std::get<std::string>(r);
                return ls + rs;
            }
            else {
                return std::get<int>(l) + std::get<int>(r);
            }
        }
        bool is_string = std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r);
        if (is_string) {
            const std::string& ls = std::get<std::string>(l);
            const std::string& rs = std::get<std::string>(r);
            if (b->op == "==") return ls == rs;
            if (b->op == "!=") return ls != rs;
            if (b->op == "<") return ls < rs;
            if (b->op == ">") return ls > rs;
            if (b->op == "<=") return ls <= rs;
            if (b->op == ">=") return ls >= rs;
            throw std::runtime_error("invalid operator for strings: " + b->op);
        }
        else {
            int li = std::get<int>(l), ri = std::get<int>(r);
            if (b->op == "-") return li - ri;
            if (b->op == "*") return li * ri;
            if (b->op == "/") {
                 if (ri == 0) throw std::runtime_error("division by zero");
                 return li / ri;
            }
            if (b->op == "%") {
                 if (ri == 0) throw std::runtime_error("modulo by zero");
                 return li % ri;
            }
            if (b->op == "==") return li == ri;
            if (b->op == "!=") return li != ri;
            if (b->op == "<") return li < ri;
            if (b->op == ">") return li > ri;
            if (b->op == "<=") return li <= ri;
            if (b->op == ">=") return li >= ri;
            throw std::runtime_error("unknown operator: " + b->op);
        }
    }
    if (auto c = dynamic_cast<CallExpr*>(e)) {
        auto it = funcs.find(c->name);
        if (it == funcs.end()) throw std::runtime_error("undefined function: " + c->name);
        auto& f = it->second;
        if (f->params.size() != c->args.size()) throw std::runtime_error("argument count mismatch for " + c->name);
        std::vector<Value> arg_vals;
        for (auto& arg : c->args) arg_vals.push_back(eval(arg.get()));
        push_scope();
        for (size_t i = 0; i < f->params.size(); ++i) {
            scopes.back()[f->params[i]] = arg_vals[i];
        }
        Value ret = 0; // Default return value
        try {
            exec(f->body.get());
        }
        catch (ReturnException& re) {
            ret = re.val;
        }
        pop_scope();
        return ret;
    }
    throw std::runtime_error("unknown expression type");
}

void Interpreter::exec(Stmt* s) {
    if (auto p = dynamic_cast<PrintStmt*>(s)) {
        bool first = true;
        for (auto& expr : p->exprs) {
            Value val = eval(expr.get());
            if (!first) std::cout << " ";
            first = false;
            if (std::holds_alternative<int>(val))
                std::cout << std::get<int>(val);
            else
                std::cout << std::get<std::string>(val);
        }
        std::cout << "\n";
    }
    else if (auto l = dynamic_cast<LetStmt*>(s)) {
        Value val = eval(l->expr.get());
        scopes.back()[l->name] = val;
    }
    else if (auto b = dynamic_cast<BlockStmt*>(s)) {
        push_scope();
        for (auto& stmt : b->stmts) exec(stmt.get());
        pop_scope();
    }
    else if (auto i = dynamic_cast<IfStmt*>(s)) {
        Value val = eval(i->cond.get());
        if (!std::holds_alternative<int>(val)) throw std::runtime_error("if condition must be integer");
        if (std::get<int>(val)) exec(i->thenStmt.get());
        else if (i->elseStmt) exec(i->elseStmt.get());
    }
    else if (auto f = dynamic_cast<ForStmt*>(s)) {
        push_scope();
        exec(f->init.get());
        while (true) {
            Value cond_val = eval(f->cond.get());
            if (!std::holds_alternative<int>(cond_val)) throw std::runtime_error("for loop condition must be integer");
            if (!std::get<int>(cond_val)) break;
            exec(f->body.get());
            (void)eval(f->step.get());
        }
        pop_scope();
    }
    else if (auto a = dynamic_cast<AssignStmt*>(s)) {
        eval(a->assign.get());
    }
    else if (auto fd = dynamic_cast<FunctionDefStmt*>(s)) {
        auto func = std::make_unique<Function>();
        func->params = fd->params;
        func->body = std::move(fd->body); // Transfer ownership of the body AST node
        funcs[fd->name] = std::move(func);
    }
    else if (auto r = dynamic_cast<ReturnStmt*>(s)) {
        Value val = eval(r->expr.get());
        throw ReturnException{ val };
    }
    else if (auto es = dynamic_cast<ExprStmt*>(s)) {
        eval(es->expr.get());
    }
    else if (s == nullptr) {
        // Handle null statements gracefully (e.g., from parsing empty else block)
    }
    else {
       throw std::runtime_error("unknown statement type");
    }
}

Interpreter::Interpreter() {
    push_scope(); // global scope
}

void Interpreter::push_scope() {
    scopes.emplace_back();
}

void Interpreter::pop_scope() {
    scopes.pop_back();
}