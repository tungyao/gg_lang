#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

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
	catch (ReturnException&) {
		std::cerr << "Error: return statement outside of function\n";
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
	}

	return 0;
}