#ifndef COMPILER
#define COMPILER
#include <string>
#include "LoxObject.h"
#include "parser.h"
enum class OpCodes;
struct InstructionSequence
{
	uint16_t* instruction;
	int instruction_offset;
	unsigned int size;
};

InstructionSequence compile(const std::string& source);
void translate(AstNode* root, const InstructionSequence& program );
InstructionSequence backend(const std::vector<AstNode*>& AstSequence);
uint16_t getInstructionValue(OpCodes opcode);

enum class OpCodes
{
	MOVE_IMMIDIATE,
};

#endif // !COMPILER
