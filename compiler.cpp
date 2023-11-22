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
    case AstNodeType::OP_EQUAL_EQUAL:
        return OpCodes::EQUAL;
    case AstNodeType::OP_NOT_EQUAL:
        return OpCodes::NOT_EQUAL;
    case AstNodeType::OP_GREATER:
        return OpCodes::GREATER;
    case AstNodeType::OP_GREATER_EQUAL:
        return OpCodes::GREATER_EQUAL;
    case AstNodeType::OP_LESS:
        return OpCodes::LESS;
    case AstNodeType::OP_LESS_EQUAL:
        return OpCodes::LESS_EQUAL;
    default:
        cout << "BACKEND ERROR: Unsupported translation" << endl;
        exit(-1);
        break;
    }
}

int find_name(AstNode* root, InstructionSequence& program)
{

    for (int index = 0; index < program.string_count; index++)
    {
        if (strcmp(((string*)root->data)->c_str(), program.stringTable[index]) == 0) return index;
    }
    return -1;
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
        int total_pos = 0;
        for (int i = metaData.scope; i > 0; i--)
        {
            auto object = metaData.scope_variables[i].find(((string*)root->data)->c_str());
            if (object != metaData.scope_variables[i].end()) 
            {
                int index = total_pos + object->second;
                EmitInstructionWithPayload(OpCodes::GET_LOCAL_VARIABLE, program, &index, sizeof(int));
                return;
            }
            total_pos = total_pos - 1 - metaData.scope_variables[i - 1].size();
        }

        auto object = metaData.scope_variables[0].find(((string*)root->data)->c_str());
        if (object == metaData.scope_variables[0].end())
        {
            cout << "BACKEND ERROR: Unknown variable" << endl;
            exit(-1);
        }
        int index = object->second;
        EmitInstructionWithPayload(OpCodes::GET_GLOBAL_VARIABLE, program, &index, sizeof(int));
        break;
    }
    case AstNodeType::VARIABLE_DECLARATION:
    {
        string* name = (string*)root->children[0]->data;
        EmitString(program, name->c_str(), name->size() + 1);
        int object_index = program.string_count - 1;
        char* variable_name = program.stringTable[object_index];
        auto object = metaData.scope_variables[metaData.scope].find(variable_name);

        if (object != metaData.scope_variables[metaData.scope].end())
        {
            cout << "BACKEND ERROR: Variable redefinition" << endl;
            exit(-1);
        }

        OpCodes opcode_def = OpCodes::DEFINE_GLOBAL_VARIABLE;
        OpCodes opcode_set = OpCodes::SET_GLOBAL_VARIABLE;
        if (metaData.scope > 0) // for local object index is position offstet relative to base of stack
        {
            opcode_def = OpCodes::DEFINE_LOCAL_VARIABLE;
            opcode_set = OpCodes::SET_LOCAL_VARIABLE;
            object_index = metaData.scope_variables[metaData.scope].size();
        }

        EmitInstructionWithPayload(opcode_def, program, &object_index, sizeof(int));
        if (root->children[1] != nullptr)
        {
            dispatch(root->children[1], program, metaData);
            EmitInstructionWithPayload(opcode_set, program, &object_index, sizeof(int));
        }
        metaData.scope_variables[metaData.scope].emplace(variable_name, object_index);

        break;
    }
    case AstNodeType::NUMBER:
        char payload[sizeof(double) +1];
        payload[0] = 1;
        memcpy(payload + 1, root->data, sizeof(double));
        EmitInstructionWithPayload(OpCodes::PUSH_IMMIDIATE, program, payload, sizeof(double) + 1);
        break;
    case AstNodeType::LOGICAL:
    {
        char payload[sizeof(bool) + 1];
        payload[0] = 2;
        memcpy(payload + 1, root->data, sizeof(bool));
        EmitInstructionWithPayload(OpCodes::PUSH_IMMIDIATE, program, &payload, sizeof(bool) + 1);
        break;
    }
    case AstNodeType::NIL:
    {
        char payload;
        payload = 0;
        EmitInstructionWithPayload(OpCodes::PUSH_IMMIDIATE, program, &payload, sizeof(char));
        break;
    }
    case AstNodeType::STRING:
    {
        EmitString(program, ((string*)root->data)->c_str(), ((string*)root->data)->size() + 1);
        char payload[sizeof(int) + 1];
        payload[0] = 3;
        int string_index = program.string_count - 1;
        memcpy(payload + 1, &string_index, sizeof(int));
        EmitInstructionWithPayload(OpCodes::PUSH_IMMIDIATE, program, payload, sizeof(int) + 1);
    }
        break;
    case AstNodeType::OP_NOT_EQUAL:
    case AstNodeType::OP_GREATER:
    case AstNodeType::OP_GREATER_EQUAL:
    case AstNodeType::OP_LESS:
    case AstNodeType::OP_LESS_EQUAL:
    case AstNodeType::OP_EQUAL_EQUAL:
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
    case AstNodeType::OP_EQUAL:
    {
        //currently only supports setting global variables
        if (root->children[0]->type != AstNodeType::IDENTIFIER)
        {
            cout << "BACKEND ERROR: Given object to assign operator is not l-value \n";
            exit(-1);
        }
        int string_index = find_name(root->children[0], program);
        dispatch(root->children[1], program, metaData);
        EmitInstructionWithPayload(OpCodes::SET_GLOBAL_VARIABLE, program, &string_index, sizeof(int));
        break;
    }
    case AstNodeType::BLOCK:
        metaData.scope += 1;
        metaData.scope_variables.push_back(unordered_map<string, int>());

        EmitInstruction(OpCodes::START_FRAME, program);
        for (AstNode* child : root->children)
        {
            dispatch(child, program, metaData);
        }
        EmitInstruction(OpCodes::END_FRAME, program);

        metaData.scope_variables.pop_back();
        metaData.scope -= 1;
        break;
    case AstNodeType::OP_BANG:
        dispatch(root->children[0], program, metaData);
        EmitInstruction(OpCodes::NOT, program);
        break;
    case AstNodeType::OP_IF:
    {
        dispatch(root->children[0], program, metaData);
        char addr[4] = { 0xff, 0xff, 0xff, 0xff };
        EmitInstructionWithPayload(OpCodes::JUMP_IF_FALSE, program, addr, sizeof(addr));
        int jump_base = program.instruction_offset;
        dispatch(root->children[1], program, metaData);
        //dispatch(root->children[2], program, metaData);
        int jump_size = program.instruction_offset - jump_base;
        *(int*)(program.instruction - jump_size - sizeof(int) ) = jump_size;
        break;
    }
    default:
        cout << "BACKEND ERROR: Unsupported instruction !!!!" << endl;
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
    metaData.scope_variables.push_back(unordered_map<string, int>());

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
