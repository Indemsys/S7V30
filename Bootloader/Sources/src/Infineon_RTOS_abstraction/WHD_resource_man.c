// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-05-12
// 15:05:56
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "app.h"

#include   "whd.h"
#include   "whd_resource_api.h"
#include   "wifi_nvram_image.h"
#include   "WHD_resource_man.h"

#define WHR_RESOURCE_FILE_BLOCK_SZ              (8*1024)

#ifdef WLAN_FIRMWARE_IN_FLASH
static uint8_t *firmware_buf_ptr;
static uint8_t *clm_buf_ptr;
static uint32_t firmware_buf_size;
static uint32_t clm_buf_size;
static T_infineon_blob const *firmware_blob;
static T_infineon_blob const *clm_blob;

const T_infineon_blob* _Find_infineon_blob(const char *name)
{
  T_infineon_blob const *blobs;

  blobs = (T_infineon_blob const*)INFINEON_BLOBS_ADDR;
  // Ищем в массиве патчей нужный файл
  for (uint32_t i = 0; i < INFINEON_BLOBS_CNT; i++)
  {
    if (strcmp(blobs->name, name) == 0)
    {
      return blobs;
    }
    blobs++;
  }
  return NULL;
}

#elif  WLAN_FIRMWARE_IN_FILE_SYSTEM
static FX_FILE                                         *firware_file_ptr;
static FX_FILE                                         *clm_file_ptr;
static uint8_t                                         *resource_data_block;
#endif

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_open_resource_files(void)
{
  #ifdef WLAN_FIRMWARE_IN_FLASH

  *firmware_buf_ptr = NULL;
  *clm_buf_ptr     = NULL;
  firmware_buf_size = 0;
  clm_buf_size     = 0;
  firmware_blob    = _Find_infineon_blob(WLAN_FIRMWARE_NAME);
  clm_blob         = _Find_infineon_blob(WLAN_CLM_NAME);

  if ((firmware_blob == NULL) || (clm_blob == NULL)) return RES_ERROR;

    #ifdef INFINEON_BLOBS_COMPRESSED

  memcpy(&firmware_buf_size, firmware_blob->data, 4);
  firmware_buf_ptr = SDRAM_malloc(firmware_buf_size);

  memcpy(&clm_buf_size, clm_blob->data, 4);
  clm_buf_ptr = SDRAM_malloc(clm_buf_size);

  if ((firmware_buf_ptr == NULL) || (clm_buf_ptr == NULL)) goto error;

  if (Decompress_mem_to_mem(COMPR_ALG_SIXPACK, (void *)firmware_blob->data, firmware_blob->size, firmware_buf_ptr, firmware_buf_size) < 0) goto error;
  if (Decompress_mem_to_mem(COMPR_ALG_SIXPACK, (void *)clm_blob->data, clm_blob->size, clm_buf_ptr, clm_buf_size) < 0) goto error;


  return RES_OK;

error:
  SDRAM_free(firmware_buf_ptr);
  SDRAM_free(clm_buf_ptr);

  return RES_ERROR;



    #else

  firmware_buf_ptr  = (uint8_t*)firmware_blob->data;
  firmware_buf_size = firmware_blob->size;
  clm_buf_ptr      = (uint8_t*)clm_blob->data;
  clm_buf_size     = clm_blob->size;
  return RES_OK;
    #endif


  #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
  int32_t      res;

  resource_data_block = NULL;
  firware_file_ptr    = NULL;
  clm_file_ptr        = NULL;


  resource_data_block = App_malloc(WHR_RESOURCE_FILE_BLOCK_SZ);
  if (resource_data_block == NULL) goto error;

  firware_file_ptr = App_malloc(sizeof(FX_FILE));
  if (firware_file_ptr == NULL) goto error;

  clm_file_ptr = App_malloc(sizeof(FX_FILE));
  if (clm_file_ptr == NULL) goto error;

  res = fx_directory_default_set(&fat_fs_media, "/");
  if (res != FX_SUCCESS) goto error;

  res = fx_file_open(&fat_fs_media, firware_file_ptr, WLAN_FIRMWARE_FILE_NAME,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS) goto error;

  res = fx_file_open(&fat_fs_media, clm_file_ptr, WLAN_CLM_FILE_NAME,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS)
  {
    fx_file_close(clm_file_ptr);
    goto error;
  }

  return RES_OK;
error:

  App_free(firware_file_ptr);
  App_free(clm_file_ptr);
  App_free(resource_data_block);
  return RES_ERROR;
  #else
  return RES_OK;
  #endif
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_close_resource_files(void)
{
  #ifdef WLAN_FIRMWARE_IN_FLASH

  SDRAM_free(firmware_buf_ptr);
  SDRAM_free(clm_buf_ptr);

  #elif WLAN_FIRMWARE_IN_FILE_SYSTEM

  fx_file_close(firware_file_ptr);
  fx_file_close(clm_file_ptr);
  App_free(firware_file_ptr);
  App_free(clm_file_ptr);
  App_free(resource_data_block);

  #endif
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param resource
  \param size_out

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_resource_size(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t *size_out)
{


  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    *size_out = firmware_buf_size;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
    *size_out = firware_file_ptr->fx_file_current_file_size;
    #else
    *size_out = sizeof(wifi_firmware_image_data);
    #endif
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    *size_out = sizeof(wifi_nvram_image);
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    *size_out = clm_buf_size;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
    *size_out = clm_file_ptr->fx_file_current_file_size;
    #else
    *size_out = sizeof(wifi_firmware_clm_blob_data);
    #endif
    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param type
  \param block_count

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_get_resource_no_of_blocks(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t *block_count)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    *block_count = 1;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
    {
      uint32_t sz  = firware_file_ptr->fx_file_current_file_size;
      uint32_t n   = sz / WHR_RESOURCE_FILE_BLOCK_SZ;
      uint32_t rem = sz % WHR_RESOURCE_FILE_BLOCK_SZ;
      if (rem != 0) n++;
      *block_count = n;
    }
    #else
    *block_count = 1;
    #endif

    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    *block_count = 1;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    *block_count = 1;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
    {
      uint32_t sz  = clm_file_ptr->fx_file_current_file_size;
      uint32_t n   = sz / WHR_RESOURCE_FILE_BLOCK_SZ;
      uint32_t rem = sz % WHR_RESOURCE_FILE_BLOCK_SZ;
      if (rem != 0) n++;
      *block_count = n;
    }
    #else
    *block_count = 1;
    #endif
    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;

  }
}

/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param type
  \param size_out

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_get_resource_block_size(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t *size_out)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    *size_out = firmware_buf_size;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
    *size_out =  WHR_RESOURCE_FILE_BLOCK_SZ;
    #else
    *size_out = sizeof(wifi_firmware_image_data);
    #endif

    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    *size_out = sizeof(wifi_nvram_image);
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    *size_out = clm_buf_size;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
    *size_out =  WHR_RESOURCE_FILE_BLOCK_SZ;
    #else
    *size_out = sizeof(wifi_firmware_clm_blob_data);
    #endif

    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param type
  \param blockno
  \param data
  \param size_out

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_get_resource_block(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t blockno, const uint8_t **data, uint32_t *size_out)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    *size_out = firmware_buf_size;
    *data = firmware_buf_ptr;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
    {
      int32_t      res;
      ULONG        actual_size;
      res = fx_file_read(firware_file_ptr,resource_data_block, WHR_RESOURCE_FILE_BLOCK_SZ,&actual_size);
      if (res != FX_SUCCESS) return WHD_HAL_ERROR;
      *size_out = actual_size;
      *data     = resource_data_block;
    }
    #else
    *size_out = sizeof(wifi_firmware_image_data);
    *data = wifi_firmware_image_data;
    #endif

    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    *size_out = sizeof(wifi_nvram_image);
    *data = (uint8_t *)wifi_nvram_image;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    *size_out = clm_buf_size;
    *data = clm_buf_ptr;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM
    {
      int32_t      res;
      ULONG        actual_size;
      res = fx_file_read(clm_file_ptr,resource_data_block, WHR_RESOURCE_FILE_BLOCK_SZ,&actual_size);
      if (res != FX_SUCCESS) return WHD_HAL_ERROR;
      *size_out = actual_size;
      *data     = resource_data_block;
    }
    #else
    *size_out = sizeof(wifi_firmware_clm_blob_data);
    *data = wifi_firmware_clm_blob_data;
    #endif

    return WHD_SUCCESS;


  default:
    return WHD_HAL_ERROR;

  }
}

/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param type
  \param offset
  \param size
  \param size_out
  \param buffer

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_resource_read(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t offset, uint32_t size, uint32_t *size_out, void *buffer)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    memcpy(buffer, firmware_buf_ptr + offset,  size);
    *size_out = size;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM

    #else
    memcpy(buffer, wifi_firmware_image_data + offset,  size);
    *size_out = size;
    #endif
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    memcpy(buffer, wifi_nvram_image + offset,  size);
    *size_out = size;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    #ifdef WLAN_FIRMWARE_IN_FLASH
    memcpy(buffer, clm_buf_ptr + offset,  size);
    *size_out = size;
    #elif WLAN_FIRMWARE_IN_FILE_SYSTEM

    #else
    memcpy(buffer, wifi_firmware_clm_blob_data + offset,  size);
    *size_out = size;
    #endif
    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;
  }
}


