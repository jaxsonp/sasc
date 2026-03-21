#pragma once

#include <exception>
#include <string>
#include <variant>

#include "utils/common.hpp"

class CompileError : public std::exception
{
	std::string msg;
	std::variant<std::monostate, SourceLoc, SourceLocRange> loc;

public:
	CompileError(const std::string &_msg);
	CompileError(const std::string &_msg, SourceLoc loc);
	CompileError(const std::string &_msg, SourceLocRange loc);

	const char *what();

	// prepends a prefix to this error
	void add_prefix(const std::string &prefix);
};