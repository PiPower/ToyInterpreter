#ifndef VIRTUAL_MACHINE
#define VIRTUAL_MACHINE
#include <string>
#include <stack>
#include "LoxObject.h"
#include "compiler.h"

void compile_and_execute(const std::string& source);

class VirtualMachine
{
public:
	VirtualMachine();
	void Execute(uint16_t* instructionData);
private:
	uint16_t* ip_base;
	uint16_t* ip;
	std::stack< LoxObject> stack;
};

#endif // !VIRTUAL_MACHINE