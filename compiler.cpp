#include "compiler.h"
#include <iostream>

using namespace std;

void dispatch(AstNode* root, InstructionSequence& program, CompilationMeta& metaData);
void injectNilReturn(AstNode* root);
void resolveUpvalue(const char* string, InstructionSequence& program, CompilationMeta& metaData);
void compileUpvalues(char* base_program_pos, InstructionSequence& program, CompilationMeta& metaData);
ScopedVariables prepareFunctionScope(AstNode* params, CompilationMeta& metaData);

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
    //printTokens(tokens);
    std::vector<AstNode*> AstSequence = parse(tokens);
    //printSequence(AstSequence);
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

int FindIdentifierLocal(CompilationMeta& metaData,const char* string, int& storage)
{
    int total_pos = 0;
    for (int i = metaData.scope; i > 0; i--)
    {
        auto object = metaData.scope_variables[i].find(string);
        if (object != metaData.scope_variables[i].end())
        {
            int index = total_pos + object->second;
            storage = index;
            return 1;
        }
        total_pos = total_pos - 1 - metaData.scope_variables[i - 1].size(); // 1 represents previous stack address stored on stack
    }
    return -1;
}

void dispatch(AstNode* root, InstructionSequence& program, CompilationMeta& metaData)
{
    switch (root->type)
    {
    case AstNodeType::IDENTIFIER:
    {
        int index = 0;
        if (FindIdentifierLocal(metaData, ((string*)root->data)->c_str(), index) == 1)
        {
            EmitInstructionWithPayload(OpCodes::GET_LOCAL_VARIABLE, program, &index, sizeof(int));
            return;
        }
        // reaching  this point means we look for global
        auto object = metaData.scope_variables[0].find(((string*)root->data)->c_str());
        if (object == metaData.scope_variables[0].end() && metaData.enclosing == nullptr)
        {
            cout << "BACKEND ERROR: Unknown variable" << endl;
            exit(-1);
        }
        if (object != metaData.scope_variables[0].end())
        {
            index = object->second;
            EmitInstructionWithPayload(OpCodes::GET_GLOBAL_VARIABLE, program, &index, sizeof(int));
        }
        else resolveUpvalue(((string*)root->data)->c_str(), program, metaData);
        break;
    }
    case AstNodeType::VARIABLE_DECLARATION:
    {
        int object_index = 0;
        string* name = (string*)root->children[0]->data;
        auto object = metaData.scope_variables[metaData.scope].find(name->c_str());

        if (object != metaData.scope_variables[metaData.scope].end())
        {
            cout << "BACKEND ERROR: Variable redefinition" << endl;
            exit(-1);
        }

        OpCodes opcode_def = OpCodes::DEFINE_GLOBAL_VARIABLE;
        OpCodes opcode_set = OpCodes::SET_GLOBAL_VARIABLE;
        if (metaData.scope > 0) // for local object index is position offset relative to base of stack
        {
            opcode_def = OpCodes::DEFINE_LOCAL_VARIABLE;
            opcode_set = OpCodes::SET_LOCAL_VARIABLE;
            object_index = metaData.scope_variables[metaData.scope].size();
        }
        else
        {  // if global variable put its name in string table
            EmitString(program, name->c_str(), name->size() + 1);
            object_index = program.string_count - 1;
        }
        EmitInstructionWithPayload(opcode_def, program, &object_index, sizeof(int));
        if (root->children[1] != nullptr)
        {
            dispatch(root->children[1], program, metaData);
            EmitInstructionWithPayload(opcode_set, program, &object_index, sizeof(int));
        }
        metaData.scope_variables[metaData.scope].emplace(name->c_str(), object_index);

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
    case AstNodeType::OP_NEGATE:
        dispatch(root->children[0], program, metaData);
        EmitInstruction(OpCodes::NEGATE, program);
        break;
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
        dispatch(root->children[1], program, metaData);

        int index = 0;
        if (FindIdentifierLocal(metaData, ((string*)root->children[0]->data)->c_str(), index) == 1)
        {
            EmitInstructionWithPayload(OpCodes::SET_LOCAL_VARIABLE, program, &index, sizeof(int));
            return;
        }
        // reaching  this point means we look for global
        auto object = metaData.scope_variables[0].find(((string*)root->children[0]->data)->c_str());
        if (object == metaData.scope_variables[0].end())
        {
            cout << "BACKEND ERROR: Unknown variable" << endl;
            exit(-1);
        }
        index = object->second;
        EmitInstructionWithPayload(OpCodes::SET_GLOBAL_VARIABLE, program, &index, sizeof(int));
        break;
    }
    case AstNodeType::BLOCK:
        metaData.scope += 1;
        metaData.scope_variables.push_back(unordered_map<string, int>());

        EmitInstruction(OpCodes::START_FRAME, program);
        for (AstNode* child : root->children)
        {
            dispatch(child, program, metaData);
            // if OP_CALL is not wrapped in another statement that means returned value is not gonna be used anymore so pop it from stack
            if (child->type == AstNodeType::OP_CALL) EmitInstruction(OpCodes::POP, program);
            
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
        EmitInstructionWithPayload(OpCodes::JUMP_IF_FALSE, program, addr, sizeof(addr)); // jump to the next block
        int jump_base = program.instruction_offset;

        dispatch(root->children[1], program, metaData);
        EmitInstructionWithPayload(OpCodes::JUMP, program, addr, sizeof(addr)); // jump out off if-s
        int jump_size = program.instruction_offset - jump_base;
        *(int*)(program.instruction - jump_size - sizeof(int) ) = jump_size;

        const int out_of_block_jump_base = program.instruction_offset;;
        if (root->children.size() == 3)
        {
            dispatch(root->children[2], program, metaData);
        }

        jump_size = program.instruction_offset - out_of_block_jump_base;
        *(int*)(program.instruction - jump_size - sizeof(int)) = jump_size;
        break;
    }
    case AstNodeType::OP_AND:
    {
        dispatch(root->children[0], program, metaData);
        char addr[4] = { 0xff, 0xff, 0xff, 0xff };
        EmitInstructionWithPayload(OpCodes::JUMP_IF_FALSE_KEEP_STACK, program, addr, sizeof(addr)); // jump to the next block
        int jump_base = program.instruction_offset;
        EmitInstruction(OpCodes::POP, program); //removes non used variable from stack
        dispatch(root->children[1], program, metaData);
        int jump_size = program.instruction_offset - jump_base;
        *(int*)(program.instruction - jump_size - sizeof(int)) = jump_size;
        break;
    }
    case AstNodeType::OP_OR:
    {
        dispatch(root->children[0], program, metaData);
        char addr[4] = { 0xff, 0xff, 0xff, 0xff };
        EmitInstructionWithPayload(OpCodes::JUMP_IF_FALSE, program, addr, sizeof(addr)); // jump to the next block
        int jump_base_if_false = program.instruction_offset;

        EmitInstructionWithPayload(OpCodes::JUMP, program, addr, sizeof(addr)); // jump to the next block
        int jump_pos = program.instruction_offset;

        int jump_size_if_false = program.instruction_offset - jump_base_if_false;
        *(int*)(program.instruction - jump_size_if_false - sizeof(int)) = jump_size_if_false;
        dispatch(root->children[1], program, metaData);

        int jump_size = program.instruction_offset - jump_pos;
        *(int*)(program.instruction - jump_size - sizeof(int)) = jump_size;
        break;
    }
    case AstNodeType::OP_WHILE:
    {
        int jump_condition = program.instruction_offset;
        dispatch(root->children[0], program, metaData);
        char addr[4] = { 0xff, 0xff, 0xff, 0xff };
        EmitInstructionWithPayload(OpCodes::JUMP_IF_FALSE, program, addr, sizeof(addr)); // jump to the next block
        int jump_pos = program.instruction_offset;

        for (int i = 1; i < root->children.size(); i++) 
            dispatch(root->children[i], program, metaData);

        int jump_condition_size = jump_condition - program.instruction_offset - 6; // 6 for jump instruction + payload
        EmitInstructionWithPayload(OpCodes::JUMP, program, &jump_condition_size, sizeof(addr));

        int jump_size = program.instruction_offset - jump_pos;
        *(int*)(program.instruction - jump_size - sizeof(int)) = jump_size;

        break;
    }
    case AstNodeType::OP_FOR:
    {

        // translate for-loop into while loop
        AstNode* FORhead = new AstNode(); // create block directly for for-loop
        FORhead->type = AstNodeType::BLOCK;
        FORhead->data = new string("NEW BLOCK");

        AstNode* loop =  new AstNode();
        loop->type = AstNodeType::OP_WHILE;
        loop->data = new string("WHILE");

        if (root->children[0] != nullptr)
            FORhead->children.push_back(root->children[0]); // run initializer expression first
        FORhead->children.push_back(loop);

        if (root->children[1] == nullptr)
        {
            root->children[1] = new AstNode();
            root->children[1]->type = AstNodeType::LOGICAL;
            root->children[1]->data = new bool(true);
        }
        loop->children.push_back(root->children[1]); // set loop condition
        for (AstNode* expr : root->children[3]->children)
        {
            loop->children.push_back(expr);
        }
        if (root->children[2] != nullptr)
            loop->children.push_back(root->children[2]); // push increment expression

        dispatch(FORhead, program, metaData);
        delete loop;
        delete FORhead;
        break;
    }
    case AstNodeType::FUNCTION_DECLARATION:
    {
        char* payload = new char[sizeof(unsigned int) + sizeof(int) ];
        unsigned int arity = root->children[0]!= nullptr? root->children[0]->children.size() : 0;
        memcpy(payload, &arity, sizeof(unsigned int));// set arity
        char* seq_beg = program.instruction;
        EmitInstructionWithPayload(OpCodes::CREATE_FUNCTION, program, payload, sizeof(unsigned int) + sizeof(int));
        int* function_size_patch =(int*)(program.instruction - sizeof(int));
        int curr_offset = program.instruction_offset;

        CompilationMeta functionMetaData = {};
        functionMetaData.enclosing = &metaData;
        functionMetaData.scope = 1;
        functionMetaData.scope_variables = prepareFunctionScope(root->children[0], metaData);
        injectNilReturn(root->children[1]);
        dispatch(root->children[1], program, functionMetaData);

        int size = program.instruction_offset - curr_offset;
        *function_size_patch = program.instruction_offset - curr_offset; // patch bytecode size
        compileUpvalues(program.instruction - size , program, functionMetaData);
        size = program.instruction_offset - curr_offset;

        break;
    }
    case AstNodeType::OP_CALL:
    {
        // emit args first
        if (root->children[1] != nullptr)
        {
            for (auto child : root->children[1]->children)
            {
                dispatch(child, program, metaData);
            }
        }

        dispatch(root->children[0], program, metaData);
        EmitInstruction(OpCodes::CALL, program);
        break;
    }
    case AstNodeType::OP_RETURN:
    {
        dispatch(root->children[0], program, metaData);
        EmitInstruction(OpCodes::RETURN, program);
        break;
    }
    default:
        cout << "BACKEND ERROR: Unsupported instruction !!!!" << endl;
        exit(-1);
        break;
    }
}

