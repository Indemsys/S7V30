// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-05-15
// 12:26:12
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "whd.h"
#include   "whd_types.h"
#include   "whd_int.h"
#include   "whd_wifi_api.h"
#include   "whd_utils.h"
#include   "whd_resource_api.h"
#include   "whd_network_types.h"
#include   "whd_wlioctl.h"
#include   "cyhal_hw_types.h"
#include   "WiFi_Host_Driver_Port.h"
#include   "WiFi_NetX_adapter.h"
#include   "WiFi_STA_network.h"
#include   "WiFi_AP_network.h"


extern whd_interface_t   g_pri_ifp;
extern whd_interface_t   g_sec_ifp;

const T_WIFI_STA_Config  WIFI_STA_Configs[WIFI_STA_CFG_NUM] =
{
  {&ivar.wifi_sta_cfg1_en,&ivar.wifi_sta_cfg1_en_dhcp, ivar.wifi_sta_cfg1_pass, ivar.wifi_sta_cfg1_ssid, ivar.wifi_sta_cfg1_default_ip_addr, ivar.wifi_sta_cfg1_default_net_mask, ivar.wifi_sta_cfg1_default_gate_addr },
  {&ivar.wifi_sta_cfg2_en,&ivar.wifi_sta_cfg2_en_dhcp, ivar.wifi_sta_cfg2_pass, ivar.wifi_sta_cfg2_ssid, ivar.wifi_sta_cfg2_default_ip_addr, ivar.wifi_sta_cfg2_default_net_mask, ivar.wifi_sta_cfg2_default_gate_addr },
};


T_app_net_props          wifi_sta_net_props;
NX_IP                   *wifi_sta_ip_ptr;
uint8_t                 *wifi_sta_ip_stack_memory;
uint8_t                 *wifi_sta_arp_memory;
static uint8_t           wifi_sta_network_active;

