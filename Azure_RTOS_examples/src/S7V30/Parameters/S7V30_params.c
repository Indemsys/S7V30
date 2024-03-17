﻿#include "S7V30.h"
#include "freemaster_tsa.h"

#define  IVAR_SIZE        59
#define  PARMNU_ITEM_NUM   15

#define  SELECTORS_NUM     7

IVAR_TYPE  ivar;


static const T_parmenu parmenu[PARMNU_ITEM_NUM]=
{
{ S7V30_WIFI_STA              , S7V30_WIFI_STA_COMMON       , "WiFi Station Common settings", "                    ", -1   }, // 
{ S7V30_WIFI_STA              , S7V30_WIFI_STA_CFG1         , "WiFi Station Configuration 1", "                    ", -1   }, // 
{ S7V30_0                     , S7V30_main                  , "Parameters and settings    ", "PARAMETERS          ", -1   }, // Основная категория
{ S7V30_WIFI_STA              , S7V30_WIFI_STA_CFG2         , "WiFi Station Configuration 2 ", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_General               , "General settings           ", "GENERAL_SETTINGS    ", -1   }, // 
{ S7V30_main                  , S7V30_USB_Interface         , "USB Interface settings     ", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_Network               , "Network settings           ", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_WIFI_STA              , "WiFi Station interface settings", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_WIFI_AP               , "WiFi Access Point interface settings", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_Telnet                , "Telnet settings            ", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_FTP_server            , "FTP server settings        ", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_MQTT                  , "MQTT settings              ", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_SNTP                  , "Net time protocol setting  ", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_MATLAB                , "MATLAB communication settings", "                    ", -1   }, // 
{ S7V30_main                  , S7V30_FreeMaster            , "FreeMaster communication settings", "                    ", -1   }, // 
};


