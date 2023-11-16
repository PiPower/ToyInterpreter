#include "compiler.h"
#include <iostream>

using namespace std;

void dispatch(AstNode* root, InstructionSequence& program, CompilationMeta& metaData);

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

void EmitInstructionWithPayload(OpCodes opcode, InstructionSequence& program, void* Payload, int payload_size)
{
    EmitInstruction(opcode, program);
    AdjustProgramSize(payload_size, program);
    memcpy(program.instruction, Payload, payload_size);
    program.instruction_offset += payload_size;
    program.instruction += payload_size;
}

//Add new string to string table
void EmitString(InstructionSequence& program,const char* string,const int size)
{
    if (program.string_count + 1 >= program.table_size)
    {
        char** buff = new char*[program.table_size * 2];
        program.table_size *= 2;
        memcpy(buff, program.stringTable, sizeof(char*) * program.string_count);
        delete[] program.stringTable;
        program.stringTable = buff;
    }
    program.stringTable[program.string_count] = new char[size];
    memcpy(program.stringTable[program.string_count], string, size);
    program.string_count += 1;
}
void translate_2_operand_op(AstNode* root, OpCodes opcode ,InstructionSequence& program, CompilationMeta& metaData)
{
    dispatch(root->children[0], program, metaData);
    dispatch(root->children[1], program, metaData);

    EmitInstruction(opcode, program);
}

void dispatch(AstNode* root, InstructionSequence& program, CompilationMeta& metaData)
{
    switch (root->type)
    {
    case AstNodeType::IDENTIFIER:
    {
        int index = 0;
        for (; index < program.string_count; index++)
        {
            if (strcmp( ((string*)root->data)->c_str(), program.stringTable[index]) == 0 ) break;
        }
        if (index >= program.string_count)
        {
            cout << "UKNOWN VARIABLE !!!" << endl;
            exit(-1);
        }
        EmitInstructionWithPayload(OpCodes::GET_GLOBAL_VARIABLE, program, &index, sizeof(int));
        break;
    }
    case AstNodeType::VARIABLE_DECLARATION:
    {
        string* name = (string*)root->children[0]->data;
        EmitString(program, name->c_str(), name->size() + 1);
        int string_index = program.string_count - 1;
        EmitInstructionWithPayload(OpCodes::DEFINE_GLOBAL_VARIABLE, program, &string_index, sizeof(int) );
        if (root->children[1] != nullptr)
        {
            dispatch(root->children[1], program, metaData);
            EmitInstructionWithPayload(OpCodes::SET_GLOBAL_VARIABLE, program, &string_index, sizeof(int));
        }
        break;
    }
    case AstNodeType::NUMBER:
        EmitInstructionWithPayload(OpCodes::PUSH_IMMIDIATE, program, root->data, sizeof(double));
        break;
    case AstNodeType::LOGICAL:
    {
        bool dataLogic = *(bool*)root->data ? true : false;
        EmitInstructionWithPayload(OpCodes::PUSH_BOOL, program, &dataLogic, sizeof(bool));
        break;
    }
    case AstNodeType::NIL:
        EmitInstruction(OpCodes::PUSH_NIL, program);
        break;
    case AstNodeType::STRING:
    {
        EmitString(program, ((string*)root->data)->c_str(), ((string*)root->data)->size() + 1);
        int string_index = program.string_count - 1;
        EmitInstructionWithPayload(OpCodes::PUSH_STRING, program, (void*)&string_index, sizeof(int));
    }
        break;
    case AstNodeType::OP_SUBTRACT:
    case AstNodeType::OP_MUL:
    case AstNodeType::OP_DIVIDE:
    case AstNodeType::OP_ADD:
        translate_2_operand_op(root, AstNodeTypeToOpCode(root->type), program, metaData);
        break;
    case AstNodeType::OP_PRINT:
        dispatch(root->children[0], program, metaData);
        EmitInstruction(OpCodes::PRINT, program);
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
    program.stringTable = new char*[3]; 
    program.string_count = 0;
    program.table_size = 3;
    CompilationMeta metaData;
    metaData.scope = 0;
    for (AstNode* root : AstSequence)
    {
        dispatch(root, program, metaData);
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
