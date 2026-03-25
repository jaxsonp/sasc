#include "Backend_RV32.hpp"

#include <set>
#include <format>
#include <unordered_map>

#include "utils/logging.hpp"
#include "utils/error.hpp"

namespace backends::rv32
{
	void Backend_RV32::dump_machine_code_buffer(std::vector<uint8_t> &destination)
	{
		destination.reserve(this->machine_code_buffer.size());
		destination.insert(
			destination.end(),
			std::make_move_iterator(this->machine_code_buffer.begin()),
			std::make_move_iterator(this->machine_code_buffer.end()));
		this->machine_code_buffer.clear();
	}

	std::string Backend_RV32::dump_asm_buffer()
	{
		return std::exchange(this->asm_buffer, "");
	}

	RegSlot *Backend_RV32::load_vreg(ir::VRegId vreg)
	{
		// first check for non-occupied registers
		for (RegSlot &reg : this->registers)
		{
			if (!reg.occupied)
			{
				reg.resident = vreg;
				reg.occupied = true;
				// TODO
				return &reg;
			}
		}
		// TODO
	}

	RegSlot *Backend_RV32::allocate_dest_reg(ir::VRegId vreg)
	{
		// first check for non-occupied registers
		for (RegSlot &reg : this->registers)
		{
			if (!reg.occupied)
			{
				reg.resident = vreg;
				reg.occupied = true;
				reg.dirty = true;
				return &reg;
			}
		}

		// then check for non-dirty registers
		for (RegSlot &reg : this->registers)
		{
			if (!reg.dirty)
			{
				reg.resident = vreg;
				reg.occupied = true;
				reg.dirty = true;
				return &reg;
			}
		}

		// finally spill register
		for (RegSlot &reg : this->registers)
		{
			if (!reg.dirty)
			{
				reg.resident = vreg;
				reg.occupied = true;
				reg.dirty = true;
				return &reg;
			}
		}
	}

	void Backend_RV32::lowerFunction(ir::Function *fn, Object *obj)
	{
		log_vvv("Lowering function \"{}\"", fn->name);

		// register this function in the object
		obj->functions.emplace_back(fn->name);

		for (RegSlot &reg : this->registers)
		{
			reg.occupied = false;
			reg.dirty = false;
			reg.used = false; // to track which registers are used, and thus must be saved in the prologue
		}

		size_t required_stack_space = 0;

		std::unordered_map<std::string, size_t> local_labels;
		std::unordered_map<ir::VRegId, uint32_t> vreg_offsets;

		// prefix traversal of body
		std::set<ir::BBlockId> seen;
		std::vector<ir::BasicBlock *> to_visit;
		to_visit.push_back(fn->entry);
		while (!to_visit.empty())
		{
			ir::BasicBlock *bb = to_visit.back();
			to_visit.pop_back();

			this->asm_buffer += std::format("__bb_{}:{}\n", bb->id, (bb->note.empty() ? "" : (" # note: " + bb->note)));

			ir::Instruction *cur_instr = bb->start;
			while (cur_instr != nullptr)
			{
				if (ir::instr::LoadImmInstruction *instr = dynamic_cast<ir::instr::LoadImmInstruction *>(cur_instr))
				{
					// checking if immediate can fit in 12 bytes
					if (instr->value >= -2048 && instr->value <= 2047)
					{
						RegSlot *dest = this->allocate_dest_reg(instr->dest);
						this->asm_buffer += std::format("\taddi\t{}, zero, {}\n", dest->name, instr->value);
					}
					else
					{
						// TODO
						// LUI stuff
						throw UnimplementedError("Cant do large integer immediates yet");
					}
				}
				else if (ir::instr::ReturnInstruction *instr = dynamic_cast<ir::instr::ReturnInstruction *>(cur_instr))
				{
					if (instr->ret_value.has_value())
					{
						RegSlot *ret_value = this->load_vreg(instr->ret_value.value());
						this->asm_buffer += std::format("\taddi\ta0, {}, 0\n", ret_value->name);
						// TODO proper offset
						this->asm_buffer += std::format("\tjal \tzero, <epilogue>\n", ret_value->name);
					}
				}
				else
				{
					throw UnimplementedError("Uncaught instruction variant");
				}

				cur_instr = cur_instr->next;
			}
		}
		std::vector<uint8_t> body_machine_code;

		// emit function prologue ---------
		this->write_asm(std::format("\n\n{0}:\n_{0}_prologue:\n", fn->name));
		// TODO check if stack space overflows 12 bits
		this->write_asm(std::format("\taddi\tsp, sp, {}\n", required_stack_space));

		/*
# Example: STACK_SIZE = 32
addi sp, sp, -32      # 1. Allocate stack space (grows down)
sw   ra, 28(sp)       # 2. Save Return Address
sw   s0, 24(sp)       # 3. Save caller's Frame Pointer
addi s0, sp, 32       # 4. Set up new Frame Pointer (pointing to start of frame)

# 5. Save any other callee-saved registers your function uses
sw   s1, 20(sp)
sw   s2, 16(sp)*/

		// emit function body ---------
		this->dump_machine_code_buffer(obj->text);
		this->write_asm(std::format("\n_{}_body:\n", fn->name));
		this->write_asm(this->dump_asm_buffer());

		// emit function epilogue ---------
		this->write_asm(std::format("\n_{}_epilogue:\n", fn->name));

		/*
# 1. Restore callee-saved registers
lw   s2, 16(sp)
lw   s1, 20(sp)

# 2. Restore FP and RA
lw   s0, 24(sp)
lw   ra, 28(sp)

# 3. Deallocate stack space
addi sp, sp, 32

# 4. Return to caller
ret                   # Pseudo-op for jalr x0, 0(ra)*/
	}

	Object *Backend_RV32::lowerIr(IrObject *ir)
	{
		log_vv("Starting lowering to RV32");
		Object *obj = new Object();

		for (auto &[name, fn] : ir->functions)
		{
			lowerFunction(fn, obj);
		}

		return obj;
	}

	void RegSlot::spill(RegSlot *slot)
	{
	}
}
