#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <gmp.h>
#include "mw_api.h"
#include <string.h>

#define PARTIAL 0
#define COMPLETE 1

struct work_t {
	unsigned long first; 
	unsigned long end;
	unsigned long temp;
	//unsigned long length; // length of the array
	unsigned long num; // number for which the factors need to be calculated
};

struct result_t {
   	unsigned long *factors; // pointer to array of factors
   	int state;
	int length;
};

unsigned char* serialize_result(result_unit *res,int *size){
	unsigned char *serialized_result;
	unsigned char *start;
	unsigned long i,length;
	// calculate the total size of work unit and allocate that much memory
	length = 2*(sizeof(int)); // for length and state 
	length += (res->length) * sizeof(unsigned long); // for the array containing the factors
	*size = (int)length;
	// allocate memory for char array
	serialized_result  = (unsigned char *)malloc(sizeof(unsigned char)*length);
	// copy memory piece by piece to the byte stream(char array)
	start = serialized_result;
	// copy length first and keep moving the pointer
	memcpy(serialized_result,&(res->length),sizeof(int));
	serialized_result += sizeof(int);
	
	// copy state and move the pointer
	memcpy(serialized_result,&(res->state),sizeof(int));
	serialized_result += sizeof(int);

	// copy the factors array
	for(i=0;i<res->length;i++){
		memcpy(serialized_result,&(res->factors[i]),sizeof(unsigned long));
		serialized_result += sizeof(unsigned long);
	}
	// return the byte stream
	return start;

}

result_unit* deserialize_result(unsigned char *serialized_result,int size){
	int i;
	// create new result unit
	result_unit *res = (result_unit *)malloc(sizeof(result_unit));
	// copy value of length
	memcpy(&(res->length),serialized_result,sizeof(int));
	serialized_result += sizeof(int);

	// copy value of state
	memcpy(&(res->state),serialized_result,sizeof(int));

	serialized_result += sizeof(int);

	// allocate memory for factors
	res->factors = (unsigned long *)malloc( sizeof(unsigned long) * (res->length));
	// copy the factors array
	for(i=0;i<res->length;i++){
		memcpy(&(res->factors[i]),serialized_result,sizeof(unsigned long));
		serialized_result += sizeof(unsigned long);
	}
	// return the result object
	return res;
}



unsigned char* serialize_work(work_unit *work,int *size){
	unsigned char *serialized_work;
	unsigned char *start;
	// calculate the total size of work unit and allocate that much memory
	unsigned long i,length;
	length = 4*(sizeof(unsigned long)); // for length and num
	*size = (int)length;
	// allocate memory for char array
	serialized_work  = (unsigned char *)malloc(sizeof(unsigned char)*length);
	// copy memory piece by piece to the byte stream(char array)
	start = serialized_work;
	// copy length and num first and keep moving the pointer
	memcpy(serialized_work,&(work->first),sizeof(unsigned long));
	
	// allocate memory for char array
	serialized_work += sizeof(unsigned long);
	
	memcpy(serialized_work,&(work->end),sizeof(unsigned long));
	serialized_work += sizeof(unsigned long);
	
	memcpy(serialized_work,&(work->temp),sizeof(unsigned long));
	serialized_work += sizeof(unsigned long);

	memcpy(serialized_work,&(work->num),sizeof(unsigned long));
	// copy the number array
	/*for(i=0;i<work->length;i++){
		memcpy(serialized_work,&(work->numbers[i]),sizeof(unsigned long));
		serialized_work += sizeof(unsigned long);
	}*/
	//printf("done serializing work\n");
	// return the byte stream
	
	return start;
}

work_unit* deserialize_work(unsigned char *serialized_work,int size){
	int i;
	// create new work unit
	work_unit *work = (work_unit *)malloc(sizeof(work_unit));
	// copy value of length
	memcpy(&(work->first),serialized_work,sizeof(unsigned long));
	//printf(" deserialize work first %lu\n",(unsigned long)(*serialized_work));
	serialized_work += sizeof(unsigned long);
	
	memcpy(&(work->end),serialized_work,sizeof(unsigned long));
	serialized_work += sizeof(unsigned long);
	
	memcpy(&(work->temp),serialized_work,sizeof(unsigned long));
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
	// number of chunks -1
	int size = 10;
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
 	num  = mpz_get_ui(big_num);
	mpz_sqrt(big_sqrt,big_num);
	sqrt =  mpz_get_ui(big_sqrt);
	//gmp_printf("%Zd  %Zd\n",big_num,big_sqrt);
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
		temp->end = count-1;
		temp->num = num;
		temp->temp = temp->first;
		//temp->length = unit_size;
		t[i]=temp;
	}
	t[size-1]=NULL;
	//printf("created_work\n");
	return t;
}

int process_results(int sz, result_unit **res){
	unsigned long result=0;
	int i=0,j=0;
	for(i=0;i<sz;i++){
		
		//printf(" length in compile %d\n", res[i]->length );
		for(j=0;j<(res[i]->length);j++){
			result = res[i]->factors[j];
			printf("%lu\t",result);
		}
	}
	printf("\n");

return 1;
}

result_unit* do_work(work_unit *work){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	res->factors = (unsigned long *)malloc(sizeof(unsigned long) * ((work->end)-(work->first)));
	unsigned long i;
	int j=0;
	double start_time = MPI_Wtime();
	res->length = 0;
	double timeout = .00050;
	// printf("doing work with first %lu\n", work->first);
	for(i=work->first;i<=(work->end);i++){
		if((MPI_Wtime() - start_time)>timeout){
			realloc(res->factors,(res->length)*sizeof(unsigned long));
			res->state = PARTIAL;
			(work->first) = i;
			//printf("doing work \n");
			return res;	
		}
		if((work->num)%(i) == 0){
			//printf("Factor %lu\n",i);
			res->factors[j++] = i;
			//res->factors[j++] = (work->num)/i;
			res->length+=1;
		}
	}
	realloc(res->factors,(res->length)*sizeof(unsigned long));
	(work->first) = i;
	res->state = COMPLETE;
	//printf("doing work \n");
	return res;
}

int get_result_state(result_unit *res){
	return res->state;
}

unsigned long get_work_first(work_unit *work){
	return work->first;
}

void reinit_work(work_unit *work){
	work->first = work->temp;
	return;
}

result_unit *get_result_object(){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	res->length = 0;
	res->state = PARTIAL;
	return res;
}

result_unit *combine_partial_results(result_unit *r1,result_unit *r2){
	result_unit* res = (result_unit *)malloc(sizeof(result_unit));
	int i,j;
	res->length = r1->length + r2->length;
	res->factors = (unsigned long *)malloc(sizeof(unsigned long) * (res->length) );
	for(i=0,j=0;j<r1->length;i++,j++){
		res->factors[i] = r1->factors[j];
	}
	
	for(j=0;j<r2->length;i++,j++){
		res->factors[i] = r2->factors[j];
	}
	res->state = (r1->state) + (r2->state);
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
  f.get_result_state = get_result_state;
  f.work_first = get_work_first;
  f.get_result_object = get_result_object;
  f.combine_partial_results = combine_partial_results;
  f.reinit = reinit_work;
  f.work_ptr_sz = sizeof(work_unit*);
  //printf("Here\n");
  MPI_Init (&argc, &argv);
  //testing();
  MW_Run (argc, argv, &f);
  MPI_Finalize ();
  
  return 0;

}
