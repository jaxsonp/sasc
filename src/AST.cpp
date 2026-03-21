#include "AST.hpp"

#include <format>

#include "utils/error.hpp"

AST::AST(Lexer &lexer)
{
	try
	{
		program = ast::node_types::Program::try_parse(lexer);
	}
	catch (CompileError e)
	{
		e.add_prefix("Error while syntax parsing: ");
		throw e;
	}
}

namespace ast
{
	namespace node_types
	{
		std::optional<IntegerLiteral> IntegerLiteral::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::INT_LITERAL)
				return std::nullopt;
			Token tok = lexer.take();
			return IntegerLiteral(tok.token_str);
		}

		std::optional<Expression> Expression::try_parse(Lexer &lexer)
		{
			if (auto int_lit = IntegerLiteral::try_parse(lexer))
			{
				return Expression(int_lit.value());
			}
			return std::nullopt;
		}

		std::optional<ReturnStatement> ReturnStatement::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::KEYWORD_RETURN)
				return std::nullopt;
			lexer.take();

			if (lexer.peek().type == TokenType::SEMICOLON)
				return ReturnStatement();

			auto expr = Expression::try_parse(lexer);
			if (!expr.has_value())
				throw CompileError("Expected expression", lexer.pos);
			lexer.expect(TokenType::SEMICOLON);

			return ReturnStatement(expr.value());
		}

		std::optional<Statement> Statement::try_parse(Lexer &lexer)
		{
			if (auto return_stmt = ReturnStatement::try_parse(lexer))
				return Statement(return_stmt.value());
			else
				return std::nullopt;
		}

		std::optional<Block> Block::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::L_BRACKET)
				return std::nullopt;
			lexer.take();

			Block block;
			while (true)
			{
				if (auto stmt = Statement::try_parse(lexer))
				{
					block.statements.push_back(stmt.value());
					continue;
				}
				break;
			}
			if (auto expr = Expression::try_parse(lexer))
			{
				block.expression = expr;
			}
			lexer.expect(TokenType::R_BRACKET);
			return block;
		}

		std::optional<ArgDefinition> ArgDefinition::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::IDENT)
				return std::nullopt;

			Token ident = lexer.take();

			lexer.expect(TokenType::COLON);
			// TODO
		}

		std::optional<FunctionDefinition> FunctionDefinition::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::KEYWORD_FN)
				return std::nullopt;
			SourceLoc src_start = lexer.take().start;

			Token name_tok = lexer.expect(TokenType::IDENT);
			lexer.expect(TokenType::L_PAREN);

			// parsing args
			std::vector<ArgDefinition> args;
			while (true)
			{
				if (auto arg = ArgDefinition::try_parse(lexer))
				{
					args.push_back(arg.value());
					TokenType next_type = lexer.peek().type;
					if (next_type == TokenType::R_PAREN)
					{
						lexer.take();
						break;
					}
					else if (next_type == TokenType::COMMA)
					{
						lexer.take();
						continue;
					}
					else
						break;
				}
				else if (lexer.peek().type == TokenType::R_PAREN)
				{

					lexer.take();
					break;
				}
			}

			// return value
			std::string return_type("void");
			if (lexer.peek().type == TokenType::ARROW)
			{
				lexer.take();
				return_type = lexer.expect(TokenType::IDENT).token_str;
			}

			// must be a block here
			auto block = Block::try_parse(lexer);
			if (!block.has_value())
				throw CompileError("Expected a function body (e.g. \"{ ... }\")");

			return FunctionDefinition(name_tok.token_str, args, return_type, block.value());
		}

		std::optional<TopLevelDeclaration> TopLevelDeclaration::try_parse(Lexer &lexer)
		{
			if (auto func = FunctionDefinition::try_parse(lexer))
			{
				return TopLevelDeclaration(func.value());
			}
			else
			{
				return std::nullopt;
			}
		}
		Program Program::try_parse(Lexer &lexer)
		{
			Program program;
			while (true)
			{
				if (auto tld = TopLevelDeclaration::try_parse(lexer))
					program.tlds.push_back(tld.value());
				else
					break;
			}
			return program;
		}
	}
}
