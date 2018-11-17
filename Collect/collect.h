#pragma once

#define FILENAME "count.csv"

#define OPCODES_LENGTH 300

typedef struct Instruction{
  char type[10];
  unsigned long long counter;
} Instruction;

static Instruction array[300];
static int size = 0;

void count_instruction(char *type);
void dump_csv();
