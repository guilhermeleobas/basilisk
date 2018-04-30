#include <iomanip>
#include <iostream>
#include <set>

#include "pin.H"
#include "instlib.H"

using namespace INSTLIB;

ofstream out;
FILTER filter;

VOID Image(IMG img, VOID *v){

    // for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym)){
    //   string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
    //   out << undFuncName << ' ' << SYM_Name(sym) << endl;
    // }
    // out << endl;

}

VOID Trace(TRACE trace, VOID *a) {

  if (!filter.SelectTrace(trace))
    return;

  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

      // if (INS_IsMemoryWrite(ins)){
      //   INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount,
      //     IARG_END);
      // }
      if (INS_IsMemoryRead(ins)){
        out << INS_Disassemble(ins) << " - " << INS_Mnemonic(ins) << " with load" << endl;
      }
      else {
        out << INS_Disassemble(ins) << " - " << INS_Mnemonic(ins) << endl;
      }

    }
  }
}

VOID Fini(INT32 code, VOID *v) {
  out.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage() {
  cerr << "Error\n";
  return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]) {
  if (PIN_Init(argc, argv)) {
    return Usage();
  }

  out.open("print.out");

  PIN_InitSymbols();
  IMG_AddInstrumentFunction(Image, 0);

  filter.Activate();

  TRACE_AddInstrumentFunction(Trace, 0);

  PIN_AddFiniFunction(Fini, NULL);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
