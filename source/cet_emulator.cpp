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
VOID RetHandler(const CONTEXT* ctxt) { 
    ADDRINT inst_ptr = (ADDRINT)PIN_GetContextReg(ctxt, REG_INST_PTR);
    std::cout << "Returned to: " << std::hex << inst_ptr << std::endl;
}

/* ===================================================================== */
/* Intrumentation routine that will be called by Pin                     */
/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v) {
    // Check to see if image is compiled with Shadow Stack
    // This check is only valid for Linux OS
    if (IMG_HasProperty(img, IMG_PROPERTY_SHSTK_ENABLED)) {
        std::cout << "Image Loaded: " << IMG_Name(img) << std::endl;
        // Loop through sections of the image
        for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
            // Loop through routines in the section 
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); RTN_Next(rtn)) {
                RTN_Open(rtn);

                // Loop through all instructions in routine
                for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
                    // Instrument each return instruction
                    if (INS_IsRet(ins)) {
                        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)RetHandler, IARG_CONTEXT, IARG_END);
                    }
                }
                RTN_Close(rtn);
            }
        }
    }

    // Check to see if image is compiled with Indirect Branch Tracking
    // This check is only valid for Linux OS
    if (IMG_HasProperty(img, IMG_PROPERTY_IBT_ENABLED)) {
        std::cout << "Image Loaded: " << IMG_Name(img) << std::endl;
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
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Run application
    PIN_StartProgram();

    return 0;
} 