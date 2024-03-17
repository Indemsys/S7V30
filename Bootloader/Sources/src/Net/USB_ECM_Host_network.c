﻿// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-07-10
// 14:09:24
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "nxd_dhcp_client.h"
#include   "nxd_telnet_server.h"



NX_IP                      *ecm_host_ip_ptr;
static T_app_net_props     ecm_host_net_props;

#define                    ECM_HOST_IP_STACK_SIZE             2048
#define                    ECM_HOST_DHCP_SERVER_STACK_SIZE    1024

NX_DHCP                    ecm_dhcp_client BSP_PLACE_IN_SECTION_V2(".stack.DHCP_ECM")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);


uint8_t                    *ecm_host_ip_stack_memory;
uint8_t                    *ecm_host_ip_arp_cache_memory;
uint8_t                    *ecm_host_dhcp_server_stack_memory;

static uint8_t             ecm_host_network_active;
static uint8_t             ecm_host_dhcp_active;


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _ECM_get_IP_proprties(void)
{
  Str_to_IP_v4((const char *)ivar.default_ip_addr,(uint8_t *)&ecm_host_net_props.ip_address);
  Str_to_IP_v4((const char *)ivar.default_net_mask,(uint8_t *)&ecm_host_net_props.network_mask);
  Str_to_IP_v4((const char *)ivar.default_gateway_addr,(uint8_t *)&ecm_host_net_props.gateway_address);
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая при изменении статуса соединения
  У интерфейса USB не вызывается

  \param ip_ptr
  \param interface_index
  \param link_up  - 0 - соединение отключено, 1 - соединение включено
-----------------------------------------------------------------------------------------------------*/
static void _ECM_host_link_status_change_callback(NX_IP *ip_ptr, UINT interface_index, UINT link_up)
{

}


/*-----------------------------------------------------------------------------------------------------
  Вызывается из задачи main_thread_entry

  Вся нициализация в этой процедуре должна быть выполнена до того как активизируется ECM Host, иначе соединения не образуется
  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t ECM_Host_init_network_stack(NX_IP **ip_p_ptr)
{
  UINT status;

  ecm_host_ip_ptr                   = App_malloc(sizeof(NX_IP));
  ecm_host_ip_stack_memory          = App_malloc(ECM_HOST_IP_STACK_SIZE);
  ecm_host_ip_arp_cache_memory      = App_malloc(NX_ARP_CACHE_SIZE);
  ecm_host_dhcp_server_stack_memory = App_malloc(ECM_HOST_DHCP_SERVER_STACK_SIZE);

  if ((ecm_host_ip_ptr == TX_NULL) || (ecm_host_ip_stack_memory == TX_NULL) || (ecm_host_ip_arp_cache_memory == TX_NULL) || (ecm_host_dhcp_server_stack_memory == TX_NULL))
  {
    App_free(ecm_host_ip_ptr);
    App_free(ecm_host_ip_stack_memory);
    App_free(ecm_host_ip_arp_cache_memory);
    App_free(ecm_host_dhcp_server_stack_memory);
    return RES_ERROR;
  }


  ux_network_driver_init();  // Запускаем сетевой драйвер чере USB


  _ECM_get_IP_proprties();

  /* Create an IP instance. */

  status = nx_ip_create(ecm_host_ip_ptr,
                        "ECM IP Instance",
                        ecm_host_net_props.ip_address,
                        ecm_host_net_props.network_mask,
                        &net_packet_pool,
                        _ux_network_driver_entry,
                        &ecm_host_ip_stack_memory[0],
                        ECM_HOST_IP_STACK_SIZE,
                        3);
  if (NX_SUCCESS != status)
  {
    APPLOG("ECM. IP error %04X", status);
    return RES_ERROR;
  }
  APPLOG("ECM. IP created successfully");

  status = nx_arp_enable(ecm_host_ip_ptr,&ecm_host_ip_arp_cache_memory[0], NX_ARP_CACHE_SIZE);
  if (NX_SUCCESS != status)
  {
    APPLOG("ECM. ARP error %04X", status);
  }

  status = nx_tcp_enable(ecm_host_ip_ptr);
  if (NX_SUCCESS != status)
  {
    APPLOG("ECM. TCP error %04X", status);
  }

  status = nx_udp_enable(ecm_host_ip_ptr);
  if (NX_SUCCESS != status)
  {
    APPLOG("ECM. UDP error %04X", status);
  }

  status = nx_icmp_enable(ecm_host_ip_ptr);
  if (NX_SUCCESS != status)
  {
    APPLOG("ECM. ICMP error %04X", status);
  }

  status = nx_ip_fragment_enable(ecm_host_ip_ptr);
  if (NX_SUCCESS != status)
  {
    APPLOG("ECM. IP fragment. error %04X", status);  // NX_NOT_ENABLED
  }

  /* Gateway IP Address */
  status = nx_ip_gateway_address_set(ecm_host_ip_ptr, ecm_host_net_props.gateway_address);

  if (NX_SUCCESS != status)
  {
    APPLOG("ECM. set gateway error %04X", status);
  }

  // Установка callback, который вызывается в цепочке _nx_ip_thread_entry -> _nx_ip_deferred_link_status_process после события NX_IP_LINK_STATUS_EVENT
  // Данное событие отправляет функция _nx_ip_driver_link_status_event
  // Переменная linkup передаваемая в callback функция получает свое значение во время выполнения _nx_ip_deferred_link_status_process из функции драйвера
  status = nx_ip_link_status_change_notify_set(ecm_host_ip_ptr, _ECM_host_link_status_change_callback);
  if (NX_SUCCESS != status)
  {
    APPLOG("ECM. notify set error %04X", status);
  }

  *ip_p_ptr = ecm_host_ip_ptr;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t ECM_Host_network_active_flag(void)
{
  return ecm_host_network_active;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается из задачи Task_Net

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ECM_Host_start_network(void)
{

  if (ecm_host_network_active == 1) return RES_OK;

  ECM_DEBUG_LOG("ECM. Start network");
  APPLOG("ECM. Start network");


  ECM_DHCP_client_start();

  TELNET_server_create(ecm_host_ip_ptr,&telnet_server, "ECM_Telnet");
  Net_FTP_server_create(ecm_host_ip_ptr, "ECM_FTP_server");
  mDNS_start(ecm_host_ip_ptr, &mdns);


  {
    ULONG ip_address;
    ULONG network_mask;
    nx_ip_address_get(ecm_host_ip_ptr,&ip_address,&network_mask);
    ECM_DEBUG_LOG("ECM interface IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d", IPADDR(ip_address), IPADDR(network_mask));
    APPLOG("ECM interface IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d", IPADDR(ip_address), IPADDR(network_mask));
  }

  Send_Net_task_event(EVT_NET_IP_READY); // Передаем сообщение основной задаче о готовности сетевого стека к работе
  ecm_host_network_active  =1;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Запуск DHCP клиента для получения IP адреса от хоста.


  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ECM_DHCP_client_start(void)
{
  UINT      status;
  UINT      actual_status;
  ULONG     ip_address;
  ULONG     network_mask;
  ULONG     server_address;

  if (ecm_host_dhcp_active != 0) return RES_OK;
  if (ivar.en_dhcp_client == 0)  return RES_ERROR;

  nx_ip_address_set(ecm_host_ip_ptr,0,0);

  status = nx_dhcp_create(&ecm_dhcp_client,ecm_host_ip_ptr,(CHAR *)ivar.this_host_name);
  if (status != NX_SUCCESS)
  {

    ECM_DEBUG_LOG("ECM. DHCP client creation error %d", status);
    APPLOG("ECM. DHCP client creation error %d", status);
    return RES_ERROR;
  }


  nx_dhcp_packet_pool_set(&ecm_dhcp_client,&net_packet_pool);

  status = nx_dhcp_start(&ecm_dhcp_client);
  if (status != NX_SUCCESS)
  {
    nx_dhcp_delete(&ecm_dhcp_client);
    ECM_DEBUG_LOG("ECM. DHCP client start error %d", status);
    APPLOG("ECM. DHCP client start error %d", status);
    return RES_ERROR;
  }

  APPLOG("ECM DHCP is started.");

  status = nx_ip_status_check(ecm_host_ip_ptr, NX_IP_ADDRESS_RESOLVED,(ULONG *)&actual_status, 100000);
  if (status != NX_SUCCESS)
  {
    nx_dhcp_delete(&ecm_dhcp_client);
    ECM_DEBUG_LOG("ECM. IP status error %d", status);
    APPLOG("ECM. IP status error %d", status);
    return RES_ERROR;
  }
  if (actual_status & NX_IP_ADDRESS_RESOLVED)
  {
    nx_ip_address_get(ecm_host_ip_ptr,&ip_address,&network_mask);
    nx_dhcp_server_address_get(&ecm_dhcp_client,&server_address);

    ECM_DEBUG_LOG("ECM DHCP addres is resolved");
    ECM_DEBUG_LOG("ECM IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d Server: %03d.%03d.%03d.%03d", INTF0IPADDR((*ecm_host_ip_ptr)), IPADDR(network_mask), IPADDR(server_address));

    APPLOG("ECM DHCP addres is resolved");
    APPLOG("ECM IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d Server: %03d.%03d.%03d.%03d", INTF0IPADDR((*ecm_host_ip_ptr)), IPADDR(network_mask), IPADDR(server_address));
    ecm_host_dhcp_active = 1;
    return RES_OK;
  }
  else
  {
    nx_dhcp_delete(&ecm_dhcp_client);
    nx_ip_address_set(ecm_host_ip_ptr,ecm_host_net_props.ip_address,ecm_host_net_props.network_mask);
    nx_ip_gateway_address_set(ecm_host_ip_ptr,ecm_host_net_props.gateway_address);

    ECM_DEBUG_LOG("ECM. IP addres don't resolved. Status = %04X", actual_status);
    ECM_DEBUG_LOG("ECM IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d Gateway: %03d.%03d.%03d.%03d", IPADDR(ecm_host_net_props.ip_address), IPADDR(ecm_host_net_props.network_mask), IPADDR(ecm_host_net_props.gateway_address));

    APPLOG("ECM. IP addres don't resolved. Status = %04X", actual_status);
    APPLOG("ECM IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d Gateway: %03d.%03d.%03d.%03d", IPADDR(ecm_host_net_props.ip_address), IPADDR(ecm_host_net_props.network_mask), IPADDR(ecm_host_net_props.gateway_address));
    return RES_OK;
  }


}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t ECM_DHCP_client_stop(void)
{
  UINT      status;
  if (ecm_host_dhcp_active == 0) return RES_OK;
  status = nx_dhcp_delete(&ecm_dhcp_client);
  nx_ip_address_set(ecm_host_ip_ptr,ecm_host_net_props.ip_address,ecm_host_net_props.network_mask);
  nx_ip_gateway_address_set(ecm_host_ip_ptr,ecm_host_net_props.gateway_address);
  ecm_host_dhcp_active = 0;
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ECM_Host_stop_network(void)
{
  if (ecm_host_network_active == 0) return RES_OK;

  ECM_DEBUG_LOG("ECM. Stop network");
  APPLOG("ECM. Stop network");


  TELNET_server_delete(&telnet_server);

  mDNS_delete(&mdns);

  Net_FTP_server_delete();

  ECM_DHCP_client_stop();


  ecm_host_network_active  =0;

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Следим и поддерживаем соединение по  RNDIS


  \param void
-----------------------------------------------------------------------------------------------------*/
void ECM_Host_network_controller(void)
{
  if (Is_ECM_usb_link_up() == NX_TRUE)
  {
    ECM_Host_start_network();
  }
  else
  {
    ECM_Host_stop_network();
  }
}
