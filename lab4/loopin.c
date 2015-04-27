#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
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

pix **allocate_pixels(int height,int width){
	pix **pixels;
	int i,j;
	// allocate space for image -> 2d array of pix
	pixels = (pix**) malloc(sizeof(pix*)* height);
	for (i=0; i< height; i++)
        pixels[i] = (pix *)malloc(width * sizeof(pix));

    return pixels;
}

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

image get_submatrix(image mat,int i,int j,int size_x,int size_y){
	int x,y;
	image part;
	part.width = size_x;
	part.height = size_y;	
	part.pixels = allocate_pixels(size_x,size_y);

	for(x=i;x<i+size_x;x++)
		for(y=j;y<j+size_y;y++){
			part.pixels[x-i][y-j] = mat.pixels[x][y];
		}

	return part;
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

	
    // printf("space allocated for image\n");
    input_image.pixels = allocate_pixels(width, height);

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
	image input_image,stencil,result;
	int processors;
	tuple * tuplerow;
	int j,n,x,y,p,k,i = 0;
    

	FILE *fp = fopen("lenn.ppm", "r");
	FILE *st = fopen("stencil.pgm", "r");
	

	pm_init(argv[0], 0);

	pnm_readpaminit(fp, &inpam, PAM_STRUCT_SIZE(tuple_type));
	pnm_readpaminit(st, &instencil, PAM_STRUCT_SIZE(tuple_type));
	
	outpam = inpam;
	tuplerow = pnm_allocpamrow(&inpam);

	p = 4;
	n = inpam.height;
	k = (instencil.width + 1)/2;
	// processors = atio(argv[1]);

	// read stencil into a 2d array
	printf("load stencil into matrix\n");
	stencil = read_stencil(&instencil);

	// read image into a 2d array with padding according to the stencil length
	printf("reading image\n");
	input_image = read_image(&inpam, &instencil);
	printf("read image\n");

	result.width = inpam.width;
	result.height = inpam.height;
	result.pixels = allocate_pixels(inpam.width,inpam.height);

	x = (int)(n/sqrt(p));
	y = (int)(n/sqrt(p));
	int x_limit = n/x;
	int y_limit = n/y;

	image partial_image,convolved_image;
	char result_p[2][2];
	omp_set_num_threads(4);
	#pragma omp parallel for collapse(2) private(i,j,partial_image,convolved_image) shared(result,input_image,x,y,result_p) schedule(static,1)
  	for(i=0; i<x_limit; i++)
	{
	  	for(j=0; j<y_limit; j++)
	  	{
	  		//if(omp_get_thread_num() == 2){
	  			int t_x = i*(x - (k-1));
	  			int t_y = j*(y - (k-1));
	  			printf ("hello there, I am thread %d and i is %d and j is %d\n", omp_get_thread_num(),i,j);
	  			
	  			// get submatrix
	  			partial_image = get_submatrix(input_image, t_x, t_y, x + 2*(k-1) , y + 2*(k-1));
	  			// convolve
	  			convolved_image = convolve(partial_image,stencil);
	  			// update the result buffer
	  			printf("convolved for (%d ,%d), block size %d x %d, output %d x %d\n", t_x, t_y, x + 2*(k-1) , y + 2*(k-1), convolved_image.width, convolved_image.height);
				printf("writing results to %d, %d of size %d x %d\n", i*x, j*y , convolved_image.height, convolved_image.width);
				
				int r,c;
				for (r = i*x; r < convolved_image.height +(i*x); r++) {
					
					for (c = j*y; c < convolved_image.width +(j*y); c++) {
						result.pixels[r][c].r = convolved_image.pixels[r - (i*x)][c - (j*y)].r;
						result.pixels[r][c].g = convolved_image.pixels[r - (i*x)][c - (j*y)].g;
						result.pixels[r][c].b = convolved_image.pixels[r - (i*x)][c - (j*y)].b;
					}
				}
				
			//}
		}	

	}

	
	// write the result
	FILE *op = fopen("lennaout.ppm","w");
	outpam.file = op;
	pnm_writepaminit(&outpam);
	printf("writing output\n");
	for (row = 0; row < result.height; row++) {
		for (column = 0; column < result.width; ++column) {
			tuplerow[column][0] = result.pixels[row][column].r;
			tuplerow[column][1] = result.pixels[row][column].g;
			tuplerow[column][2] = result.pixels[row][column].b;
		}
		pnm_writepamrow(&outpam, tuplerow);
	}

	pnm_freepamrow(tuplerow);
 	return 0;

}
