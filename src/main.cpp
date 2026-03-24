#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdint>

#include "utils/CliParser.hpp"
#include "utils/common.hpp"
#include "utils/logging.hpp"
#include "utils/error.hpp"

#include "Lexer.hpp"
#include "AST.hpp"

void compile(const std::string &);

int main(int argc, char **argv)
{
	CliParser cli("sasc-compiler");
	auto &filename_arg = cli.add_positional("file", "Input file to compile").required();
	auto &verbosity_flag = cli.add_flag("verbose", "Increase compiler verbosity").short_name('v').allow_multi();
	auto &debug_print_ast_flag = cli.add_flag("debug-print-ast", "Print a textual representation of the parsed abstract syntax tree");
	cli.add_help_flag(exit_code_as_int(ExitCode::Success));

	try
	{
		cli.parse(argc, argv);
	}
	catch (const CliError &ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl
				  << std::endl;
		cli.print_help();
		return exit_code_as_int(ExitCode::UncaughtInternalError);
	}

	settings::log_verbosity = verbosity_flag.count();
	log_vv("verbosity: {}", settings::log_verbosity);

	settings::debug_print_ast = debug_print_ast_flag.present();
	log_vv("debug print ast: {}", bool_str(settings::debug_print_ast));

	std::string filename = argv[1];
	log_v("input file: {:s}", filename.c_str());

	try
	{
		log("{}: compiling...", filename);
		compile(filename);
		log("{}: compilation complete", filename);
	}
	catch (const CompileError &e)
	{
		std::cerr << std::endl
				  << e.what() << std::endl;
		log("{}: compilation failed", filename);
		return exit_code_as_int(e.exit_code());
	}
	catch (const std::exception &e)
	{
		std::cerr << std::endl
				  << "Uncaught exception occurred: " << e.what() << std::endl;
		return exit_code_as_int(ExitCode::UncaughtInternalError);
	}

	return exit_code_as_int(ExitCode::Success);
}

void compile(const std::string &filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
		throw FileReadError(std::format("Failed to open file \"{}\"", filename));

	log_vv("File opened");

	log_vv("Initializing lexer");
	Lexer lexer(file);

	log_vv("Parsing AST");
	AST ast(lexer);

	if (settings::debug_print_ast)
	{
		log_vv("Printing AST");
		ast.debug_print();
	}

	log_vv("Emitting IR");
	auto functions = ast.emitIr();
}