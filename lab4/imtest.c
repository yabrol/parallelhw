#include <stdio.h>
#include <netpbm/pam.h>

// http://netpbm.sourceforge.net/doc/libpm.html#initialization

int main(int argc, char **argv){
	struct pam inpam, outpam;
	tuple * tuplerow;
	unsigned int row;
	unsigned int grand_total;
	FILE *fp = fopen("lenna.ppm", "r");
	FILE *op = fopen("lennaout.ppm", "w");
	pm_init(argv[0], 0);

	pnm_readpaminit(fp, &inpam, PAM_STRUCT_SIZE(tuple_type));

	outpam = inpam;
	outpam.file = op;

	pnm_writepaminit(&outpam);

	tuplerow = pnm_allocpamrow(&inpam);

	for (row = 0; row < inpam.height; row++) {
	   unsigned int column;
	   pnm_readpamrow(&inpam, tuplerow);
	   for (column = 0; column < inpam.width; ++column) {
	       unsigned int plane;
	       for (plane = 0; plane < inpam.depth; ++plane) {
	           grand_total += tuplerow[column][plane];
	       }
	   }
	   pnm_writepamrow(&outpam, tuplerow);
	}

	pnm_freepamrow(tuplerow);
}