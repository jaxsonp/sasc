#pragma once

#include <vector>
#include <string>
#include <stdint.h>

struct FunctionSignature
{
	std::string name;
	FunctionSignature(const std::string &name);
};

enum class RelocationType
{
	/// U-type RISC-V instruction, where the upper 20 bits of an immediate are placed in the upper 20 bits of the instruction
	RV32_U_TYPE
};

struct Relocation
{
	std::string symbol_name;
};

struct Object
{
	std::vector<FunctionSignature> functions;
	std::vector<Relocation> relocations;

	std::vector<uint8_t> text;
};
