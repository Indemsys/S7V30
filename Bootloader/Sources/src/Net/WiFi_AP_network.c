// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-05-15
// 12:26:03
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"

#define  AP_INTERFACE_NAME "WIFI_AP"

NX_IP                    *wifi_ap_ip_ptr;
NX_DHCP_SERVER           *wifi_ap_dhcp_srv_ptr;
static whd_mac_t          remote_sta_addr;
static uint8_t            ap_connection_active = 0;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t возвращаем ссылку на IP если линк активен
-----------------------------------------------------------------------------------------------------*/
NX_INTERFACE *WIFI_AP_link_state(void)
{
  NX_IP *ip = Net_get_ip_ptr();

  if (ap_connection_active == 0) return NULL;

  if (ip->nx_ip_interface[WIFI_AP_INTF_NUM].nx_interface_link_up)
  {
    return &(ip->nx_ip_interface[WIFI_AP_INTF_NUM]);
  }
  return NULL;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr_ptr
-----------------------------------------------------------------------------------------------------*/
void WIFI_AP_save_remote_sta_addr(whd_mac_t *addr_ptr)
{
  memcpy(&remote_sta_addr,addr_ptr, sizeof(remote_sta_addr));
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return whd_mac_t*
-----------------------------------------------------------------------------------------------------*/
whd_mac_t* WIFI_AP_get_remote_sta_addr(void)
{
  return &remote_sta_addr;
}

/*-----------------------------------------------------------------------------------------------------
  К устройство подлючилась удаленная станция
  Нужно начать работу сервера  DHCP

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_AP_accept_STA_connection(void)
{
  ULONG ret_val;

  Net_mDNS_enable(WIFI_AP_INTF_NUM);

  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_ENABLE, WIFI_AP_INTF_NUM, &ret_val);

  ap_connection_active = 1;
  return NX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  От устройства отключилась удаленная станция
  Нужно остановить работу сервера  DHCP.
  Поскольку клиент может быть только один, то надо очистить список от предыдущего клиента.


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_AP_accept_STA_disconnection(void)
{
  ULONG ret_val;

  Net_mDNS_disable(WIFI_AP_INTF_NUM);

  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_DISABLE, WIFI_AP_INTF_NUM,&ret_val);

  ap_connection_active = 0;
  return NX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_AP_start(void)
{
  uint32_t       res;
  uint32_t       err_line;
  whd_ssid_t     ssid;
  ULONG          ret_val;



  strncpy((char *)ssid.value, (char *)ivar.wifi_ap_ssid, SSID_NAME_SIZE - 1);
  ssid.length = strlen((char *)ivar.wifi_ap_ssid);

  res = whd_wifi_init_ap(g_secd_whd_intf_ptr,&ssid, WHD_SECURITY_WPA2_WPA_AES_PSK, ivar.wifi_ap_key, strlen((char *)ivar.wifi_ap_key), ivar.wifi_ap_channel);
  if (res != WHD_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  if (ivar.en_wifi_ap)
  {
    res = whd_wifi_start_ap(g_secd_whd_intf_ptr);
    if (res != WHD_SUCCESS)
    {
      err_line = __LINE__;
      goto _err;
    }
  }

  // Здесь надо отдать команду драйверу на инициализацию, поскольку еще не согласован MAC адрес в WiFi модуле и драйвере интерфейса
  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_INITIALIZE, WIFI_AP_INTF_NUM, &ret_val);

  Wait_ms(10);
  Send_wifi_event(NET_EVT_WIFI_AP_READY); // Сообщаем задче Thread_net о том что закончили инициализацию WiFi точки доступа

  NETLOG("WIFI AP initialized");
  return RES_OK;
_err:
  NETLOG("WIFI AP initialization error in line %d. Result=%04X", err_line , res);
  return RES_ERROR;
}


