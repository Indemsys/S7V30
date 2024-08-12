// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-07-26
// 16:39:05
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define MAX_STR_SZ   64

typedef struct
{
    float          cpu_usage;
    rtc_time_t     ct;
    char           sta_ip_address[MAX_STR_SZ];
    char           sta_mac_address[MAX_STR_SZ];
    char           sta_remote_mac_address[MAX_STR_SZ];
    char           sta_ssid[MAX_STR_SZ];
    char           ap_ip_address[MAX_STR_SZ];
    char           ap_mac_address[MAX_STR_SZ];
    char           ap_remote_mac_address[MAX_STR_SZ];
    char           usb_ip_type[MAX_STR_SZ];
    char           usb_ip_address[MAX_STR_SZ];
    char           usb_mac_address[MAX_STR_SZ];
    char           gateway_ip[MAX_STR_SZ];
    uint32_t       ram_free;
    uint32_t       sdram_free;
    uint32_t       up_time;
} T_dev_status;


/*-----------------------------------------------------------------------------------------------------

  \param sz_ptr

  \return char*
-----------------------------------------------------------------------------------------------------*/
static void  _Print_DeviceInfo(char *buff, uint32_t *buff_sz)
{
  uint32_t offset = 0;
  offset += Print_to(buff, offset, "{");
  offset += Print_to(buff, offset, "\"CPU_ID\":\"%s\",",g_cpu_id_str);
  offset += Print_to(buff, offset, "\"Product_name\":\"%s\",",ivar.product_name);
  offset += Print_to(buff, offset, "\"SW_Ver\":\"%s\",",ivar.manuf_date);
  offset += Print_to(buff, offset, "\"HW_Ver\":\"%s\",",ivar.hardware_version);
  offset += Print_to(buff, offset, "\"CompDate\":\"%s\",", Get_build_date());
  offset += Print_to(buff, offset, "\"CompTime\":\"%s\"", Get_build_time());
  offset += Print_to(buff, offset, "}");
  *buff_sz = offset;
}

