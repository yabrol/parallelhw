#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include "mw_api.h"

struct work_t {
	int t; 
};

struct result_t {
   	int t;
};

work_unit** create_work(int argc, char **argv){
	int size =10;
	work_unit** t=(work_unit**)malloc(size*sizeof(work_unit*)); //array
	int i=0;	
	for(i=0;i<size;i++){
		work_unit* temp = (work_unit*)malloc(sizeof(work_unit));
		temp->t = 9;
		t[i]=temp;
	}
	return t;
}

int process_results(int sz, result_unit *res){
	int result;
	result = res->t;
	printf("%d\n",result);
	return result;
}

result_unit* do_work(work_unit *work){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	res->t=work->t;
	return res;
}

// Assume that a and b point to array with at least length elements. 
// Assume that none of the intermediate values overflows a double. 
double dotProduct(double *a, double *b, int length) {
	int index = 0;
	double runningSum=0.0;
	for (index = 0; index < length; index++)
	runningSum += a[index] * b[index];
	return runningSum;
}

int main (int argc, char **argv)
{
  struct mw_api_spec f;

  //MPI_Init (&argc, &argv);

  f.create = create_work;
  f.compile = process_results;
  f.compute = do_work;
  f.work_sz = sizeof (work_unit);
  f.res_sz = sizeof (result_unit);
  MW_Run (argc, argv, &f);
  testing();
  //MPI_Finalize ();
  
  return 0;

}
