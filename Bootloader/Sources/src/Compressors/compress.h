#ifndef __COMPRESS_H
  #define __COMPRESS_H


  #include <stdlib.h>
  #include <stdio.h>
  #include <stdint.h>
  #include "tx_api.h"
  #include "fx_api.h"

typedef long T_compress_size;

struct T_compress_string
{
    unsigned char *pData;
    T_compress_size nPos;
    T_compress_size nLen;
};

typedef struct T_compress_string compress_string_t;


typedef enum
{
  COMPRESS_FILE_TYPE,
  COMPRESS_STRING_TYPE

} T_DATA_TYPE;

struct T_compress_data
{
    T_DATA_TYPE  eType;
    union
    {
        FX_FILE  *file;
        compress_string_t str;
    } pData;
};

typedef struct T_compress_data compress_data_t;


  #define COMPRESSION_SUCCESS          0
  #define COMPRESSION_FAIL            -1
  #define COMPRESSION_MEMORY          -2

  #define COMPR_ALG_NONE               0
  #define COMPR_ALG_SIXPACK            1
  #define COMPR_ALG_LZSS               2

  #define MAX_COMPRESSIBLE_BLOCK_SIZE       (1024*30ul)  // Максимальный размер сжимаемого блока данных.
// Если размер данных превышает эту величину то данные разбиваются на блоки размером равным или меньшим данной величине.
// Ограничение размера вызвано нехваткой памяти
  #define SPARE_AREA_SIZE                   (1024)

  #include "compress_io.h"
  #include "Lzss.h"
  #include "Sixpack.h"


uint32_t Compress_file_to_file(int alg, char *infname, char *outfname);
uint32_t Compress_file_to_file_by_handler(int alg, FX_FILE *infid, FX_FILE *outfid);
uint32_t Compress_mem_to_mem(uint8_t alg, void *in_buf, unsigned int in_buf_sz, void *out_buf, unsigned int *out_buf_sz);
uint32_t Decompress_file_to_file(int alg, char *infname, char *outfname);
uint32_t Decompress_file_to_mem(int alg, char *input_file_name, void *out_buf, unsigned int out_buf_sz);
int32_t  Decompress_mqtt_mem_to_mem(uint8_t alg, void *in_buf, unsigned int in_buf_sz, void *out_buf, unsigned int out_buf_sz);
int32_t  Decompress_mem_to_mem(uint8_t alg, void *in_buf, unsigned int in_buf_sz, void *out_buf, unsigned int out_buf_sz);

T_compress_size Compress_lzss(compress_data_t *pInput, compress_data_t *pOutput);
T_compress_size Uncompress_lzss(compress_data_t *pInput, compress_data_t *pOutput);
T_compress_size Compress_sixp(compress_data_t *pInput, compress_data_t *pOutput);
T_compress_size Uncompress_sixp(compress_data_t *pInput, compress_data_t *pOutput);

#endif
