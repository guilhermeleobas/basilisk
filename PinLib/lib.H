#pragma once

static std::map<std::string, unsigned long long int> mapa;
static bool valid = true;
static std::string prefix = "before_";

static std::string load  = "load";
static std::string store = "store";

VOID mark_start(){
  valid = false;
}

VOID mark_end(){
  valid = true;
}

VOID main_start(){
  prefix = "main_";
}

VOID main_end(){
  prefix = "end_";
}

VOID Image(IMG img, VOID *v){

    RTN rtn = RTN_FindByName(img, "main");
    if (RTN_Valid(rtn)){
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)main_start, IARG_END);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)main_end, IARG_END);
        RTN_Close(rtn);
    }

}
