#ifndef S7V30_FLASHER_H
  #define S7V30_FLASHER_H

// Сруктура Code Flash памяти кода
// 0x00000000...0x0000FFFF - стирание по 8192   байта, запись по 256 байт
// 0x00010000...0x003FFFFF - стирание по 32768  байта, запись по 256 байт

#define CODE_FLASH_START      0x00000000
#define CODE_FLASH_END        0x003FFFFF
#define CODE_FLASH_EBLOCK_SZ  32768          // Размер стираемого блока
#define CODE_FLASH_WR_SZ      256            // Размер записываемого блока

// Сруктура Data Flash памяти данных
// 0x40100000...0x4010FFFF - стирание по 64 байта, запись по 4 байта

#define DATA_FLASH_START      0x40100000
#define DATA_FLASH_END        0x4010FFFF
#define DATA_FLASH_EBLOCK_SZ  64          // Размер стираемого блока
#define DATA_FLASH_WR_SZ      4           // Размер записываемого блока



#define   BIT_FLASH_EVENT_ERASE_COMPLETE    BIT(0)
#define   BIT_FLASH_EVENT_WRITE_COMPLETE    BIT(1)
#define   BIT_FLASH_EVENT_BLANK             BIT(2)
#define   BIT_FLASH_EVENT_NOT_BLANK         BIT(3)
#define   BIT_FLASH_EVENT_ERR_DF_ACCESS     BIT(4)
#define   BIT_FLASH_EVENT_ERR_CF_ACCESS     BIT(5)
#define   BIT_FLASH_EVENT_ERR_CMD_LOCKED    BIT(6)
#define   BIT_FLASH_EVENT_ERR_FAILURE       BIT(7)
#define   BIT_FLASH_EVENT_ERR_ONE_BIT       BIT(8)

uint32_t             Flash_driver_init(void);
uint32_t             Flash_driver_deinit(void);
uint32_t             Flash_driver_bgo_init(void);
uint32_t             Flash_driver_bgo_deinit(void);
uint32_t             Get_bgo_status(void);
void                 Set_bgo_status(uint32_t stat);
uint32_t             Wait_bgo_end(ULONG wait_option);
uint32_t             DataFlash_bgo_EraseArea(uint32_t start_addr, uint32_t area_size);
uint32_t             DataFlash_bgo_WriteArea(uint32_t start_addr, uint8_t *buf, uint32_t buf_size);
uint32_t             DataFlash_bgo_ReadArea(uint32_t start_addr, uint8_t *buf, uint32_t buf_size);
flash_id_code_mode_t Get_flash_protection_command(uint8_t protection_byte, uint8_t prot_flag);
ssp_err_t            Set_Flash_protection(uint8_t const * const p_id_bytes, flash_id_code_mode_t mode);
uint32_t             Switch_Flash_driver_to_no_bgo(void);
uint32_t             Switch_Flash_driver_to_bgo(void);
uint32_t             Flash_blank_check(uint32_t const address, uint32_t const num_bytes, flash_result_t * const p_blank_check_result);
uint32_t             Flash_erase_block(uint32_t const address, uint32_t const num_blocks);
uint32_t             Flash_write_block(uint32_t const src_address, uint32_t const flash_address, uint32_t const num_bytes);
void                 Read_OSIS_to_buf(uint8_t *buf, uint32_t sz);
uint8_t              Is_flash_protection_disabled(void);
uint8_t              Is_flash_protection_type_3(void);
#endif



