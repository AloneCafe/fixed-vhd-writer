#define _CRT_SECURE_NO_WARNINGS
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "writer.h"

int init_writer_object(struct writer_object * wo_p, const char * name) {
	FILE *fp = fopen(name, "rb+");
	if (fp == NULL) {
		set_last_error(wo_p, OPEN_FILE_ERROR);
		return 0;
	}
	wo_p->vhd_fp = fp;
	wo_p->fixed = 0;
	wo_p->vaild = 0;
	wo_p->size = 0;
	wo_p->last_error = NOERR;
	return 1;
}

void release_writer_object(struct writer_object * wo_p) {
	fclose(wo_p->vhd_fp);
}

int vaild_vhd(struct writer_object * wo_p) {
	char vhd_cookie[9] = { 0 };
	/* the beginning of last sector means VHD cookie */
	fseek(wo_p->vhd_fp, -512, SEEK_END);
	fread(vhd_cookie, 1, 8, wo_p->vhd_fp);
	return strcmp(vhd_cookie, VHD_COOKIE_STRING) == 0 && (wo_p->vaild = 1);
}

int fixed_vhd(struct writer_object * wo_p) {
	int vhd_type;
	/* the last sector with offset 0x3C means VHD type */
	fseek(wo_p->vhd_fp, -512 + 0x3C, SEEK_END);
	fread(&vhd_type, sizeof(int), 1, wo_p->vhd_fp);
	return vhd_type == 0x02000000 && (wo_p->fixed = 1);
}

int64_t size_vhd(struct writer_object * wo_p) {
	int64_t size;
	/* the last sector with offset 0x28 means original size */
	fseek(wo_p->vhd_fp, -512 + 0x28, SEEK_END);
	fread(&size, sizeof(int64_t), 1, wo_p->vhd_fp);
	/* reverse big-endian into little-endian */
	char *p = (char *)&size, temp;
	int i;
	for (i = 0; i < sizeof(size) / 2; i++) {
		temp = *p;
		*(p + i) = *(p + 7 - i);
		*(p + 7 - i) = temp;
	}
	return wo_p->size = size;
}

int write_a_vhd_sector(struct writer_object * wo_p, const int64_t lba, const struct vhd_sector * sector_p) {
	fseek(wo_p->vhd_fp, lba * 512, SEEK_SET);
	return fwrite(sector_p->raw, 1, sector_p->vaild_bytes, wo_p->vhd_fp);
}

int64_t write_hvd_sector_from_data_file(struct writer_object * wo_p, int64_t lba, const char * name) {
	int vaild_bytes, written_bytes;
	int64_t total_written_bytes = 0, lba_index = lba, lba_max = wo_p->size / 512;
	if (lba_index < 0 || lba_index >= lba_max) {
		set_last_error(wo_p, LBA_OUT_OF_RANGE);
		return 0;
	}

	FILE *fp = fopen(name, "rb");
	struct vhd_sector sector;

	if (fp == NULL) {
		set_last_error(wo_p, OPEN_FILE_ERROR);
		return 0;
	}

	do {
		vaild_bytes = fread(sector.raw, 1, 512, fp);
		sector.vaild_bytes = vaild_bytes;
		written_bytes = write_a_vhd_sector(wo_p, lba_index++, &sector);
		total_written_bytes += written_bytes;
	} while (lba_index < lba_max && vaild_bytes == 512 && written_bytes);

	fclose(fp);
	return total_written_bytes;
}

enum writer_error get_last_error(const struct writer_object * wo_p) {
	return wo_p->last_error;
}

void set_last_error(struct writer_object * wo_p, enum writer_error last_error) {
	wo_p->last_error = last_error;
}


void _err_msg(const char * fmt, ...) {
	va_list vl;
	va_start(vl, fmt);
	vfprintf(stderr, fmt, vl);
	fprintf(stderr, "\n");
	va_end(vl);
}

int64_t get_file_size_by_name(const char *name) {
    FILE *fp = fopen(name, "rb");
    if (fp == NULL)
        return 0; 

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0; 
    }
	
    long size = ftell(fp);
    fclose(fp);

    if (size == -1) 
        return 0; 

    return (int64_t)size; 
}

// arg parser
#define MAX_OPTIONS 100
#define MAX_OPTION_LENGTH 50

typedef struct {
    char *option;
    char *value;
} Option;

typedef struct {
    Option options[MAX_OPTIONS];
    int option_count;
    char **args;
    int arg_count;
} ArgParser;

void init_parser(ArgParser *parser) {
    parser->option_count = 0;
    parser->arg_count = 0;
    parser->args = NULL;
}

int add_option(ArgParser *parser, const char *option, const char *value) {
    if (parser->option_count >= MAX_OPTIONS) {
        return -1; // 超过最大选项
    }
    parser->options[parser->option_count].option = strdup(option);
    parser->options[parser->option_count].value = strdup(value);
    parser->option_count++;
    return 0;
}

