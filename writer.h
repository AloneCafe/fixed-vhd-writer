#ifndef _FIXED_VHD_WRITER_H_
#define _FIXED_VHD_WRITER_H_

#include "stdint.h"
#include "stdio.h"

#define VHD_COOKIE_STRING "conectix"


#ifdef __cplusplus 
extern "C" {
#endif

typedef int BOOL;

typedef struct _vhd_sector {
	int vaild_bytes; /* 0 ~ 512 */
	uint8_t raw[512];
}vhd_sector;

typedef enum _writer_error {
	NO_ERROR,
	LBA_OUT_OF_RANGE,
	OPEN_FILE_ERROR
}writer_error;

typedef struct _writer_object {
	FILE *vhd_fp; /* file pointer */
	BOOL vaild;
	BOOL fixed; /* fixed VHD flag */
	int64_t size;
	writer_error last_error;
}writer_object;

typedef enum _option_flag {
	FLAG_SET_VHD = 1,
	FLAG_SET_DATA_FILE = 2,
	FLAG_SET_LBA = 4
}option_flag;

int init_writer_object(writer_object * const wo_p, const char * const name);
void release_writer_object(writer_object * const wo_p);
int vaild_vhd(writer_object * const wo_p);
int fixed_vhd(writer_object * const wo_p);
int write_a_vhd_sector(writer_object * const wo_p, const int64_t lba, const vhd_sector * const sector_p);
int64_t write_hvd_sector_from_data_file(writer_object * const wo_p, const int64_t lba, const char * const name);
writer_error get_last_error(const writer_object * const wo_p);
void set_last_error(writer_object * const wo_p, const writer_error last_error);

#ifdef __cplusplus 
}
#endif

#endif
