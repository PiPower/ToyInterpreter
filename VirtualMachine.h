#ifndef VIRTUAL_MACHINE
#define VIRTUAL_MACHINE
#include <string>
#include <stack>
#include <unordered_map>
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
	op_type number_resolver(OpCodes opcode, const LoxObject& leftOperand, const LoxObject& rightOperand);
	op_type string_resolver(OpCodes opcode, const LoxObject& leftOperand, const LoxObject& rightOperand);
	void InsertGlobal(char* string, LoxObject obj);
	LoxObject GetGlobal(char* string);
	LoxObject LoadObject(char** instructionData, char** stringTable, char type);
	void RemoveGlobal(char* string, LoxObject obj);
	void UpdateGlobal(char* string, LoxObject obj);
private:
	int stack_base;
	std::vector<LoxObject> stack;
	std::unordered_map <std::string, LoxObject> globals;
};

#endif // !VIRTUAL_MACHINE