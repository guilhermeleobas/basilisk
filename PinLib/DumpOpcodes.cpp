#include <iomanip>
#include <iostream>
#include <set>
#include <map>
#include <string>

#include "pin.H"
#include "instlib.H"
#include "lib.H"

using namespace INSTLIB;

std::set<std::string> opcodes;

ofstream out;
FILTER filter;

void count_inst(const string *type){
  if (valid){
    if (prefix == "before")
      bef[*type]++;
    else if (prefix == "main")
      ma[*type]++;
    else
      en[*type]++;
  }
}

std::string check_mnemonic(const string &m){

  if (m == "ADD" || m == "INC"){
    return "ADD";
  }

  if (m == "SUB" || m == "DEC" || m == "SBB" ||
      m == "PSUBB" || m == "PSUBW" || m == "PSUBD"){
    return "SUB";
  }

  if (m == "MUL" || m == "IMUL"){
    return "MUL";
  }

  if (m == "DIV" || m == "IDIV")
    return "DIV";

  // floating-point instructions
  if (m == "ADDPD" || m == "ADDPS" || m == "ADDSD" || m == "ADDSS" ||
      m == "FADD" || m == "FADDP" || m == "FIADD")
    return "FADD";

  if (m == "SUBPD" || m == "SUBPS" || m == "SUBSD" || m == "SUBSS" ||
      m == "FSUB" || m == "FSUBP" || m == "FISUB")
    return "FSUB";

  if (m == "MULPD" || m == "MULPS" || m == "MULSD" || m == "MULSS" ||
      m == "FMUL" || m == "FMULP" || m == "FIMUL")
    return "FMUL";

  if (m == "DIVPD" || m == "DIVPS" || m == "DIVSD" || m == "DIVSS" ||
      m == "FDIV" || m == "FDIVP" || m == "FIDIV" ||
      m == "FDIVR" || m == "FDIVRP" || m == "FIDIVR")
    return "FDIV";

  return "";
}


VOID Trace(TRACE trace, VOID *a) {
  
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

      // std::string mnemonic = check_mnemonic(INS_Mnemonic(ins));
      opcodes.insert(INS_Mnemonic(ins));

    }
  }
}


VOID Fini(INT32 code, VOID *v) {
  
  for (std::set<std::string>::iterator i = opcodes.begin(), e = opcodes.end();
       i != e; ++i){
    
    std::string mnemonic = check_mnemonic(*i);
    
    if (mnemonic.size())
      out << *i << ", " << mnemonic << '\n';
    else
      out << *i << ", None\n";
  }
  
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
  
  out.open("opcodes.csv");

  // filter.Activate();

  PIN_InitSymbols();
  IMG_AddInstrumentFunction(Image, 0);

  TRACE_AddInstrumentFunction(Trace, 0);

  PIN_AddFiniFunction(Fini, NULL);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
