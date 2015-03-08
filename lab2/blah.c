#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <gmp.h>
#include "mw_api.h"

struct work_t {
	unsigned long int * numbers; // pointer to array of numbers to check
	unsigned long length; // length of the array
	unsigned long int num; // number for which the factors need to be calculated
};

struct result_t {
   	unsigned long int *factors; // pointer to array of factors
	int length;
};

work_unit** create_work(int argc, char **argv){
	printf("Here\n");	
	int size = 11;
	mpz_t big_num, big_sqrt;
	mpz_init(big_num);
	mpz_init(big_sqrt);
	unsigned long int num,sqrt;
	unsigned long int unit_size;
	//char *ptr;
	// Get the number to be factored
	//num = strtoul(argv[1],&ptr,10);
	mpz_init_set_str(big_num,argv[1],10);
	
	// Calculate the square root
	mpz_sqrt(big_sqrt,big_num);
 	num  = mpz_get_ui(big_num);
	sqrt =  mpz_get_ui(big_sqrt);
	gmp_printf("%Zd  %Zd\n",big_num,big_sqrt);
	// from 1 to square root divide it into chunks
	// taking ceiling for quotient with division
	unit_size = (sqrt/(size-1));
	//unit_size = 8;
	work_unit** t=(work_unit**)malloc(size*sizeof(work_unit*)); 
	int j,i=0;	
	unsigned long int count =1;
	for(i=0;i<size-1;i++){
		work_unit* temp = (work_unit *)malloc(sizeof(work_unit));
		temp->numbers = (unsigned long *)malloc(unit_size*sizeof(unsigned long));
		for(j=0;j<unit_size;j++){
			temp->numbers[j] = count;
			count++;
		}
		temp->num = num;
		temp->length = unit_size;
		t[i]=temp;
	}
	t[size-1]=NULL;
	printf("created_work\n");
	return t;
}

int process_results(int sz, result_unit **res){
	unsigned long result=0;
int i=0,j=0;
for(i=0;i<sz;i++)
{
	for(j=0;j<8;j++)
	{
		result += res[i]->factors[j];
	}
}
printf("%lu\n",result);

return result;
}

result_unit* do_work(work_unit *work){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	res->factors = (unsigned long *)malloc(work->length*sizeof(unsigned long));
	int i;
	for(i=0;i<(work->length);i++)
	{
		res->factors[i] = work->numbers[i];
		printf("%lu\n",res->factors[i]);
	}
	printf("doing work \n");
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

  //printf("Here\n");
  MPI_Init (&argc, &argv);
  testing();
  MW_Run (argc, argv, &f);
  MPI_Finalize ();
  
  return 0;

}
