#ifndef WIFI_NETX_ADAPTER_H
  #define WIFI_NETX_ADAPTER_H

VOID         WHD_STA_NetXDuo_driver_entry( NX_IP_DRIVER* driver );
VOID         WHD_AP_NetXDuo_driver_entry( NX_IP_DRIVER* driver );
void         WHD_network_process_ethernet_data(whd_interface_t ifp, whd_buffer_t buffer);


#endif // WIFI_NETX_ADAPTER_H



