#ifndef mw_api_h
#define mw_api_h

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
      /*copying piece by piece of the work struct to a byte stream. 
      size will contain the size of the character stream. 
      needed for communication between master and workers*/
   unsigned char *(*serialize_result)(result_unit *res,int *size);
      /*copying piece by piece of the result struct to a byte stream. 
      size will contain the size of the character stream. 
      needed for communication between master and workers*/
   work_unit *(*deserialize)(unsigned char *serialized_work,int size);
      /*taking the byte stream and returns work_units. 
      size will contain the size of the character stream. 
      needed for communication between master and workers*/
   result_unit *(*deserialize_result)(unsigned char *serialized_result,int size);
      /*taking the byte stream and returns the result_unit. 
      size will contain the size of the character stream. 
      needed for communication between master and workers*/
   int (*get_result_state)(result_unit *res);
   unsigned long (*work_first)(work_unit *work);
   result_unit *(*combine_partial_results)(result_unit *r1,result_unit *r2);
   result_unit *(*get_result_object)();
   void (*reinit)(work_unit *work_unit);
   int work_sz, res_sz; 
      /* size in bytes of the work structure and result structure, needed to send/receive messages */
};

void MW_Run (int argc, char **argv, struct mw_api_spec *f); /* run master-worker */
void testing();

#endif
