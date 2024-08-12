#ifndef SD_CARD_CONTROL_H
  #define SD_CARD_CONTROL_H


void      Do_SD_card_control(uint8_t keycode);
uint32_t  Get_card_csd_text(sdmmc_priv_csd_reg_t *p_csd_reg, char *str_buf, uint32_t maxsz);

#endif // SD_CARD_CONTROL_H



