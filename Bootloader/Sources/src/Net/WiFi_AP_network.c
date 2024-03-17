// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-05-15
// 12:26:03
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

uint8_t   wifi_ap_ip_stack_memory[WIFI_AP_IP_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.WiFi_AP")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);


T_app_net_props          wifi_ap_net_props;
NX_IP                   *wifi_ap_ip_ptr;
uint8_t                 *wifi_ap_arp_memory;
uint8_t                 *wifi_ap_dhcp_server_stack_memory;
static uint8_t           wifi_ap_network_active;
uint8_t                  wifi_ap_dhcp_server_active;

NX_DHCP_SERVER           wifi_ap_dhcp_server;
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t  0 - если сеть не активна, 1 - если сеть активна
-----------------------------------------------------------------------------------------------------*/
uint8_t WIFI_AP_network_active_flag(void)
{
  return wifi_ap_network_active;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void WIFI_AP_get_IP_proprties(void)
{
  if (ivar.wifi_ap_addr_assign_method == IP_ADDRESS_ASSIGNMENT_METHOD_PRECONFIGURED_DHCP_SERVER)
  {
    Str_to_IP_v4((char const *)ivar.ap_default_ip_addr,(uint8_t *)&wifi_ap_net_props.ip_address);
    Str_to_IP_v4((char const *)ivar.ap_default_net_mask,(uint8_t *)&wifi_ap_net_props.network_mask);
    wifi_ap_net_props.gateway_address  = wifi_ap_net_props.ip_address;
    wifi_ap_net_props.dhcp_dns_ip      = wifi_ap_net_props.ip_address;
    wifi_ap_net_props.dhcp_subnet_mask = wifi_ap_net_props.network_mask;
    Str_to_IP_v4((char const *)ivar.ap_dhcp_serv_start_ip,(uint8_t *)&wifi_ap_net_props.dhcp_start_ip);
    wifi_ap_net_props.dhcp_end_ip = wifi_ap_net_props.dhcp_start_ip + DHCP_SERVER_ADDITIONAL_ADDR_NUM;
  }
  else if (ivar.wifi_ap_addr_assign_method == IP_ADDRESS_ASSIGNMENT_METHOD_WINDOWS_HOME_NETWORK)
  {
    Str_to_IP_v4("192.168.137.2",(uint8_t *)&wifi_ap_net_props.ip_address);
    Str_to_IP_v4("255.255.255.0",(uint8_t *)&wifi_ap_net_props.network_mask);
    Str_to_IP_v4("192.168.137.1",(uint8_t *)&wifi_ap_net_props.gateway_address);
    Str_to_IP_v4("255.255.255.0",(uint8_t *)&wifi_ap_net_props.dhcp_subnet_mask);
    Str_to_IP_v4("192.168.137.1",(uint8_t *)&wifi_ap_net_props.dhcp_dns_ip);
    Str_to_IP_v4("192.168.137.1",(uint8_t *)&wifi_ap_net_props.dhcp_start_ip);
    wifi_ap_net_props.dhcp_end_ip = wifi_ap_net_props.dhcp_start_ip + DHCP_SERVER_ADDITIONAL_ADDR_NUM;
  }
}



/*-----------------------------------------------------------------------------------------------------

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t WIFI_AP_create_DHCP_server(NX_IP  *ip_ptr)
{
  UINT status;
  UINT addresses_added;

  nx_ip_address_set(ip_ptr, wifi_ap_net_props.ip_address, wifi_ap_net_props.dhcp_subnet_mask);

  status = nx_dhcp_server_create(&wifi_ap_dhcp_server,
                                 ip_ptr,
                                 wifi_ap_dhcp_server_stack_memory,
                                 WIFI_DHCP_SERVER_STACK_SIZE,
                                 "WIFI AP DHCP Server",
                                 &net_packet_pool);

  if (NX_SUCCESS != status)
  {
    APPLOG("WIFI AP. Failed to create DHCP server. Error %d", status);
    return RES_ERROR;
  }
  wifi_ap_dhcp_server_active = 1;

  status = nx_dhcp_create_server_ip_address_list(&wifi_ap_dhcp_server, 0, wifi_ap_net_props.dhcp_start_ip, wifi_ap_net_props.dhcp_end_ip,&addresses_added);
  if (NX_SUCCESS != status)
  {
    APPLOG("WIFI AP. Failed to create DHCP server address list. Error %d", status);
    return RES_ERROR;
  }
  else
  {
    APPLOG("WIFI AP. DHCP server address list created.  %d addresses added", addresses_added);
  }

  status = nx_dhcp_set_interface_network_parameters(&wifi_ap_dhcp_server, 0,wifi_ap_net_props.dhcp_subnet_mask, wifi_ap_net_props.ip_address, wifi_ap_net_props.dhcp_dns_ip);
  if (NX_SUCCESS != status)
  {
    APPLOG("WIFI AP. Failed to set DHCP server net pareameters. Error %d", status);
    return RES_ERROR;
  }

  status = nx_dhcp_server_start(&wifi_ap_dhcp_server);
  if (NX_SUCCESS != status)
  {
    APPLOG("WIFI AP. Failed to start DHCP server. Error %d", status);
    return RES_ERROR;
  }
  else
  {
    APPLOG("WIFI AP. DHCP server started");
    APPLOG("WIFI AP. host IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d Gateway: %03d.%03d.%03d.%03d", IPADDR(wifi_ap_net_props.ip_address), IPADDR(wifi_ap_net_props.dhcp_subnet_mask), IPADDR(wifi_ap_net_props.gateway_address));
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_AP_init_TCP_stack(void)
{
  UINT status;
  whd_interface_t  ap_ifp;

  wifi_ap_ip_ptr                    = App_malloc(sizeof(NX_IP));
  wifi_ap_arp_memory                = App_malloc(NX_ARP_CACHE_SIZE);
  wifi_ap_dhcp_server_stack_memory  = App_malloc(WIFI_DHCP_SERVER_STACK_SIZE);


  if ((wifi_ap_ip_ptr == NULL)  || (wifi_ap_arp_memory == NULL) || (wifi_ap_dhcp_server_stack_memory == NULL))  goto _err;

  if (wifi_composition == WIFI_STA_AND_AP) ap_ifp = g_sec_ifp;
  else ap_ifp = g_pri_ifp;

  whd_ssid_t   ssid;
  strncpy((char *)ssid.value, (char *)ivar.wifi_ap_ssid, SSID_NAME_SIZE-1);
  ssid.length = strlen((char *)ivar.wifi_ap_ssid);

  if (whd_wifi_init_ap(ap_ifp,&ssid, WHD_SECURITY_WPA2_WPA_AES_PSK, ivar.wifi_ap_key, strlen((char *)ivar.wifi_ap_key), ivar.wifi_ap_channel) != WHD_SUCCESS) goto _err;

  if (whd_wifi_start_ap(ap_ifp) != WHD_SUCCESS) goto _err;


  WIFI_AP_get_IP_proprties();

  status = nx_ip_create(wifi_ap_ip_ptr,
                        "WIFI AP IP Instance",
                        wifi_ap_net_props.ip_address,
                        wifi_ap_net_props.network_mask,
                        &net_packet_pool,
                        WHD_AP_NetXDuo_driver_entry,
                        wifi_ap_ip_stack_memory,
                        WIFI_AP_IP_STACK_SIZE,
                        3);

  if (status != NX_SUCCESS)
  {
    APPLOG("WIFI AP. IP creation error %04X", status)
    goto _err;
  }
  APPLOG("WiFI AP. IP created successfully");


  // Инициализация структуры и назначение callback-ов связанных с протоколом ARP
  status |= nx_ip_gateway_address_set(wifi_ap_ip_ptr, wifi_ap_net_props.gateway_address);
  status |= nx_arp_enable(wifi_ap_ip_ptr, wifi_ap_arp_memory, NX_ARP_CACHE_SIZE);
  status |= nx_tcp_enable(wifi_ap_ip_ptr);
  status |= nx_udp_enable(wifi_ap_ip_ptr);
  status |= nx_icmp_enable(wifi_ap_ip_ptr);
  //status |= nx_igmp_enable(wifi_ap_ip_ptr);       // Включение этого протокола нарушает работу FTP
  status |= nx_ip_fragment_enable(wifi_ap_ip_ptr);
  if (status != NX_SUCCESS) goto _err2;

  APPLOG("WIFI. ARP, TCP, UDP, ICMP, IGMP, IP frag. started successfully.");

  return RES_OK;
_err2:
  nx_ip_delete(wifi_ap_ip_ptr);
_err:
  App_free(wifi_ap_ip_ptr);
  App_free(wifi_ap_arp_memory);
  App_free(wifi_ap_dhcp_server_stack_memory);
  APPLOG("WiFI. AP initialisation failed");
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается из задачи Task_Net

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_AP_start_network(void)
{
  ULONG ip_address;
  ULONG network_mask;

  if (wifi_ap_network_active == 1) return RES_OK;

  APPLOG("WIFI AP. Start network");

  if (ivar.wifi_ap_addr_assign_method == IP_ADDRESS_ASSIGNMENT_METHOD_PRECONFIGURED_DHCP_SERVER)
  {
    WIFI_AP_create_DHCP_server(wifi_ap_ip_ptr);
  }
  TELNET_server_create(wifi_ap_ip_ptr,&telnet_server, "WIFI_Telnet");
  Net_FTP_server_create(wifi_ap_ip_ptr, "WIFI_FTP_server");

  nx_ip_address_get(wifi_ap_ip_ptr,&ip_address,&network_mask);
  APPLOG("WIFI AP. Interface IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d", IPADDR(ip_address), IPADDR(network_mask));


  Send_Net_task_event(EVT_NET_IP_READY); // Передаем сообщение основной задаче о готовности сетевого стека к работе

  wifi_ap_network_active  =1;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_AP_stop_network(void)
{
  UINT status;

  if (wifi_ap_network_active == 0) return RES_OK;

  APPLOG("WIFI AP. Stop network");

  TELNET_server_delete(&telnet_server);

  status = nx_dhcp_server_delete(&wifi_ap_dhcp_server);
  APPLOG("WIFI AP. DHCP server delete result: %d", status);

  Net_FTP_server_delete();

  wifi_ap_network_active  =0;

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Следим и поддерживаем соединение по  RNDIS


  \param void
-----------------------------------------------------------------------------------------------------*/
void WIFI_AP_network_controller(void)
{
  if (WIFI_AP_network_active_flag() == 0)
  {
    WIFI_AP_start_network();
  }
}

