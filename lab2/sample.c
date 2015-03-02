#include "mw_api.h"
#include "stdio.h"
#include "mpi.h"

struct userdef_work_t {
	int t; 
};

struct userdef_result_t {
   	int t;
};

mw_work_t **create_work(int argc, char **argv);
int process_results(int sz, mw_result_t *res);
mw_result_t *do_work(mw_work_t *work);


int main (int argc, char **argv)
{
  struct mw_api_spec f;

  MPI_Init (&argc, &argv);

  f.create = create_work;
  f.result = process_results;
  f.compute = do_work;
  f.work_sz = sizeof (struct userdef_work_t);
  f.res_sz = sizeof (struct userdef_result_t);

  MW_Run (argc, argv, &f);
  testing();
  MPI_Finalize ();

  return 0;

}

mw_work_t **create_work(int argc, char **argv){
	mw_work_t **work;
	return work;
}
int process_results(int sz, mw_result_t *res){
	int result;
	return result;
}
mw_result_t *do_work(mw_work_t *work){
	mw_result_t *result;
	return result;
}
