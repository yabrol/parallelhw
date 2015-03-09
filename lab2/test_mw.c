#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <gmp.h>
#include "mw_api.h"
#include <string.h>

struct work_t {
	unsigned long *numbers; // pointer to array of numbers to check
	int length; // number for which the factors need to be calculated
};

struct result_t {
   	unsigned long sum; // pointer to array of factors
};

unsigned char* serialize_work(work_unit *res,int *size){
	printf("serializing\n");
	unsigned char *serialized_work;
	unsigned char *start;
	unsigned long i,length;
	// calculate the total size of work unit and allocate that much memory
	length = (sizeof(int)); // for length 
	length += (res->length) * sizeof(unsigned long); // for the array containing the numbers
	*size = (int)length;
	// allocate memory for char array
	serialized_work  = (unsigned char *)malloc(sizeof(unsigned char)*length);
	// copy memory piece by piece to the byte stream(char array)
	start = serialized_work;
	// copy length first and keep moving the pointer
	memcpy(serialized_work,&(res->length),sizeof(int));
	serialized_work += sizeof(int);
	
	// copy the factors array
	for(i=0;i<res->length;i++){
		memcpy(serialized_work,&(res->numbers[i]),sizeof(unsigned long));
		serialized_work += sizeof(unsigned long);
	}
	// return the byte stream
	return start;

}

work_unit* deserialize_work(unsigned char *serialized_work,int size){
	printf("deserializing\n");
	int i;
	// create new result unit
	work_unit *res = (work_unit *)malloc(sizeof(work_unit));
	// copy value of length
	memcpy(&(res->length),serialized_work,sizeof(int));
	serialized_work += sizeof(int);
	// allocate memory for factors
	res->numbers = (unsigned long *)malloc( sizeof(unsigned long) * (res->length));
	// copy the factors array
	for(i=0;i<res->length;i++){
		memcpy(&(res->numbers[i]),serialized_work,sizeof(unsigned long));
		serialized_work += sizeof(unsigned long);
	}
	// return the result object
	return res;
}



unsigned char* serialize_result(result_unit *res,int *size){
	unsigned char *serialized_result;
	unsigned char *start;
	// calculate the total size of work unit and allocate that much memory
	unsigned long i,length;
	length = (sizeof(unsigned long)); // for sum
	*size = (int)length;
	// allocate memory for char array
	serialized_result  = (unsigned char *)malloc(sizeof(unsigned char)*length);
	// copy memory piece by piece to the byte stream(char array)
	start = serialized_result;
	// copy length and num first and keep moving the pointer
	memcpy(serialized_result,&(res->sum),sizeof(unsigned long));
	
	return start;
}

result_unit* deserialize_result(unsigned char *serialized_result,int size){
	int i;
	// create new work unit
	result_unit *result = (result_unit *)malloc(sizeof(result_unit));
	// copy value of length
	memcpy(&(result->sum),serialized_result,sizeof(unsigned long));
	// return the byte stream
	return result;
}

work_unit** create_work(int argc, char **argv){
	printf("create work\n");
	int size = 9;
	int unit_size = 10;
	work_unit** t=(work_unit**)malloc(size*sizeof(work_unit*)); 
	int j,i=0;	
	unsigned long int count =1;
	for(i=0;i<size-1;i++){
		work_unit* temp = (work_unit *)malloc(sizeof(work_unit));
		temp->numbers = (unsigned long *)malloc(unit_size*sizeof(unsigned long));
		temp->length = unit_size;
		for(j=0;j<unit_size;j++){
			temp->numbers[j] = count;
			count++;
		}
		t[i]=temp;
	}
	t[size-1]=NULL;
	printf("created_work\n");
	return t;
}

int process_results(int sz, result_unit **res){
	unsigned long result=0;
	int i=0,j=0;
	for(i=0;i<sz;i++){
		
			result += res[i]->sum;
	}

return 1;
}

result_unit* do_work(work_unit *work){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	unsigned long i;
	int sum;
	int j=0;
	for(i=0;i<=(work->length);i++){
		res->sum += work->numbers[i];
	}
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
  f.serialize_result = serialize_result;
  f.deserialize_result = deserialize_result;
  f.work_sz = sizeof (work_unit);
  f.res_sz = sizeof (result_unit);

  //printf("Here\n");
  MPI_Init (&argc, &argv);
  //testing();
  MW_Run (argc, argv, &f);
  MPI_Finalize ();
  
  return 0;

}
