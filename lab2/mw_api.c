#include "mw_api.h"
#include <stdio.h>
#include <stdlib.h>
void testing(){
	printf("testing\n");
}

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	int i;
	printf("running\n");
	mw_work_t *work = (mw_work_t *)malloc(f->work_sz);
	work = f->create(argc,argv);
	for(i=0;i<11;i++){
		//mw_work_t *data = (mw_work_t *)malloc(sizeof(f->work_sz));
		//data = (mw_work_t *)(work[i]);
		//mw_work_t* temp = (mw_work_t *)(work[i]);
		//int d;
		//mw_work_t blah = malloc(sizeof(mw_work_t));
		//blah = temp[0];
		//work[i]->t = 90;
                printf("%d \n",work->t);
	}
free(work);
}