static const T_NV_parameters arr_ivar[IVAR_SIZE]=
{
// N: 0
  {
    "en_freemaster",
    "Enable FreeMaster protocol",
    "-",
    (void*)&ivar.en_freemaster,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_FreeMaster,
    "",
    "%d",
    0,
    sizeof(ivar.en_freemaster),
    0,
    1,
  },
// N: 1
  {
    "en_log_to_freemaster",
    "Enable logging to FreeMaster pipe",
    "-",
    (void*)&ivar.en_log_to_freemaster,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_FreeMaster,
    "",
    "%d",
    0,
    sizeof(ivar.en_log_to_freemaster),
    0,
    1,
  },
// N: 2
  {
    "enable_ftp_server",
    "Enable FTP server",
    "-",
    (void*)&ivar.enable_ftp_server,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_FTP_server,
    "",
    "%d",
    0,
    sizeof(ivar.enable_ftp_server),
    1,
    1,
  },
// N: 3
  {
    "ftp_serv_login",
    "Login",
    "-",
    (void*)&ivar.ftp_serv_login,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_FTP_server,
    "ftp_login",
    "%s",
    0,
    sizeof(ivar.ftp_serv_login)-1,
    2,
    0,
  },
// N: 4
  {
    "ftp_serv_password",
    "Password ",
    "-",
    (void*)&ivar.ftp_serv_password,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_FTP_server,
    "ftp_pass",
    "%s",
    0,
    sizeof(ivar.ftp_serv_password)-1,
    3,
    0,
  },
// N: 5
  {
    "name",
    "Product  name",
    "SYSNAM",
    (void*)&ivar.name,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_General,
    "S7V30",
    "%s",
    0,
    sizeof(ivar.name)-1,
    1,
    0,
  },
// N: 6
  {
    "software_version",
    "Software version",
    "-",
    (void*)&ivar.software_version,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_General,
    "S7V30",
    "%s",
    0,
    sizeof(ivar.software_version)-1,
    2,
    0,
  },
// N: 7
  {
    "hardware_version",
    "Hardware version",
    "-",
    (void*)&ivar.hardware_version,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_General,
    "S7V30 ",
    "%s",
    0,
    sizeof(ivar.hardware_version)-1,
    3,
    0,
  },
// N: 8
  {
    "manuf_date",
    "Manufacturing date",
    "FRMVER",
    (void*)&ivar.manuf_date,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_General,
    "2023 04 25",
    "%s",
    0,
    sizeof(ivar.manuf_date)-1,
    4,
    0,
  },
// N: 9
  {
    "en_log_to_file",
    "Enable logging to file (1-yes, 0-no)",
    "-",
    (void*)&ivar.en_log_to_file,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_General,
    "",
    "%d",
    0,
    sizeof(ivar.en_log_to_file),
    5,
    1,
  },
// N: 10
  {
    "en_compress_settins",
    "Enable compress settings file (1-yes, 0-no)",
    "-",
    (void*)&ivar.en_compress_settins,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_General,
    "",
    "%d",
    0,
    sizeof(ivar.en_compress_settins),
    6,
    1,
  },
// N: 11
  {
    "en_formating_settings",
    "Enable formating in  settings file (1-yes, 0-no)",
    "-",
    (void*)&ivar.en_formated_settings,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_General,
    "",
    "%d",
    0,
    sizeof(ivar.en_formated_settings),
    7,
    1,
  },
// N: 12
  {
    "en_matlab",
    "Enable MATLAB communication server",
    "-",
    (void*)&ivar.en_matlab,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_MATLAB,
    "",
    "%d",
    0,
    sizeof(ivar.en_matlab),
    1,
    1,
  },
// N: 13
  {
    "mqtt_enable",
    "Enable MQTT client ",
    "-",
    (void*)&ivar.mqtt_enable,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_MQTT,
    "",
    "%d",
    0,
    sizeof(ivar.mqtt_enable),
    1,
    1,
  },
// N: 14
  {
    "mqtt_client_id",
    "Client ID",
    "-",
    (void*)&ivar.mqtt_client_id,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_MQTT,
    "Client1",
    "%s",
    0,
    sizeof(ivar.mqtt_client_id)-1,
    2,
    0,
  },
// N: 15
  {
    "mqtt_server_ip",
    "MQTT server IP address",
    "-",
    (void*)&ivar.mqtt_server_ip,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_MQTT,
    "192.168.3.2",
    "%s",
    0,
    sizeof(ivar.mqtt_server_ip)-1,
    3,
    0,
  },
// N: 16
  {
    "mqtt_server_port",
    "MQTT server port number",
    "-",
    (void*)&ivar.mqtt_server_port,
    tint32u,
    1883,
    0,
    65535,
    0,
    S7V30_MQTT,
    "",
    "%d",
    0,
    sizeof(ivar.mqtt_server_port),
    4,
    0,
  },
// N: 17
  {
    "mqtt_user_name",
    "User name",
    "-",
    (void*)&ivar.mqtt_user_name,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_MQTT,
    "user",
    "%s",
    0,
    sizeof(ivar.mqtt_user_name)-1,
    5,
    0,
  },
// N: 18
  {
    "mqtt_password",
    "User password",
    "-",
    (void*)&ivar.mqtt_password,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_MQTT,
    "pass",
    "%s",
    0,
    sizeof(ivar.mqtt_password)-1,
    6,
    0,
  },
// N: 19
  {
    "default_ip_addr",
    "Default IP address",
    "DEFIPAD",
    (void*)&ivar.default_ip_addr,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_Network,
    "192.168.8.200",
    "%s",
    0,
    sizeof(ivar.default_ip_addr)-1,
    1,
    2,
  },
// N: 20
  {
    "default_net_mask",
    "Default network mask ",
    "DEFNTMS",
    (void*)&ivar.default_net_mask,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_Network,
    "255.255.255.0",
    "%s",
    0,
    sizeof(ivar.default_net_mask)-1,
    2,
    2,
  },
// N: 21
  {
    "default_gateway_addr",
    "Default gateway address",
    "DEFGATE",
    (void*)&ivar.default_gateway_addr,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_Network,
    "192.168.8.1",
    "%s",
    0,
    sizeof(ivar.default_gateway_addr)-1,
    3,
    2,
  },
// N: 22
  {
    "en_dhcp_client",
    "Enable DHCP client (0-No, 1-Yes)",
    "ENDHCPC",
    (void*)&ivar.en_dhcp_client,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_Network,
    "",
    "%d",
    0,
    sizeof(ivar.en_dhcp_client),
    4,
    1,
  },
// N: 23
  {
    "this_host_name",
    "This device host name",
    "HOSTNAM",
    (void*)&ivar.this_host_name,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_Network,
    "S7V30",
    "%s",
    0,
    sizeof(ivar.this_host_name)-1,
    5,
    0,
  },
// N: 24
  {
    "en_sntp",
    "Enable SNTP client",
    "-",
    (void*)&ivar.en_sntp,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_SNTP,
    "",
    "%d",
    0,
    sizeof(ivar.en_sntp),
    1,
    1,
  },
// N: 25
  {
    "en_sntp_time_receiving",
    "Allow to receive time from time servers",
    "-",
    (void*)&ivar.en_sntp_time_receiving,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_SNTP,
    "",
    "%d",
    0,
    sizeof(ivar.en_sntp_time_receiving),
    2,
    1,
  },
// N: 26
  {
    "utc_offset",
    "UTC offset (difference in hours +-)",
    "-",
    (void*)&ivar.utc_offset,
    tfloat,
    3.00,
    -12.00,
    12.00,
    0,
    S7V30_SNTP,
    "",
    "%0.0f",
    0,
    sizeof(ivar.utc_offset),
    3,
    0,
  },
// N: 27
  {
    "time_server_1",
    "Time server 1 URL",
    "-",
    (void*)&ivar.time_server_1,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_SNTP,
    "pool.ntp.org",
    "%s",
    0,
    sizeof(ivar.time_server_1)-1,
    4,
    0,
  },
// N: 28
  {
    "time_server_2",
    "Time server 2 URL",
    "-",
    (void*)&ivar.time_server_2,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_SNTP,
    "129.6.15.28",
    "%s",
    0,
    sizeof(ivar.time_server_2)-1,
    5,
    0,
  },
// N: 29
  {
    "time_server_3",
    "Time serber 3 URL",
    "-",
    (void*)&ivar.time_server_3,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_SNTP,
    "time.nist.gov",
    "%s",
    0,
    sizeof(ivar.time_server_3)-1,
    6,
    0,
  },
// N: 30
  {
    "sntp_poll_interval",
    "Poll interval (s)",
    "-",
    (void*)&ivar.sntp_poll_interval,
    tint32u,
    120,
    1,
    1000000,
    0,
    S7V30_SNTP,
    "",
    "%d",
    0,
    sizeof(ivar.sntp_poll_interval),
    7,
    0,
  },
// N: 31
  {
    "en_telnet",
    "Enable Telnet (1-yes, 0-no)",
    "-",
    (void*)&ivar.en_telnet,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_Telnet,
    "",
    "%d",
    0,
    sizeof(ivar.en_telnet),
    0,
    1,
  },
// N: 32
  {
    "usb_mode",
    "USB mode(1-VCOM, 2-MSD, 3-VCOM&MSD, 4-VCOM&FMST, 5-RNDIS, 6-Host ECM)",
    "USBIMOD",
    (void*)&ivar.usb_mode,
    tint32u,
    4,
    0,
    6,
    0,
    S7V30_USB_Interface,
    "",
    "%d",
    0,
    sizeof(ivar.usb_mode),
    1,
    4,
  },
// N: 33
  {
    "rndis_ip_addr_assign_method",
    "RNDIS IP address assignment method (0-Win home net, 1 - DHCP server)",
    "RNDSCFG",
    (void*)&ivar.rndis_ip_addr_assign_method,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_USB_Interface,
    "",
    "%d",
    0,
    sizeof(ivar.rndis_ip_addr_assign_method),
    2,
    5,
  },
// N: 34
  {
    "usd_dev_interface",
    "USB device interface (0- HS, 1- FS)",
    "-",
    (void*)&ivar.usd_dev_interface,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_USB_Interface,
    "",
    "%d",
    0,
    sizeof(ivar.usd_dev_interface),
    3,
    6,
  },
// N: 35
  {
    "en_wifi_ap",
    "Enable Accces Point mode (1-yes, 0-no)",
    "-",
    (void*)&ivar.en_wifi_ap,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_WIFI_AP,
    "",
    "%d",
    0,
    sizeof(ivar.en_wifi_ap),
    0,
    1,
  },
// N: 36
  {
    "wifi_ap_ssid",
    "Access Point SSID",
    "-",
    (void*)&ivar.wifi_ap_ssid,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_AP,
    "S7V30",
    "%s",
    0,
    sizeof(ivar.wifi_ap_ssid)-1,
    1,
    0,
  },
// N: 37
  {
    "wifi_ap_key",
    "Access Point password (>=8 symbols)",
    "-",
    (void*)&ivar.wifi_ap_key,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_AP,
    "12345678",
    "%s",
    0,
    sizeof(ivar.wifi_ap_key)-1,
    2,
    0,
  },
// N: 38
  {
    "ap_default_ip_addr",
    "Default IP address",
    "-",
    (void*)&ivar.ap_default_ip_addr,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_AP,
    "192.168.1.1",
    "%s",
    0,
    sizeof(ivar.ap_default_ip_addr)-1,
    3,
    0,
  },
// N: 39
  {
    "ap_default_net_mask",
    "Default network mask ",
    "-",
    (void*)&ivar.ap_default_net_mask,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_AP,
    "255.255.255.0",
    "%s",
    0,
    sizeof(ivar.ap_default_net_mask)-1,
    4,
    0,
  },
// N: 40
  {
    "ap_enable_dhcp_server",
    "Enable DHCP server (1-yes, 0-no)",
    "-",
    (void*)&ivar.ap_enable_dhcp_server,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_WIFI_AP,
    "",
    "%d",
    0,
    sizeof(ivar.ap_enable_dhcp_server),
    5,
    1,
  },
// N: 41
  {
    "ap_dhcp_serv_start_ip",
    "DHCP server start IP addres",
    "-",
    (void*)&ivar.ap_dhcp_serv_start_ip,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_AP,
    "192.168.1.100",
    "%s",
    0,
    sizeof(ivar.ap_dhcp_serv_start_ip)-1,
    6,
    0,
  },
// N: 42
  {
    "ap_mac_addr",
    "AP MAC address",
    "-",
    (void*)&ivar.ap_mac_addr,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_AP,
    "00:B0:51:23:F8:0F",
    "%s",
    0,
    sizeof(ivar.ap_mac_addr)-1,
    7,
    0,
  },
// N: 43
  {
    "wifi_ap_channel",
    "WIFI channell",
    "-",
    (void*)&ivar.wifi_ap_channel,
    tint8u,
    1,
    0,
    255,
    0,
    S7V30_WIFI_AP,
    "",
    "%d",
    0,
    sizeof(ivar.wifi_ap_channel),
    8,
    0,
  },
// N: 44
  {
    "wifi_ap_addr_assign_method",
    "IP address assignment method  (0-Win home net, 1 - DHCP server)",
    "-",
    (void*)&ivar.wifi_ap_addr_assign_method,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_WIFI_AP,
    "",
    "%d",
    0,
    sizeof(ivar.wifi_ap_addr_assign_method),
    9,
    5,
  },
// N: 45
  {
    "wifi_sta_cfg1_pass",
    "Password ",
    "-",
    (void*)&ivar.wifi_sta_cfg1_pass,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG1,
    "wifi_pass",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg1_pass)-1,
    1,
    0,
  },
