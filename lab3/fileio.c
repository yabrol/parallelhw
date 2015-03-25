#include "mw_api.h"
#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


void write_result_list(unsigned char *serial_result, int charLength){
	FILE *workList = fopen("result.txt", "ab+");
	if (workList == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(workList, "%d\n", charLength);
	fwrite(serial_result, 1, charLength, workList);
	fwrite("\n",1,1,workList);
	printf("write result length %d\n",(int)(*serial_result));

	fclose(workList);
}


void write_workList(unsigned char *serial_work, int charLength){
	FILE *workList = fopen("workList.txt", "ab+");
	if (workList == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(workList, "%d\n", charLength);
	fwrite(serial_work, 1, charLength, workList);
	fwrite("\n",1,1,workList);
	printf("write ser length %d\n",(int)(*serial_work));

	fclose(workList);
}


void write_workList_length(int length){
	FILE *workList = fopen("workList.txt", "ab+");
	if (workList == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(workList, "%d\n", length);
	
	printf("write list length %d\n",length);

	//for( int i = 0; i < charLength; i++){
	//fprintf(workList, "%c", serial_work[i]);}
	//fprintf(workList, "\n\n");

	// //the file is structured in tab deliminated
	// //workID   first   end   num
	fclose(workList);
}

work_unit** read_work_list( struct mw_api_spec *f){
	work_unit **w;
	// TODO: change it to read mode
	FILE *workList = fopen("workList.txt", "ab+");
	if (workList == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	rewind(workList);
	int charLength, length;
	int i=0;
	size_t len;
	// read length of the work array
	fscanf(workList, "%d\n", &length);
	// initialize memory for the work array according to the length
	w = (work_unit **)malloc((length+1)*sizeof(work_unit *));
	// read length number of elements
	while(i<length){
		fscanf(workList, "%d\n", &charLength);
		unsigned char *serial_work = (unsigned char *) malloc(charLength);
		fread(serial_work,1,charLength,workList);
		w[i] = (work_unit *)malloc(f->work_sz);
		w[i] = f->deserialize(serial_work,charLength);
		//printf("char length %d\n", charLength);
		//printf("read ser length %d\n", (int)(*serial_work));
		i++;
	}
	w[length]=NULL;
	fclose(workList);
	return w;
}

void write_completed_workID(int id){
	FILE *workID = fopen("completedWorkID.txt", "ab+");
	if (workID == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	//this file only contains work IDs that have been completed
	fprintf(workID, "%d\n",id);
	fclose(workID);
}
