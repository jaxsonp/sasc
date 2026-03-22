#pragma once

#include <istream>
#include <optional>
#include <string>
#include <format>

#include "utils/common.hpp"

enum class TokenType
{
	END_OF_FILE,
	IDENT,
	INT_LITERAL,
	KEYWORD_FN,
	KEYWORD_RETURN,
	COLON,
	SEMICOLON,
	L_PAREN,
	R_PAREN,
	L_SQR_BRACKET,
	R_SQR_BRACKET,
	L_CURLY_BRACKET,
	R_CURLY_BRACKET,
	COMMA,
	ARROW,
	ERROR_UNEXPECTED_CHAR,
};

std::string to_string(TokenType type);

struct Token
{
	SourceLocRange loc;
	std::string str;
	TokenType type;
};

std::string to_string(Token tok);

/// @brief Lexer, converts characters to tokens on demand. Infinitely returns EOF token when completed.
class Lexer
{
public:
	SourceLoc pos;

	Lexer(std::istream &in);
	/// Consumes and returns the next token
	Token take();
	/// Returns the next token without consuming it
	Token peek();
	/// Consumes and returns the next token if it matches the expected type, throwing a `CompileError` if not
	Token expect(TokenType expected_type);
	inline bool is_done() const noexcept { return done; }

private:
	std::istream &in;
	std::optional<char> peeked_char;
	bool done;
	std::optional<Token> peeked_token;

	char take_char();
	char peek_char();
};