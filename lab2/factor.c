#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <gmp.h>
#include "mw_api.h"

struct work_t {
	mpz_t *numbers; // pointer to array of numbers to check
	int length; // length of the array
	mpz_t num; // number for which the factors need to be calculated
};

struct result_t {
   	mpz_t *factors; // pointer to array of factors
	int length;
};

work_unit** create_work(int argc, char **argv){
	int size = 10;
	mpz_t num, sqrt, unit_size;
	// Get the number to be factored
	mpz_init_set_str(num,argv[1],10);
	// Calculate the square root
	mpz_sqrt(sqrt,num);
	gmp_printf("%Zd  %Zd\n",num,sqrt);
	// from 1 to square root divide it into chunks
	// taking ceiling for quotient with division
	mpz_cdiv_q_ui(unit_size, sqrt, size);
	
	work_unit** t=(work_unit**)malloc(size*sizeof(work_unit*)); 
	int j,i=0;	
	for(i=0;i<size;i++){
		work_unit* temp = (work_unit*)malloc(sizeof(work_unit));
		temp->numbers = (mpz_t *)malloc(unit_size*sizeof(mpz_t));
		for(j=1;j<=unit_size;j++){
			temp->numbers[i])
		}
		t[i]=temp;
	}
	t[size-1]=NULL;
	printf("created_work\n");
	return t;
}

int process_results(int sz, result_unit **res){
double result=0;
int i=0;
/*for(i=0;i<sz;i++)
{result += res[i]->t;

printf("result %f %f %d\n",res[i]->t,result,sz);
}
printf("%f\n",result);
*/
return result;
}

result_unit* do_work(work_unit *work){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	//printf("doing work %f\n",res->t);
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
