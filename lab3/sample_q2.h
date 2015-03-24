#ifndef sample_q2_h
#define sample_q2_h

struct work_t {
	unsigned long first;
	unsigned long end;
	//unsigned long length; // length of the array
	unsigned long num; // number for which the factors need to be calculated
};

struct result_t {
   	unsigned long *factors; // pointer to array of factors
   	int state;
	int length;
};

#endif