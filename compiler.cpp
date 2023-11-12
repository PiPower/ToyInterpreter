#include "compiler.h"

InstructionSequence compile(const std::string& source)
{
    std::vector<Token> tokens = scan(source);
    printTokens(tokens);
    std::vector<AstNode*> AstSequence = parse(tokens);
    printSequence(AstSequence);
    return backend(AstSequence);
}

void EmitInstruction(OpCodes opcode, const InstructionSequence& program)
{

}

void translate_OP_ADD(AstNode* root, const InstructionSequence& program)
{

}

void translate(AstNode* root, const InstructionSequence& program)
{
    

}

InstructionSequence backend(const std::vector<AstNode*>& AstSequence)
{
    InstructionSequence program;
    program.instruction =(uint16_t*) new char[1000];
    program.instruction_offset;
    program.size = 1000;

    for (AstNode* root : AstSequence)
    {
        translate_OP_ADD(root, program);
    }

    return program;
}

uint16_t getInstructionValue(OpCodes opcode)
{
    return (uint16_t)opcode;
}
