#ifndef __PARAMS_IVAR_H
  #define __PARAMS_IVAR_H

#define  S7V30_0                        0
#define  S7V30_main                     1
#define  S7V30_General                  2
#define  S7V30_USB_Interface            3
#define  S7V30_Network                  4
#define  S7V30_MQTT                     5
#define  S7V30_FTP_server               6
#define  S7V30_MATLAB                   7
#define  S7V30_SNTP                     8
#define  S7V30_WIFI_STA                 9
#define  S7V30_WIFI_AP                  10
#define  S7V30_Telnet                   11
#define  S7V30_WIFI_STA_COMMON          12
#define  S7V30_WIFI_STA_CFG1            13
#define  S7V30_WIFI_STA_CFG2            14
#define  S7V30_FreeMaster               15

typedef struct
{
  uint8_t        ap_default_ip_addr[16+1];      // Default IP address | def.val.= 192.168.1.1
  uint8_t        ap_default_net_mask[16+1];     // Default network mask  | def.val.= 255.255.255.0
  uint8_t        ap_dhcp_serv_start_ip[16+1];   // DHCP server start IP addres | def.val.= 192.168.1.100
  uint8_t        ap_enable_dhcp_server;         // Enable DHCP server (1-yes, 0-no) | def.val.= 1
  uint8_t        ap_mac_addr[18+1];             // AP MAC address | def.val.= 00:B0:51:23:F8:0F
  uint8_t        default_gateway_addr[16+1];    // Default gateway address | def.val.= 192.168.8.1
  uint8_t        default_ip_addr[16+1];         // Default IP address | def.val.= 192.168.8.200
  uint8_t        default_net_mask[16+1];        // Default network mask  | def.val.= 255.255.255.0
  uint8_t        en_compress_settins;           // Enable compress settings file (1-yes, 0-no) | def.val.= 1
  uint8_t        en_dhcp_client;                // Enable DHCP client (0-No, 1-Yes) | def.val.= 1
  uint8_t        en_formated_settings;          // Enable formating in  settings file (1-yes, 0-no) | def.val.= 1
  uint8_t        en_freemaster;                 // Enable FreeMaster protocol | def.val.= 1
  uint8_t        en_log_to_file;                // Enable logging to file (1-yes, 0-no) | def.val.= 0
  uint8_t        en_log_to_freemaster;          // Enable logging to FreeMaster pipe | def.val.= 0
  uint8_t        en_matlab;                     // Enable MATLAB communication server | def.val.= 0
  uint8_t        en_sntp;                       // Enable SNTP client | def.val.= 1
  uint8_t        en_sntp_time_receiving;        // Allow to receive time from time servers | def.val.= 1
  uint8_t        en_telnet;                     // Enable Telnet (1-yes, 0-no) | def.val.= 1
  uint8_t        en_wifi_ap;                    // Enable Accces Point mode (1-yes, 0-no) | def.val.= 0
  uint8_t        enable_ftp_server;             // Enable FTP server | def.val.= 0
  uint8_t        ftp_serv_login[16+1];          // Login | def.val.= ftp_login
  uint8_t        ftp_serv_password[64+1];       // Password  | def.val.= ftp_pass
  uint8_t        hardware_version[64+1];        // Hardware version | def.val.= S7V30 
  uint8_t        manuf_date[64+1];              // Manufacturing date | def.val.= 2023 04 25
  uint8_t        mqtt_client_id[16+1];          // Client ID | def.val.= Client1
  uint8_t        mqtt_enable;                   // Enable MQTT client  | def.val.= 0
  uint8_t        mqtt_password[16+1];           // User password | def.val.= pass
  uint8_t        mqtt_server_ip[16+1];          // MQTT server IP address | def.val.= 192.168.3.2
  uint32_t       mqtt_server_port;              // MQTT server port number | def.val.= 1883
  uint8_t        mqtt_user_name[16+1];          // User name | def.val.= user
  uint8_t        name[64+1];                    // Product  name | def.val.= S7V30
  uint8_t        rndis_ip_addr_assign_method;   // RNDIS IP address assignment method (0-Win home net, 1 - DHCP server) | def.val.= 0
  uint32_t       sntp_poll_interval;            // Poll interval (s) | def.val.= 120
  uint8_t        software_version[64+1];        // Software version | def.val.= S7V30
  uint8_t        this_host_name[16+1];          // This device host name | def.val.= S7V30
  uint8_t        time_server_1[64+1];           // Time server 1 URL | def.val.= pool.ntp.org
  uint8_t        time_server_2[64+1];           // Time server 2 URL | def.val.= 129.6.15.28
  uint8_t        time_server_3[64+1];           // Time serber 3 URL | def.val.= time.nist.gov
  uint32_t       usb_mode;                      // USB mode(1-VCOM, 2-MSD, 3-VCOM&MSD, 4-VCOM&FMST, 5-RNDIS, 6-Host ECM) | def.val.= 4
  uint8_t        usd_dev_interface;             // USB device interface (0- HS, 1- FS) | def.val.= 0
  float          utc_offset;                    // UTC offset (difference in hours +-) | def.val.= 3
  uint8_t        wifi_ap_addr_assign_method;    // IP address assignment method  (0-Win home net, 1 - DHCP server) | def.val.= 1
  uint8_t        wifi_ap_channel;               // WIFI channell | def.val.= 1
  uint8_t        wifi_ap_key[32+1];             // Access Point password (>=8 symbols) | def.val.= 12345678
  uint8_t        wifi_ap_ssid[32+1];            // Access Point SSID | def.val.= S7V30
  uint8_t        wifi_sta_cfg1_default_gate_addr[16+1]; // Default gateway address | def.val.= 192.168.0.254
  uint8_t        wifi_sta_cfg1_default_ip_addr[16+1]; // Default IP address | def.val.= 192.168.0.1
  uint8_t        wifi_sta_cfg1_default_net_mask[16+1]; // Default network mask  | def.val.= 255.255.255.0
  uint8_t        wifi_sta_cfg1_en;              // Enable configuration (1-yes, 0-no) | def.val.= 0
  uint8_t        wifi_sta_cfg1_en_dhcp;         // Enable DHCP client (1-yes, 0-no) | def.val.= 1
  uint8_t        wifi_sta_cfg1_pass[32+1];      // Password  | def.val.= wifi_pass
  uint8_t        wifi_sta_cfg1_ssid[32+1];      // SSID | def.val.= wifi_login
  uint8_t        wifi_sta_cfg2_default_gate_addr[16+1]; // Default gateway address | def.val.= 192.168.0.254
  uint8_t        wifi_sta_cfg2_default_ip_addr[16+1]; // Default IP address | def.val.= 192.168.0.1
  uint8_t        wifi_sta_cfg2_default_net_mask[16+1]; // Default network mask  | def.val.= 255.255.255.0
  uint8_t        wifi_sta_cfg2_en;              // Enable configuration (1-yes, 0-no) | def.val.= 0
  uint8_t        wifi_sta_cfg2_en_dhcp;         // Enable DHCP client (1-yes, 0-no) | def.val.= 1
  uint8_t        wifi_sta_cfg2_pass[32+1];      // Password  | def.val.= wifi_pass
  uint8_t        wifi_sta_cfg2_ssid[32+1];      // SSID | def.val.= wifi_login
} IVAR_TYPE;


