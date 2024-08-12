#ifndef WIFI_MODULE_CONTROL_H
  #define WIFI_MODULE_CONTROL_H


#define     NET_EVT_WIFI_AP_READY           BIT(0)
#define     NET_EVT_WIFI_STA_DISJOINED      BIT(1)
#define     NET_EVT_WIFI_INIT_DONE          BIT(2)
#define     NET_EVT_START_WIFI_SCAN         BIT(3)



void        WIFi_control_init(void);
void        WiFi_module_switch_ON(void);
void        WiFi_module_switch_OFF(void);
void        WiFi_module_switch_on_Bluetooth(void);
UINT        Send_wifi_event(uint32_t event_flag);
uint32_t    Wait_wifi_event(uint32_t event_flags, uint32_t *actual_flags, uint32_t timeout_ms);


#endif



