#pragma once

#include <exception>
#include <string>
#include <variant>
#include <cstdint>

#include "utils/common.hpp"

class CompileError : public std::exception
{
public:
	using LocVariant = std::variant<std::monostate, SourceLoc, SourceLocRange>;

private:
	const ExitCode code;

protected:
	// ctor for implementors
	CompileError(
		const char *type_name,
		ExitCode code,
		const std::string &msg,
		LocVariant loc);

public:
	std::string display;
	const LocVariant loc;

	inline ExitCode exit_code() const noexcept { return this->code; }

	inline const char *what() const noexcept override { return display.c_str(); };
};

class SyntaxError : public CompileError
{
public:
	SyntaxError(const std::string &msg, LocVariant loc = std::monostate{})
		: CompileError("Syntax error", ExitCode::SyntaxError, msg, loc) {}
};

class NameError : public CompileError
{
public:
	NameError(const std::string &msg, LocVariant loc = std::monostate{})
		: CompileError("Name error", ExitCode::NameError, msg, loc) {}
};

class TypeError : public CompileError
{
public:
	TypeError(const std::string &msg, LocVariant loc = std::monostate{})
		: CompileError("Type error", ExitCode::TypeError, msg, loc) {}
};

class UnimplementedError : public CompileError
{
public:
	UnimplementedError(const std::string &msg, LocVariant loc = std::monostate{})
		: CompileError("Unimplemented", ExitCode::Unimplemented, msg, loc) {}
};

class InternalError : public CompileError
{
public:
	InternalError(const std::string &msg, LocVariant loc = std::monostate{})
		: CompileError("Internal error", ExitCode::InternalError, msg, loc) {}
};