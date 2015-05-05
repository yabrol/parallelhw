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

typedef struct stencil_struct
{
	/* data */
	double **pixels;
	int width;
	int height;
}stencil_image;

pix **allocate_pixels(int height,int width){
	pix **pixels;
	int i,j;
	// allocate space for image -> 2d array of pix
	pixels = (pix**) malloc(sizeof(pix*)* height);
	for (i=0; i< height; i++)
        pixels[i] = (pix *)malloc(width * sizeof(pix));

    return pixels;
}

double **allocate_stencil_pixels(int height,int width){
	double **pixels;
	int i,j;
	// allocate space for image -> 2d array of double
	pixels = (double**) malloc(sizeof(double*)* height);
	for (i=0; i< height; i++)
        pixels[i] = (double *)malloc(width * sizeof(double));

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


image pad_image(image input_image,struct pam *pam_stencil){
	tuple *tuplerow;
	int height = input_image.height + pam_stencil->height - 1;
	int width = input_image.width + pam_stencil->width - 1;
	int k = (pam_stencil->width + 1)/2;
	int nw = input_image.width;
	int nh = input_image.height;
	int i,j;
	image padded_image;

	padded_image.width = width;
	padded_image.height = height;

	printf("padded image %d x %d\n", width, height);
	printf("image:%d x %d , stencil %d x %d \n", nw,nh, pam_stencil->width,pam_stencil->height);

	
    // printf("space allocated for image\n");
    padded_image.pixels = allocate_pixels(width, height);

    // read in the image and load it into the array
	for (i=0; i< height; i++){
		// assign zero values if one of the padded rows
    	if( i< (k-1) || i>((k-1)+ nh -1 ) ){
	    	for(j=0;j< width;j++){
	    		padded_image.pixels[i][j].r = 0;
	    		padded_image.pixels[i][j].g = 0;
	    		padded_image.pixels[i][j].b = 0;
	    	}
	    }
	    else{
	    	for(j=0;j< width;j++){
	    		// asssign zero value if one of the padded columns
	    		if( j< (k-1) || j>((k-1)+ nw -1 ) ){
		    		padded_image.pixels[i][j].r = 0;
		    		padded_image.pixels[i][j].g = 0;
		    		padded_image.pixels[i][j].b = 0;
		    	}
		    	else{
		    		padded_image.pixels[i][j] = input_image.pixels[i-(k-1)][j-(k-1)];
		    	}
	    	}
	    }
    }

    return padded_image;
}

image get_submatrix(image mat,int i,int j,int size_x,int size_y,struct pam *stencil){
	int x,y;
	image part;
	part.width = size_x;
	part.height = size_y;	
	part.pixels = allocate_pixels(size_x,size_y);

	image padded_image = pad_image(mat,stencil);

	for(x=i;x<i+size_x;x++)
		for(y=j;y<j+size_y;y++){
			part.pixels[x-i][y-j] = padded_image.pixels[x][y];
		}

	return part;
}

image read_color_image(struct pam *pam_image){
	tuple *tuplerow;
	// if stencil is 2*k -1, k-1 padding needed on all sides
	int height = pam_image->height;
	int width = pam_image->width;
	int i,j,k;
	image input_image;

	input_image.width = width;
	input_image.height = height;

	// allocate tuplerow
	tuplerow = pnm_allocpamrow(pam_image);

    printf("space allocated for image\n");
    input_image.pixels = allocate_pixels(width, height);

    // read in the image and load it into the array
	for (i=0; i< height; i++){
	    	// read row into tuple
    	pnm_readpamrow(pam_image, tuplerow);
    	for(j=0;j< width;j++){
			input_image.pixels[i][j].r = tuplerow[j][0];
    		input_image.pixels[i][j].g = tuplerow[j][1];
    		input_image.pixels[i][j].b = tuplerow[j][2];	
    	}
    }

    // free tuple
    pnm_freepamrow(tuplerow);
    return input_image;
}


image read_and_pad_image(struct pam *pam_image, struct pam *pam_stencil){
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


stencil_image read_stencil(struct pam *pam_stencil){
	tuple *tuplerow;
	int height = pam_stencil->height;
	int width = pam_stencil->width;
	double maxval = pam_stencil->maxval;
	int i,j;
	stencil_image stencil;

	stencil.height = height;
	stencil.width = width;

	// allocate tuplerow
	tuplerow = pnm_allocpamrow(pam_stencil);


	// allocate space for image -> 2d array of pix
	stencil.pixels = allocate_stencil_pixels(height,width);

    // read in the image and load it into the array
	for (i=0; i< height; i++){
    	// read row into tuple
    	printf("\n");
    	pnm_readpamrow(pam_stencil, tuplerow);
    	for(j=0;j< width;j++){
    		stencil.pixels[i][j] = -4 + (8 * (double)tuplerow[j][0])/(maxval - 1);
    		printf("%f,%f ", stencil.pixels[i][j],(double)tuplerow[j][0]);
    	}
    }

    // free tuple
    pnm_freepamrow(tuplerow);
    
    return stencil;
}

pix convolve_pixel(image mat, stencil_image stencil, int i,int j, int size){
	int x,y;
	pix value;
	double temp_r = 0;
	double temp_g = 0;
	double temp_b = 0;
	double sum = 0;
	//printf("i %d, j %d, size %d\n",i,j,size );
	for(x=i;x<i+size;x++)
		for(y=j;y<j+size;y++){
			//printf("%d %d %d\n",x-i,y-j,a[x][y] );
			temp_r += (mat.pixels[x][y].r * stencil.pixels[x-i][y-j]);
			temp_g += (mat.pixels[x][y].g * stencil.pixels[x-i][y-j]);
			temp_b += (mat.pixels[x][y].b * stencil.pixels[x-i][y-j]);

			sum+= stencil.pixels[x-i][y-j];
		}	

	value.r = temp_r/sum;
	value.g = temp_g/sum;
	value.b = temp_b/sum;

	//printf("%f %f %f\n\n", temp_r/sum, temp_g/sum, temp_b/sum);
	return value;
}

image convolve(image input_image, stencil_image stencil ){
	int k = (stencil.width + 1)/2;
	int i,j;
	int width = input_image.width;
	int height = input_image.height;
	image result;
	result.height = input_image.height - 2*(k-1);
	result.width = input_image.width - 2*(k-1);

		// allocate space for image -> 2d array of pix
	result.pixels = allocate_pixels(result.height,result.width);

	printf("image to convolve %d x %d\n",input_image.width,input_image.height);
	printf("stencil to convolve %d x %d\n",stencil.width,stencil.height);

	for (i=0; i< height; i++){
		if( !( i< (k-1) || i>( height - (k-1) - 1 ) ) ){
	    	for(j=0;j< width;j++){
	    		// asssign zero value if one of the padded columns
	    		if( !( j< (k-1) || j>( width - (k-1) - 1 ) ) ){
	    			// do convolution
	    			/*
	    			pix temp = convolve_pixel(input_image, stencil, i-(k-1), j-(k-1), 2*(k-1));
	    			result.pixels[i-(k-1)][j-(k-1)].r = input_image.pixels[i][j].r + temp.r; 
	    			result.pixels[i-(k-1)][j-(k-1)].g = input_image.pixels[i][j].g + temp.g;
	    			result.pixels[i-(k-1)][j-(k-1)].b = input_image.pixels[i][j].b + temp.b;
	    			*/
	    			result.pixels[i-(k-1)][j-(k-1)] = convolve_pixel(input_image, stencil, i-(k-1), j-(k-1), (2*k)-1 );
		    		//result.pixels[i-(k-1)][j-(k-1)] = input_image.pixels[i][j];
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
	image input_image,result;
	stencil_image stencil;
	int processors;
	tuple * tuplerow;
	int j,n,x,y,p,k,i = 0;
	// write the result

	FILE *fp = fopen("lenn.ppm", "r");
	FILE *st = fopen("gauss.pgm", "r");
	

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
	input_image = read_color_image(&inpam); 

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
	int n_iter = 20;
	#pragma omp parallel private(i,j,partial_image,convolved_image) shared(result,input_image,x,y,n_iter)
	{
		int l;
		for(l=0;l<n_iter;l++){
			int pid = omp_get_thread_num();
			i = pid/x_limit;
			j = pid%y_limit;
  			int t_x = i*(x ) ;
  			int t_y = j*(y );
			int r,c,p,o;

  			printf ("hello there, I am thread %d and i is %d and j is %d\n", omp_get_thread_num(),i,j);
  			
  			// get submatrix
  			partial_image = get_submatrix(input_image, t_x, t_y, x + 2*(k-1) , y + 2*(k-1),&instencil);
  			// convolve
  			convolved_image = convolve(partial_image,stencil);
  			printf("convolved image\n");

  			// update the result buffer
  			printf("convolved for (%d ,%d), block size %d x %d, output %d x %d\n", t_x, t_y, x + 2*(k-1) , y + 2*(k-1), convolved_image.width, convolved_image.height);
			printf("writing results to %d, %d of size %d x %d\n", i*x, j*y , convolved_image.height, convolved_image.width);
				// printf("\n");
  			for(p=0;p<convolved_image.height;p++){
  				// printf("\n");
  				for(o=0;o<convolved_image.width;o++){
  					// printf("(%d %d %d)", convolved_image.pixels[p][o].r,convolved_image.pixels[p][o].g,convolved_image.pixels[p][o].b);
  				}
  			}
  			// printf("\nblocked\n");
			// printf("block\n");
			for (r = i*x; r < convolved_image.height +(i*x); r++) {
				// printf("\n");
				for (c = j*y; c < convolved_image.width +(j*y); c++) {
					result.pixels[r][c].r = convolved_image.pixels[r - (i*x)][c - (j*y)].r;
					result.pixels[r][c].g = convolved_image.pixels[r - (i*x)][c - (j*y)].g;
					result.pixels[r][c].b = convolved_image.pixels[r - (i*x)][c - (j*y)].b;
					// printf("(%d %d %d)", result.pixels[r][c].r,result.pixels[r][c].g,result.pixels[r][c].b);
				}
			}
		
			// make sure every processor computed result
			// copy result back to image
			#pragma omp barrier	
			for (r = i*x; r < convolved_image.height +(i*x); r++) {
				for (c = j*y; c < convolved_image.width +(j*y); c++) {
						input_image.pixels[r][c].r = result.pixels[r][c].r;
						input_image.pixels[r][c].g = result.pixels[r][c].g;
						input_image.pixels[r][c].b = result.pixels[r][c].b;
						// printf("(%d %d %d)", result.pixels[r][c].r,result.pixels[r][c].g,result.pixels[r][c].b);
				}
			}				
			#pragma omp barrier	

		}
	}

	
	FILE *op = fopen("lennaout.ppm","w");
	outpam.file = op;    
	pnm_writepaminit(&outpam);
	printf("writing output\n");
	for (row = 0; row < result.height; row++) {
		// printf("\n");
		for (column = 0; column < result.width; ++column) {
			tuplerow[column][0] = result.pixels[row][column].r;
			tuplerow[column][1] = result.pixels[row][column].g;
			tuplerow[column][2] = result.pixels[row][column].b;
			// printf("(%d %d %d)", result.pixels[row][column].r,result.pixels[row][column].g,result.pixels[row][column].b);
		}
		pnm_writepamrow(&outpam, tuplerow);
	}

	pnm_freepamrow(tuplerow);
 	return 0;

}
