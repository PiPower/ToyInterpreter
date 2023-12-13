#include "VirtualMachine.h"
#include <iostream>
#include <algorithm>
#include <chrono>
using namespace std;

void compile_and_execute(const std::string& source)
{
    VirtualMachine vm;
    InstructionSequence program = compile(source);

    vm.Execute(program);
}

VirtualMachine::VirtualMachine()
{
    total_time = 0;
    stack_base = 0;
    currentFunction = nullptr;
}

void VirtualMachine::Execute(InstructionSequence program)
{
    char* instructionData = program.instruction - program.instruction_offset;
    while (true)
    {
        chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        uint16_t instruction = *instructionData;
        instructionData += 2;
        OpCodes instructionCode = valueToOpcode(instruction);
        op_type op;
        switch (instructionCode)
        {
        case OpCodes::EXIT:
            return;
        case OpCodes::POP:
            Pop();
            break;
        case OpCodes::NEGATE:
        {
            const LoxObject* obj = Pop();
            if (obj->type != LoxType::NUMBER)
            {
                cout << "VM ERROR: cannot negate non-number types \n";
                exit(-1);
            }
            obj->value.number = -obj->value.number;
            Push(obj);
            break;
        }
        case OpCodes::PUSH_IMMIDIATE:
        {
            char obj_type = *instructionData;
            instructionData += 1;

            const LoxObject* c = LoadObject(&instructionData, program.stringTable, obj_type);
            Push(c);
            break;
        }
        case OpCodes::DEFINE_GLOBAL_VARIABLE:
        {
            int index = *(int*)instructionData;
            instructionData += sizeof(int);
            const LoxObject* place_holder = CreateLoxObject(LoxType::NIL);
            InsertGlobal(program.stringTable[index], place_holder);
            break;
        }
        case OpCodes::SET_GLOBAL_VARIABLE:
        {
            int index = *(int*)instructionData;
            instructionData += sizeof(int);
            const LoxObject* obj = Pop();
            UpdateGlobal(program.stringTable[index], obj);
            break;
        }
        case OpCodes::GET_GLOBAL_VARIABLE:
        {
            int index = *(int*)instructionData;
            instructionData += sizeof(int);
            const LoxObject* global = GetGlobal(program.stringTable[index]);
            Push(global);
            break;
        }
        case OpCodes::DEFINE_LOCAL_VARIABLE:
        {
            int stack_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            const LoxObject* place_holder = CreateLoxObject(LoxType::NIL);
            Push(place_holder);
            break;
        }
        case OpCodes::SET_LOCAL_VARIABLE:
        {
            int stack_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            const LoxObject* obj = Pop();
            this->stack[this->stack_base + stack_offset] = obj;
            break;
        }
        case OpCodes::GET_LOCAL_VARIABLE:
        {
            int stack_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            const LoxObject* obj = this->stack[this->stack_base + stack_offset];
            Push(obj);
            break;
        }
        case OpCodes::GET_UPVALUE:
        {
            unsigned int table_index = *(unsigned int*)instructionData;
            instructionData += sizeof(unsigned int);

            Push(currentFunction->upvalueTable[table_index]);
            break;
        }
        case OpCodes::LESS_EQUAL:
        case OpCodes::LESS:
        case OpCodes::GREATER_EQUAL:
        case OpCodes::GREATER:
        case OpCodes::NOT_EQUAL:
        case OpCodes::EQUAL:
        case OpCodes::SUBTRACT:
        case OpCodes::MULTIPLY:
        case OpCodes::DIVIDE:
        case OpCodes::ADD:
        {
            const LoxObject* b = Pop();
            const LoxObject* a = Pop(); 

            op = select_op(instructionCode, a , b);
            const LoxObject* c = SetLoxObject(op(*a, *b));
            Push(c);
            break;
        }
        case OpCodes::PRINT:
        {
            const LoxObject* obj = Pop();
            printLoxObject(*obj);
            cout << endl;
            break;
        }
        case OpCodes::START_FRAME:
        {
            const LoxObject* previousFrame = CreateLoxObject(LoxType::NUMBER);
            previousFrame->value.number = stack_base;
            Push(previousFrame);
            stack_base = stack.size();
            break;
        }
        case OpCodes::END_FRAME:
        {
            stack.erase(stack.begin() + stack_base, stack.end());
            const LoxObject* previousFrame = stack[stack.size() - 1];
            stack_base = previousFrame->value.number;
            stack.pop_back();
            break;
        }
        case OpCodes::NOT:
        {
            const LoxObject* obj = Pop();
            Push(isFalsey(obj));
            break;
        }
        case OpCodes::JUMP_IF_FALSE:
        {
            int jump_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            const LoxObject* obj = Pop();

            if (isFalsey(obj)->value.boolean) instructionData += jump_offset;
            break;
        }
        case OpCodes::JUMP:
        {
            int jump_offset = *(int*)instructionData;
            instructionData += sizeof(int);

            instructionData += jump_offset;
            break;
        }
        case OpCodes::JUMP_IF_FALSE_KEEP_STACK:
        {
            int jump_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            const LoxObject* obj = Pop();

            if (isFalsey(obj)->value.boolean) instructionData += jump_offset;
            Push(obj);
            break;
        }
        case OpCodes::CREATE_FUNCTION:
        {

            unsigned int arity = *(unsigned int*)instructionData;
            instructionData += sizeof(unsigned int);
            int code_size = *(int*)instructionData;
            instructionData += sizeof(int);
            int upvalues_count = *(int*)instructionData;
            instructionData += sizeof(int);

            const LoxObject* func = CreateLoxObject(LoxType::FUNCTION);
            AS_FUNCTION(func)->arity = arity;
            AS_FUNCTION(func)->size = code_size;
            AS_FUNCTION(func)->upvalueTable.resize(upvalues_count);
            for (int i = 0; i < upvalues_count; i++)
            {
                UpvalueDescriptor* desc = (UpvalueDescriptor*)instructionData;
                instructionData += sizeof(UpvalueDescriptor);
                AS_FUNCTION(func)->upvalueTable[desc->table_index] = stack[stack_base + desc->stack_pos];
            }

            AS_FUNCTION(func)->instruction = instructionData;
            Push(func);
            instructionData += code_size;
            break;
        }
        case OpCodes::CALL:
        {
           
            const LoxObject* function = Pop();
            const LoxObject* ip_buffer = CreateLoxObject(LoxType::STATE_BUFFER);
            AS_SB(ip_buffer)->instruction = instructionData;
            AS_SB(ip_buffer)->stack_base = stack_base;
            AS_SB(ip_buffer)->stack_size = stack.size() - AS_FUNCTION(function)->arity;
            AS_SB(ip_buffer)->caller = this->currentFunction;
            this->currentFunction = (LoxFunction*) function->value.data;
            Push(ip_buffer); // store current ip

            instructionData = AS_FUNCTION(function)->instruction;
            break;
        }
        case OpCodes::RETURN:
        {
            const LoxObject* returned = Pop();
            const LoxObject* ip_buffer = nullptr;
            for (int i = stack.size() - 1; i >= 0; i--)
            {
                if (stack[i]->type == LoxType::STATE_BUFFER)
                {
                    ip_buffer = stack[i];
                    break;
                }
            }
            stack.erase(stack.begin() + AS_SB(ip_buffer)->stack_size , stack.end());
            this->stack_base = AS_SB(ip_buffer)->stack_base;
            instructionData = AS_SB(ip_buffer)->instruction;
            Push(returned);
            break;
        }
        default:
            cout << "VM ERROR: Unsupported instruction by VM!!!! \n";
            exit(-1);
            break;
        }

        chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        total_time += chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
        if (total_time >= 300)
        {
            total_time = 0;
            TraceObjects();
            SweepObjects();
        }
    }
}

