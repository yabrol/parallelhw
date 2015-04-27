#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <netpbm/pam.h>

typedef struct pixel{
	int r;
	int g;
	int b;
}pix;

void print_matrix(int a[][4],int n){
	int i,j;
	for(i=0;i<n;i++){
		printf("\n");
		for(j=0;j<n;j++)
			printf("%d ", a[i][j]);
	}
}

void print_submatrix(int a[][2],int n){
	int i,j;
	for(i=0;i<n;i++){
		printf("\n");
		for(j=0;j<n;j++)
			printf("%d ", a[i][j]);
	}
}

void get_submatrix(int a[][4],int i,int j,int size){
	int x,y;
	int r[size][size];
	for(x=i;x<i+size;x++)
		for(y=j;y<j+size;y++){
			//printf("%d %d %d\n",x-i,y-j,a[x][y] );
			r[x-i][y-j] = a[x][y];
		}

	print_submatrix(r,size);
}

pix **read_image(struct pam *pam_image, struct pam *pam_stencil){
	tuple *tuplerow;
	// if stencil is 2*k -1, k-1 padding needed on all sides
	int height = pam_image->height + pam_stencil->height - 1;
	int width = pam_image->width + pam_stencil->width - 1;
	int k = (pam_stencil->width + 1)/2;
	int nw = pam_image->width;
	int nh = pam_image->height;
	int i,j;

	// allocate tuplerow
	tuplerow = pnm_allocpamrow(pam_image);

	// allocate space for image -> 2d array of pix
	pix **image = (pix**) malloc(sizeof(pix*)* height);
	for (i=0; i< height; i++)
        image[i] = (pix *)malloc(width * sizeof(pix));

    // read in the image and load it into the array
	for (i=0; i< height; i++){
    	if( i< (k-1) || i>((k-1)+ nh -1 ) ){
	    	for(j=0;j< width;j++){
	    		image[i][j].r = 0;
	    		image[i][j].g = 0;
	    		image[i][j].b = 0;
	    	}
	    }
	    else{
	    	// read row into tuple
	    	pnm_readpamrow(pam_image, tuplerow);
	    	for(j=0;j< width;j++){
	    		if( j< (k-1) || j>((k-1)+ nw -1 ) ){
		    		image[i][j].r = 0;
		    		image[i][j].g = 0;
		    		image[i][j].b = 0;
		    	}
		    	else{
		    		image[i][j].r = tuplerow[j][0];
		    		image[i][j].g = tuplerow[j][1];
		    		image[i][j].b = tuplerow[j][2];	
		    	}
	    	}
	    }
    }

    // free tuple
    pnm_freepamrow(tuplerow);
    
    return image;
}

int **read_stencil(struct pam *pam_stencil){
	tuple *tuplerow;
	int height = pam_stencil->height;
	int width = pam_stencil->width;
	int i,j;
	// allocate tuplerow
	tuplerow = pnm_allocpamrow(pam_stencil);

	// allocate space for image -> 2d array of pix
	int **stencil = (int**) malloc(sizeof(int*)* height);
	for (i=0; i< height; i++)
        stencil[i] = (int *)malloc(width * sizeof(int));

    // read in the image and load it into the array
	for (i=0; i< height; i++){
    	// read row into tuple
    	pnm_readpamrow(pam_stencil, tuplerow);
    	for(j=0;j< width;j++){
    		stencil[i][j] = tuplerow[j][0];
    	}
    }

    // free tuple
    pnm_freepamrow(tuplerow);
    
    return stencil;
}

int main (int argc, char **argv)
{
	struct pam inpam, outpam, instencil;
	tuple * tuplerow;
	unsigned int row;
	unsigned int grand_total;
	pix **image;
	int **stencil;
	int processors;

	FILE *fp = fopen("lenn.ppm", "r");
	FILE *st = fopen("stencil.pgm", "r");
	FILE *op = fopen("lennaout.ppm", "w");

	pm_init(argv[0], 0);

	pnm_readpaminit(fp, &inpam, PAM_STRUCT_SIZE(tuple_type));
	pnm_readpaminit(st, &instencil, PAM_STRUCT_SIZE(tuple_type));
	
	processors = 4;
	// processors = atio(argv[1]);

	// read in the stencil
	stencil = read_stencil(&instencil);

	// read image into a 2d array with padding according to the stencil length
	image = read_image(&inpam, &instencil);

	// divide it into chunks


  	int j,i = 0;
    //omp_set_num_threads(4);
    int count = 0;
  	int a[4][4];
  	for(i=0;i<4;i++)
  		for(j=0;j<4;j++)
  			a[i][j]=count++;

  	for(i=0;i<4;i++)
  		for(j=0;j<4;j++)
  			printf("%d ",a[i][j]);
  	int p=4;
  	int root_p = 2;
  	printf("initialized\n");
  	print_matrix(a,p);

	
	#pragma omp parallel for collapse(2) private(i,j) schedule(static,1)
  	for(i=0;i<root_p;i++)
	{
	  	for(j=0;j<root_p;j++)
	  	{
	  		if(omp_get_thread_num() == 2){
	  			printf ("hello there, I am thread %d and i is %d and j is %d\n", omp_get_thread_num(),i,j);
	  			get_submatrix(a,i*root_p,j*root_p,root_p);
			}
		}	

	}
	
 	return 0;

}