void injectNilReturn(AstNode* root)
{
    if (root->children[root->children.size() - 1]->type != AstNodeType::OP_RETURN)
    {
        AstNode* ret_node, * nil_node;
        nil_node = new AstNode;
        nil_node->type = AstNodeType::NIL;
        nil_node->data = nullptr;

        ret_node = new AstNode;
        ret_node->type = AstNodeType::OP_RETURN;
        ret_node->children.push_back(nil_node);
        ret_node->data = new string("return: ");
        root->children.push_back(ret_node);
    }
}

void resolveUpvalue(const char* string, InstructionSequence& program, CompilationMeta& metaData)
{
    CompilationMeta* currentlyProcessed = metaData.enclosing;
    int indexGlobal = 0;
    auto existing = metaData.existingUpvalues.find(string);
    if (existing != metaData.existingUpvalues.cend())
    {
        EmitInstructionWithPayload(OpCodes::GET_UPVALUE, program, &existing->second.table_index, sizeof(unsigned int));
        return ;
    }

    while (currentlyProcessed->enclosing != nullptr) // if enclosing is nullptr that means we are in global scope which has been checked
    {   
        int index = 0;
        if (FindIdentifierLocal(*currentlyProcessed, string, index) == 1)
        {
            indexGlobal += index;
            UpvalueDescriptor uvDes = {};
            uvDes.stack_pos = index;
            uvDes.table_index = metaData.existingUpvalues.size();
            metaData.existingUpvalues.emplace(string, uvDes);
            EmitInstructionWithPayload(OpCodes::GET_UPVALUE, program, &uvDes.table_index, sizeof(unsigned int));
            return;
        }
        indexGlobal += index;
        currentlyProcessed = currentlyProcessed->enclosing;
    }

    cout << "BACKEND ERROR: Unknown variable" << endl;
    exit(-1);

}

