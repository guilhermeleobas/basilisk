#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "collect.h"

#define FILENAME "count.out"
#define HEADER "store,load,add,mul"

// #define MAXLEN 20
typedef struct Instruction{
  char name[10];
  unsigned long long counter;
} Instruction;

static Instruction array[100];
static int size = 0;

void count_instruction(char *name){
  printf("opa %s\n", name);
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

// typedef struct List {
//   Instruction *array;
//   unsigned size;
// } List;

// static List *list = NULL;

// void init_list(){
//   list = malloc(sizeof(List));
//   list->size = 0;
//   list->array = malloc(MAXLEN * sizeof(Instruction));

//   for (int i=0; i<MAXLEN; i++)
//     list->array[i] = NULL;
// }

// Instruction* get_elem(int pos){
//   assert (pos < MAXLEN);
//   return &list->array[pos];
// }

// void add_instruction(char *name){
//   if (list == NULL)
//     init_list();

//   for (int i=0; i<list->size; i++){
//     Instruction *inst = get_elem(i);

//   }
// }




static unsigned long long store_count = 0;
static unsigned long long load_count = 0;
static unsigned long long add_count = 0;
static unsigned long long mul_count = 0;

void increment_store_count(){
  store_count++;
}

void increment_load_count(){
  load_count++;
}

void increment_add_count(){
  add_count++;
}

void increment_mul_count(){
  mul_count++;
}

void dump(){

  for (int i=0; i<size; i++){
    printf("%s %llu\n", array[i].name, array[i].counter);
  }

  FILE *f;
  f = fopen(FILENAME, "w");
  if (f != NULL){
    fprintf(f, "%s\n", HEADER);
    fprintf(f, "%llu,%llu,%llu,%llu\n", store_count, load_count, add_count, mul_count);
    fclose(f);
  }
  else {
    printf("Cannot create file\n");
  }
}