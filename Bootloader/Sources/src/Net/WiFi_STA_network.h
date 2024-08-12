#ifndef WIFI_STA_NETWORK_H
  #define WIFI_STA_NETWORK_H

#define        WIFI_STA_CFG_NUM    2

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
uint32_t        WIFI_STA_accept_connection_to_AP();
uint32_t        WIFI_STA_accept_disconnection_from_AP();
uint32_t        WIFI_STA_disjoint_time_sec(void);
NX_INTERFACE   *WIFI_STA_link_state(void);
char const     *WIFI_STA_get_def_ip(void);
char const     *WIFI_STA_get_def_mask(void);
void            WIFI_STA_save_remote_ap_addr(whd_mac_t *addr_ptr);
whd_mac_t      *WIFI_STA_get_remote_ap_addr(void);

#endif