void compileUpvalues(char* base_program_pos, InstructionSequence& program, CompilationMeta& metaData)
{
    unsigned int compiledFunctionSize = program.instruction - base_program_pos ;

    int up_value_count = metaData.existingUpvalues.size();
    int total_upvalues_size = up_value_count * sizeof(UpvalueDescriptor);
    char* functionBodyBuffer = new char[sizeof(int) + total_upvalues_size + compiledFunctionSize];
    char* functionBodyBufferBase = functionBodyBuffer;

    *(int*)functionBodyBuffer = up_value_count;
    functionBodyBuffer += sizeof(int);
    for (auto upvalue : metaData.existingUpvalues)
    {
        memcpy(functionBodyBuffer, &upvalue.second, sizeof(UpvalueDescriptor));
        functionBodyBuffer += sizeof(UpvalueDescriptor);
    }

    memcpy(functionBodyBuffer, base_program_pos, compiledFunctionSize);
    AdjustProgramSize(total_upvalues_size, program);
    memcpy(base_program_pos, functionBodyBufferBase, sizeof(int) + total_upvalues_size + compiledFunctionSize);
    program.instruction_offset += total_upvalues_size + sizeof(int);
    program.instruction += total_upvalues_size + sizeof(int);
    delete[] functionBodyBufferBase;
}

