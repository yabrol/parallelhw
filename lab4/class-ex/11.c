#include <stdio.h>
#include <omp.h>

#define N 256

int main (void)
{
  int i = 0;
  int a[N];

  for (i=0; i < N; i++) a[i] = i;

  i = 17;

#pragma omp parallel num_threads(4) private(i)
{
  /* i is private */
}
  return 0;
}