// N: 46
  {
    "wifi_sta_cfg1_ssid",
    "SSID",
    "-",
    (void*)&ivar.wifi_sta_cfg1_ssid,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG1,
    "wifi_login",
    "%d",
    0,
    sizeof(ivar.wifi_sta_cfg1_ssid)-1,
    2,
    0,
  },
// N: 47
  {
    "wifi_sta_cfg1_en_dhcp",
    "Enable DHCP client (1-yes, 0-no)",
    "-",
    (void*)&ivar.wifi_sta_cfg1_en_dhcp,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_WIFI_STA_CFG1,
    "",
    "%d",
    0,
    sizeof(ivar.wifi_sta_cfg1_en_dhcp),
    3,
    1,
  },
// N: 48
  {
    "wifi_sta_cfg1_default_ip_addr",
    "Default IP address",
    "-",
    (void*)&ivar.wifi_sta_cfg1_default_ip_addr,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG1,
    "192.168.0.1",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg1_default_ip_addr)-1,
    4,
    2,
  },
// N: 49
  {
    "wifi_sta_cfg1_default_net_mask",
    "Default network mask ",
    "-",
    (void*)&ivar.wifi_sta_cfg1_default_net_mask,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG1,
    "255.255.255.0",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg1_default_net_mask)-1,
    5,
    2,
  },
