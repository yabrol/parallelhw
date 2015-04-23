#include <stdio.h>
#include <omp.h>

int main (void)
{
  int i = 0;

#pragma omp parallel 
 {
   printf ("hello there, I am thread %d\n", omp_get_thread_num());
 }
 return 0;
}
