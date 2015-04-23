#include <stdio.h>
#include <omp.h>

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



int main (void)
{
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
