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

VOID IBT_Instrumentation(CONTEXT* ctxt) {
    UINT32* first_instr = (UINT32*)PIN_GetContextReg(ctxt, REG_INST_PTR);
    UINT32 instruction;
    PIN_SafeCopy(&instruction, first_instr, sizeof(UINT32));
    
    //std::cout << "Instruction: " << std::hex <<  first_instr << std::endl;
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
                if (INS_IsIndirectControlFlow(ins) && !INS_IsRet(ins)) {
                    INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)IBT_Instrumentation, IARG_CONTEXT, IARG_END);
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
            UINT32* first_instr = (UINT32*) RTN_Address(rtn);
            UINT32 instruction;
            PIN_SafeCopy(&instruction, first_instr, sizeof(UINT32));
 
            // Check to see if the first instruction is an endbranch
            // This can determine if the image is compiled with IBT
            if (instruction == ENDBR64) {
                // Close routine before moving to function
                RTN_Close(rtn);
                // Instrument the Image for IBT
                InstrumentImage(img);

                // Mark this image as IBT compatible
                COMPATIBLE_IMGS[IMG_Name(img)] = true;
                return;
            }

            // Close the RTN.
            RTN_Close(rtn);
        }
    }

    COMPATIBLE_IMGS[IMG_Name(img)] = false;
}

VOID SHSTK_Push(INS ins) {
    std::cout << "Instruction: " << INS_Disassemble(ins) << std::endl;
}

VOID SHSTK_PopAndCheck() {
    
}

VOID Instruction(INS ins, VOID* v) {
    if(INS_IsCall(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)SHSTK_Push, IARG_PTR, ins, IARG_END);
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
    INS_AddInstrumentFunction(Instruction, 0);
 
    // Start the program, never returns
    PIN_StartProgram();
 
    return 0;
}