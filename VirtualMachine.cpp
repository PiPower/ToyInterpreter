#include "VirtualMachine.h"

void compile_and_execute(const std::string& source)
{
    VirtualMachine vm;
    InstructionSequence program = compile(source);
    //vm.Execute(instruction);
}

VirtualMachine::VirtualMachine()
{
    ip_base = nullptr;
    ip = nullptr;
}

void VirtualMachine::Execute(uint16_t* instructionData)
{
}
