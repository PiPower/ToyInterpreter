#ifndef VIRTUAL_MACHINE
#define VIRTUAL_MACHINE
#include <string>
#include <stack>
#include <unordered_map>
#include "LoxObject.h"
#include "compiler.h"
/*
Important note.
Currently VM leakes memory due to lack of proper memory management.
TODO: Add garbage collector
*/


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
	op_type number_resolver(OpCodes opcode);
	op_type string_resolver(OpCodes opcode);
	op_type logical_resolver(OpCodes opcode);
	void InsertGlobal(char* string, LoxObject obj);
	LoxObject GetGlobal(const char* string);
	LoxObject LoadObject(char** instructionData, char** stringTable, char type);
	void RemoveGlobal(char* string, LoxObject obj);
	void UpdateGlobal(char* string, LoxObject obj);
	LoxObject isFalsey(LoxObject& obj);
private:
	int stack_base;
	std::vector<LoxObject> stack;
	std::unordered_map <std::string, LoxObject> globals;
	LoxFunction* currentFunction;
};

#endif // !VIRTUAL_MACHINE