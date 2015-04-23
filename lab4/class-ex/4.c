#include <stdio.h>
#include <omp.h>

int main (void)
{
  int i = 0;

#pragma omp parallel num_threads(4)
 {
   printf ("hello there, I am thread %d\n", omp_get_thread_num ());
#pragma omp parallel num_threads(2)
  {
    printf ("nested, I am thread %d\n", omp_get_thread_num());
  }
  printf ("I am done, thread %d\n", omp_get_thread_num ());
 }
 return 0;
}
