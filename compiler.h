#ifndef COMPILER
#define COMPILER
#include <string>
#include "LoxObject.h"
#include "parser.h"
enum class OpCodes;
struct InstructionSequence
{
	char* instruction; // instruction is of type uint16, char is used to make offsets easier to operate
	int instruction_offset;
	unsigned int size;
};

InstructionSequence compile(const std::string& source);
InstructionSequence backend(const std::vector<AstNode*>& AstSequence);
uint16_t getInstructionValue(OpCodes opcode);
OpCodes valueToOpcode(uint16_t value);

enum class OpCodes
{
	EXIT,
	PUSH_IMMIDIATE,
	POP,
	PUSH,
	ADD,
	DIVIDE,
	SUBTRACT,
	MULTIPLY,
};

#endif // !COMPILER
