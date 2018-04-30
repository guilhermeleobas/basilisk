#include <iomanip>
#include <iostream>
#include <set>

#include "pin.H"
#include "instlib.H"

using namespace INSTLIB;

ofstream out;
// FILTER filter;
// KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "pin.out",
//                             "specify output file name");

static long long unsigned int scount = 0;
static long long unsigned int collect_data_count = 0;
static bool valid = true;

void docount(){
  if (valid)
    scount++;
  else
    collect_data_count++;
}

VOID mark_start(){
  valid = false;
}

VOID mark_end(){
  valid = true;
}

VOID Image(IMG img, VOID *v){

    // for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym)){
    //   string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
    //   out << undFuncName << ' ' << SYM_Name(sym) << endl;
    // }
    // out << endl;

    RTN rtn = RTN_FindByName(img, "count_instruction");
    if (RTN_Valid(rtn)){
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)mark_start, IARG_END);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)mark_end, IARG_END);
        RTN_Close(rtn);
    }
    // else{
    //   out << "Not found" << endl;
    // }
}

VOID Trace(TRACE trace, VOID *a) {
  // if (!filter.SelectTrace(trace))
    // return;

  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

      if (INS_IsMemoryWrite(ins)){
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount,
          IARG_END);
        out << INS_Disassemble(ins) << " - " << INS_Mnemonic(ins) << endl;
      }

      // out << hex << setw(8) << INS_Address(ins) << " ";
      //
      // RTN rtn = TRACE_Rtn(trace);
      // if (RTN_Valid(rtn))
      // {
      //     IMG img = SEC_Img(RTN_Sec(rtn));
      //     if (IMG_Valid(img)) {
      //        out << IMG_Name(img) << ":" << RTN_Name(rtn) << " " ;
      //     }
      // }
      //
      // out << INS_Disassemble(ins) << endl;
    }
  }
}

VOID Fini(INT32 code, VOID *v) {
  out << dec << scount << endl;
  // out << dec << collect_data_count << endl;
  out.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage() {
  cerr << "This pin tool demonstrates use of FILTER to identify "
          "instrumentation points\n"
          "\n";

  return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]) {
  if (PIN_Init(argc, argv)) {
    return Usage();
  }

  out.open("pin.out");

  PIN_InitSymbols();
  IMG_AddInstrumentFunction(Image, 0);

  TRACE_AddInstrumentFunction(Trace, 0);

  PIN_AddFiniFunction(Fini, NULL);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
