#include "mw_api.h"
#include "queue.h"
#include "fileio.h"
#include "resultQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void write_workList(unsigned char *serial_work){
	FILE *workList = fopen("workList.txt", "ab+");
	if (workList == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	int charLength = strlen(serial_work);
	fprintf(workList, "%s\t%d\n", serial_work, charLength);

	// //the file is structured in tab deliminated
	// //workID   first   end   num
	// fprintf(workList, "%d\t", workNode->id);//work ID
	// //work unit is pointer to array of numbers, length, and number
	// fprintf(workList, "%lu\t", workNode->work->first);//first
	// fprintf(workList, "%lu\t", workNode->work->end);//end
	// fprintf(workList, "%lu\n", workNode->work->num);//num
	fclose(workList);
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

// void write_remainingWork(){
// 	FILE *workList = fopen("workList.txt", "ab+");
// 	if (workList == NULL)
// 	{
// 		printf("Error opening file!\n");
// 		exit(1);
// 	}
// 	rewind(workList);

// 	FILE *workID = fopen("completedWorkID.txt", "ab+");
// 	if (workID == NULL)
// 	{
// 		printf("Error opening file!\n");
// 		exit(1);
// 	}
// 	rewind(workList);
// 	int ch, number_of_lines = 0;
// 	do 
// 	{
// 		ch = fgetc(myfile);
// 		if(ch == '\n')
// 			number_of_lines++;
// 	} while (ch != EOF);
// 	// last line doesn't end with a new line!
// 	// but there has to be a line at least before the last line
// 	if(ch != '\n' && number_of_lines != 0) 
// 		number_of_lines++;
// 	rewind(workID);
// 	int completedIDs[number_of_lines];
// 	int id;
// 	int i = 0;
// 	unsigned long firstNum, endNum, numNum;
// 	//put compledted ids in an array
// 	while(fscanf(workID, "%d", &id) != EOF){
// 		completedIDs[i] = id;
// 	}

// 	FILE *remainingWork = fopen("remainingWork.txt", "w");
// 	if (workID == NULL)
// 	{
// 		printf("Error opening file!\n");
// 		exit(1);
// 	}
// 	//now 
// 	while(fscanf(workID, "%d", &id) != EOF){
// 		completedIDs[i] = id;
// 	}

// }


