#ifndef S7V30_SDRAM_MEM_POOL_H
  #define S7V30_SDRAM_MEM_POOL_H


uint32_t  SDRAM_pool_create(void);
void*     SDRAM_malloc_pending(ULONG size, ULONG wait_option);
void*     SDRAM_malloc(size_t size);
void      SDRAM_free(VOID *block_ptr);
void*     SDRAM_calloc(size_t num, size_t size);
void*     SDRAM_malloc_named(char const *name, size_t size);
void*     SDRAM_calloc_named(char const *name, size_t num, size_t size);
uint32_t  Get_SDRAM_pool_size(void);
void      App_get_SDRAM_pool_statistic(uint32_t *avail_bytes, uint32_t *fragments);


#endif // SDRAM_MEM_POOL_H



