#ifndef WEB_MODULE_CONTROL_H
  #define WEB_MODULE_CONTROL_H

uint32_t HTTP_POST_send_device_info(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_send_device_status(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_upload_time(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_Reset_device(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_Creal_Gen_Log_File(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_Creal_Net_Log_File(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_Save_module_params(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_Get_module_params(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_Start_WIFI_scan(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_Send_WIFI_scan_results(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);

#endif



