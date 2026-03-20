#pragma once

#include <string>
#include <iostream>
#include <optional>
#include <format>

#ifdef DEBUG_OUTPUT
#define DBG_PRINT(fmt, ...)         \
	do                              \
	{                               \
		printf(fmt, ##__VA_ARGS__); \
	} while (0)
#else
#define DBG_PRINT(fmt, ...) \
	do                      \
	{                       \
	} while (0)
#endif

struct SourceLoc
{
	unsigned int line = 0;
	unsigned int col = 0;
};

class CompileError : public std::exception
{
	std::string msg;
	std::optional<SourceLoc> src_start;
	std::optional<SourceLoc> src_end;

public:
	CompileError(const std::string &_msg)
		: std::exception(), msg(_msg), src_start(), src_end() {}

	CompileError(const std::string &_msg, SourceLoc start)
		: std::exception(), src_start(start), src_end()
	{
		this->msg = std::format("{} (at {}:{})", _msg, start.line, start.col);
	}

	CompileError(const std::string &_msg, SourceLoc start, SourceLoc end)
		: std::exception(), src_start(start), src_end(end)
	{
		this->msg = std::format("{} (at {}:{})", _msg, start.line, start.col);
	}

	const char *what()
	{
		return msg.c_str();
	};

	// prepends a prefix to this error
	void add_prefix(const std::string &prefix)
	{
		msg = prefix + msg;
	}
};