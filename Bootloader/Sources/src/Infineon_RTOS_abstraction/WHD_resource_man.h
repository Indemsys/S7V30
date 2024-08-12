#ifndef WHD_RESOURCE_MAN_H
  #define WHD_RESOURCE_MAN_H


//#define WLAN_FIRMWARE_IN_FILE_SYSTEM  // Указываем на то что фирваре для WiFi модуля находится в файлах
//#define WLAN_FIRMWARE_FILE_NAME  "WHD/4373A0.bin"        // Путь и имя файла в файловой системе
//#define WLAN_CLM_FILE_NAME       "WHD/4373A0.clm_blob"   //


#define WLAN_FIRMWARE_IN_FLASH     // Указываем на то что фирваре для WiFi модуля находится во Flash
#define WLAN_FIRMWARE_NAME  "4373A0.bin"        // Имя в списке
#define WLAN_CLM_NAME       "4373A0.clm_blob"   //


uint32_t     WHD_open_resource_files(void);
uint32_t     WHD_close_resource_files(void);
uint32_t     WHD_resource_size(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t *size_out);
uint32_t     WHD_get_resource_block(whd_driver_t whd_drv, whd_resource_type_t type, uint32_t blockno, const uint8_t **data, uint32_t *size_out);
uint32_t     WHD_get_resource_no_of_blocks(whd_driver_t whd_drv, whd_resource_type_t type, uint32_t *block_count);
uint32_t     WHD_get_resource_block_size(whd_driver_t whd_drv, whd_resource_type_t type, uint32_t *size_out);
uint32_t     WHD_resource_read(whd_driver_t whd_drv, whd_resource_type_t type, uint32_t offset, uint32_t size, uint32_t *size_out, void *buffer);



#endif



