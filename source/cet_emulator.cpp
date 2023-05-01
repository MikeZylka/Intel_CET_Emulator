#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <vector>
#include <utility>
#include <unordered_map>
#include "pin.H"

#define ENDBR64 4196274163
#define ENDBR32 4213051379
#define NOTRACK 2212560702

// Data structure that will store the IDs of routines compiled with IBT
std::unordered_map<ADDRINT, BOOL> COMPATIBLE_RTNS;

// Shadow stack implemented as a stack of instruction pointer addresses
std::stack<ADDRINT> SHADOW_STACK;


/* ======================================================================== */
/* Function called for every indirect branch instruction inside routines    */
/* compiled with IBT.                                                       */
/* ======================================================================== */
VOID IBT_Instrumentation(ADDRINT* target_intr) {
    // Determine routine from target address
    PIN_LockClient();
    RTN rtn = RTN_FindByAddress((ADDRINT) target_intr);
    PIN_UnlockClient();

    // Retrieve the routine ID from the routine
    UINT32 rtn_id = RTN_Id(rtn);

    // Check if the routine is compiled with IBT
    if (COMPATIBLE_RTNS.count(rtn_id)) {
        UINT32 instruction;
        // Read the instruction at the branch target address
        PIN_SafeCopy(&instruction, target_intr, sizeof(UINT32));


        // Check if the instruction is ENDBR64
        if (instruction != ENDBR64 && instruction != ENDBR32) {
            std::cout << "Branch target address is not endbr! Execution stopped."  << std::endl;
            PIN_ExitProcess(1);
        }
    }

}

/* ======================================================================== */
/* Image Loading to determine IBT compatibility.                            */
/* Function called for every loaded image in the application.               */
/* ======================================================================== */
VOID ImageLoad(IMG img, VOID* v) {

    if (IMG_HasProperty(img, IMG_PROPERTY_IBT_ENABLED)) {
        std::cout << IMG_Name(img) << std::endl;
    }

    // Iterate over image sections
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        // Iterate over routines in section
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            // Open the routine for analysis
            RTN_Open(rtn);

            // Get the first instruction of the routine
            ADDRINT* first_instr = (ADDRINT*) RTN_Address(rtn);
            UINT32 instruction;
            PIN_SafeCopy(&instruction, first_instr, sizeof(UINT32));
 
            // Check if the first instruction is an ENDBR64 instruction
            if (instruction == ENDBR64 || instruction == ENDBR64) {
                // Mark routine as IBT compatible
                COMPATIBLE_RTNS[RTN_Id(rtn)] = true;

                // Instument indirect branched inside of routine
                for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
                    // Check if the instruction is an indirect branch 
                    // that is not a return or an xend or an xbegin instruction
                    if (INS_IsIndirectControlFlow(ins) && !INS_IsRet(ins) && INS_Opcode(ins) != XED_ICLASS_XEND && INS_Opcode(ins) != XED_ICLASS_XBEGIN) {
                        
                        // Get value at instruction pointer
                        ADDRINT* notrack_check = (ADDRINT*) INS_Address(ins);
                        UINT32 notrack_instr;
                        PIN_SafeCopy(&notrack_instr, notrack_check, sizeof(UINT32));

                        // Check to see if instruction is not a notrack jump
                        if (notrack_instr != NOTRACK)
                            // Insert a call to IBT_Instrumentation before the branch instruction
                            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IBT_Instrumentation, IARG_BRANCH_TARGET_ADDR, IARG_END);
                    }

                }
            }

            // Close the routine after analysis
            RTN_Close(rtn);

        }
    }
}

/* ======================================================================== */
/* Clears routines from compatible ibt routines when image is unloaded      */
/* ======================================================================== */
VOID ImageUnload(IMG img, VOID* v) {
    // Iterate image sections
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        // Iterate through routines in sections
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {

            // Open the RTN.
            RTN_Open(rtn);

            // Remove all compatible routines in section
            // This needs to be done in case an image gets unloaded and reloaded since IDs change
            COMPATIBLE_RTNS.erase(RTN_Id(rtn));

            // Close the RTN.
            RTN_Close(rtn);
        }
    }
    
}

/* ===================================================================== */
/* Push address into shadow stack                                        */
/* ===================================================================== */
VOID SHSTK_Push(ADDRINT ip, UINT32 size) {
    SHADOW_STACK.push(ip + size);
}

/* ===================================================================== */
/* Pop address from shadow stack and compare return addresses            */
/* ===================================================================== */
VOID SHSTK_PopAndCheck(CONTEXT* ctxt) {
    // Get the current stack pointer
    ADDRINT* stack_addr = (ADDRINT*)PIN_GetContextReg(ctxt, REG_STACK_PTR);
    ADDRINT ret_addr;

    // Get the return address from the top of the shadow stack
    PIN_SafeCopy(&ret_addr, stack_addr, sizeof(ADDRINT));

    // Check if the return address matches the top of the shadow stack
    if (!SHADOW_STACK.empty() && SHADOW_STACK.top() != ret_addr) { 
        std::cout << "Ret Address Overwritten! Execution stopped." << std::endl;
        PIN_ExitProcess(1);
    }

    // Pop the top of the shadow stack
    SHADOW_STACK.pop();
}

/* ===================================================================== */
/* Instrument all call and ret instructions                              */
/* ===================================================================== */
VOID InstrumentInstruction(INS ins, VOID* v) {
    // Check to see if this is a call instruction
    if(INS_IsCall(ins) || INS_IsFarCall(ins)) {
        // Insert SHSTK_Push before a call instruction
        USIZE ins_size = INS_Size(ins);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)SHSTK_Push, IARG_INST_PTR, IARG_UINT32, ins_size, IARG_END);
    }
    // Check to see if this is a ret instruction
    else if (INS_IsRet(ins) || INS_IsFarRet(ins)){
        // Insert SHSTK_PopAndCheck before a ret instruction
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)SHSTK_PopAndCheck, IARG_CONTEXT, IARG_END);
    }
}

 
/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
 
int main(int argc, char* argv[])
{
    // Initialize symbol table code, needed for rtn instrumentation
    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);
 
    // Handles image loading
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Handles image unloading
    IMG_AddUnloadFunction(ImageUnload, 0);
    
    // Handles Instrumentation of every call and ret insturction
    INS_AddInstrumentFunction(InstrumentInstruction, 0);
 
    // Start the program, never returns
    PIN_StartProgram();
 
    return 0;
}