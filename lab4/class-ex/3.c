#include <stdio.h>
#include <omp.h>

int main (void)
{
  int i = 0;

#pragma omp parallel num_threads(6)
 {
   printf ("hello there, I am thread %d of %d\n", omp_get_thread_num (), omp_get_num_threads ());
 }
 return 0;
}
