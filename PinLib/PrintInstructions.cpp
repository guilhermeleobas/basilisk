#include <iomanip>
#include <iostream>
#include <map>
#include <set>

#include "pin.H"
#include "instlib.H"

using namespace INSTLIB;

ofstream out;
FILTER filter;
std::map<std::string, set<std::string> > mapa;

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

      RTN rtn = TRACE_Rtn(trace);
      
      bool load = false, store = false;
      
      if (INS_IsMemoryRead(ins))
        load = true;
      if (INS_IsMemoryWrite(ins))
        store = true;
      
      if (RTN_Valid(rtn)){
        IMG img = SEC_Img(RTN_Sec(rtn));
        if (IMG_Valid(img)){
          out << IMG_Name(img) << ":" << RTN_Name(rtn) << " " ;
        }

      }

      out << INS_Disassemble(ins) << ' ' << load << ' ' << store << ' ' << INS_Mnemonic(ins) << endl;

      std::string m = INS_Mnemonic(ins);
      if (m.find("SUB") != std::string::npos){
        mapa["SUB"].insert (m);
      }

    }
  }
}

VOID Fini(INT32 code, VOID *v) {
  for (set<std::string>::iterator it = mapa["SUB"].begin();
       it != mapa["SUB"].end(); it++){
    out << *it << ' ';
  }
  out << "\n";
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
