#include "VirtualMachine.h"
#include <iostream>
using namespace std;

#define AS_DOUBLE(ptr) *(double*)(ptr)

typedef double (*op_type)(double, double);

double add(double x, double y)
{
    return x + y;
}

double divide(double x, double y)
{
    return x / y;
}

double subtract(double x, double y)
{
    return x - y;
}

double multiply(double x, double y)
{
    return x * y;
}
op_type select_op(OpCodes opcode)
{
    switch (opcode)
    {
    case OpCodes::ADD:
        return add;
    case OpCodes::DIVIDE:
        return divide;
    case OpCodes::SUBTRACT:
        return subtract;
    case OpCodes::MULTIPLY:
        return multiply;
    default:
        cout << "Unspported OP\n";
        exit(-1);
        break;
    }
}
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
            AS_DOUBLE(c.value.data) = AS_DOUBLE(instructionData);
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
            op = select_op(instructionCode);

            LoxObject b = Pop();
            LoxObject a = Pop();

            LoxObject c;
            c.type = LoxType::VALUE;
            c.value.data = new double;
            AS_DOUBLE(c.value.data) = op(AS_DOUBLE(a.value.data), AS_DOUBLE(b.value.data) );

            delete b.value.data;
            delete a.value.data;
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