// N: 50
  {
    "wifi_sta_cfg1_default_gate_addr",
    "Default gateway address",
    "-",
    (void*)&ivar.wifi_sta_cfg1_default_gate_addr,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG1,
    "192.168.0.254",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg1_default_gate_addr)-1,
    6,
    2,
  },
// N: 51
  {
    "wifi_sta_cfg2_pass",
    "Password ",
    "-",
    (void*)&ivar.wifi_sta_cfg2_pass,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG2,
    "wifi_pass",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg2_pass)-1,
    1,
    0,
  },
// N: 52
  {
    "wifi_sta_cfg2_ssid",
    "SSID",
    "-",
    (void*)&ivar.wifi_sta_cfg2_ssid,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG2,
    "wifi_login",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg2_ssid)-1,
    2,
    0,
  },
// N: 53
  {
    "wifi_sta_cfg2_en_dhcp",
    "Enable DHCP client (1-yes, 0-no)",
    "-",
    (void*)&ivar.wifi_sta_cfg2_en_dhcp,
    tint8u,
    1,
    0,
    1,
    0,
    S7V30_WIFI_STA_CFG2,
    "",
    "%d",
    0,
    sizeof(ivar.wifi_sta_cfg2_en_dhcp),
    3,
    1,
  },
// N: 54
  {
    "wifi_sta_cfg2_default_ip_addr",
    "Default IP address",
    "-",
    (void*)&ivar.wifi_sta_cfg2_default_ip_addr,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG2,
    "192.168.0.1",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg2_default_ip_addr)-1,
    4,
    0,
  },
