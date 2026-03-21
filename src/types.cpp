#include "types.hpp"

#include <stdexcept>

FrontendType::FrontendType(Token tok) : FrontendType(tok.str, tok.loc) {}

FrontendType::FrontendType(std::string s, SourceLocRange loc)
{
	if (s == "void")
		this->variant = ConcreteType::VOID;
	else if (s == "u32")
		this->variant = ConcreteType::U32;
	else if (s == "i32")
		this->variant = ConcreteType::I32;
	else
	{
		this->variant = FrontendType::Unknown{s, loc};
	}
}

bool FrontendType::is_concrete() const
{
	return std::holds_alternative<ConcreteType>(this->variant);
}

std::optional<FrontendType::Unknown> FrontendType::is_unknown() const
{
	if (std::holds_alternative<Unknown>(this->variant))
		return std::get<Unknown>(this->variant);
	else
		return std::nullopt;
}

std::string FrontendType::to_string() const
{
	if (std::holds_alternative<ConcreteType>(this->variant))
	{
		switch (std::get<ConcreteType>(this->variant))
		{
		case ConcreteType::VOID:
			return "void";
		case ConcreteType::U32:
			return "u32";
		case ConcreteType::I32:
			return "i32";
		default:
			throw std::logic_error("unhandled type variant");
		}
	}
	else if (std::holds_alternative<Unknown>(this->variant))
	{
		return "unknown";
	}
	else
	{
		throw std::logic_error("unhandled type variant");
	}
}