ScopedVariables prepareFunctionScope(AstNode* params, CompilationMeta& metaData)
{
    /* 
    stack for function looks like  
    unaccesable_stack_variable, param_1, param_2, ..., param_k, function_frame, block_frame, ....
    */
    ScopedVariables out;
    out.push_back(metaData.scope_variables[0]); // push global variables
    out.push_back(unordered_map<string, int>()); // params layer
    int i;
    if (params != nullptr)
    {
        for (i = 0; i < params->children.size(); i++)
        {
            out[1].emplace(*(string*)params->children[i]->data, i);
        }
    }
    out[1].emplace("function store object", i); // offset by -1 because of function_frame
    return out;
}

InstructionSequence backend(const std::vector<AstNode*>& AstSequence)
{
    InstructionSequence program;
    program.instruction = new char[10000];
    program.instruction_offset = 0;
    program.size = 10000;
    program.stringTable = new char*[10]; 
    program.string_count = 0;
    program.table_size = 10;
    CompilationMeta metaData;
    metaData.enclosing = nullptr;
    metaData.scope = 0;
    metaData.scope_variables.push_back(unordered_map<string, int>());
    for (AstNode* root : AstSequence)
    {
        dispatch(root, program, metaData);
        // if OP_CALL is not wrapped in another statement that means returned value is not gonna be used anymore so pop it from stack
        if (root->type == AstNodeType::OP_CALL) EmitInstruction(OpCodes::POP, program);
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
