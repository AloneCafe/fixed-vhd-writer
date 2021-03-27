#ifndef _FIXED_VHD_WRITER_H_
#define _FIXED_VHD_WRITER_H_

#include "stdint.h"
#include "stdio.h"

#define VHD_COOKIE_STRING "conectix"

typedef int BOOL;

#ifdef __cplusplus
struct vhd_sector {
	int vaild_bytes; /* 0 ~ 512 */
	uint8_t raw[512];
};

enum writer_error {
	NOERR,
	LBA_OUT_OF_RANGE,
	OPEN_FILE_ERROR
};

struct writer_object {
	FILE *vhd_fp; /* file pointer */
	BOOL vaild;
	BOOL fixed; /* fixed VHD flag */
	int64_t size;
	enum writer_error last_error;
};

enum option_flag {
	FLAG_SET_VHD = 1,
	FLAG_SET_DATA_FILE = 2,
	FLAG_SET_LBA = 4
};
#else
struct vhd_sector {
	int vaild_bytes; /* 0 ~ 512 */
	uint8_t raw[512];
};

enum writer_error {
	NOERR,
	LBA_OUT_OF_RANGE,
	OPEN_FILE_ERROR
};

struct writer_object {
	FILE * vhd_fp; /* file pointer */
	BOOL vaild;
	BOOL fixed; /* fixed VHD flag */
	int64_t size;
	enum writer_error last_error;
};

enum option_flag {
	FLAG_SET_VHD = 1,
	FLAG_SET_DATA_FILE = 2,
	FLAG_SET_LBA = 4
};
#endif

#ifdef __cplusplus 
extern "C" {
#endif

	int init_writer_object(struct writer_object * wo_p, const char * name);
	void release_writer_object(struct writer_object * wo_p);
	int vaild_vhd(struct writer_object * wo_p);
	int fixed_vhd(struct writer_object * wo_p);
	int64_t size_vhd(struct writer_object * wo_p);
	int write_a_vhd_sector(struct writer_object * wo_p, int64_t lba, const struct vhd_sector * sector_p);
	int64_t write_hvd_sector_from_data_file(struct writer_object * wo_p, const int64_t lba, const char * name);
	enum writer_error get_last_error(const struct writer_object * wo_p);
	void set_last_error(struct writer_object * wo_p, enum writer_error last_error);
	int64_t get_file_size_by_name(const char * const name);

#ifdef __cplusplus 
}
#endif

#endif
