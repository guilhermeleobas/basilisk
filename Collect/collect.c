#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collect.h"

void count_instruction(char *type) {
  for (int i = 0; i < size; i++) {
    if (strcmp(array[i].type, type) == 0 /* found */) {
      ++array[i].counter;
      return;
    }
  }
  
  // Create a new entry
  strcpy(array[size].type, type);
  ++array[size].counter;
  ++size; // The total number of ids
}

void dump_csv() {

  FILE *f;
  f = fopen(FILENAME, "w");
  if (f != NULL) {

    if (size) {
      fprintf(f, "%s", array[0].type);
      for (int i = 1; i < size; i++)
        fprintf(f, ",%s", array[i].type);

      fprintf(f, "\n");

      fprintf(f, "%llu", array[0].counter);
      for (int i = 1; i < size; i++)
        fprintf(f, ",%llu", array[i].counter);
      fprintf(f, "\n");
    }
    fclose(f);
  } else {
    printf("Cannot create file\n");
  }
}
