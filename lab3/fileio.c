#include "mw_api.h"
#include "sample_q2.h"
#include "queue.h"
#include "fileio.h"
#include "resultQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int counter = 0;

void write_workList(work_node_pt workNode){
	FILE *workList = fopen("workList.txt", "ab+");
	if (workList == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(workList, "Work ID: %d\n", workNode->id);
	//work unit is pointer to array of numbers, length, and number
	fprintf(workList, "Work Unit: {\n");
	fprintf(workList, "First: %lu\n", workNode->work->first);
	fprintf(workList, "End: %lu\n", workNode->work->end);
	fprintf(workList, "Num: %lu\n", workNode->work->num);
	fprintf(workList, "}\n\n");
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