const LoxObject* VirtualMachine::Pop()
{
    const LoxObject* top = stack.back();
    stack.pop_back();
    return top;
}

void VirtualMachine::Push(const LoxObject* obj)
{
    stack.push_back(obj);
}

op_type VirtualMachine::select_op(OpCodes opcode, const LoxObject* leftOperand, const LoxObject* rightOperand)
{
    if (opcode ==  OpCodes::LESS_EQUAL ||
        opcode ==  OpCodes::LESS ||
        opcode ==  OpCodes::GREATER_EQUAL ||
        opcode ==  OpCodes::GREATER ||
        opcode ==  OpCodes::NOT_EQUAL ||
        opcode ==  OpCodes::EQUAL) return logical_resolver(opcode);

    if (leftOperand->type == LoxType::NUMBER && rightOperand->type == LoxType::NUMBER) return number_resolver(opcode);
    if (leftOperand->type == LoxType::STRING && rightOperand->type == LoxType::STRING) return string_resolver(opcode);
    cout << "VM ERROR: Incorrect combination of operands \n";
    exit(-1);
}

op_type VirtualMachine::number_resolver(OpCodes opcode)
{
    switch (opcode)
    {
    case OpCodes::ADD:
        return add_number;
    case OpCodes::DIVIDE:
        return divide_number;
    case OpCodes::SUBTRACT:
        return subtract_number;
    case OpCodes::MULTIPLY:
        return multiply_number;
    default:
        cout << "VM ERROR: Unsupported number OP\n";
        exit(-1);
        break;
    }
}

