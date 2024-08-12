#ifndef CRC_UTILS_H
  #define CRC_UTILS_H

uint16_t Get_CRC16_of_block(void *b, uint32_t len, uint16_t crc);
uint16_t CRC16_matlab(uint8_t *buf, uint32_t len);

#endif



