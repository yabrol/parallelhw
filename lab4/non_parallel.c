#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <netpbm/pam.h>

typedef struct pixel{
	int r;
	int g;
	int b;
}pix;

typedef struct image_struct
{
	/* data */
	pix **pixels;
	int width;
	int height;
}image;

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

image read_image(struct pam *pam_image, struct pam *pam_stencil){
	tuple *tuplerow;
	// if stencil is 2*k -1, k-1 padding needed on all sides
	int height = pam_image->height + pam_stencil->height - 1;
	int width = pam_image->width + pam_stencil->width - 1;
	int k = (pam_stencil->width + 1)/2;
	int nw = pam_image->width;
	int nh = pam_image->height;
	int i,j;
	image input_image;

	input_image.width = width;
	input_image.height = height;

	// allocate tuplerow
	tuplerow = pnm_allocpamrow(pam_image);

	printf("padded image %d x %d\n", width, height);
	printf("image:%d x %d , stencil %d x %d \n", nw,nh, pam_stencil->width,pam_stencil->height);

	// allocate space for image -> 2d array of pix
	input_image.pixels = (pix**) malloc(sizeof(pix*)* height);
	for (i=0; i< height; i++)
        input_image.pixels[i] = (pix *)malloc(width * sizeof(pix));
    // printf("space allocated for image\n");

    // read in the image and load it into the array
	for (i=0; i< height; i++){
		// assign zero values if one of the padded rows
    	if( i< (k-1) || i>((k-1)+ nh -1 ) ){
	    	for(j=0;j< width;j++){
	    		input_image.pixels[i][j].r = 0;
	    		input_image.pixels[i][j].g = 0;
	    		input_image.pixels[i][j].b = 0;
	    	}
	    }
	    else{
	    	// read row into tuple
	    	pnm_readpamrow(pam_image, tuplerow);
	    	for(j=0;j< width;j++){
	    		// asssign zero value if one of the padded columns
	    		if( j< (k-1) || j>((k-1)+ nw -1 ) ){
		    		input_image.pixels[i][j].r = 0;
		    		input_image.pixels[i][j].g = 0;
		    		input_image.pixels[i][j].b = 0;
		    	}
		    	else{
		    		input_image.pixels[i][j].r = tuplerow[j-(k-1)][0];
		    		input_image.pixels[i][j].g = tuplerow[j-(k-1)][1];
		    		input_image.pixels[i][j].b = tuplerow[j-(k-1)][2];	
		    	}
	    	}
	    }
    }

    // free tuple
    pnm_freepamrow(tuplerow);
    return input_image;
}

image read_stencil(struct pam *pam_stencil){
	tuple *tuplerow;
	int height = pam_stencil->height;
	int width = pam_stencil->width;
	int maxval = pam_stencil->maxval;
	int i,j;
	image stencil;

	stencil.height = height;
	stencil.width = width;

	// allocate tuplerow
	tuplerow = pnm_allocpamrow(pam_stencil);


	// allocate space for image -> 2d array of pix
	stencil.pixels = (pix**) malloc(sizeof(int*)* height);
	for (i=0; i< height; i++)
        stencil.pixels[i] = (pix *)malloc(width * sizeof(pix));

    // read in the image and load it into the array
	for (i=0; i< height; i++){
    	// read row into tuple
    	pnm_readpamrow(pam_stencil, tuplerow);
    	for(j=0;j< width;j++){
    		stencil.pixels[i][j].r = -4 + (8 * tuplerow[j][0])/(maxval - 1);
    		stencil.pixels[i][j].g = 0;
    		stencil.pixels[i][j].b = 0;
    	}
    }

    // free tuple
    pnm_freepamrow(tuplerow);
    
    return stencil;
}

pix convolve_pixel(image mat, image stencil, int i,int j, int size){
	int x,y;
	pix value;
	value.r = 0;
	value.g = 0;
	value.b = 0;
	for(x=i;x<i+size;x++)
		for(y=j;y<j+size;y++){
			//printf("%d %d %d\n",x-i,y-j,a[x][y] );
			value.r += (mat.pixels[i][j].r * stencil.pixels[x-i][y-j].r);
			value.g += (mat.pixels[i][j].g * stencil.pixels[x-i][y-j].r);
			value.b += (mat.pixels[i][j].b * stencil.pixels[x-i][y-j].r);
		}	
	return value;
}

image convolve(image input_image, image stencil ){
	int k = (stencil.width + 1)/2;
	int i,j;
	int width = input_image.width;
	int height = input_image.height;
	image result;
	result.height = input_image.height - 2*(k-1);
	result.width = input_image.width - 2*(k-1);

		// allocate space for image -> 2d array of pix
	result.pixels = (pix**) malloc(sizeof(pix*)* result.height);
	for (i=0; i< height; i++)
        result.pixels[i] = (pix *)malloc(result.width * sizeof(pix));

	printf("image to convolve %d x %d\n",input_image.width,input_image.height);
	printf("stencil to convolve %d x %d\n",stencil.width,stencil.height);

	for (i=0; i< height; i++){
		if( !( i< (k-1) || i>( height - (k-1) - 1 ) ) ){
	    	for(j=0;j< width;j++){
	    		// asssign zero value if one of the padded columns
	    		if( !( j< (k-1) || j>( width - (k-1) - 1 ) ) ){
	    			// do convolution
	    			result.pixels[i-(k-1)][j-(k-1)] = convolve_pixel(input_image, stencil, i-(k-1), j-(k-1), (2*k-1));
		    	}
	    	}
	    }
    }

    // save convolved image
    return result;

}

int main (int argc, char **argv)
{
	struct pam inpam, outpam, instencil;
	unsigned int row,column,plane;
	unsigned int grand_total;
	image input_image,stencil,convolved_image;
	int processors;
	tuple * tuplerow;
	int j,i = 0;
    

	FILE *fp = fopen("lenn.ppm", "r");
	FILE *st = fopen("gauss.pgm", "r");
	

	pm_init(argv[0], 0);

	pnm_readpaminit(fp, &inpam, PAM_STRUCT_SIZE(tuple_type));
	pnm_readpaminit(st, &instencil, PAM_STRUCT_SIZE(tuple_type));
	outpam = inpam;
	tuplerow = pnm_allocpamrow(&inpam);

	processors = 4;
	// processors = atio(argv[1]);

	// read stencil into a 2d array
	printf("load stencil into matrix\n");
	stencil = read_stencil(&instencil);

	// read image into a 2d array with padding according to the stencil length
	printf("reading image\n");
	input_image = read_image(&inpam, &instencil);
	printf("read image\n");

	// do convolution
	convolved_image = convolve(input_image,stencil);

	FILE *op = fopen("lennaout.ppm","w");
	outpam.file = op;
	pnm_writepaminit(&outpam);
	printf("writing output\n");
	for (row = 0; row < convolved_image.height; row++) {
		for (column = 0; column < convolved_image.width; ++column) {
			tuplerow[column][0] = convolved_image.pixels[row][column].r;
			tuplerow[column][1] = convolved_image.pixels[row][column].g;
			tuplerow[column][2] = convolved_image.pixels[row][column].b;
		}
		pnm_writepamrow(&outpam, tuplerow);
	}

	pnm_freepamrow(tuplerow);


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