// N: 55
  {
    "wifi_sta_cfg2_default_net_mask",
    "Default network mask ",
    "-",
    (void*)&ivar.wifi_sta_cfg2_default_net_mask,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG2,
    "255.255.255.0",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg2_default_net_mask)-1,
    5,
    0,
  },
// N: 56
  {
    "wifi_sta_cfg2_default_gate_addr",
    "Default gateway address",
    "-",
    (void*)&ivar.wifi_sta_cfg2_default_gate_addr,
    tstring,
    0,
    0,
    0,
    0,
    S7V30_WIFI_STA_CFG2,
    "192.168.0.254",
    "%s",
    0,
    sizeof(ivar.wifi_sta_cfg2_default_gate_addr)-1,
    6,
    0,
  },
// N: 57
  {
    "wifi_sta_cfg1_en",
    "Enable configuration (1-yes, 0-no)",
    "-",
    (void*)&ivar.wifi_sta_cfg1_en,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_WIFI_STA_COMMON,
    "",
    "%d",
    0,
    sizeof(ivar.wifi_sta_cfg1_en),
    0,
    1,
  },
// N: 58
  {
    "wifi_sta_cfg2_en",
    "Enable configuration (1-yes, 0-no)",
    "-",
    (void*)&ivar.wifi_sta_cfg2_en,
    tint8u,
    0,
    0,
    1,
    0,
    S7V30_WIFI_STA_COMMON,
    "",
    "%d",
    0,
    sizeof(ivar.wifi_sta_cfg2_en),
    0,
    1,
  },
};
 
 
// Selector description:  Выбор между Yes и No
static const T_selector_items selector_1[2] = 
{
  { 0 , "No                                          " , 0},
  { 1 , "Yes                                         " , 1},
};
 
// Selector description:  LED mode
static const T_selector_items selector_3[2] = 
{
  { 0 , "Always OFF                                  " , -1},
  { 1 , "Normal work                                 " , -1},
};
 
// Selector description:  USB mode
static const T_selector_items selector_4[7] = 
{
  { 0 , "None                                        " , -1},
  { 1 , "VCOM port                                   " , -1},
  { 2 , "Mass storage                                " , -1},
  { 3 , "VCOM and Mass storage                       " , -1},
  { 4 , "VCOM and FreeMaster port                    " , -1},
  { 5 , "RNDIS                                       " , -1},
  { 6 , "Host ECM                                    " , -1},
};
 
// Selector description:  IP address assignment method
static const T_selector_items selector_5[2] = 
{
  { 0 , "Windows home network                        " , -1},
  { 1 , "Preconfigured DHCP server                   " , -1},
};
 
// Selector description:  Выбор интерфейса для работы USB device
static const T_selector_items selector_6[2] = 
{
  { 0 , "High speed interface                        " , -1},
  { 1 , "Full speed interface                        " , -1},
};
 
static const T_selectors_list selectors_list[SELECTORS_NUM] = 
{
  {"string"                      , 0    , 0             },
  {"binary"                      , 2    , selector_1    },
  {"ip_addr"                     , 0    , 0             },
  {"leds_mode"                   , 2    , selector_3    },
  {"usb_mode"                    , 7    , selector_4    },
  {"IP_address_assignment_method", 2    , selector_5    },
  {"usb_dev_interface"           , 2    , selector_6    },
};
 
const T_NV_parameters_instance ivar_inst =
{
  IVAR_SIZE,
  arr_ivar,
  PARMNU_ITEM_NUM,
  parmenu,
  SELECTORS_NUM,
  selectors_list
};
