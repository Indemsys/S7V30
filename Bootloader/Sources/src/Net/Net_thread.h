#ifndef NET_THREAD_H
  #define NET_THREAD_H

#define  WIFI_AP_INTF_NUM             1
#define  WIFI_STA_INTF_NUM            2
#define  NET_USB_INTF_NUM             3


#define  NET_FLG_RNDIS_STOP        BIT(0)
#define  NET_FLG_RNDIS_START       BIT(1)
#define  NET_FLG_STA_CONNECTED     BIT(2) // Произошло подключение данного устройства к удаленной точке доступа
#define  NET_FLG_STA_DISCONNECTED  BIT(3) // Произошло отключение данного устройства от удаленной точки доступа
#define  NET_FLG_AP_CONNECTED      BIT(4) // Произошло подключение к данному устройству  удаленной станции
#define  NET_FLG_AP_DISCONNECTED   BIT(5) // Произошло отключение от данного устройства  удаленной точки станции
#define  NET_FLG_ECM_HOST_START    BIT(6)
#define  NET_FLG_ECM_HOST_STOP     BIT(7)

#define  NET_EVT_MQTT_MSG          BIT(8)

uint32_t    Thread_Net_create(void);
uint32_t    Send_net_event(uint32_t event_flag);
uint32_t    Wait_net_event(uint32_t event_flags, uint32_t *actual_flags, uint32_t timeout_ms);
NX_IP      *Net_get_ip_ptr(void);
#endif



