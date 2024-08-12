#ifndef _COMPRESSED_WEB_FILES_H
  #define _COMPRESSED_WEB_FILES_H

typedef struct {
    const char* name;
    const uint8_t* data;
    uint32_t size;
} T_compressed_file_rec ;

#define COMPRESSED_WEB_FILES_NUM 8

extern const T_compressed_file_rec compressed_web_files[COMPRESSED_WEB_FILES_NUM];

#endif
