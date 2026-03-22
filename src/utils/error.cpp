#include "error.hpp"

#include <format>

CompileError::CompileError(const char *type_name, ExitCode exit_code, const std::string &error_msg, LocVariant src_loc)
	: code(exit_code)
{
	if (std::holds_alternative<SourceLoc>(src_loc))
	{
		SourceLoc loc = std::get<SourceLoc>(src_loc);
		this->display = std::format("{}: {} (at {}:{})", type_name, error_msg, loc.line, loc.col);
	}
	else if (std::holds_alternative<SourceLocRange>(src_loc))
	{
		SourceLocRange loc = std::get<SourceLocRange>(src_loc);
		this->display = std::format("{}: {} (at {}:{})", type_name, error_msg, loc.start.line, loc.start.col);
	}
	else
	{
		this->display = std::format("{}: {}", type_name, error_msg);
	}
}
