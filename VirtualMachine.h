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
	void Execute(InstructionSequence instructionData);
	LoxObject Pop();
	void Push(LoxObject obj);
private:
	op_type select_op(OpCodes opcode, const LoxObject& leftOperand, const LoxObject& rightOperand);
private:
	uint16_t* ip_base;
	uint16_t* ip;
	std::stack<LoxObject> stack;
};

#endif // !VIRTUAL_MACHINE