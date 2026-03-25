#include "Backend.hpp"

#include <iostream>

void Backend::write_asm(std::string_view s)
{
	if (this->asm_out != nullptr)
	{
		*(this->asm_out) << s;
		this->asm_out->flush();
	}
}

void Backend::enable_asm_output(std::shared_ptr<std::ostream> out)
{
	this->asm_out = out;
}