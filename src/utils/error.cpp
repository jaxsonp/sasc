#include "error.hpp"

#include <format>

CompileError::CompileError(const std::string &_msg)
	: std::exception(), msg(_msg), loc() {}

CompileError::CompileError(const std::string &_msg, SourceLoc loc)
	: std::exception(), loc(loc)
{
	this->msg = std::format("{} (at {}:{})", _msg, loc.line, loc.col);
}

CompileError::CompileError(const std::string &_msg, SourceLocRange loc)
	: std::exception(), loc(loc)
{
	this->msg = std::format("{} (at {}:{})", _msg, loc.start.line, loc.start.col);
}

const char *CompileError::what()
{
	return msg.c_str();
}

void CompileError::add_prefix(const std::string &prefix)
{
	msg = prefix + msg;
}
