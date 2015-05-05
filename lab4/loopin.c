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

image pad_image(image input_image,struct pam *pam_stencil){
	tuple *tuplerow;
	int height = input_image.height + pam_stencil->height - 1;
	int width = input_image.width + pam_stencil->width - 1;
	int pad_top = floor((double)(pam_stencil->height - 1)/2);
	int pad_bottom = ceil((double)(pam_stencil->height - 1)/2);
	int pad_left = floor((double)(pam_stencil->width - 1)/2);
	int pad_right = ceil((double)(pam_stencil->width - 1)/2);
	int nw = input_image.width;
	int nh = input_image.height;
	int i,j;
	image padded_image;

	padded_image.width = width;
	padded_image.height = height;

	// printf("image:%d x %d , stencil %d x %d , pad_width (%d,%d), pad_height (%d,%d)\n", nw,nh, pam_stencil->width,pam_stencil->height,pad_left,pad_right,pad_top,pad_bottom);

	
    padded_image.pixels = allocate_pixels( height, width);
    // printf("space allocated for image\n");

    // read in the image and load it into the array
	for (i=0; i< height; i++){
		// assign zero values if one of the padded rows
    	if( i< pad_top || i>( height - 1 - pad_bottom ) ){
	    	for(j=0;j< width;j++){
	    		padded_image.pixels[i][j].r = 0;
	    		padded_image.pixels[i][j].g = 0;
	    		padded_image.pixels[i][j].b = 0;
	    	}
	    }
	    else{
	    	for(j=0;j< width;j++){
	    		// asssign zero value if one of the padded columns
	    		if( j< pad_left || j>( width - 1 - pad_right ) ){
		    		padded_image.pixels[i][j].r = 0;
		    		padded_image.pixels[i][j].g = 0;
		    		padded_image.pixels[i][j].b = 0;
		    	}
		    	else{
		    		padded_image.pixels[i][j] = input_image.pixels[i-pad_top][j- pad_left];
		    	}
	    	}
	    }
    }

    return padded_image;
}

image get_submatrix(image mat,int i,int j,int size_x,int size_y,struct pam *stencil){
	int x,y;
	image part;
	part.width = size_y;
	part.height = size_x;	
	part.pixels = allocate_pixels(size_x,size_y);

	image padded_image = pad_image(mat,stencil);

	for(x=i;x<i+size_x;x++)
		for(y=j;y<j+size_y;y++){
			part.pixels[x-i][y-j] = padded_image.pixels[x][y];
		}

	// TODO: Free padded image
	return part;
}

