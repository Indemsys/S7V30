#ifndef WIFI_NETWORK_H
  #define WIFI_NETWORK_H

#define                 WIFI_AP_IP_STACK_SIZE             2048
#define                 WIFI_STA_IP_STACK_SIZE            2048


#define     WIFI_ONLY_STA     0
#define     WIFI_ONLY_AP      1
#define     WIFI_STA_AND_AP   2

#define     WIFI_STA_CFG_NUM  2

extern uint8_t               wifi_composition;

extern  NX_IP               *wifi_sta_ip_ptr;
extern  NX_IP               *wifi_ap_ip_ptr;
extern  T_app_net_props      wifi_ap_net_props;
extern  NX_TELNET_SERVER     telnet_server;

uint32_t                     WIFI_init_network_stack(NX_IP **ip_p_ptr);
uint8_t                      WiFi_module_initialized_flag(void);
uint8_t                      WIFI_STA_network_active_flag(void);
uint8_t                      WIFI_AP_network_active_flag(void);
uint32_t                     WIFI_STA_enabled_flag(void);
uint32_t                     WIFI_AP_init_TCP_stack(void);
uint32_t                     WIFI_STA_init_TCP_stack(void);
void                         WIFI_STA_network_controller(void);
void                         WIFI_AP_network_controller(void);

#endif



