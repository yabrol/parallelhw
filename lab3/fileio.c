#include "mw_api.h"
#include "queue.h"
#include "fileio.h"
#include "resultQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int counter = 0;

void write_workList(work_node_pt work){
	FILE *workList = fopen("workList.txt", "ab+");
	if (workList == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(workList, "%d) Work ID: %d\n",++counter, work->id);
	fclose(workList);
}

void write_workID(int id){
	FILE *workID = fopen("workID.txt", "ab+");
	if (workID == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(workID, "%d) Work ID: %d\n",++counter, id);
	fclose(workID);
}

void write_numbers(){
	FILE *fp = fopen("numbers.txt", "ab+");
	if( fp == NULL){
		printf("Error opening file!\n");
		exit(1);
	}
	srand(time(NULL));
	int num;
	for(int i = 0; i < 50; i++){
		num = rand()%50;
		fprintf(fp, "%d\n", num);
	}
	
}