NX_DHCP                 wifi_sta_dhcp_client;
uint32_t                current_sta_cfg;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_enabled_flag(void)
{
  for (uint32_t i=0; i < WIFI_STA_CFG_NUM; i++)
  {
    if (*(WIFI_STA_Configs[i].enable_cfg) != 0)
    {
      return 1;
    }
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t 0 - если сеть не активна, 1 - если сеть активна
-----------------------------------------------------------------------------------------------------*/
uint8_t WIFI_STA_network_active_flag(void)
{
  return wifi_sta_network_active;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void WIFI_STA_get_IP_proprties(void)
{
  for (uint32_t i=0; i < WIFI_STA_CFG_NUM; i++)
  {
    if (*(WIFI_STA_Configs[i].enable_cfg) != 0)
    {
      wifi_sta_net_props.en_dhcp =*(WIFI_STA_Configs[i].enable_dhcp);
      Str_to_IP_v4((char const *)WIFI_STA_Configs[i].ip,(uint8_t *)&wifi_sta_net_props.ip_address);
      Str_to_IP_v4((char const *)WIFI_STA_Configs[i].mask,(uint8_t *)&wifi_sta_net_props.network_mask);
      Str_to_IP_v4((char const *)WIFI_STA_Configs[i].gate,(uint8_t *)&wifi_sta_net_props.gateway_address);
      return;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_Join(void)
{
  uint32_t res;

  // Выпоняем 5-ть попыток присоедениться к заданным AP
  for (uint32_t n=0;n<5;n++)
  {

    for (uint32_t i=0; i < WIFI_STA_CFG_NUM; i++)
    {
      if (*(WIFI_STA_Configs[i].enable_cfg) != 0)
      {
        whd_ssid_t ssid;
        uint8_t    *key = WIFI_STA_Configs[i].pass;

        ssid.length = strlen((char *)WIFI_STA_Configs[i].ssid);
        if (ssid.length > SSID_NAME_SIZE) ssid.length = SSID_NAME_SIZE;
        memcpy(ssid.value, WIFI_STA_Configs[i].ssid, ssid.length);

        res = whd_wifi_enable_sup_set_passphrase(g_pri_ifp, key, strlen((char *)key), WHD_SECURITY_WPA3_WPA2_PSK);
        if (res != WHD_SUCCESS)
        {
           APPLOG("WIFI STA. Jwhd_wifi_enable_sup_set_passphrase failed. Error=%08X", res );
        }
        else
        {
          res = whd_wifi_join(g_pri_ifp ,&ssid, WHD_SECURITY_WPA3_WPA2_PSK, key, strlen((char *)key));// WHD_SECURITY_WPA2_WPA_AES_PSK
          if (res == WHD_SUCCESS)
          {
            current_sta_cfg = i;
            APPLOG("WIFI STA. Joining to %s done successfully.", WIFI_STA_Configs[i].ssid);
            return RES_OK;
          }
          else
          {
            APPLOG("WIFI STA. Joining to %s failed. Error=%08X", WIFI_STA_Configs[i].ssid, res );
          }
        }
      }
    }
  }

  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_init_TCP_stack(void)
{
  UINT status;

  wifi_sta_ip_ptr                   = App_malloc(sizeof(NX_IP));
  wifi_sta_ip_stack_memory          = App_malloc(WIFI_STA_IP_STACK_SIZE);
  wifi_sta_arp_memory               = App_malloc(NX_ARP_CACHE_SIZE);


  if ((wifi_sta_ip_ptr == NULL) || (wifi_sta_ip_stack_memory == NULL) || (wifi_sta_arp_memory == NULL)) goto _err;

  if (WIFI_STA_Join()==RES_ERROR) goto _err;


  WIFI_STA_get_IP_proprties();

  status = nx_ip_create(wifi_sta_ip_ptr,
                        "WIFI STA IP Instance",
                        wifi_sta_net_props.ip_address,
                        wifi_sta_net_props.network_mask,
                        &net_packet_pool,
                        WHD_STA_NetXDuo_driver_entry,
                        wifi_sta_ip_stack_memory,
                        WIFI_STA_IP_STACK_SIZE,
                        3);
  if (status != NX_SUCCESS)
  {
    APPLOG("WIFI STA. IP creation error %04X", status)
    goto _err;
  }
  APPLOG("WiFI STA. IP created successfully");

  // Инициализация структуры и назначение callback-ов связанных с протоколом ARP
  status |= nx_ip_gateway_address_set(wifi_sta_ip_ptr, wifi_sta_net_props.gateway_address);
  status |= nx_arp_enable(wifi_sta_ip_ptr, wifi_sta_arp_memory, NX_ARP_CACHE_SIZE);
  status |= nx_tcp_enable(wifi_sta_ip_ptr);
  status |= nx_udp_enable(wifi_sta_ip_ptr);
  status |= nx_icmp_enable(wifi_sta_ip_ptr);
  status |= nx_ip_fragment_enable(wifi_sta_ip_ptr);
  if (status != NX_SUCCESS) goto _err2;

  APPLOG("WIFI STA. ARP, TCP, UDP, ICMP, IGMP, IP frag. started successfully.");


  return RES_OK;
_err2:
  nx_ip_delete(wifi_sta_ip_ptr);
_err:
  App_free(wifi_sta_ip_ptr);
  App_free(wifi_sta_ip_stack_memory);
  App_free(wifi_sta_arp_memory);
  return RES_ERROR;

}




/*-----------------------------------------------------------------------------------------------------
  Запуск DHCP клиента для получения IP адреса от хоста.
  Используется только в режиме станции


  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t WIFI_STA_DHCP_client_start(void)
{

  UINT      status;
  UINT      actual_status;
  ULONG     ip_address;
  ULONG     network_mask;
  ULONG     server_address;


  nx_ip_address_set(wifi_sta_ip_ptr,0,0);

  status = nx_dhcp_create(&wifi_sta_dhcp_client,wifi_sta_ip_ptr,(CHAR *)ivar.this_host_name);
  if (status != NX_SUCCESS)
  {
    APPLOG("WIFI STA. nx_dhcp_create error %d", status);
    return RES_ERROR;
  }
  APPLOG("WIFI STA. DHCP client created");


  nx_dhcp_packet_pool_set(&wifi_sta_dhcp_client,&net_packet_pool);

  status = nx_dhcp_start(&wifi_sta_dhcp_client);
  if (status != NX_SUCCESS)
  {
    nx_dhcp_delete(&wifi_sta_dhcp_client);
    APPLOG("WIFI STA. nx_dhcp_start error %d", status);
    return RES_ERROR;
  }
  APPLOG("WIFI STA. DHCP client started");

  status = nx_ip_status_check(wifi_sta_ip_ptr, NX_IP_ADDRESS_RESOLVED,(ULONG *)&actual_status, 100000);
  if (status != NX_SUCCESS)
  {
    nx_dhcp_delete(&wifi_sta_dhcp_client);
    APPLOG("WIFI STA. nx_ip_status_check error %d", status);
    return RES_ERROR;
  }
  if (actual_status & NX_IP_ADDRESS_RESOLVED)
  {
    nx_ip_address_get(wifi_sta_ip_ptr,&ip_address,&network_mask);
    nx_dhcp_server_address_get(&wifi_sta_dhcp_client,&server_address);
    APPLOG("WIFI STA. DHCP client started");
    APPLOG("WIFI STA. DHCP IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d Server: %03d.%03d.%03d.%03d", IPADDR(ip_address), IPADDR(network_mask), IPADDR(server_address));
    return RES_OK;
  }
  else
  {
    nx_ip_address_set(wifi_sta_ip_ptr,wifi_sta_net_props.ip_address,wifi_sta_net_props.network_mask);
    nx_ip_gateway_address_set(wifi_sta_ip_ptr,wifi_sta_net_props.gateway_address);
    APPLOG("WIFI STA. DHCP IP addres don't resolved. Status = %04X", actual_status);
    APPLOG("WIFI STA. STATIC IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d Gateway: %03d.%03d.%03d.%03d", IPADDR(wifi_sta_net_props.ip_address), IPADDR(wifi_sta_net_props.network_mask), IPADDR(wifi_sta_net_props.gateway_address));
    return RES_OK;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return char*
-----------------------------------------------------------------------------------------------------*/
char* WIFI_STA_get_current_SSID(void)
{
  return (char*)WIFI_STA_Configs[current_sta_cfg].ssid;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается из задачи Task_Net

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_STA_start_network(void)
{
  ULONG ip_address;
  ULONG network_mask;

  if (wifi_sta_network_active == 1) return RES_OK;

  APPLOG("WIFI STA. Start network");

  if (*(WIFI_STA_Configs[current_sta_cfg].enable_dhcp)!=0)
  {
    WIFI_STA_DHCP_client_start();
  }
  TELNET_server_create(wifi_sta_ip_ptr,&telnet_server, "WIFI_Telnet");
  Net_FTP_server_create(wifi_sta_ip_ptr, "WIFI_FTP_server");
  MATLAB_connection_server_create(wifi_sta_ip_ptr);
  mDNS_start(wifi_sta_ip_ptr, &mdns);

  nx_ip_address_get(wifi_sta_ip_ptr,&ip_address,&network_mask);
  APPLOG("WIFI STA. Interface IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d", IPADDR(ip_address), IPADDR(network_mask));


  Send_Net_task_event(EVT_NET_IP_READY); // Передаем сообщение основной задаче о готовности сетевого стека к работе

  wifi_sta_network_active  =1;
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void WIFI_STA_network_controller(void)
{
  if (WIFI_STA_network_active_flag() == 0)
  {
    WIFI_STA_start_network();
  }
}

