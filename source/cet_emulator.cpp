#include "pin.H"
#include <iostream>
#include <vector>

// Global vector representing the shadow stack
std::vector<ADDRINT> shadowStack;

/* ===================================================================== */
/* Trigger function whenever a call instruction is encountered           */
/* ===================================================================== */
VOID CallHandler(ADDRINT InstructionAddress) {
    
}


/* ===================================================================== */
/* Trigger function whenever a RET instruction is encountered           */
/* ===================================================================== */
VOID RetHandler(ADDRINT retAddr, CONTEXT* ctxt) { 
    
}

/* ===================================================================== */
/* Intrumentation routine that will be called by Pin                     */
/* ===================================================================== */
VOID InstructionInstrumentation(INS ins, VOID *v) {

    // Sets callback function for call instruction
    if (INS_IsCall(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CallHandler), IARG_INST_PTR, IARG_END);
    }
    // Sets callback function for ret instructions
    else if (INS_IsRet(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(RetHandler), IARG_BRANCH_TARGET_ADDR, IARG_CONST_CONTEXT, IARG_END);
    }

    // Sets callback function for any indirect branch or call instruction
    if (INS_IsIndirectControlFlow(ins)) {

    }
}

/* ===================================================================== */
/* Prints out help message                                               */
/* ===================================================================== */
INT32 Usage() {
    PIN_ERROR("A Control-flow enforcement emulator that runs a given executable with CET enabled, and detects violations of control-flow integrity.\n\
    Usage: cet_emulator [OPTIONS] -- [PROGRAM] [ARGS]\n" + \
    KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}


/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
int main(int argc, char *argv[]) {
    
    // Initialize Pin
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitSymbols();

    // Register InstructionInstrumentation to be called for every instruction
    INS_AddInstrumentFunction(InstructionInstrumentation, 0);

    // Run application
    PIN_StartProgram();

    return 0;
} 