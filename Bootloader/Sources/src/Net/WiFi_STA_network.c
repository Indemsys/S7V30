// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-05-15
// 12:26:12
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"

#define  STA_INTERFACE_NAME            "WIFI_STA"
#define  WIFI_NAT_ENTRY_CACHE_SIZE      1024

const T_WIFI_STA_Config  WIFI_STA_Configs[WIFI_STA_CFG_NUM] =
{
  {&ivar.wifi_sta_cfg1_en,&ivar.wifi_sta_cfg1_en_dhcp, ivar.wifi_sta_cfg1_pass, ivar.wifi_sta_cfg1_ssid, ivar.wifi_sta_cfg1_default_ip_addr, ivar.wifi_sta_cfg1_default_net_mask, ivar.wifi_sta_cfg1_default_gate_addr },
  {&ivar.wifi_sta_cfg2_en,&ivar.wifi_sta_cfg2_en_dhcp, ivar.wifi_sta_cfg2_pass, ivar.wifi_sta_cfg2_ssid, ivar.wifi_sta_cfg2_default_ip_addr, ivar.wifi_sta_cfg2_default_net_mask, ivar.wifi_sta_cfg2_default_gate_addr },
};


uint32_t                  sta_cfg_indx = 0;
static uint8_t            sta_joined;
static T_sys_timestump    disjoint_time;
static whd_mac_t          remote_ap_addr;
static uint8_t            sta_connection_active = 0;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t возвращаем 1 если линк активен
-----------------------------------------------------------------------------------------------------*/
NX_INTERFACE* WIFI_STA_link_state(void)
{
  NX_IP *ip = Net_get_ip_ptr();
  if (sta_connection_active == 0) return NULL;

  if (ip->nx_ip_interface[WIFI_STA_INTF_NUM].nx_interface_link_up)
  {
    return &(ip->nx_ip_interface[WIFI_STA_INTF_NUM]);
  }
  return NULL;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* WIFI_STA_get_def_ip(void)
{
  return (char const *)WIFI_STA_Configs[sta_cfg_indx].ip;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* WIFI_STA_get_def_mask(void)
{
  return (char const *)WIFI_STA_Configs[sta_cfg_indx].mask;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr_ptr
-----------------------------------------------------------------------------------------------------*/
void WIFI_STA_save_remote_ap_addr(whd_mac_t *addr_ptr)
{
  memcpy(&remote_ap_addr,addr_ptr, sizeof(remote_ap_addr));
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return whd_mac_t*
-----------------------------------------------------------------------------------------------------*/
whd_mac_t* WIFI_STA_get_remote_ap_addr(void)
{
  return &remote_ap_addr;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return char*
-----------------------------------------------------------------------------------------------------*/
char* WIFI_STA_get_current_SSID(void)
{
  return (char *)WIFI_STA_Configs[sta_cfg_indx].ssid;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_disjoint_time_sec(void)
{
  return Time_elapsed_sec(&disjoint_time);
}



/*-----------------------------------------------------------------------------------------------------
  Устройство подлючилось к удаленной точке доступа.
  Нужно начать работу клиента DHCP


  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_accept_connection_to_AP(void)
{
  uint32_t   res = NX_SUCCESS;
  uint32_t   ip_address;
  uint32_t   network_mask;
  uint32_t   gateway_address;
  ULONG      ret_val;
  NX_IP     *ip_ptr;

  sta_connection_active = 1;

  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_INITIALIZE, WIFI_STA_INTF_NUM,&ret_val);
  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_ENABLE, WIFI_STA_INTF_NUM,&ret_val);

  Str_to_IP_v4((char const *)WIFI_STA_Configs[sta_cfg_indx].ip,  (uint8_t *)&ip_address);
  Str_to_IP_v4((char const *)WIFI_STA_Configs[sta_cfg_indx].mask,(uint8_t *)&network_mask);
  Str_to_IP_v4((char const *)WIFI_STA_Configs[sta_cfg_indx].gate,(uint8_t *)&gateway_address);

  ip_ptr = Net_get_ip_ptr();

  nx_ip_interface_address_set(ip_ptr, WIFI_STA_INTF_NUM, ip_address, network_mask);
  nx_ip_gateway_address_set(ip_ptr, gateway_address);

  if (*(WIFI_STA_Configs[sta_cfg_indx].enable_dhcp))
  {
    res = nx_dhcp_interface_start(Net_get_dhcp_client_ptr(), WIFI_STA_INTF_NUM);
    if (res != NX_SUCCESS)
    {
      NETLOG("WIFI STA DHCP client error %04X", res);
    }
    else
    {
      NETLOG("WIFI STA DHCP client started");
    }
  }
  Net_mDNS_enable(WIFI_STA_INTF_NUM);


  NETLOG("WIFI STA IP activated");
  sta_joined = 1;
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Устройство отключилось от удаленной точки доступа.
  Нужно прекратить работу клиента DHCP


  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_accept_disconnection_from_AP(void)
{
  uint32_t  res = NX_SUCCESS;
  ULONG     ret_val;

  sta_connection_active = 0;


  Net_mDNS_disable(WIFI_STA_INTF_NUM);

  res = nx_dhcp_interface_stop(Net_get_dhcp_client_ptr(), WIFI_STA_INTF_NUM);
  if (res != NX_SUCCESS)
  {
    NETLOG("WIFI STA DHCP client error %04X", res);
  }
  else
  {
    NETLOG("WIFI STA DHCP client stoped");
  }

  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_DISABLE, WIFI_STA_INTF_NUM,&ret_val);

  sta_joined = 0;
  Get_hw_timestump(&disjoint_time);


  NETLOG("WIFI STA IP deactivated");
  Send_wifi_event(NET_EVT_WIFI_STA_DISJOINED); // Посылаем событие по которому задача Thread_wifi_manager вызовет процедуру WiFi Join
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param cfg_indx

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_Join_to(uint32_t    indx)
{
  uint32_t    res;
  uint32_t    err_line;
  whd_ssid_t  ssid;
  uint8_t    *pass_str;


  if (*(WIFI_STA_Configs[indx].enable_cfg) == 0) return RES_ERROR;

  pass_str = WIFI_STA_Configs[indx].pass;

  ssid.length = strlen((char *)WIFI_STA_Configs[indx].ssid);
  if (ssid.length > SSID_NAME_SIZE) ssid.length = SSID_NAME_SIZE;
  memcpy(ssid.value, WIFI_STA_Configs[indx].ssid, ssid.length);

  res = whd_wifi_enable_sup_set_passphrase(g_prim_whd_intf_ptr, pass_str, strlen((char *)pass_str), WHD_SECURITY_WPA3_WPA2_PSK);
  if (res != WHD_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }
  res = whd_wifi_join(g_prim_whd_intf_ptr ,&ssid, WHD_SECURITY_WPA3_WPA2_PSK, pass_str, strlen((char *)pass_str)); // WHD_SECURITY_WPA2_WPA_AES_PSK
  if (res != WHD_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }


  NETLOG("WIFI STA: Joining to %s done", WIFI_STA_Configs[indx].ssid);
  return RES_OK;
_err:

  NETLOG("WIFI STA: Joining to %s error in line %d. Result=%04X", WIFI_STA_Configs[indx].ssid, err_line, res);
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_Join(void)
{
  Get_hw_timestump(&disjoint_time);
  if (sta_joined == 0)
  {
    // Выпоняем 5-ть попыток присоединения по списку заданных конфигураций
    for (uint32_t n = 0; n < 5 * WIFI_STA_CFG_NUM; n++)
    {
      if (WIFI_STA_Join_to(sta_cfg_indx) == RES_OK)
      {
        return RES_OK;
      }
      sta_cfg_indx++;
      if (sta_cfg_indx >= WIFI_STA_CFG_NUM) sta_cfg_indx = 0;
    }
  }
  return RES_ERROR;
}

