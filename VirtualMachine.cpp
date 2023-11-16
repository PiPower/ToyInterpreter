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
    ip_base = nullptr;
    ip = nullptr;
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
        case OpCodes::DEFINE_GLOBAL_VARIABLE:
        {
            int index = *(int*)instructionData;
            instructionData += sizeof(int);
            LoxObject place_holder;
            place_holder.type = LoxType::NIL;
            place_holder.value.data = nullptr;
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
        case OpCodes::PUSH_STRING:
        {
            LoxObject c;
            c.type = LoxType::STRING;
            int index = *(int*)instructionData;
            instructionData += sizeof(int);
            c.value.data = program.stringTable[index];
            Push(c);
            break;
        }
        case OpCodes::PUSH_IMMIDIATE:
        {
            LoxObject c;
            c.type = LoxType::VALUE;
            c.value.data = new double;
            c.value.number = AS_DOUBLE(instructionData);
            Push(c);
            instructionData += sizeof(double);
            break;
        }
        case OpCodes::PUSH_NIL:
        {
            LoxObject c;
            c.type = LoxType::NIL;
            c.value.data = nullptr;
            Push(c);
            break;
        }
        case OpCodes::PUSH_BOOL:
        {
            LoxObject c;
            c.type = LoxType::BOOL;
            c.value.boolean = *(bool*)instructionData;
            Push(c);
            instructionData += sizeof(double);
            break;
        }
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

            FreeLoxObject(a);
            FreeLoxObject(b);
            Push(c);
            break;
        }
        case OpCodes::PRINT:
            printLoxObject(Pop());
            cout << endl;
            break;
        default:
            cout << "Unsupported instruction by VM!!!! \n";
            exit(-1);
            break;
        }

    }
}

LoxObject VirtualMachine::Pop()
{
    LoxObject top = stack.top();
    stack.pop();
    return top;
}

void VirtualMachine::Push(LoxObject obj)
{
    stack.push(obj);
}

op_type VirtualMachine::select_op(OpCodes opcode, const LoxObject& leftOperand, const LoxObject& rightOperand)
{
    if (leftOperand.type == LoxType::VALUE && rightOperand.type == LoxType::VALUE) return number_resolver(opcode, leftOperand, rightOperand);
    if (leftOperand.type == LoxType::STRING && rightOperand.type == LoxType::STRING) return string_resolver(opcode, leftOperand, rightOperand);
    cout << "Incorrect combination of operands \n";
    exit(-1);
}

op_type VirtualMachine::number_resolver(OpCodes opcode, const LoxObject& leftOperand, const LoxObject& rightOperand)
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
        cout << "Unspported OP\n";
        exit(-1);
        break;
    }
}

op_type VirtualMachine::string_resolver(OpCodes opcode, const LoxObject& leftOperand, const LoxObject& rightOperand)
{
    switch (opcode)
    {
    case OpCodes::ADD:
        return concat_strings;
    default:
        cout << "Unspported OP\n";
        exit(-1);
        break;
    }
}

void VirtualMachine::InsertGlobal(char* string, LoxObject obj)
{
    unordered_map <std::string, LoxObject>::iterator pos = globals.find(string);
    if (pos != globals.cend())
    {
        cout << "ERROR Redefinition of element \n";
        exit(-1);
    }
    globals.insert({ string, obj });
}

LoxObject VirtualMachine::GetGlobal(char* string)
{
    unordered_map <std::string, LoxObject>::iterator pos = globals.find(string);
    if (pos == globals.cend())
    {
        cout << "ERROR Uknown element \n";
        exit(-1);
    }
    return pos->second;
}

void VirtualMachine::UpdateGlobal(char* string, LoxObject obj)
{
    unordered_map <std::string, LoxObject>::iterator pos = globals.find(string);
    if (pos == globals.cend())
    {
        cout << "ERROR Uknown element \n";
        exit(-1);
    }
    pos->second = obj;
}


