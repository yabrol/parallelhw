#ifndef fileio_h
#define fileio_h

void write_workList(unsigned char *serial_work, int charLength);

void write_result_list(unsigned char *serial_result, int charLength, int work_id);

void write_completed_workID(int id);

void read_workList();

work_unit** read_work_list( struct mw_api_spec *f);

void create_blank_files();

result_unit **read_result_list(int n_chunks, struct mw_api_spec *f, int *work_processed);

void write_workList_length(int length);
#endif