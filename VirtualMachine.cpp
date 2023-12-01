#include "VirtualMachine.h"
#include <iostream>
using namespace std;

void compile_and_execute(const std::string& source)
{
    VirtualMachine vm;
    InstructionSequence program = compile(source);

    vm.Execute(program);
}

VirtualMachine::VirtualMachine()
{
    stack_base = 0;
}

void VirtualMachine::Execute(InstructionSequence program)
{
    char* instructionData = program.instruction - program.instruction_offset;
    while (true)
    {
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
            LoxObject obj = Pop();
            if (obj.type != LoxType::NUMBER)
            {
                cout << "VM ERROR: cannot negate non-number types \n";
                exit(-1);
            }
            obj.value.number = -obj.value.number;
            Push(obj);
            break;
        }
        case OpCodes::PUSH_IMMIDIATE:
        {
            char obj_type = *instructionData;
            instructionData += 1;

            LoxObject c = LoadObject(&instructionData, program.stringTable, obj_type);
            Push(c);
            break;
        }
        case OpCodes::DEFINE_GLOBAL_VARIABLE:
        {
            int index = *(int*)instructionData;
            instructionData += sizeof(int);
            LoxObject place_holder;
            place_holder.type = LoxType::NIL;
            place_holder.value.data = nullptr;
            string lol = program.stringTable[index];
            InsertGlobal(program.stringTable[index], place_holder);
            break;
        }
        case OpCodes::SET_GLOBAL_VARIABLE:
        {
            int index = *(int*)instructionData;
            instructionData += sizeof(int);
            LoxObject obj = Pop();
            UpdateGlobal(program.stringTable[index], obj);
            break;
        }
        case OpCodes::GET_GLOBAL_VARIABLE:
        {
            int index = *(int*)instructionData;
            instructionData += sizeof(int);
            LoxObject global = GetGlobal(program.stringTable[index]);
            Push(global);
            break;
        }
        case OpCodes::DEFINE_LOCAL_VARIABLE:
        {
            int stack_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            LoxObject place_holder;
            place_holder.type = LoxType::NIL;
            place_holder.value.data = nullptr;
            Push(place_holder);
            break;
        }
        case OpCodes::SET_LOCAL_VARIABLE:
        {
            int stack_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            LoxObject obj = Pop();
            this->stack[this->stack_base + stack_offset] = obj;
            break;
        }
        case OpCodes::GET_LOCAL_VARIABLE:
        {
            int stack_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            LoxObject obj = this->stack[this->stack_base + stack_offset];
            Push(obj);
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
            LoxObject c;
            LoxObject b = Pop();
            LoxObject a = Pop(); 

            op = select_op(instructionCode, a , b);
            c = op(a, b);

            Push(c);
            break;
        }
        case OpCodes::PRINT:
        {
            LoxObject obj = Pop();
            printLoxObject(obj);
            cout << endl;
            break;
        }
        case OpCodes::START_FRAME:
        {
            LoxObject previousFrame;
            previousFrame.type = LoxType::NUMBER;
            previousFrame.value.number = stack_base;
            Push(previousFrame);
            stack_base = stack.size();
            break;
        }
        case OpCodes::END_FRAME:
        {
            stack.erase(stack.begin() + stack_base, stack.end());
            LoxObject previousFrame = stack[stack.size() - 1];
            stack_base = previousFrame.value.number;
            stack.pop_back();
            break;
        }
        case OpCodes::NOT:
        {
            LoxObject obj = Pop();
            Push(isFalsey(obj));
            break;
        }
        case OpCodes::JUMP_IF_FALSE:
        {
            int jump_offset = *(int*)instructionData;
            instructionData += sizeof(int);
            LoxObject obj = Pop();

            if (isFalsey(obj).value.boolean) instructionData += jump_offset;
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
            LoxObject obj = Pop();

            if (isFalsey(obj).value.boolean) instructionData += jump_offset;
            Push(obj);
            break;
        }
        case OpCodes::CREATE_FUNCTION:
        {
            int name_size = *(int*)instructionData;
            instructionData += sizeof(int);
            char* name = instructionData;
            instructionData += name_size;
            unsigned int arity = *(unsigned int*)instructionData;
            instructionData += sizeof(unsigned int);
            int code_size = *(int*)instructionData;
            instructionData += sizeof(int);

            LoxObject func = newLoxFunction();
            AS_FUNCTION(func)->name = name;
            AS_FUNCTION(func)->arity = arity;
            AS_FUNCTION(func)->instruction = instructionData;
            AS_FUNCTION(func)->size = code_size;

            InsertGlobal(AS_FUNCTION(func)->name, func);
            instructionData += code_size;
            break;
        }
        case OpCodes::CALL:
        {
            string name = "";
            name.append(instructionData);
            instructionData += name.size() + 1;
            LoxObject function = GetGlobal(name.c_str());
            LoxObject ip_buffer = newStateBuffer();
            AS_SB(ip_buffer)->instruction = instructionData;
            AS_SB(ip_buffer)->stack_base = stack_base;
            AS_SB(ip_buffer)->stack_size = stack.size() - AS_FUNCTION(function)->arity;
            Push(ip_buffer); // store current ip

            instructionData = AS_FUNCTION(function)->instruction;
            break;
        }
        case OpCodes::RETURN:
        {
            LoxObject returned = Pop();
            LoxObject ip_buffer;
            for (int i = stack.size() - 1; i >= 0; i--)
            {
                if (stack[i].type == LoxType::STATE_BUFFER)
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

    }
}

LoxObject VirtualMachine::Pop()
{
    LoxObject top = stack.back();
    stack.pop_back();
    return top;
}

void VirtualMachine::Push(LoxObject obj)
{
    stack.push_back(obj);
}

op_type VirtualMachine::select_op(OpCodes opcode, const LoxObject& leftOperand, const LoxObject& rightOperand)
{
    if (opcode ==  OpCodes::LESS_EQUAL ||
        opcode ==  OpCodes::LESS ||
        opcode ==  OpCodes::GREATER_EQUAL ||
        opcode ==  OpCodes::GREATER ||
        opcode ==  OpCodes::NOT_EQUAL ||
        opcode ==  OpCodes::EQUAL) return logical_resolver(opcode);

    if (leftOperand.type == LoxType::NUMBER && rightOperand.type == LoxType::NUMBER) return number_resolver(opcode);
    if (leftOperand.type == LoxType::STRING && rightOperand.type == LoxType::STRING) return string_resolver(opcode);
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

void VirtualMachine::InsertGlobal(char* string, LoxObject obj)
{
    unordered_map <std::string, LoxObject>::iterator pos = globals.find(string);
    if (pos != globals.cend())
    {
        cout << "VM ERROR: Redefinition of element \n";
        exit(-1);
    }
    globals.insert({ string, obj });
}

LoxObject VirtualMachine::GetGlobal(const char* string)
{
    unordered_map <std::string, LoxObject>::iterator pos = globals.find(string);
    if (pos == globals.cend())
    {
        cout << "VM ERROR: Uknown element \n";
        exit(-1);
    }
    return pos->second;
}

LoxObject VirtualMachine::LoadObject(char** instructionData, char** stringTable, char type)
{
    switch (type)
    {
    case 0:
    {
        LoxObject c;
        c.type = LoxType::NIL;
        c.value.data = nullptr;
        return c;
    }
    case 1: 
    {
        LoxObject c;
        c.type = LoxType::NUMBER;
        c.value.number = *(double*)(*instructionData);
        *instructionData += sizeof(double);
        return c;
    }
    case 2:
    {
        LoxObject c;
        c.type = LoxType::BOOL;
        c.value.boolean = *(bool*)(*instructionData);
        *instructionData += sizeof(bool);
        return c;
    }
    case 3:
    {
        LoxObject c;
        c.type = LoxType::STRING;
        int index = *(int*)(*instructionData);
        *instructionData += sizeof(int);
        c.value.data = stringTable[index];
        return c;
    }
    default:
        cout << "VM ERROR: uknown lox type" << endl;
        exit(-1);
    }
}



void VirtualMachine::UpdateGlobal(char* string, LoxObject obj)
{
    unordered_map <std::string, LoxObject>::iterator pos = globals.find(string);
    if (pos == globals.cend())
    {
        cout << "VM ERROR: Uknown element \n";
        exit(-1);
    }
    pos->second = obj;
}

LoxObject VirtualMachine::isFalsey(LoxObject& obj)
{
    LoxObject result;
    result.type = LoxType::BOOL;
    result.value.boolean = true;
    if(obj.type== LoxType::NIL) result.value.boolean = false;
    if (obj.type == LoxType::BOOL) result.value.boolean = obj.value.boolean;
    result.value.boolean = !result.value.boolean;
    return result;
}


