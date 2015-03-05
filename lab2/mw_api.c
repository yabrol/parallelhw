#include "mw_api.h"
#include "stdio.h"

void testing(){
	printf("testing\n");
}

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	printf("running\n");
	f->create(argc,argv);
}
