#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "collect.h"

void count_instruction(char *name){
  for (int i=0; i<size; i++){
    if (strcmp(array[i].name, name) == 0 /* found */){
      ++array[i].counter;
      return;
    }
  }

  strcpy(array[size].name, name);
  ++array[size].counter;
  ++size;
}

void dump_csv(){

  FILE *f;
  f = fopen(FILENAME, "w");
  if (f != NULL){
    
    fprintf(f,"STORE,LOAD,CMP,ADD,SUB,MUL,UDIV,SDIV,UREM,SREM,FADD,FSUB,FMUL,FDIV,FCMP,AND,OR,XOR,BR,CALL,SELECT\n");
    fprintf(f, "%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n", 
              store_inc, load_inc, icmp_inc, add_inc, sub_inc,
              mul_inc, udiv_inc, sdiv_inc, urem_inc, srem_inc,
              fadd_inc, fsub_inc, fmul_inc, fdiv_inc, fcmp_inc,
              and_inc, or_inc, xor_inc, br_inc, call_inc, select_inc);

    /* if (size){ */
    /*   fprintf(f, "%s", array[0].name); */
    /*   for (int i=1; i<size; i++) */
    /*     fprintf(f, ",%s", array[i].name); */
    /*  */
    /*   fprintf(f, "\n"); */
    /*  */
    /*   fprintf(f, "%llu", array[0].counter); */
    /*   for (int i=1; i<size; i++) */
    /*     fprintf(f, ",%llu", array[i].counter); */
    /*   fprintf(f, "\n"); */
    /*  */
    /* } */
    /* fclose(f); */
  }
  else {
    printf("Cannot create file\n");
  }
}