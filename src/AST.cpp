#include "AST.hpp"

#include <format>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "utils/error.hpp"
#include "utils/logging.hpp"

AST::AST(Lexer &lexer)
{

	log_vv("Attempting to parse AST");
	try
	{
		while (true)
		{
			if (auto parsed = ast::TopLevelDeclaration::try_parse(lexer))
			{

				this->tlds.push_back(std::move(parsed.value()));

				continue;
			}
			break;
		}
		lexer.expect(TokenType::END_OF_FILE);
	}
	catch (CompileError e)
	{
		e.add_prefix("syntax error: ");
		throw e;
	}

	log_vv("Performing semantic analysis on AST");

	// top level hoisting
	this->top_level_symbols = std::make_shared<SymbolTable>();
	for (const std::unique_ptr<ast::TopLevelDeclaration> &tld : this->tlds)
	{
		auto [name, type] = tld->declares();
		this->top_level_symbols->add(name, type);
	}

	try
	{
		for (const std::unique_ptr<ast::TopLevelDeclaration> &tld : this->tlds)
		{
			tld->check(this->top_level_symbols);
		}
	}
	catch (CompileError e)
	{
		e.add_prefix("semantic error: ");
		throw e;
	}
}

void AST::debug_print() const
{
	for (const std::unique_ptr<ast::TopLevelDeclaration> &tld : this->tlds)
	{
		tld->debug_print(0);
	}
}

bool SymbolTable::add(std::string name, FrontendType type)
{
	return this->add({name, type});
}

bool SymbolTable::add(std::pair<std::string, FrontendType> symbol)
{
	auto [_, success] = this->symbols.insert(symbol);
	return success;
}

namespace ast
{
	namespace node_types
	{
		void IntegerLiteralExpression::check(std::shared_ptr<SymbolTable> symbols)
		{
		}

		void IntegerLiteralExpression::debug_print(unsigned int depth) const
		{
			std::cout << std::string(depth * 2, ' ');
			std::cout << "Integer literal expression (value: \"" << this->value << "\", type annotation: " << this->type.to_string() << ")";
			std::cout << " [" << this->src_loc.to_string() << "]" << std::endl;
		}

		std::optional<std::unique_ptr<IntegerLiteralExpression>> IntegerLiteralExpression::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::INT_LITERAL)
				return std::nullopt;

			auto ret = std::make_unique<IntegerLiteralExpression>();
			Token tok = lexer.take();
			ret->src_loc = tok.loc;

			// finding where numbers stop
			auto type_annotation_pos = tok.str.begin();
			while (type_annotation_pos != tok.str.end() && is_numeric(*type_annotation_pos))
				++type_annotation_pos;

			ret->value = std::string(tok.str.begin(), type_annotation_pos);

			std::string type_str(type_annotation_pos, tok.str.end());
			ret->type = FrontendType(type_str, tok.loc);

