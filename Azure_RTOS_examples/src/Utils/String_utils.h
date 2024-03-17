#ifndef STRING_UTILS_H
  #define STRING_UTILS_H

uint8_t        BCD2ToBYTE(uint8_t val);
uint8_t        BYTEToBCD2(uint8_t val);
uint8_t        ascii_to_hex(uint8_t c);
uint8_t        hex_to_ascii(uint8_t c);
char*          Trim_and_dequote_str(char *str);
int            Read_cstring_from_buf(char **buf, char *str, uint32_t len);
uint8_t*       Isolate_string_in_buf(uint8_t **buf, uint32_t *buf_len);


#endif



