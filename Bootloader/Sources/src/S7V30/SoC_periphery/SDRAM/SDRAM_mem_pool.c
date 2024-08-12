// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-04-01
// 12:04:57
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


#pragma section="FREE_SDRAM_MEM"
__root void * __free_sdram_start @ "FREE_SDRAM_MEM";

uint8_t         *g_sdram_pool_memory;
uint32_t         g_sdram_pool_size;
TX_BYTE_POOL     g_sdram_pool;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t  SDRAM_pool_create(void)
{
  g_sdram_pool_memory = (uint8_t*)&__free_sdram_start;
  g_sdram_pool_size   = SDRAM_END - (uint32_t)g_sdram_pool_memory + 1;

  return tx_byte_pool_create(&g_sdram_pool,"sdram_mem", g_sdram_pool_memory,g_sdram_pool_size);
}


/*-----------------------------------------------------------------------------------------------------


  \param memory_size
  \param wait_option

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* SDRAM_malloc_pending(ULONG size, ULONG wait_option)
{
  void *ptr;
  if (tx_byte_allocate(&g_sdram_pool,&ptr, size,wait_option) != TX_SUCCESS) return NULL;
  memset(ptr, 0, size);
  return ptr;
}

/*-----------------------------------------------------------------------------------------------------
  Выделение блока памяти
  Блоки выделяются с выравниванием равным ALIGN_TYPE. Макрос ALIGN_TYPE определен в tx_api.h и равен ULONG

  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* SDRAM_malloc(size_t size)
{
  return SDRAM_malloc_pending(size, TX_NO_WAIT);
}


/*-----------------------------------------------------------------------------------------------------

  \param block_ptr  - Pointer to the previously allocated memory block.

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void SDRAM_free(VOID *block_ptr)
{
  tx_byte_release(block_ptr);
}

/*-----------------------------------------------------------------------------------------------------
    Функция используется в WICED

  \param num
  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* SDRAM_calloc(size_t num, size_t size)
{
  void    *ptr;
  uint32_t res;
  res =  tx_byte_allocate(&g_sdram_pool,&ptr, num * size, TX_NO_WAIT);
  if (res != TX_SUCCESS) return NULL;
  return ptr;
}

/*-----------------------------------------------------------------------------------------------------
  Функция используется в WICED

  \param name
  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void*  SDRAM_malloc_named(char const *name, size_t size)
{
  return SDRAM_malloc(size);
}

/*-----------------------------------------------------------------------------------------------------
  Функция используется в WICED

  \param name
  \param num
  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void*  SDRAM_calloc_named(char const *name, size_t num, size_t size)
{
  return SDRAM_calloc(num,size);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_SDRAM_pool_size(void)
{
  return g_sdram_pool_size;
}


/*-----------------------------------------------------------------------------------------------------


  \param avail_bytes
  \param fragments
-----------------------------------------------------------------------------------------------------*/
void App_get_SDRAM_pool_statistic(uint32_t *avail_bytes, uint32_t *fragments)
{
  CHAR           *pool_name;
  TX_THREAD      *first_suspended;
  ULONG           suspended_count;
  TX_BYTE_POOL   *next_pool;

  tx_byte_pool_info_get(&g_sdram_pool,&pool_name,(ULONG *)avail_bytes,(ULONG *)fragments,&first_suspended,&suspended_count,&next_pool);
}

