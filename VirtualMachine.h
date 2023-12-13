#ifndef VIRTUAL_MACHINE
#define VIRTUAL_MACHINE
#include <string>
#include <stack>
#include <unordered_map>
#include "LoxObject.h"
#include "compiler.h"
/*
const LoxObject* is required to prevent hidden memory leaks 
expample if ptr point to string type and we assign number to it we leak string
*/

void compile_and_execute(const std::string& source);


class VirtualMachine
{
public:
	VirtualMachine();
	void Execute(InstructionSequence instructionData);
	const LoxObject* Pop();
	void Push(const LoxObject* obj);
private:
	op_type select_op(OpCodes opcode, const LoxObject* leftOperand, const LoxObject* rightOperand);
	op_type number_resolver(OpCodes opcode);
	op_type string_resolver(OpCodes opcode);
	op_type logical_resolver(OpCodes opcode);
	void InsertGlobal(char* string,const LoxObject* obj);
	const LoxObject* GetGlobal(const char* string);
	const LoxObject* LoadObject(char** instructionData, char** stringTable, char type);
	const LoxObject* LoadObject(LoxType type, const void* data);
	void UpdateGlobal(char* string, const LoxObject* obj);
	const LoxObject* isFalsey(const LoxObject* obj);
	const LoxObject* CreateLoxObject(LoxType type);
	const LoxObject* SetLoxObject(const LoxObject& src);
	void MarkObject(const LoxObject* object);
	void TraceObjects();
	void SweepObjects();
private:
	int stack_base;
	double total_time;
	std::vector<const LoxObject*> stack;
	std::unordered_map <std::string, const LoxObject*> globals;
	LoxFunction* currentFunction;
	std::vector<const LoxObject*> tracedObjects;
};

#endif // !VIRTUAL_MACHINE