void parse_args(ArgParser *parser, int argc, char *argv[]) {
    parser->args = malloc((argc - 1) * sizeof(char *)); 
    parser->arg_count = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            // 处理没有值的选项
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                // 存在值
                add_option(parser, argv[i], argv[i + 1]);
                i++; // 跳过值
            } else {
                // 没有值
                add_option(parser, argv[i], NULL);
            }
        } else {
            parser->args[parser->arg_count++] = argv[i];
        }
    }
}

void free_parser(ArgParser *parser) {
    for (int i = 0; i < parser->option_count; i++) {
        free(parser->options[i].option);
        free(parser->options[i].value);
    }
    free(parser->args);
}

// arg paser


int main(int argc, char **argv) {
#define __err(s, ...) _err_msg(s, ##__VA_ARGS__);

	if (argc <= 1) {
		__err("Fixed VHD Writer\n[-h] usage help\n[-r] specify data file name (read)\n[-w] specify VHD file name (write)\n[-a] specify LBA to writing data\n");
		return 1;
	}

	ArgParser parser;
    init_parser(&parser);
    parse_args(&parser, argc, argv);

	struct writer_object wo;
	int i, j, states = 0;
	char cc, nc;

	char *vhd_file_name = NULL, *data_file_name = NULL;
	int64_t lba;
	enum option_flag last_option_flag = 0; /* 3 options must be set up */


	for (int i = 0; i < parser.option_count; i++) {
		const char * o = parser.options[i].option;
		const char * v = parser.options[i].value;
		if (strcmp(o, "-w") == 0) {
			if (!v) {
				__err("VHD image file is not specified");
				free_parser(&parser);
				return 1;
			}
			vhd_file_name = v;
			last_option_flag |= FLAG_SET_VHD;
		} else if (strcmp(o, "-a") == 0) {
			if (!v) {
				__err("LBA offset is not specified");
				free_parser(&parser);
				return 1;
			}
			sscanf(v, "%lld", &lba);
			last_option_flag |= FLAG_SET_LBA;

		} else if (strcmp(o, "-r") == 0) {
			if (!v) {
				__err("Data file is not specified");
				free_parser(&parser);
				return 1;
			}
			data_file_name = v;
			last_option_flag |= FLAG_SET_DATA_FILE;

		} else if (strcmp(o, "-h") == 0) {
			__err("Fixed VHD Writer\n[-h] usage help\n[-r] specify data file name (read)\n[-w] specify VHD file name (write)\n[-a] specify LBA to writing data\n");
			free_parser(&parser);
			return 1;

		} else {
			__err("Invaild option \'%s\'", o); 
			free_parser(&parser);
			return 1;
		}
    }

	if ((last_option_flag & FLAG_SET_VHD) == 0) {
		__err("VHD image file is not specified");
		return -1;
	}
	else if ((last_option_flag & FLAG_SET_LBA) == 0) {
		__err("LBA offset is not specified");
		return -1;
	}
	else if ((last_option_flag & FLAG_SET_DATA_FILE) == 0) {
		__err("Data file is not specified");
		return -1;
	}

	int64_t data_file_size;

	init_writer_object(&wo, vhd_file_name);
	if (get_last_error(&wo) == OPEN_FILE_ERROR) {
		__err("Open VHD image file error");
		return -1;
	}

	if (!vaild_vhd(&wo)) {
		__err("Invaild or broken VHD image file");
		release_writer_object(&wo);
		return -1;
	}

	if (!fixed_vhd(&wo)) {
		__err("The VHD image is not fixed which is still not support");
		release_writer_object(&wo);
		return -1;
	}

	size_vhd(&wo);

	/* read data from the data file, then write it to VHD image file */
	data_file_size = get_file_size_by_name(data_file_name);
	if (data_file_size == 0) {
		__err("Data file is invaild");
		release_writer_object(&wo);
		return -1;
	}

	int64_t total_written_bytes = write_hvd_sector_from_data_file(&wo, lba, data_file_name);
	enum writer_error err = get_last_error(&wo);
	if (err == LBA_OUT_OF_RANGE) {
		__err("LBA is out of range (0 - %d)", wo.size / 512 - 1);
		release_writer_object(&wo);
		return -1;
	}
	else if (err == OPEN_FILE_ERROR) {
		__err("Open data file error");
		release_writer_object(&wo);
		return -1;
	}

	fprintf(stdout, "Data: %s\nVHD: %s (offset LBA: %lld)\nTotal bytes to write: %lld\nTotal sectors to write: %lld\nTotal bytes written: %lld\nTotal sectors written: %lld\n",
		data_file_name, vhd_file_name, lba, data_file_size, data_file_size / 512 + (data_file_size % 512 != 0), total_written_bytes, total_written_bytes / 512 + (total_written_bytes % 512 != 0));

	if (total_written_bytes < data_file_size) {
		fprintf(stdout, "\n!!! Detected the tail of VHD image file, the writing data has been truncated!\n");
	}

	release_writer_object(&wo);
	return 0;
}