op_type VirtualMachine::string_resolver(OpCodes opcode)
{
    switch (opcode)
    {
    case OpCodes::ADD:
        return concat_strings;
    default:
        cout << "VM ERROR: Unsupported string OP\n";
        exit(-1);
        break;
    }
}

op_type VirtualMachine::logical_resolver(OpCodes opcode)
{
    switch (opcode)
    {
    case OpCodes::EQUAL:
        return equalValue;
    case OpCodes::NOT_EQUAL:
        return notEqualValue;
    case OpCodes::LESS_EQUAL:
        return lessEqualValue;
    case OpCodes::LESS:
        return lessValue;
    case OpCodes::GREATER_EQUAL:
        return greaterEqualValue;
    case OpCodes::GREATER:
        return greaterValue;
    default:
        cout <<"VM ERROR: Unsupported logical operation" << endl;
        exit(-1);
    }
}

void VirtualMachine::InsertGlobal(char* string, const LoxObject* obj)
{
    unordered_map<std::string,const LoxObject*>::iterator pos = globals.find(string);
    if (pos != globals.cend())
    {
        cout << "VM ERROR: Redefinition of element \n";
        exit(-1);
    }
    globals.insert({ string, obj });
}

const LoxObject* VirtualMachine::GetGlobal(const char* string)
{
    unordered_map <std::string , const LoxObject* > ::iterator pos = globals.find(string);
    if (pos == globals.cend())
    {
        cout << "VM ERROR: Uknown element \n";
        exit(-1);
    }
    return pos->second;
}

const LoxObject* VirtualMachine::LoadObject(char** instructionData, char** stringTable, char type)
{
    LoxObject* c = new LoxObject();
    switch (type)
    {
    case 0:
    {
        c->type = LoxType::NIL;
        c->value.data = nullptr;
        break;
    }
    case 1: 
    {
        c->type = LoxType::NUMBER;
        c->value.number = *(double*)(*instructionData);
        *instructionData += sizeof(double);
        break;
    }
    case 2:
    {
        c->type = LoxType::BOOL;
        c->value.boolean = *(bool*)(*instructionData);
        *instructionData += sizeof(bool);
        break;
    }
    case 3:
    {
        c->type = LoxType::STRING;
        int index = *(int*)(*instructionData);
        *instructionData += sizeof(int);
        int string_len = strlen(stringTable[index]) + 1;
        c->value.data = new char[string_len];
        memcpy(c->value.data, stringTable[index], string_len);
        break;
    }
    default:
        cout << "VM ERROR: uknown lox type" << endl;
        exit(-1);
    }
    MarkObject(c);
    return c;
}

