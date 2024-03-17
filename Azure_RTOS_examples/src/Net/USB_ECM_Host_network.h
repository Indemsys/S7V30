﻿#ifndef USB_ECM_HOST_LAN_H
  #define USB_ECM_HOST_LAN_H


//#define ENABLE_USB_ECM_DEBUG_LOG



#ifdef ENABLE_USB_ECM_DEBUG_LOG
  #define ECM_DEBUG_LOG(...)  RTT_printf_str(0, ##__VA_ARGS__ )
#else
  #define ECM_DEBUG_LOG(...)
#endif



uint8_t  ECM_Host_network_active_flag(void);
uint32_t ECM_Host_init_network_stack(NX_IP **ip_p_ptr);
void     ECM_Host_network_controller(void);
uint32_t ECM_DHCP_client_start(void);

#endif // USB_ECM_HOST_LAN_H



