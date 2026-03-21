#pragma once

#include <string>

enum class ExitCode : u_int8_t
{
	Success = 0,

	// compilation errors
	SyntaxError = 1,
	NameError = 2,
	TypeError = 3,

	// non-compilation errors
	FileReadError = 0xE0,

	// internal errors
	Unimplemented = 0xFD,
	InternalError = 0xFE,
	UncaughtInternalError = 0xFF,
};

inline int exit_code_as_int(ExitCode code) { return static_cast<int>(code); }

struct SourceLoc
{
	unsigned int line = 0;
	unsigned int col = 0;

	std::string to_string() const;
};

struct SourceLocRange
{
	SourceLoc start;
	SourceLoc end;

	std::string to_string() const;
};

bool is_whitespace(char c);

bool is_numeric(char c);

bool is_alpha(char c);

bool is_delimiter(char c);

std::string_view bool_str(bool b);
