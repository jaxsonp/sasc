#pragma once

#include <string>

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
