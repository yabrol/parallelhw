#include "mw_api.h"
#include "stdio.h"
#include "stdlib.h"
struct userdef_work_t {
	int t; 
};

struct userdef_result_t {
   	int t;
};


void testing(){
	printf("testing\n");
}

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	int i;
	printf("running\n");
	struct userdef_work_t **work;
	work = (struct userdef_work_t **)f->create(argc,argv);
	for(i=0;i<11;i++){
		//mw_work_t *data = (mw_work_t *)malloc(sizeof(f->work_sz));
		//data = (mw_work_t *)(work[i]);
		struct userdef_work_t* temp = (struct userdef_work_t *)work[i];
		printf("%d \n",(int)temp->t);
	}
}
