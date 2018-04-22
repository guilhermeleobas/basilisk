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

void dump(){

  FILE *f;
  f = fopen(FILENAME, "w");
  if (f != NULL){
    if (size){
      fprintf(f, "%s", array[0].name);
      for (int i=1; i<size; i++)
        fprintf(f, ",%s", array[i].name);

      fprintf(f, "\n");

      fprintf(f, "%llu", array[0].counter);
      for (int i=1; i<size; i++)
        fprintf(f, ",%llu", array[i].counter);

    }
    fclose(f);
  }
  else {
    printf("Cannot create file\n");
  }
}