#endif



// Selector description:  Выбор между Yes и No
#define BINARY_NO                                 0
#define BINARY_YES                                1

// Selector description:  LED mode
#define LEDS_MODE_ALWAYS_OFF                      0
#define LEDS_MODE_NORMAL_WORK                     1

// Selector description:  USB mode
#define USB_MODE_NONE                             0
#define USB_MODE_VCOM_PORT                        1
#define USB_MODE_MASS_STORAGE_                    2
#define USB_MODE_VCOM_AND_MASS_STORAGE            3
#define USB_MODE_VCOM_AND_FREEMASTER_PORT         4
#define USB_MODE_RNDIS                            5
#define USB_MODE_HOST_ECM                         6

// Selector description:  IP address assignment method
#define IP_ADDRESS_ASSIGNMENT_METHOD_WINDOWS_HOME_NETWORK  0
#define IP_ADDRESS_ASSIGNMENT_METHOD_PRECONFIGURED_DHCP_SERVER  1

// Selector description:  Выбор интерфейса для работы USB device
#define USB_DEV_INTERFACE_HIGH_SPEED_INTERFACE    0
#define USB_DEV_INTERFACE_FULL_SPEED_INTERFACE    1


extern IVAR_TYPE  ivar;
extern const T_NV_parameters_instance ivar_inst;

