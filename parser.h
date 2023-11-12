#ifndef PARSER_H
#define PARSER_H
#include <vector>
#include "scanner.h"
enum class AstNodeType
{
	NUMBER,
	STRING,
	IDENTIFIER,
	NIL,
	LOGICAL,
	BLOCK,
	FUNCTION_DECLARATION,
	VARIABLE_DECLARATION,
	PARAMS,
	ARGS,
	OP_CALL,
	OP_ADD,
	OP_DIVIDE,
	OP_MUL,
	OP_SUBTRACT,
	OP_DEC,
	OP_BANG,
	OP_NEGATE,
	OP_GREATER,
	OP_GREATER_EQUAL,
	OP_LESS,
	OP_LESS_EQUAL,
	OP_EQUAL,
	OP_NOT_EQUAL,
	OP_EQUAL_EQUAL,
	OP_AND,
	OP_OR,
	OP_PRINT,
	OP_RETURN,
	OP_WHILE,
	OP_FOR,
	OP_IF
};

struct AstNode
{
	AstNodeType type;
	void* data;
	std::vector<AstNode*> children;
};
typedef AstNode AstRoot;


std::vector<AstNode*> parse(const std::vector<Token>& tokens);
void printAST(AstRoot* root);
void printSequence(std::vector<AstNode*>& AstSequence);
#endif // !COMPILER_H

