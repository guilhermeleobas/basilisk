#pragma once

static std::map<std::string, unsigned long long int> mapa;
static bool valid = true;
static std::string prefix = "before_";


VOID mark_start(){
  valid = false;
}

VOID mark_end(){
  valid = true;
}

VOID main_start(){
  prefix = "dmain_";
}

VOID main_end(){
  prefix = "end_";
}

VOID Image(IMG img, VOID *v){
    RTN rtn = RTN_FindByName(img, "count_instruction");
    if (RTN_Valid(rtn)){
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)mark_start, IARG_END);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)mark_end, IARG_END);
        RTN_Close(rtn);
    }

    RTN rtn2 = RTN_FindByName(img, "dump_csv");
    if (RTN_Valid(rtn2)){
        RTN_Open(rtn2);
        RTN_InsertCall(rtn2, IPOINT_BEFORE, (AFUNPTR)mark_start, IARG_END);
        RTN_InsertCall(rtn2, IPOINT_AFTER, (AFUNPTR)mark_end, IARG_END);
        RTN_Close(rtn2);
    }

    RTN rtn3 = RTN_FindByName(img, "main");
    if (RTN_Valid(rtn3)){
        RTN_Open(rtn3);
        RTN_InsertCall(rtn3, IPOINT_BEFORE, (AFUNPTR)main_start, IARG_END);
        RTN_InsertCall(rtn3, IPOINT_AFTER, (AFUNPTR)main_end, IARG_END);
        RTN_Close(rtn3);
    }

}