image read_color_image(struct pam *pam_image){
	tuple *tuplerow;
	int height = pam_image->height;
	int width = pam_image->width;
	int i,j;
	image input_image;

	input_image.width = width;
	input_image.height = height;

	// allocate tuplerow
	tuplerow = pnm_allocpamrow(pam_image);

    printf("space allocated for image\n");
    input_image.pixels = allocate_pixels( height, width);

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
	int height = pam_image->height + pam_stencil->height - 1;
	int width = pam_image->width + pam_stencil->width - 1;
	int pad_top = floor((double)(pam_stencil->height - 1)/2);
	int pad_bottom = ceil((double)(pam_stencil->height - 1)/2);
	int pad_left = floor((double)(pam_stencil->width - 1)/2);
	int pad_right = ceil((double)(pam_stencil->width - 1)/2);
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
    input_image.pixels = allocate_pixels( height, width);

    // read in the image and load it into the array
	for (i=0; i< height; i++){
		// assign zero values if one of the padded rows
    	if( i< pad_top || i>( pad_top + nh -1 ) ){
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
	    		if( j< pad_left || j>( pad_left + nw -1 ) ){
		    		input_image.pixels[i][j].r = 0;
		    		input_image.pixels[i][j].g = 0;
		    		input_image.pixels[i][j].b = 0;
		    	}
		    	else{
		    		input_image.pixels[i][j].r = tuplerow[j-pad_left][0];
		    		input_image.pixels[i][j].g = tuplerow[j-pad_left][1];
		    		input_image.pixels[i][j].b = tuplerow[j-pad_left][2];	
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

pix convolve_pixel(image mat, stencil_image stencil, int i,int j){
	int x,y;
	pix value;
	double temp_r = 0;
	double temp_g = 0;
	double temp_b = 0;
	double sum = 0;
	for(x=i;x< (i + stencil.height) ;x++)
		for(y=j;y< (j + stencil.width) ;y++){
			temp_r += (mat.pixels[x][y].r * stencil.pixels[x-i][y-j]);
			temp_g += (mat.pixels[x][y].g * stencil.pixels[x-i][y-j]);
			temp_b += (mat.pixels[x][y].b * stencil.pixels[x-i][y-j]);

			sum+= stencil.pixels[x-i][y-j];
		}	

	value.r = temp_r/sum;
	value.g = temp_g/sum;
	value.b = temp_b/sum;
	return value;
}

image convolve(image input_image, stencil_image stencil, int t_num ){
	int i,j;
	int width = input_image.width;
	int height = input_image.height;
	int pad_top = floor((double)(stencil.height - 1)/2);
	int pad_bottom = ceil((double)(stencil.height - 1)/2);
	int pad_left = floor((double)(stencil.width - 1)/2);
	int pad_right = ceil((double)(stencil.width - 1)/2);
	image result;
	result.height = input_image.height - (stencil.height - 1);
	result.width = input_image.width - (stencil.width - 1);

		// allocate space for image -> 2d array of pix
	result.pixels = allocate_pixels(result.height,result.width);

	printf("image to convolve %d x %d for thread %d\n",input_image.width,input_image.height, t_num);
	printf("stencil to convolve %d x %d\n",stencil.width,stencil.height);

	for (i=0; i< height; i++){
		if( !( i< pad_top || i>( height - pad_bottom - 1 ) ) ){
	    	for(j=0;j< width;j++){
	    		// asssign zero value if one of the padded columns
	    		if( !( j< pad_left || j>( width - pad_right - 1 ) ) ){
	    			// do convolution
	    			result.pixels[i- pad_top][j- pad_left] = convolve_pixel(input_image, stencil, i- pad_top, j- pad_left );
//	    			result.pixels[i- pad_top][j- pad_left] = input_image.pixels[i][j];
		    	}
	    	}
	    }
    }
    // save convolved image
    return result;

}

int main (int argc, char **argv)
{
	omp_set_dynamic(0);     // Explicitly disable dynamic teams
	struct pam inpam, outpam, instencil;
	unsigned int row,column,plane;
	unsigned int grand_total;
	image input_image,result;
	stencil_image stencil;
	int processors;
	tuple * tuplerow;
	int j,n,n_x,n_y,x,y,p,p_x,p_y,s_w,s_h,x_remainder,y_remainder,root_p,i = 0;
	// write the result

	p = 8;
	omp_set_num_threads(p);


	FILE *fp = fopen("lencut.ppm", "r");
	FILE *st = fopen("gauss.pgm", "r");
	

	pm_init(argv[0], 0);

	pnm_readpaminit(fp, &inpam, PAM_STRUCT_SIZE(tuple_type));
	pnm_readpaminit(st, &instencil, PAM_STRUCT_SIZE(tuple_type));
	
	outpam = inpam;
	tuplerow = pnm_allocpamrow(&inpam);

	n_x = inpam.height;
	n_y = inpam.width;
	s_w = instencil.width;
	s_h = instencil.height;
	// processors = atio(argv[1]);

	// read stencil into a 2d array
	printf("load stencil into matrix\n");
	stencil = read_stencil(&instencil);

	// read image into a 2d array with padding according to the stencil length
	printf("reading image\n");
	input_image = read_color_image(&inpam); 

	result.width = inpam.width;
	result.height = inpam.height;
	result.pixels = allocate_pixels(inpam.height, inpam.width);

	root_p = sqrt(p);
	p_x = root_p - (p%(root_p * root_p)); // to make sure integers p_x * p_y = p 
	p_y = p/p_x;
	x = (int)(n_x/p_x);
	x_remainder = n_x%p_x;
	y = (int)(n_y/p_y);
	y_remainder = n_y%p_y;
	printf("x %d y %d\n", x, y);
	int x_limit = n_x/x;
	int y_limit = n_y/y;
	printf("x_limit %d y_limit %d\n", x_limit, y_limit);

	image partial_image,convolved_image;
	int n_iter = 1;
	#pragma omp parallel private(i,j,partial_image,convolved_image) shared(result,input_image,x,y,n_iter,x_limit,y_limit)
	{
		int l;
		for(l=0;l<n_iter;l++){
			int pid, t_x, t_y, r, c, p, o, size_x, size_y;
			pid = omp_get_thread_num();
			i = pid/y_limit;
			j = pid%y_limit;
			t_x = i*(x ) ;
  			t_y = j*(y );
			size_x = x + (i == (x_limit -1))*x_remainder;
			size_y = y + (j == (y_limit -1))*y_remainder;

  			printf ("hello there, I am thread %d and i is %d and j is %d requesting submatrix %d x %d\n", pid,i,j,size_y,size_x);
  			
  			// get submatrix
  			partial_image = get_submatrix(input_image, t_x, t_y, size_x + s_h - 1 , size_y + s_w - 1,&instencil);
  			// convolve
  			convolved_image = convolve(partial_image,stencil,omp_get_thread_num());
  			printf("convolved image\n");

  			// update the result buffer
  			printf("convolved for (%d ,%d), block size %d x %d, output %d x %d\n", t_x, t_y, size_x + s_h - 1 , size_y + s_w - 1, convolved_image.width, convolved_image.height);
			printf("writing results to %d, %d of size %d x %d\n", i*x, j*y , convolved_image.height, convolved_image.width);


			for (r = i*x; r < convolved_image.height +(i*x); r++) {
				// printf("\n");
				for (c = j*y; c < convolved_image.width +(j*y); c++) {
					result.pixels[r][c].r = convolved_image.pixels[r - (i*x)][c - (j*y)].r;
					result.pixels[r][c].g = convolved_image.pixels[r - (i*x)][c - (j*y)].g;
					result.pixels[r][c].b = convolved_image.pixels[r - (i*x)][c - (j*y)].b;
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
				}
			}				
			#pragma omp barrier	
			// TODO: free partial & convolved image

		}
	}

	
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
