#include <stdio.h>
#include <omp.h>

#define N 256

int main (void)
{
  int i = 0;
  int a[N];

  for (i=0; i < N; i++) a[i] = i;

  i = 17;

#pragma omp parallel num_threads(4)
{
#pragma omp for
  for (i=0; i < N; i++) {
	if (i == 100) { printf ("[%d] i = %d!\n", omp_get_thread_num (), i); }
	a[i] = a[i] + 2;
  }
  printf ("[%d] Value of i = %d\n", omp_get_thread_num(), i);
}

  return 0;
}
