#ifndef NET__H
  #define NET__H


#define ENABLE_NET_LOG

#ifdef ENABLE_NET_LOG
  #define NETLOG  NET_LOG
#else
  #define NETLOG(...)
#endif


  #include   "nx_secure_tls_api.h"
  #include   "nxd_dns.h"
  #include   "nxd_telnet_server.h"
  #include   "nxd_bsd.h"
  #include   "nxd_dhcp_client.h"
  #include   "nxd_dhcp_server.h"
  #include   "nxd_telnet_server.h"
  #include   "nxd_mqtt_client.h"
  #include   "nxd_sntp_client.h"
  #include   "nx_web_http_common.h"
  #include   "nx_web_http_server.h"
  #include   "nx_nat.h"

  #include   "whd.h"
  #include   "whd_types.h"
  #include   "whd_int.h"
  #include   "whd_wifi_api.h"
  #include   "whd_utils.h"
  #include   "whd_resource_api.h"
  #include   "whd_network_types.h"
  #include   "whd_wlioctl.h"
  #include   "whd_resource_api.h"
  #include   "whd_network_types.h"
  #include   "WHD_resource_man.h"
  #include   "WHD_buffer_man.h"

  #include   "cyhal_hw_types.h"

  #include   "USB_host_cdc_ecm.h"
  #include   "WiFi_NetX_adapter.h"
  #include   "WiFi_STA_network.h"
  #include   "WiFi_AP_network.h"
  #include   "USB_RNDIS_network.h"
  #include   "USB_ECM_Host_network.h"
  #include   "NXD_exFAT_ftp_server.h"

  #include   "Net_common.h"
  #include   "Net_utils.h"
  #include   "Net_thread.h"
  #include   "Net_DHCP_factory.h"
  #include   "Net_mDNS.h"
  #include   "Net_DNS.h"
  #include   "Net_MQTT_client_man.h"
  #include   "Net_MQTT_Msg_Controller.h"
  #include   "Net_FTP_server_man.h"
  #include   "Net_TCP_server.h"
  #include   "Net_FreeMaster.h"
  #ifdef ENABLE_MATLAB_CONNECTION
    #include "Net_MATLAB_connection.h"
  #endif
  #include   "Net_SNTP_client.h"
  #include   "WiFi_network.h"
  #include   "WiFi_debug.h"




#endif



