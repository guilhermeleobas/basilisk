#pragma once

#define FILENAME "count.csv"

typedef struct Instruction{
  char name[10];
  unsigned long long counter;
} Instruction;

static Instruction array[100];
static int size = 0;

void count_instruction(char*);
void dump();