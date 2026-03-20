#include "common.hpp"
#include "AST.hpp"
#include <fstream>
#include <iostream>

void compile(const std::string &);

int main(int argc, char **argv)
{
	DBG_PRINT("debug output enabled\n");
	if (argc < 2)
	{
		std::cerr << "Usage: sasc-compiler <file>" << std::endl;
		return 1;
	}

	std::string filename = argv[1];
	DBG_PRINT("file: %s\n", filename.c_str());

	try
	{
		compile(filename);
	}
	catch (CompileError e)
	{
		std::cerr << "\n\nCompilation failed: " << e.what() << "\n";
		return 1;
	}
	catch (std::exception e)
	{
		std::cerr << "\n\nException occured: " << e.what() << "\n";
		return 2;
	}

	return 0;
}

void compile(const std::string &filename)
{
	DBG_PRINT("starting compilation\n");
	std::ifstream file(filename);
	if (!file.is_open())
		throw std::runtime_error(std::format("Failed to open file \"{}\"", filename));

	Lexer lexer(file);
	AST ast(lexer);

	DBG_PRINT("compilation complete\n");
}

void print_error(const std::string &msg, SourceLoc start, SourceLoc end)
{
	std::cerr << "ERROR: " << msg << "\n\tat line " << start.line << ", col " << start.col << "\n";
	(void)end; // suppress unused parameter warning
}