#include <stdio.h>
#include <omp.h>

int main (void)
{
  int i = 0;

#pragma omp parallel num_threads(4)
 {
   int tid = omp_get_thread_num ();
   printf ("hello there, I am thread %d\n", tid);
#pragma omp parallel num_threads(2)
  {
    printf ("nested, I am thread %d (in %d)\n", omp_get_thread_num(), tid);
  }
  printf ("I am done, thread %d\n", omp_get_thread_num ());
 }
 return 0;
}
