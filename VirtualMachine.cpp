#include "VirtualMachine.h"
#include <iostream>
using namespace std;
#define AS_DOUBLE(ptr) *(double*)(ptr)


double (*operation)(double, double);

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

void compile_and_execute(const std::string& source)
{
    VirtualMachine vm;
    InstructionSequence program = compile(source);

    vm.Execute(program.instruction - program.instruction_offset);
}

VirtualMachine::VirtualMachine()
{
    ip_base = nullptr;
    ip = nullptr;
}

void VirtualMachine::Execute(char* instructionData)
{
    while (true)
    {
        uint16_t instruction = *instructionData;
        instructionData += 2;
        OpCodes instructionCode = valueToOpcode(instruction);
        switch (instructionCode)
        {
        case OpCodes::EXIT:
            return;
        case OpCodes::PUSH_IMMIDIATE:
        {
            LoxObject c;
            c.type = LoxType::VALUE;
            c.data = new double;
            AS_DOUBLE(c.data) = AS_DOUBLE(instructionData);
            Push(c);
            instructionData += sizeof(double);
            break;
        }
        case OpCodes::SUBTRACT:
        case OpCodes::MULTIPLY:
        case OpCodes::DIVIDE:
        case OpCodes::ADD:
        {
            if (instructionCode == OpCodes::ADD) operation = add;
            if (instructionCode == OpCodes::DIVIDE) operation = divide;
            if (instructionCode == OpCodes::MULTIPLY) operation = multiply;
            if (instructionCode == OpCodes::SUBTRACT) operation = subtract;
            LoxObject b = Pop();
            LoxObject a = Pop();

            LoxObject c;
            c.type = LoxType::VALUE;
            c.data = new double;
            AS_DOUBLE(c.data) = operation(AS_DOUBLE(a.data), AS_DOUBLE(b.data) );

            double x = AS_DOUBLE(c.data);
            delete b.data;
            delete a.data;
            Push(c);
            break;
        }
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