/*-----------------------------------------------------------------------------------------------------


  \param buff
  \param buff_sz
-----------------------------------------------------------------------------------------------------*/
static void  _Print_DeviceStatus(T_dev_status *d_p, char *buff, uint32_t *buff_sz)
{
  uint32_t offset = 0;
  offset += Print_to(buff, offset, "{");
  offset += Print_to(buff, offset, "\"cpu_usage\":%f,",(double)d_p->cpu_usage);
  offset += Print_to(buff, offset, "\"local_time\":\"%04d.%02d.%02d  %02d:%02d:%02d\",",d_p->ct.tm_year + 1900, d_p->ct.tm_mon, d_p->ct.tm_mday, d_p->ct.tm_hour, d_p->ct.tm_min, d_p->ct.tm_sec);
  offset += Print_to(buff, offset, "\"sta_ip_address\":\"%s\",",d_p->sta_ip_address);
  offset += Print_to(buff, offset, "\"sta_mac_address\":\"%s\",",d_p->sta_mac_address);
  offset += Print_to(buff, offset, "\"sta_rem_mac_address\":\"%s\",",d_p->sta_remote_mac_address);
  offset += Print_to(buff, offset, "\"sta_ssid\":\"%s\",",d_p->sta_ssid);
  offset += Print_to(buff, offset, "\"ap_ip_address\":\"%s\",",d_p->ap_ip_address);
  offset += Print_to(buff, offset, "\"ap_mac_address\":\"%s\",",d_p->ap_mac_address);
  offset += Print_to(buff, offset, "\"ap_rem_mac_address\":\"%s\",",d_p->ap_remote_mac_address);
  offset += Print_to(buff, offset, "\"usb_ip_type\":\"%s\",",d_p->usb_ip_type);
  offset += Print_to(buff, offset, "\"usb_ip_address\":\"%s\",",d_p->usb_ip_address);
  offset += Print_to(buff, offset, "\"usb_mac_address\":\"%s\",",d_p->usb_mac_address);
  offset += Print_to(buff, offset, "\"gateway_ip_address\":\"%s\",",d_p->gateway_ip);
  offset += Print_to(buff, offset, "\"ram_free\":\"%d\",",d_p->ram_free);
  offset += Print_to(buff, offset, "\"sdram_free\":\"%d\",",d_p->sdram_free);
  offset += Print_to(buff, offset, "\"up_time\":%d",d_p->up_time);
  offset += Print_to(buff, offset, "}");
  *buff_sz = offset;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
char*  DeviceStatus_serializer_to_buff(uint32_t *sz_ptr)
{
  T_dev_status    *ds_ptr = NULL;
  char            *buffer = NULL;
  uint32_t         buff_sz = 0;
  NX_INTERFACE    *ip_interface;
  uint8_t          mac[6];
  whd_mac_t       *mac_ptr;
  uint8_t          net_active = 0;

  ds_ptr = App_malloc_pending(sizeof(T_dev_status), MS_TO_TICKS(100));
  if (ds_ptr == NULL) return NULL;

  // Получим снимок статуса устройства
  rtc_cbl.p_api->calendarTimeGet(rtc_cbl.p_ctrl,&(ds_ptr->ct));
  ds_ptr->ct.tm_mon++;

  ip_interface = WIFI_STA_link_state();
  if (ip_interface == NULL)
  {
    strcpy(ds_ptr->sta_ip_address, "");
    strcpy(ds_ptr->sta_mac_address, "");
    strcpy(ds_ptr->sta_ssid, "");
    strcpy(ds_ptr->sta_remote_mac_address,"");
  }
  else
  {
    net_active =1;
    sprintf(ds_ptr->sta_ip_address, "%d.%d.%d.%d", IPADDR(ip_interface->nx_interface_ip_address));
    Get_IP_MAC(ip_interface, mac);
    sprintf(ds_ptr->sta_mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    sprintf(ds_ptr->sta_ssid,"%s",WIFI_STA_get_current_SSID());
    mac_ptr = WIFI_STA_get_remote_ap_addr();
    sprintf(ds_ptr->sta_remote_mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac_ptr->octet[0],mac_ptr->octet[1],mac_ptr->octet[2],mac_ptr->octet[3],mac_ptr->octet[4],mac_ptr->octet[5]);
  }

  ip_interface = WIFI_AP_link_state();
  if (ip_interface == NULL)
  {
    strcpy(ds_ptr->ap_ip_address, "");
    strcpy(ds_ptr->ap_mac_address, "");
    strcpy(ds_ptr->ap_remote_mac_address,"");
  }
  else
  {
    net_active =1;
    sprintf(ds_ptr->ap_ip_address, "%d.%d.%d.%d", IPADDR(ip_interface->nx_interface_ip_address));
    Get_IP_MAC(ip_interface, mac);
    sprintf(ds_ptr->ap_mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    mac_ptr = WIFI_AP_get_remote_sta_addr();
    sprintf(ds_ptr->ap_remote_mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac_ptr->octet[0],mac_ptr->octet[1],mac_ptr->octet[2],mac_ptr->octet[3],mac_ptr->octet[4],mac_ptr->octet[5]);
  }

  strcpy(ds_ptr->usb_ip_address, "");
  strcpy(ds_ptr->usb_mac_address, "");
  if (ivar.usb_mode == USB_MODE_RNDIS)
  {
    ip_interface = RNDIS_link_state();
    strcpy(ds_ptr->usb_ip_type, "RNDIS");
    if (ip_interface != NULL)
    {
      net_active =1;
      sprintf(ds_ptr->usb_ip_address, "%d.%d.%d.%d", IPADDR(ip_interface->nx_interface_ip_address));
      Get_IP_MAC(ip_interface, mac);
      sprintf(ds_ptr->usb_mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    }
  }
  else if (ivar.usb_mode == USB_MODE_HOST_ECM)
  {
    ip_interface = ECM_host_link_state();
    strcpy(ds_ptr->usb_ip_type, "ECM HOST");
    if (ip_interface != NULL)
    {
      net_active =1;
      sprintf(ds_ptr->usb_ip_address, "%d.%d.%d.%d", IPADDR(ip_interface->nx_interface_ip_address));
      Get_IP_MAC(ip_interface, mac);
      sprintf(ds_ptr->usb_mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    }
  }

  if (net_active)
  {
    ULONG         ip_addr;
    nx_ip_gateway_address_get(Net_get_ip_ptr(),&ip_addr);
    sprintf(ds_ptr->gateway_ip, "%d.%d.%d.%d", IPADDR(ip_addr));
  }
  else
  {
    strcpy(ds_ptr->gateway_ip, "");
  }


  {
    uint32_t avail_bytes;
    uint32_t fragments;
    App_get_RAM_pool_statistic(&avail_bytes,&fragments);
    ds_ptr->ram_free = avail_bytes;
    App_get_SDRAM_pool_statistic(&avail_bytes,&fragments);
    ds_ptr->sdram_free = avail_bytes;
  }



  ds_ptr->up_time = _tx_time_get() / TX_TIMER_TICKS_PER_SECOND;
  ds_ptr->cpu_usage = (float)g_aver_cpu_usage / 10.0f;

  _Print_DeviceStatus(ds_ptr, NULL,&buff_sz);

  buffer = App_malloc_pending(buff_sz + 1, MS_TO_TICKS(100));
  if (buffer == NULL)
  {
    App_free(ds_ptr);
    return NULL;
  }

  _Print_DeviceStatus(ds_ptr, buffer, sz_ptr);
  App_free(ds_ptr);
  return buffer;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
char*  DeviceInfo_serializer_to_buff(uint32_t *sz_ptr)
{
  char    *buffer = NULL;
  uint32_t buff_sz = 0;

  _Print_DeviceInfo(NULL,&buff_sz);

  buffer = App_malloc_pending(buff_sz + 1, MS_TO_TICKS(100));
  if (buffer == NULL) return NULL;

  _Print_DeviceInfo(buffer, sz_ptr);
  return buffer;
}


