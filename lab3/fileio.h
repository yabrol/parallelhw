#ifndef fileio_h
#define fileio_h

void write_workList(unsigned char *serial_work, int charLength);

void write_result_list(unsigned char *serial_result, int charLength);

void write_completed_workID(int id);

void read_workList();

work_unit** read_work_list( struct mw_api_spec *f);

void write_workList_length(int length);
#endif