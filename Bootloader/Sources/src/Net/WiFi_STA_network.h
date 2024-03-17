#ifndef WIFI_STA_NETWORK_H
  #define WIFI_STA_NETWORK_H


typedef struct
{
  uint8_t    *enable_cfg;
  uint8_t    *enable_dhcp;
  uint8_t    *pass;
  uint8_t    *ssid;
  uint8_t    *ip;
  uint8_t    *mask;
  uint8_t    *gate;
} T_WIFI_STA_Config;


uint32_t        WIFI_STA_Join(void);
char*           WIFI_STA_get_current_SSID(void);


#endif



