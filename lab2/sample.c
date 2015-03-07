#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include "mw_api.h"

struct work_t {
	double t; 
};

struct result_t {
   	double t;
};

work_unit** create_work(int argc, char **argv){
	int size =6;
	printf("%d\n",size);
	work_unit** t=(work_unit**)malloc(size*sizeof(work_unit*)); 
	int i=0;	
	for(i=0;i<size-1;i++){
		work_unit* temp = (work_unit*)malloc(sizeof(work_unit));
		temp->t = (double)i;
		t[i]=temp;
	}
	t[size-1]=NULL;
	printf("created_work\n");
	return t;
}

int process_results(int sz, result_unit **res){
double result=0;
int i=0;
for(i=0;i<sz;i++)
{result += res[i]->t;

printf("result %f %f %d\n",res[i]->t,result,sz);
}
printf("%f\n",result);
return result;
}

result_unit* do_work(work_unit *work){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	res->t= work->t;
	printf("doing work %f\n",res->t);
	return res;
}

int main (int argc, char **argv)
{
  struct mw_api_spec f;


  f.create = create_work;
  f.compile = process_results;
  f.compute = do_work;
  f.work_sz = sizeof (work_unit);
  f.res_sz = sizeof (result_unit);

  MPI_Init (&argc, &argv);
  MW_Run (argc, argv, &f);
  MPI_Finalize ();
  
  testing();
  return 0;

}
