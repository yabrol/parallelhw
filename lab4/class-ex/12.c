#include <stdio.h>
#include <omp.h>

#define N 256

int main (void)
{
  int i = 0;
  int a[N];

  for (i=0; i < N; i++) a[i] = i;

  i = 17;

#pragma omp parallel num_threads(4) firstprivate(i)
{
  /* i is private */

  /* code goes here */

#pragma omp barrier

  /* more code goes here */
}
  return 0;
}

/*
  Other constructs:

#pragma omp critical
   critical_section();

#pragma omp atomic
   x += a[i];
   ... an atomic operation ...
*/
