#pragma once

#include <variant>
#include <tuple>
#include <vector>
#include <optional>
#include <unordered_map>
#include <memory>

#include "Lexer.hpp"
#include "types.hpp"
#include "utils/common.hpp"

class SymbolTable
{
	std::unordered_map<std::string, std::optional<FrontendType>> symbols;

public:
	/// @brief Parent (nullptr if root)
	std::weak_ptr<SymbolTable> parent;
	std::vector<std::shared_ptr<SymbolTable>> children;

	// std::optional<Type> find_symbol(const std::string &name, bool recursive = true);

	/// @brief Attempts to insert a symbol into this symbol table scope, reporting wh
	/// @param name Symbol name
	/// @param type Symbol type
	/// @return Returns true if insertion was successful, false if symbol is already defined here
	inline bool add(std::string name, FrontendType type);
	/// @brief Attempts to insert a symbol into this symbol table scope, reporting wh
	/// @param symbol Name/type pair
	/// @return Returns true if insertion was successful, false if symbol is already defined here
	bool add(std::pair<std::string, FrontendType> symbol);

	SymbolTable() = default;
};

// AST nodes stuff
namespace ast
{

	// Node interface
	class Node
	{
	public:
		SourceLocRange src_loc;

		virtual void check(std::shared_ptr<SymbolTable> symbols) = 0;
		virtual void debug_print(unsigned int depth) const = 0;

		virtual ~Node() = default;
	};

	// TLD interface
	class TopLevelDeclaration : public Node
	{
	public:
		virtual std::pair<std::string, FrontendType> declares() const = 0;

		static std::optional<std::unique_ptr<TopLevelDeclaration>> try_parse(Lexer &lexer);
	};

	// expression interface
	class ExpressionNode : public Node
	{
	public:
		static std::optional<std::unique_ptr<ExpressionNode>> try_parse(Lexer &lexer);
	};

	// statement interface
	class StatementNode : public Node
	{
	public:
		static std::optional<std::unique_ptr<StatementNode>> try_parse(Lexer &lexer);
	};

	// AST node types
	namespace node_types
	{

		struct IntegerLiteralExpression : ExpressionNode
		{
			std::string value;
			FrontendType type;

			void check(std::shared_ptr<SymbolTable> scope) override;
			void debug_print(unsigned int depth = 0) const override;

			static std::optional<std::unique_ptr<IntegerLiteralExpression>> try_parse(Lexer &lexer);
		};

		struct ReturnStatement : StatementNode
		{
			std::unique_ptr<ExpressionNode> expr;

			void check(std::shared_ptr<SymbolTable> scope) override;
			void debug_print(unsigned int depth = 0) const override;

			static std::optional<std::unique_ptr<ReturnStatement>> try_parse(Lexer &lexer);
		};

		struct Block : Node
		{
			std::vector<std::unique_ptr<StatementNode>> statements;
			std::unique_ptr<ExpressionNode> expression;
			std::shared_ptr<SymbolTable> symbols;

			void check(std::shared_ptr<SymbolTable> scope) override;
			void debug_print(unsigned int depth = 0) const override;

			static std::optional<std::unique_ptr<Block>> try_parse(Lexer &lexer);
		};

		struct ArgDefinition : Node
		{
			FrontendType type;
			std::string name;

			void check(std::shared_ptr<SymbolTable> scope) override;
			void debug_print(unsigned int depth = 0) const override;

			static std::optional<ArgDefinition> try_parse(Lexer &lexer);

		private:
			ArgDefinition() = default;
		};

		struct FunctionDefinition : TopLevelDeclaration
		{
			std::string name;
			std::vector<ArgDefinition> args;
			FrontendType return_type;
			std::string return_type_str;
			std::unique_ptr<Block> body;

			void check(std::shared_ptr<SymbolTable> scope) override;
			void debug_print(unsigned int depth = 0) const override;

			static std::optional<FunctionDefinition> try_parse(Lexer &lexer);

			inline std::pair<std::string, FrontendType> declares() const override
			{
				return {this->name, this->return_type};
			}

		private:
			FunctionDefinition() = default;
		};

	}
}

class AST
{
	std::vector<std::unique_ptr<ast::TopLevelDeclaration>> tlds;
	std::shared_ptr<SymbolTable> top_level_symbols;

public:
	// attempt to create an AST from tokens
	AST(Lexer &lexer);

	void debug_print() const;
};