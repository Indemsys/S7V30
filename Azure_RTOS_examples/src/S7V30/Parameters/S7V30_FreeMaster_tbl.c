#ifndef __FREEMASTER_IVAR_H
  #define __FREEMASTER_IVAR_H
#include "S7V30.h"
#include "freemaster_tsa.h"

FMSTR_TSA_TABLE_BEGIN(ivar_tbl)
FMSTR_TSA_RW_VAR( ivar.ap_enable_dhcp_server           ,FMSTR_TSA_UINT8     ) // Enable DHCP server (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.en_compress_settins             ,FMSTR_TSA_UINT8     ) // Enable compress settings file (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.en_dhcp_client                  ,FMSTR_TSA_UINT8     ) // Enable DHCP client (0-No, 1-Yes) | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.en_formated_settings            ,FMSTR_TSA_UINT8     ) // Enable formating in  settings file (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.en_freemaster                   ,FMSTR_TSA_UINT8     ) // Enable FreeMaster protocol | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.en_log_to_file                  ,FMSTR_TSA_UINT8     ) // Enable logging to file (1-yes, 0-no) | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.en_log_to_freemaster            ,FMSTR_TSA_UINT8     ) // Enable logging to FreeMaster pipe | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.en_matlab                       ,FMSTR_TSA_UINT8     ) // Enable MATLAB communication server | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.en_sntp                         ,FMSTR_TSA_UINT8     ) // Enable SNTP client | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.en_sntp_time_receiving          ,FMSTR_TSA_UINT8     ) // Allow to receive time from time servers | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.en_telnet                       ,FMSTR_TSA_UINT8     ) // Enable Telnet (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.en_wifi_ap                      ,FMSTR_TSA_UINT8     ) // Enable Accces Point mode (1-yes, 0-no) | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.enable_ftp_server               ,FMSTR_TSA_UINT8     ) // Enable FTP server | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.mqtt_enable                     ,FMSTR_TSA_UINT8     ) // Enable MQTT client  | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.mqtt_server_port                ,FMSTR_TSA_UINT32    ) // MQTT server port number | def.val.= 1883
FMSTR_TSA_RW_VAR( ivar.rndis_ip_addr_assign_method     ,FMSTR_TSA_UINT8     ) // RNDIS IP address assignment method (0-Win home net, 1 - DHCP server) | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.sntp_poll_interval              ,FMSTR_TSA_UINT32    ) // Poll interval (s) | def.val.= 120
FMSTR_TSA_RW_VAR( ivar.usb_mode                        ,FMSTR_TSA_UINT32    ) // USB mode(1-VCOM, 2-MSD, 3-VCOM&MSD, 4-VCOM&FMST, 5-RNDIS, 6-Host ECM) | def.val.= 4
FMSTR_TSA_RW_VAR( ivar.usd_dev_interface               ,FMSTR_TSA_UINT8     ) // USB device interface (0- HS, 1- FS) | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.utc_offset                      ,FMSTR_TSA_FLOAT     ) // UTC offset (difference in hours +-) | def.val.= 3
FMSTR_TSA_RW_VAR( ivar.wifi_ap_addr_assign_method      ,FMSTR_TSA_UINT8     ) // IP address assignment method  (0-Win home net, 1 - DHCP server) | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.wifi_ap_channel                 ,FMSTR_TSA_UINT8     ) // WIFI channell | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.wifi_sta_cfg1_en                ,FMSTR_TSA_UINT8     ) // Enable configuration (1-yes, 0-no) | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.wifi_sta_cfg1_en_dhcp           ,FMSTR_TSA_UINT8     ) // Enable DHCP client (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( ivar.wifi_sta_cfg2_en                ,FMSTR_TSA_UINT8     ) // Enable configuration (1-yes, 0-no) | def.val.= 0
FMSTR_TSA_RW_VAR( ivar.wifi_sta_cfg2_en_dhcp           ,FMSTR_TSA_UINT8     ) // Enable DHCP client (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_TABLE_END();


#endif
