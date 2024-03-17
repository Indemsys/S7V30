#ifndef S7V30_SDMMC_UTILS_H
  #define S7V30_SDMMC_UTILS_H


  #define    SD_SET_PASSWORD   1
  #define    SD_UNLOCK         2
  #define    SD_CLEAR_PASSWORD 3


uint32_t   SD_card_open(void);
uint32_t   SD_card_close(void);
uint32_t   SD_get_password(char **buf, uint8_t *len);
uint8_t    SD_fill_lock_struct_with_password(T_Lock_Card_Data_Structure *p_lcds);
uint32_t   SD_get_card_locked_satus(uint8_t *p_lock_stat);
uint32_t   SD_password_operations(uint8_t operation, char *pass_buff, uint8_t pass_len);



#endif



