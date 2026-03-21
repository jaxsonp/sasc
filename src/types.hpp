#pragma once

#include <string>
#include <variant>

#include "Lexer.hpp"

/// @brief A "physical" type
enum ConcreteType
{
	VOID,
	U32,
	I32,
};

std::string to_string(ConcreteType &t);

/// @brief A type in the context of the source code
class FrontendType
{
public:
	struct Unknown
	{
		std::string str;
		SourceLocRange loc;
	};

	FrontendType() : variant(ConcreteType::VOID) {};
	FrontendType(ConcreteType type) : variant(ConcreteType::VOID) {};
	FrontendType(Token tok);
	FrontendType(std::string s, SourceLocRange loc);

	bool is_concrete() const;
	std::optional<Unknown> is_unknown() const;

	std::string to_string() const;

private:
	std::variant<ConcreteType, Unknown> variant;
};