			return ret;
		}

		void ReturnStatement::check(std::shared_ptr<SymbolTable> symbols)
		{
			if (this->expr != nullptr)
			{
				this->expr->check(symbols);
			}
		}

		void ReturnStatement::debug_print(unsigned int depth) const
		{
			std::cout << std::string(depth * 2, ' ');
			std::cout << "Return statement (expression present: " << bool_str(this->expr != nullptr) << ")";
			std::cout << " [" << this->src_loc.to_string() << "]" << std::endl;
			if (this->expr != nullptr)
				this->expr->debug_print(depth + 1);
		}

		std::optional<std::unique_ptr<ReturnStatement>> ReturnStatement::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::KEYWORD_RETURN)
				return std::nullopt;

			auto ret = std::make_unique<ReturnStatement>();
			ret->src_loc.start = lexer.take().loc.start;

			if (lexer.peek().type == TokenType::SEMICOLON)
				return ret;

			if (auto parsed_expr = ExpressionNode::try_parse(lexer))
				ret->expr = std::move(parsed_expr.value());
			else
				throw CompileError("Expected expression", lexer.pos);

			ret->src_loc.end = lexer.expect(TokenType::SEMICOLON).loc.end;

			return ret;
		}

		void Block::check(std::shared_ptr<SymbolTable> parent_scope)
		{
			// new scope
			this->symbols = std::make_shared<SymbolTable>();
			this->symbols->parent = parent_scope;
			parent_scope->children.push_back(this->symbols);

			for (std::unique_ptr<StatementNode> &stmt : this->statements)
			{
				stmt->check(this->symbols);
			}

			if (this->expression != nullptr)
				this->expression->check(this->symbols);
		}

		void Block::debug_print(unsigned int depth) const
		{
			std::cout << std::string(depth * 2, ' ');
			std::cout << "Block (statements: " << this->statements.size() << ", expression present: " << bool_str(this->expression != nullptr) << ")";
			std::cout << " [" << this->src_loc.to_string() << "]" << std::endl;
			for (const auto &stmt : this->statements)
			{
				stmt->debug_print(depth + 1);
			}

			if (this->expression != nullptr)
				this->expression->debug_print(depth + 1);
		}

		std::optional<std::unique_ptr<Block>> Block::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::L_BRACKET)
				return std::nullopt;

			auto ret = std::make_unique<Block>();
			ret->src_loc.start = lexer.take().loc.start;

			while (true)
			{
				if (auto stmt = StatementNode::try_parse(lexer))
				{
					ret->statements.push_back(std::move(stmt.value()));
					continue;
				}
				break;
			}
			if (auto expr = ExpressionNode::try_parse(lexer))
			{
				ret->expression = std::move(expr.value());
			}

			ret->src_loc.end = lexer.expect(TokenType::R_BRACKET).loc.end;
			return std::move(ret);
		}

		void ArgDefinition::check(std::shared_ptr<SymbolTable> symbols)
		{
			// TODO
		}

		void ArgDefinition::debug_print(unsigned int depth) const
		{
			std::cout << std::string(depth * 2, ' ');
			std::cout << "Argument definition (type: " << this->type.to_string() << ", name: \"" << this->name << "\")";
			std::cout << " [" << this->src_loc.to_string() << "]" << std::endl;
		}

		std::optional<ArgDefinition> ArgDefinition::try_parse(Lexer &lexer)
		{
			if (lexer.peek().type != TokenType::IDENT)
				return std::nullopt;

			ArgDefinition ret;

			Token name_tok = lexer.take();
			ret.name = name_tok.str;
			ret.src_loc.start = name_tok.loc.start;

			lexer.expect(TokenType::COLON);

			Token type_tok = lexer.expect(TokenType::IDENT);
			ret.type = FrontendType(type_tok);
			ret.src_loc.end = type_tok.loc.end;

			return ret;
		}

		void FunctionDefinition::check(std::shared_ptr<SymbolTable> symbols)
		{
			// TODO check function name

			// check return type
			if (std::optional<FrontendType::Unknown> unknown = this->return_type.is_unknown())
				throw CompileError(std::format("Invalid type: \"{}\"", unknown->str), unknown->loc);

			// check args
			for (ArgDefinition &arg : this->args)
			{
				arg.check(symbols);
			}

			// check body
			this->body->check(symbols);

			// check if body has return or expr
			// check if return value matches
		}

		void FunctionDefinition::debug_print(unsigned int depth) const
		{
			std::cout << std::string(depth * 2, ' ');
			std::cout << "Function definition (name: \"" << this->name << "\", args: " << this->args.size() << ", return type: " << this->return_type.to_string() << ")";
			std::cout << " [" << this->src_loc.to_string() << "]" << std::endl;
			for (const ArgDefinition &arg : this->args)
			{
				arg.debug_print(depth + 1);
			}
			this->body->debug_print(depth + 1);
		}

		std::optional<FunctionDefinition> FunctionDefinition::try_parse(Lexer &lexer)
		{
			// fn keyword
			if (lexer.peek().type != TokenType::KEYWORD_FN)
				return std::nullopt;

			FunctionDefinition ret;
			ret.src_loc.start = lexer.take().loc.start;

			// name
			Token name_tok = lexer.expect(TokenType::IDENT);
			ret.name = name_tok.str;

			// args
			lexer.expect(TokenType::L_PAREN);
			while (true)
			{
				if (auto arg = ArgDefinition::try_parse(lexer))
				{
					ret.args.push_back(arg.value());
					if (lexer.peek().type == TokenType::R_PAREN)
					{
						lexer.take();
						break;
					}
					else if (lexer.peek().type == TokenType::COMMA)
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

			// return type
			ret.return_type = FrontendType(ConcreteType::VOID);
			if (lexer.peek().type == TokenType::ARROW)
			{
				lexer.take();
				ret.return_type = FrontendType(lexer.expect(TokenType::IDENT));
			}

			// must be a block here
			if (std::optional<std::unique_ptr<Block>> parsed = Block::try_parse(lexer))
			{
				ret.body = std::move(parsed.value());
			}
			else
				throw CompileError("Expected a function body (e.g. \"{ ... }\")");

			ret.src_loc.end = ret.body->src_loc.end;
			return ret;
		}

	}

	std::optional<std::unique_ptr<TopLevelDeclaration>> TopLevelDeclaration::try_parse(Lexer &lexer)
	{
		if (auto function_def = node_types::FunctionDefinition::try_parse(lexer))
		{
			auto ret = std::make_unique<node_types::FunctionDefinition>(std::move(function_def.value()));
			ret->src_loc = function_def->src_loc;
			return ret;
		}

		return std::nullopt;
	}

	std::optional<std::unique_ptr<ExpressionNode>> ExpressionNode::try_parse(Lexer &lexer)
	{
		if (auto expr = node_types::IntegerLiteralExpression::try_parse(lexer))
		{
			return std::move(expr.value());
		}
		return std::nullopt;
	}

	std::optional<std::unique_ptr<StatementNode>> StatementNode::try_parse(Lexer &lexer)
	{
		if (auto stmt = node_types::ReturnStatement::try_parse(lexer))
		{
			return std::move(stmt.value());
		}
		return std::nullopt;
	}

}
