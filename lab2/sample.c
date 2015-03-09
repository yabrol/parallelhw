#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include "api.h"

struct work_t { //same as work_unit
	int t[10]; 
};

struct result_t { //same as result_unit
   	int t;
};

work_unit** create_work(int argc, char **argv){
	int size =10;
	work_unit** t=(work_unit**)malloc(size*sizeof(work_unit*)); //2darray
	int i=0, j=0;
	for(i=0;i<size-1;i++){
		work_unit* temp = (work_unit*)malloc(sizeof(work_unit));
		for(j = 0; j < 10; j++){
			temp->t[j] = j;
		}
		//temp->t = 9;
		t[i]=temp;
	}
	t[size-1]=NULL;
	return t;
}

int process_results(int sz, result_unit **res){
	int result;
	result = 0;//res->t;
	printf("%d\n",result);
	return result;
}

result_unit* do_work(work_unit *work){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	//res->t=work->t;
	int len;
	len = sizeof(work->t);
	//res->t = sum(len,*work);
	return res;
}

//sums the array
int sum(int length, int *a) {
	int index = 0;
	double runningSum=0;
	for (index = 0; index < length; index++)
	runningSum += a[index];
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
