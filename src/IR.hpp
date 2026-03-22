#pragma once

#include <vector>
#include <unordered_map>
#include <optional>

#include "types.hpp"

namespace ir
{
	using BBlockId = unsigned int;
	using VRegId = unsigned short;

	enum class Op
	{
		// arithmetic ---
		Add,
		Sub,
		Mul,
		Div,
		Rem,
		And,
		Or,
		Xor,
		Shl,
		Shr,
		// memory ---
		Load,
		Store,
		// control flow ---
		Jump,
		BranchEq,
		BranchNe,
		BranchLt,
		BranchLe,
		BranchGt,
		BranchGe,
		Call,
		Return,
		// Specials ---
		Move,	 // copy reg to reg
		LoadImm, // load constant into reg
		LoadArg, // load argument into reg
	};

	class BasicBlock;

	/// @brief Instruction base class
	struct Instruction
	{
		Op opcode;

		Instruction *next;
		Instruction *prev;

		/// @brief Whether this instruct is a terminal instruction. Defaults to false
		virtual bool is_terminal() const { return false; }

		Instruction(Op op) : opcode(op) {}
	};

	/// @brief Base class for instructions that belongs at the end of a basic block
	struct TerminalInstruction : public Instruction
	{
		explicit TerminalInstruction(Op op) : Instruction(op) {}

		virtual bool is_terminal() const { return true; }

		// TerminalInstruction(Op op) : Instruction(op) {}
		virtual std::vector<BasicBlock *> successors() const = 0;
	};

	namespace instr
	{
		struct LoadArgInstruction : public Instruction
		{
			unsigned short arg_index;
			VRegId dst;

			LoadArgInstruction(VRegId dst, unsigned short index);
		};

		struct ReturnInstruction : public TerminalInstruction
		{
			std::optional<VRegId> ret_value;
			ReturnInstruction(VRegId ret_value);
		};

		/// @brief Unconditional jump
		struct JumpInstruction : public TerminalInstruction
		{
			BasicBlock *target;

			explicit JumpInstruction(BasicBlock *t)
				: TerminalInstruction(Op::Jump), target(t) {}

			std::vector<BasicBlock *> successors() const override { return {this->target}; }
		};
	}

	/// TODO conditional jump

	struct BasicBlock
	{
		const unsigned int id;
		std::string note;
		Instruction *start;
		Instruction *end;

		BasicBlock(std::string note = "");
	};

	/*enum class OperandType {
		Register,  // virtual register (v0, v1, ...)
		Immediate, // 32-bit integer constant
		Label	   // For branch/jump targets
	};

	struct Operand
	{
		OperandType type;
		union
		{
			int reg_id;		   // Index for virtual registers
			int32_t val;	   // Constant value
			const char *label; // if
		};
	};*/

	struct Function
	{
		std::string name;
		BasicBlock *entry;
		unsigned short vreg_count;

		Function(const std::string &name);
	};
}

/// @brief Wraps a map of functions, for building CFGs
class IrWriter
{
	using VRegMap = std::unordered_map<std::string, ir::VRegId>;

	std::unordered_map<std::string, ir::Function *> &functions;

	/// @brief A stack of vreg maps, mapping names to assigned virtual registers
	std::vector<VRegMap> vreg_map_scopes;

public:
	ir::Function *cur_function;
	ir::BasicBlock *cur_bblock;
	ir::Instruction *cur_instr;

	IrWriter(std::unordered_map<std::string, ir::Function *> &_functions)
		: functions(_functions) {}

	/// @brief Creates a new function, and sets this writer's context there
	void new_function(const std::string &name);

	/// @brief Creates a new local in the current scope, returning its vreg
	ir::VRegId new_local(const std::string &name);

	/// @brief Find the vreg allocation of a name in the current or surrounding scopes (throws if cannot find)
	ir::VRegId get_local(const std::string &name) const;

	void push_scope();
	void pop_scope();

	/// @brief Reserve a new virtual reg
	ir::VRegId new_vreg();

	/// @brief Write an instruction at the current position
	void emit(ir::Instruction *new_instr);
};
