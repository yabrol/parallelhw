//#ifndef mw_api_h
//#define mw_api_h

struct work_t; /* definition provided by user */
struct result_t; /* definition provided by user */
typedef struct work_t work_unit;
typedef struct result_t result_unit;

struct mw_api_spec {
   work_unit **(*create) (int argc, char **argv); 
      /* create work: return a NULL-terminated list of work. Return NULL if it fails. */

   int (*compile) (int sz, result_unit **res);      
      /* process result. Input is a collection of results, of size sz. Returns 1 on success, 0 on failure. */

   result_unit *(*compute) (work_unit *work);
      /* compute, returning NULL if there is no result, non-NULL if there is a result to be returned. */

   unsigned char *(*serialize)(work_unit *work,int *size);
   unsigned char *(*serialize_result)(result_unit *res,int *size);
   work_unit *(*deserialize)(unsigned char *serialized_work,int size);
   result_unit *(*deserialize_result)(unsigned char *serialized_result,int size);
   int work_sz, res_sz; 
      /* size in bytes of the work structure and result structure, needed to send/receive messages */
};

void MW_Run (int argc, char **argv, struct mw_api_spec *f); /* run master-worker */
void testing();

//#endif
