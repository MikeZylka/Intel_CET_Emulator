#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <unordered_map>
#include "pin.H"

#define ENDBR64 4196274163

std::unordered_map<std::ADDR, int> COMPATIBLE_PROCEDURES;
 
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

VOID ImageLoad(IMG img, VOID* v) {
    
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
 
    // Start the program, never returns
    PIN_StartProgram();
 
    return 0;
}