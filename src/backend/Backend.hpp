#pragma once

#include <ostream>
#include <memory>

#include "Object.hpp"
#include "IR.hpp"

class Backend
{
	std::shared_ptr<std::ostream> asm_out = nullptr;

protected:
	virtual void write_asm(std::string_view s);

public:
	virtual Object *lowerIr(IrObject *ir) = 0;

	/// @brief Enables dumping assembly to an output stream. ostream object must live as long as this object.
	virtual void enable_asm_output(std::shared_ptr<std::ostream> out);
};