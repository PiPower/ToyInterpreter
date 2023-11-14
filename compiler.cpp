#include "compiler.h"
#include <iostream>

using namespace std;

void dispatch(AstNode* root, InstructionSequence& program);

OpCodes AstNodeTypeToOpCode(AstNodeType node_typ)
{
    switch (node_typ)
    {
    case AstNodeType::OP_ADD:
        return OpCodes::ADD;
    case AstNodeType::OP_DIVIDE:
        return OpCodes::DIVIDE;
    case AstNodeType::OP_SUBTRACT:
        return OpCodes::SUBTRACT;
    case AstNodeType::OP_MUL:
        return OpCodes::MULTIPLY;
    default:
        cout << "Unsupported translation" << endl;
        exit(-1);
        break;
    }
}

int AllocationSchema(int prev_size)
{
    return prev_size * 2;
}

InstructionSequence compile(const std::string& source)
{
    std::vector<Token> tokens = scan(source);
    printTokens(tokens);
    std::vector<AstNode*> AstSequence = parse(tokens);
    printSequence(AstSequence);
    return backend(AstSequence);
}

void AdjustProgramSize(int additional_size, InstructionSequence& program)
{
    while (program.instruction_offset + additional_size > program.size)
    {
        int new_size = AllocationSchema(program.size);
        program.size = new_size;
        char* buffer = new char[new_size];
        memcpy(buffer, program.instruction - program.instruction_offset, program.instruction_offset);
        delete[] program.instruction;
        program.instruction = buffer;
    }
}
void EmitInstruction(OpCodes opcode, InstructionSequence& program)
{
    uint16_t instructionValue = getInstructionValue(opcode);
    AdjustProgramSize(sizeof(uint16_t), program);
    *(uint16_t*)program.instruction = instructionValue;
    program.instruction_offset += sizeof(uint16_t);
    program.instruction += sizeof(uint16_t);
}

void EmitInstructionWithPayload(OpCodes opcode,  InstructionSequence& program, void* Payload, int payload_size)
{
    EmitInstruction(opcode, program);
    AdjustProgramSize(payload_size, program);
    memcpy(program.instruction, Payload, payload_size);
    program.instruction_offset += payload_size;
    program.instruction += payload_size;
}

void translate_NUMBER(AstNode* root,  InstructionSequence& program)
{
    EmitInstructionWithPayload(OpCodes::PUSH_IMMIDIATE, program, root->data, sizeof(double));
}
void translate_2_operand_op(AstNode* root, OpCodes opcode ,InstructionSequence& program)
{
    dispatch(root->children[0], program);
    dispatch(root->children[1], program);

    EmitInstruction(opcode, program);
}

void dispatch(AstNode* root, InstructionSequence& program)
{
    switch (root->type)
    {
    case AstNodeType::NUMBER:
        translate_NUMBER(root, program);
        break;
    case AstNodeType::OP_SUBTRACT:
    case AstNodeType::OP_MUL:
    case AstNodeType::OP_DIVIDE:
    case AstNodeType::OP_ADD:
        translate_2_operand_op(root, AstNodeTypeToOpCode(root->type), program);
        break;
    default:
        cout << "Unsupported instruction !!!!" << endl;
        exit(-1);
        break;
    }
}

InstructionSequence backend(const std::vector<AstNode*>& AstSequence)
{
    InstructionSequence program;
    program.instruction = new char[1000];
    program.instruction_offset = 0;
    program.size = 1000;

    for (AstNode* root : AstSequence)
    {
        dispatch(root, program);
    }
    EmitInstruction(OpCodes::EXIT, program);
    return program;
}

uint16_t getInstructionValue(OpCodes opcode)
{
    return (uint16_t)opcode;
}

OpCodes valueToOpcode(uint16_t value)
{
    return static_cast<OpCodes>(value);
}
