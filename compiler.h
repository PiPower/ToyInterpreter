#ifndef COMPILER
#define COMPILER
#include <string>
#include "LoxObject.h"
#include "parser.h"
#include <unordered_map>
#include <vector>
#include <string>

enum class OpCodes;
struct InstructionSequence
{
	//instruction data region
	char* instruction; // instruction is of type uint16, char is used to make offsets easier to operate
	int instruction_offset;
	unsigned int size;
	//string data region
	char** stringTable;
	int string_count;
	int table_size;
};

struct CompilationMeta
{
	int scope;
	// for globals int is index in string table, for local offset from begging of the stack
	std::vector<std::unordered_map< std::string, int>> scope_variables;
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
	NEGATE,
	ADD,
	DIVIDE,
	SUBTRACT,
	MULTIPLY,
	EQUAL,
	NOT_EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL,
	PRINT,
	DEFINE_GLOBAL_VARIABLE,
	SET_GLOBAL_VARIABLE,
	GET_GLOBAL_VARIABLE,
	DEFINE_LOCAL_VARIABLE,
	SET_LOCAL_VARIABLE,
	GET_LOCAL_VARIABLE,
	START_FRAME,
	END_FRAME,
	NOT,
	JUMP,
	JUMP_IF_FALSE
};

#endif // !COMPILER
