#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <vector>
#include <utility>
#include <unordered_map>
#include "pin.H"

#define ENDBR64 4196274163

std::unordered_map<std::string, BOOL> COMPATIBLE_IMGS;
std::stack<ADDRINT> SHADOW_STACK;
 
// Pin calls this function every time a new rtn is executed
VOID Routine(RTN rtn, VOID* v)
{
    RTN_Open(rtn);
 
    UINT32* first_instr = (UINT32*) RTN_Address(rtn);
    UINT32 instruction;
    PIN_SafeCopy(&instruction, first_instr, sizeof(UINT32));
 
    
    if (instruction == ENDBR64) {
        
    }

    RTN_Close(rtn);
}

VOID IBT_Instrumentation(ADDRINT* target_intr) {

    ADDRINT instruction;
    PIN_SafeCopy(&instruction, target_intr, sizeof(ADDRINT));
    PIN_LockClient();
    std::cout << "Image: " <<  IMG_Name(IMG_FindByAddress((ADDRINT)target_intr)) << std::endl;
    PIN_UnlockClient();
}

/* ======================================================================== */
/* Instrument every Indirect Branch in the image                            */
/* ======================================================================== */
VOID InstrumentImage(IMG img) {
    // Iterate image sections
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        // Iterate through routines in sections
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            // Open routine
            RTN_Open(rtn);

            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
                if (INS_IsIndirectControlFlow(ins) && !INS_IsRet(ins) && INS_Opcode(ins) != XED_ICLASS_XEND && INS_Opcode(ins) != XED_ICLASS_XBEGIN) {
                    //std::cout << "Instruction = " << INS_Disassemble(ins) << "; Address = " << std::hex << INS_Address(ins) << std::endl;
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IBT_Instrumentation, IARG_BRANCH_TARGET_ADDR, IARG_END);
                }
            }

            // close routine
            RTN_Close(rtn);
        }
    }
}


/* ======================================================================== */
/* Image Loading to determine IBT compatibility                             */
/* ======================================================================== */
VOID ImageLoad(IMG img, VOID* v) {
    // Iterate image sections
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        // Iterate through routines in sections
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            // Open the RTN.
            RTN_Open(rtn);
 
            // Get the first instruction of a routine
            ADDRINT* first_instr = (ADDRINT*) RTN_Address(rtn);
            UINT32 instruction32;
            PIN_SafeCopy(&instruction, first_instr, sizeof(ADDRINT));
 
            // Check to see if the first instruction is an endbranch
            // This can determine if the image is compiled with IBT
            if (instruction == ENDBR64) {
                // Close routine before moving to function
                RTN_Close(rtn);

                // Instrument the Image for IBT
                InstrumentImage(img);
                return;
            }

            // Close the RTN.
            else {
                RTN_Close(rtn);
            }
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
    if(INS_IsCall(ins) || INS_IsFarCall(ins)) {
        // Insert SHSTK_Push before a call instruction
        USIZE ins_size = INS_Size(ins);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)SHSTK_Push, IARG_INST_PTR, IARG_UINT32, ins_size, IARG_END);
    }
    else if (INS_IsRet(ins) || INS_IsFarRet(ins)){
        // Insert SHSTK_PopAndCheck before a ret instruction
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)SHSTK_PopAndCheck, IARG_CONTEXT, IARG_END);
    }
}
 
/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
 
INT32 Usage()
{
    return -1;
}
 
/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
 
int main(int argc, char* argv[])
{
    // Initialize symbol table code, needed for rtn instrumentation
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
 
    // Register Routine to be called to instrument rtn
    IMG_AddInstrumentFunction(ImageLoad, 0);
    // INS_AddInstrumentFunction(InstrumentInstruction, 0);
 
    // Start the program, never returns
    PIN_StartProgram();
 
    return 0;
}