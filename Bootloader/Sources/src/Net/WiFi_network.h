#ifndef WIFI_NETWORK_H
  #define WIFI_NETWORK_H

#define  MAX_SCAN_RESULTS_RECORDS  50

typedef struct
{
    whd_scan_result_t          scan_result;
    whd_scan_result_t         *scan_results;
    uint32_t                   scan_results_num;
    uint8_t                    scan_in_busy;

} T_wifi_scan_cbl;


extern whd_interface_t       g_prim_whd_intf_ptr;
extern whd_interface_t       g_secd_whd_intf_ptr;

uint32_t                     WIFI_init_module(void);
uint32_t                     Thread_WiFi_manager_create(void);
T_wifi_scan_cbl*             WIFI_get_scan_cbl(void);
#endif



