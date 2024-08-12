#ifndef __LOADER_H
  #define __LOADER_H



  #define  FIRMWARE_FLASHED_SUCCESFULLY      1
  #define  DIGEST_CHECK_ERROR                2
  #define  DECRYPT_IMAGE_DATA_HEADER_ERROR   3
  #define  DECRYPT_IMAGE_SIGN_ERROR          4
  #define  IMAGE_SIZE_ERROR                  5
  #define  FIRMWARE_FILE_READ_ERROR          6
  #define  FIRMWARE_FILE_HEADER_CRC_ERROR    7
  #define  IMAGE_DATA_HEADER_READ_ERROR      8
  #define  INCORECT_FIRMWARE_START_ADDRESS   9
  #define  FIRMWARE_FLASHING_FAIL            10
  #define  FIRMWARE_FILE_NOT_FOUND           11
  #define  LOADING_FIRMWARE_UNKNOWN_ERROR    12

typedef struct
{
    uint32_t  flasher_err;
    uint32_t  load_duration_us;
    uint32_t  protection_err;
    uint8_t   protection_type;
    uint32_t  settings_err;
}
T_loader_cbl;

extern T_loader_cbl         loader_cbl;


void     Jump_to_app(void);
void     Generate_CRC32_table(void);
uint32_t Get_CRC32(unsigned char *buf, uint32_t len);
void     Get_Init_CRC32(uint32_t *p_crc);
void     Update_CRC32(uint32_t *p_crc, uint8_t *buf, uint32_t len);
void     Get_finish_CRC32(uint32_t *p_crc);
uint32_t Check_boot_MassStorage_mode(void);
void     Clear_boot_MassStorage_mode(void);
void     Set_boot_MassStorage_mode(void);
uint32_t Load_and_Flash_Image_File(void);
uint32_t Integrity_check_App_firmware(void);
void     Restart_to_App_firmware(void);
uint32_t Erase_firmware_area(void);
uint32_t Clear_app_DataFlash(void);
void     Auto_protection(void);
#endif

