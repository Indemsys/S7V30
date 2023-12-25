#ifndef NET_THREAD_H
  #define NET_THREAD_H

  #define EVT_NET_IP_READY    BIT(0)
  #define EVT_MQTT_MSG        BIT(1)


typedef enum
{
  NET_NONE,
  NET_BY_RNDIS,
  NET_BY_ECM,
  NET_BY_WIFI_AP,
  NET_BY_WIFI_STA,
}
T_network_type;

extern T_network_type       g_network_type;
extern uint8_t              g_BSD_initialised;

void       Thread_Net_create(void);
UINT       Send_Net_task_event(uint32_t event_flag);
uint32_t   Wait_Net_task_event(uint32_t event_flags, uint32_t timeout_ms);
void       Determine_network_type(void);
NX_IP     *Get_net_ip(void);
uint32_t   Get_device_ip_addr(char *ip_addr_str, uint32_t ip_addr_str_len );
uint32_t   Get_device_MAC_addr(char *mac_addr_str, uint32_t mac_addr_str_len);
uint32_t   Get_connected_SSID(char *ssid_str, uint32_t ssid_str_len);
#endif



