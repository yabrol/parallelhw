#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <gmp.h>
#include "mw_api.h"
#include <string.h>

struct work_t {
	unsigned long first; // pointer to array of numbers to check
	unsigned long end;
	//unsigned long length; // length of the array
	unsigned long num; // number for which the factors need to be calculated
};

struct result_t {
   	unsigned long *factors; // pointer to array of factors
	int length;
};

unsigned char* serialize_work(work_unit *work,int *size){
	printf("serializing work\n");
	unsigned char *serialized_work;
	unsigned char *start;
	//unsigned char **temp = (unsigned char **)malloc(sizeof(unsigned char *));
	// calculate the total size of work unit and allocate that much memory
	unsigned long i,length;
	length = 3*(sizeof(unsigned long)); // for length and num
	//length += (work->length) * sizeof(unsigned long); // for the array containing the numbers
	*size = (int)length;
	// allocate memory for char array
	

	serialized_work  = (unsigned char *)malloc(sizeof(unsigned char)*length);
	// copy memory piece by piece to the byte stream(char array)
	start = serialized_work;
	// copy length and num first and keep moving the pointer
	memcpy(serialized_work,&(work->first),sizeof(unsigned long));
	
	printf("work first %lu\n",(unsigned long)(*serialized_work));
	// allocate memory for char array
	serialized_work += sizeof(unsigned long);
	
	memcpy(serialized_work,&(work->end),sizeof(unsigned long));
	serialized_work += sizeof(unsigned long);
	memcpy(serialized_work,&(work->num),sizeof(unsigned long));
	//serialized_work += sizeof(unsigned long);
	// copy the number array
	/*for(i=0;i<work->length;i++){
		memcpy(serialized_work,&(work->numbers[i]),sizeof(unsigned long));
		serialized_work += sizeof(unsigned long);
	}*/
	printf("done serializing\n");
	// return the byte stream
	
	return start;
}

work_unit* deserialize_work(unsigned char *serialized_work,int size){
	int i;
	// create new work unit
	work_unit *work = (work_unit *)malloc(sizeof(work_unit));
	// copy value of length
	memcpy(&(work->first),serialized_work,sizeof(unsigned long));
	printf(" deserialize work first %lu\n",(unsigned long)(*serialized_work));
	serialized_work += sizeof(unsigned long);
	
	memcpy(&(work->end),serialized_work,sizeof(unsigned long));
	serialized_work += sizeof(unsigned long);
	
	memcpy(&(work->num),serialized_work,sizeof(unsigned long));
	//serialized_work += sizeof(unsigned long);
	// copy the number array
	/*
	for(i=0;i<work->length;i++){
		memcpy(&(work->numbers[i]),serialized_work,sizeof(unsigned long));
		serialized_work += sizeof(unsigned long);
	}*/
	// return the byte stream
	return work;
}

work_unit** create_work(int argc, char **argv){
	printf("Here\n");	
	int size = 5;
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
		//temp->numbers = (unsigned long *)malloc(unit_size*sizeof(unsigned long));
		temp->first = count;
		for(j=0;j<unit_size;j++){
			//temp->numbers[j] = count;
			count++;
		}
		temp->end = count;
		temp->num = num;
		//temp->length = unit_size;
		t[i]=temp;
	}
	t[size-1]=NULL;
	printf("created_work\n");
	return t;
}

int process_results(int sz, result_unit **res){
	unsigned long result=0;
int i=0,j=0;
/*for(i=0;i<sz;i++)
{
	for(j=0;j<8;j++)
	{
		result += res[i]->factors[j];
	}
}*/
printf("%lu\n",result);

return result;
}

result_unit* do_work(work_unit *work){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	res->factors = (unsigned long *)malloc(sizeof(unsigned long) * ((work->end)-(work->first)));
	unsigned long i;
	int j=0;
	res->length = 0;
	for(i=work->first;i<=(work->end);i++)
	{
		if((work->num)%(i) == 0){
			//printf("Factor %lu\n",i);
			res->factors[j++] = i;
			res->factors[j++] = (work->num)/i;
			res->length+=2;
		}
	}
	realloc(res->factors,(res->length)*sizeof(unsigned long));
	for(j=0;j<res->length;j++){
		printf(" factor %lu\n",res->factors[j]);
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
  f.serialize = serialize_work;
  f.deserialize = deserialize_work;
  f.work_sz = sizeof (work_unit);
  f.res_sz = sizeof (result_unit);

  //printf("Here\n");
  MPI_Init (&argc, &argv);
  testing();
  MW_Run (argc, argv, &f);
  MPI_Finalize ();
  
  return 0;

}
