#ifndef STRING_UTILS_H
  #define STRING_UTILS_H

uint8_t        BCD2ToBYTE(uint8_t val);
uint8_t        BYTEToBCD2(uint8_t val);
uint8_t        ascii_to_hex(uint8_t c);
uint8_t        hex_to_ascii(uint8_t c);
char*          Trim_and_dequote_str(char *str);
int            Read_cstring_from_buf(char **buf, char *str, uint32_t len);
uint8_t*       Isolate_string_in_buf(uint8_t **buf, uint32_t *buf_len);
void           Get_nums_from_date(const char *date_str, uint32_t *pmonts, uint32_t *pday, uint32_t *pyear);
void           Get_nums_from_time(const char *time_str, uint32_t *hours, uint32_t *mins, uint32_t *secs);
void           Get_build_date_time(char *ver_str);
const char    *Get_build_date(void);
const char    *Get_build_time(void);

#endif



