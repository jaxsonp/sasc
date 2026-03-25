#pragma once

#include <stdint.h>
#include <array>
#include <utility>
#include <vector>

#include "backend/Backend.hpp"

namespace backends::rv32
{
	class RegSlot
	{
	public:
		/// Physical register
		const uint8_t physical;
		/// Human name
		const char *name;
		/// AKA caller-saved
		const bool clobbered;

	private:
		/// ID of the current virtual register living here (if there is one)
		ir::VRegId resident;
		/// Whether there is a virtual register loaded in this register
		bool occupied = false;
		/// Whether the virtual register here has been written to
		bool dirty = false;
		/// Set whenever this slot is occupied, never unset automatically
		bool used = false;

		size_t next_to_spill = 0;

		RegSlot(uint8_t physical, const char *name, bool clobbered = false)
			: physical(physical), name(name), clobbered(clobbered) {}

		/// @brief Saves this slot's resident vreg to the stack so that it can be used by another vreg
		void spill(RegSlot *slot);

		friend class Backend_RV32;
	};

	/// @brief RISC-V 32bit
	///
	/// Register allocation strategy:
	/// Local allocation - Per basic-block, assign vregs to registers, spill to stack as needed or at end of bb
	class Backend_RV32 : public Backend
	{
		/// @brief Register assignment states, in order of priority (heuristic = caller saved first (is this good? idk))
		std::array<RegSlot, 15> registers = {
			RegSlot(5, "t0"),
			RegSlot(6, "t1"),
			RegSlot(7, "t2"),
			RegSlot(28, "t3"),
			RegSlot(29, "t4"),
			RegSlot(30, "t5"),
			RegSlot(31, "t6"),
			RegSlot(10, "a0"),
			RegSlot(11, "a1"),
			RegSlot(12, "a2"),
			RegSlot(13, "a3"),
			RegSlot(14, "a4"),
			RegSlot(15, "a5"),
			RegSlot(16, "a6"),
			RegSlot(17, "a7"),
			// TODO use s registers
		};

		/// Functions without context can write here, functions with context can move the output around
		std::vector<uint8_t> machine_code_buffer;

		/// Moves the machine_code_buffer contents into destination, and resets the MCB
		void dump_machine_code_buffer(std::vector<uint8_t> &destination);

		/// See machine_code_buffer
		std::string asm_buffer;

		std::string dump_asm_buffer();

		/// @brief Loads a vreg value from the stack into a new register (if its not already in one)
		/// @param vreg ID of vreg to put into a register
		/// @return The physical register containing vreg
		RegSlot *load_vreg(ir::VRegId vreg);

		/// @brief Assigns a register as okay to put the destination of an operation into, and marks it as changed
		/// @param vreg ID of vreg being written to
		/// @return The destination register slot
		RegSlot *allocate_dest_reg(ir::VRegId vreg);

		void lowerFunction(ir::Function *fn, Object *obj);

	public:
		virtual Object *lowerIr(IrObject *ir) override;
	};
}