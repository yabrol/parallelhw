#include <stdio.h>
#include <omp.h>

#define N 256

int main (void)
{
  int i = 0;
  int a[N];
  
  for (i=0; i < N; i++) a[i] = i;

#pragma omp parallel num_threads(4)
 {
#pragma omp master 
  {
    printf ("Hello there! my id = %d\n", omp_get_thread_num());
  }

#pragma omp for
   for (i=0; i < N; i++) {
	a[i] = a[i] + 2;
   }
 }
 for (i=0; i < N; i++) {
	if (a[i] != i+2) {
		printf ("Argh!\n");
        }
 }
 return 0;
}
