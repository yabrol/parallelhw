#include "mw_api.h"
#include <stdio.h>
#include <stdlib.h>
void testing(){
	printf("testing\n");
}

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	int i;
	printf("running\n");
	work_unit **work;// = (work_unit *)malloc(f->work_sz);
	work = f->create(argc,argv);
	for(i=0;i<10;i++){
		work_unit *chunk = (work_unit *)malloc(f->work_sz);
		chunk = work[i];
		result_unit *r = (result_unit *)malloc(f->res_sz);
		r = f->compute(chunk);
		int p = f->compile(9,r);

	}
//free(work);
}