const LoxObject* VirtualMachine::LoadObject(LoxType type, const void* data)
{
    LoxObject* c = new LoxObject();
    switch (type)
    {
    case LoxType::NIL:
    case LoxType::NUMBER:
    case LoxType::BOOL:
        c->type = LoxType::BOOL;
        cout << "unsupported loaded object type" << endl;
        exit(-1);

    case LoxType::STRING:
    {
        c->type = LoxType::STRING;
        int string_len = strlen((char*)data) + 1;
        c->value.data = new char[string_len];
        memcpy(c->value.data, (char*)data, string_len);
        break;
    }
    default:
        cout << "VM ERROR: uknown lox type" << endl;
        exit(-1);
    }
    MarkObject(c);
    return c;
}



void VirtualMachine::UpdateGlobal(char* string, const LoxObject* obj)
{
    unordered_map <std::string, const LoxObject*>::iterator pos = globals.find(string);
    if (pos == globals.cend())
    {
        cout << "VM ERROR: Uknown element \n";
        exit(-1);
    }
    pos->second = obj;
}

const LoxObject* VirtualMachine::isFalsey(const LoxObject* obj)
{
    const LoxObject* result = CreateLoxObject(LoxType::BOOL) ;
    result->value.boolean = true;
    if(obj->type== LoxType::NIL) result->value.boolean = false;
    if (obj->type == LoxType::BOOL) result->value.boolean = obj->value.boolean;
    result->value.boolean = !result->value.boolean;
    return result;
}

const LoxObject* VirtualMachine::CreateLoxObject(LoxType type)
{
    LoxObject* out = new LoxObject();
    switch (type)
    {
    case LoxType::NUMBER:
    {
        out->type = LoxType::NUMBER;
        out->value.number = 0;
        break;
    }
    case LoxType::BOOL:
        break;
    case LoxType::NIL: 
    {
        out->type = LoxType::NIL;
        out->value.data = nullptr;
        break;
    }
    case LoxType::STATE_BUFFER:
    {
        *out = newStateBuffer();
        break;
    }
    case LoxType::STRING:
    {
        out->type = LoxType::STRING;
        out->value.data = nullptr;
        break;
    }
    case LoxType::FUNCTION:
    {
        *out = newLoxFunction();
        break;
    }
    default:
        cout << "UNKNOWN TYPE TO CREATE" << endl;
        exit(-1);
        break;
    }
    MarkObject(out);
    return out;
}

const LoxObject* VirtualMachine::SetLoxObject(const LoxObject& src)
{
    LoxObject* out = new LoxObject();
    *out = src;
    MarkObject(out);
    return out;
}

void VirtualMachine::MarkObject(const LoxObject* object)
{
    object->trackState = TrackingState::WHITE;
    tracedObjects.push_back(object);
}

void VirtualMachine::TraceObjects()
{
    for (const LoxObject* obj : stack)
    {
        ColorObject(obj);
    }

    for (auto& global : globals)
    {
        ColorObject(global.second);
    }
}

void VirtualMachine::SweepObjects()
{
    for (int i = 0; i < tracedObjects.size(); i++)
    {
        const LoxObject** obj = &tracedObjects[i];
        if ((*obj)->trackState == TrackingState::WHITE)
        {
            FreeLoxObject(*obj);
            *obj = nullptr;
        }
        else (*obj)->trackState = TrackingState::WHITE;

    }
    erase(tracedObjects, nullptr);
}


