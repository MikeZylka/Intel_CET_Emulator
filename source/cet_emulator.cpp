#include "pin.H"
#include <iostream>

// This function will be called whenever a call instruction is encountered
VOID CallInstrumentation(ADDRINT callInstructionAddress) {
    std::cout << "Call Instruction: " << std::hex << callInstructionAddress << std:: endl; 
}

// Intrumentation routine that will be called by Pin
VOID InstructionInstrumentation(INS ins, VOID *v) {

    // Insert a call to CallInstrumentation before every call instruction
    if (INS_IsCall(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CallInstrumentation), IARG_INST_PTR, IARG_END);
    }
}

int main(int argc, char *argv[]) {
    
    // Initialize Pin
    PIN_Init(argc, argv);

    // Register InstructionInstrumentation to be called for every instruction
    INS_AddInstrumentFunction(InstructionInstrumentation, 0);

    // Run application
    PIN_StartProgram();

    return 0;
} 