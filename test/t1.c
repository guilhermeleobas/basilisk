#include <stdio.h>

int s = 0;

int sum(int *v, int n){
  /* int s = 0; */
  
  for (int i=0; i<n; i++)
    s += v[i];
  
  return s;
}

int main (){
  
  int n = 1;
  int v[n];

  for (int i=0; i<n; i++)
    v[i] = i;
  
  sum(v, n);
  
  printf("%d\n", s);

  return 0;
}
