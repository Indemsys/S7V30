#ifndef _INFINEON_BLOBS_H
  #define _INFINEON_BLOBS_H

typedef struct {
    const char* name;
    const uint8_t* data;
    uint32_t size;
} T_infineon_blob;

#define INFINEON_BLOBS_ADDR        0x700
#define INFINEON_BLOBS_COMPRESSED  1
#define INFINEON_BLOBS_CNT         3

extern const T_infineon_blob infineon_blobs[INFINEON_BLOBS_CNT];

const T_infineon_blob* _Find_infineon_blob(const char *name);

#endif
