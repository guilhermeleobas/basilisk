#include <stdio.h>
#include <stdlib.h>

void collect_d(int *sum){
  FILE *f;
  f = fopen("count.out", "w");
  if (f != NULL){
    fprintf(f, "Sum is: %d\n", sum);
    fclose(f);
  }
  else {
    printf("Cannot create file\n");
  }
}
