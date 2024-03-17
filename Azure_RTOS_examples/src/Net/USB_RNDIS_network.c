// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-09-30
// 14:40:09
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "USB_descriptors.h"
#include   "nxd_dhcp_server.h"
#include   "ux_device_class_rndis.h"
#include   "ux_device_class_cdc_ecm.h"
#include   "nxd_telnet_server.h"


static UX_SLAVE_CLASS_RNDIS_PARAMETER   rndis_parameter;
static UX_SLAVE_CLASS_CDC_ECM_PARAMETER cdc_ecm_parameter;

static uint8_t             rndis_interface_active;
static uint8_t             rndis_network_active;
static uint8_t             rndis_dhcp_server_active;

NX_IP                      *rndis_ip_ptr;
static T_app_net_props     rndis_net_props;

#define                    RNDIS_IP_STACK_SIZE             2048
#define                    RNDIS_DHCP_SERVER_STACK_SIZE    1024



uint8_t                    *rndis_ip_stack_memory;
uint8_t                    *rndis_ip_arp_cache_memory;
uint8_t                    *rndis_dhcp_server_stack_memory;

static NX_DHCP_SERVER      rndis_dhcp_server;


volatile uint32_t          g_sof_cnt;

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте прерывания usbfs_int_isr в момент активизации класса RNDIS (подключения разъема или после подачи питания с уже подключенным разъемом)

  \param arg
-----------------------------------------------------------------------------------------------------*/
static void RNDIS_instance_activate_callback(void *arg)
{
  rndis_interface_active = 1;
  g_uinf.inserted        = 1;
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте прерывания usbfs_int_isr в момент деактивизации класса RNDIS (отключения разъема)

  \param arg
-----------------------------------------------------------------------------------------------------*/
static void RNDIS_instance_deactivate_callback(void *arg)
{
  rndis_interface_active = 0;
  g_uinf.inserted        = 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t Is_RNDIS_interface_active(void)
{
  return rndis_interface_active;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t  0 - если сеть не активна, 1 - если сеть активна
-----------------------------------------------------------------------------------------------------*/
uint8_t RNDIS_network_active_flag(void)
{
  return rndis_network_active;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
UINT Register_rndis_class(void)
{
  UINT               ret = UX_SUCCESS;
  rndis_parameter.ux_slave_class_rndis_instance_activate   = RNDIS_instance_activate_callback;
  rndis_parameter.ux_slave_class_rndis_instance_deactivate = RNDIS_instance_deactivate_callback;

  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[0] = 0x00;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[1] = 0x1e;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[2] = 0x58;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[3] = 0x41;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[4] = 0xb8;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[5] = 0x78;

  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[0] = 0x00;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[1] = 0x1e;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[2] = 0x58;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[3] = 0x41;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[4] = 0xb8;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[5] = 0x79;

  rndis_parameter.ux_slave_class_rndis_parameter_vendor_id      = 0x045B;
  rndis_parameter.ux_slave_class_rndis_parameter_driver_version = 0x0003;

  g_uinf.idVendor  = DEVICE_VID2;
  g_uinf.idProduct = DEVICE_PID2;


  ux_utility_memory_copy(rndis_parameter.ux_slave_class_rndis_parameter_vendor_description,"RNDIS", 5);

  ret = ux_device_stack_class_register(_ux_system_slave_class_rndis_name, ux_device_class_rndis_entry, 1, USBD_RNDIS_INTERFACE_INDEX, &rndis_parameter);
  return ret;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
UINT Register_cdc_ecm_class(void)
{
  UINT               ret = UX_SUCCESS;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_instance_activate   = RNDIS_instance_activate_callback;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_instance_deactivate = RNDIS_instance_deactivate_callback;

  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[0] = 0x00;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[1] = 0x02;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[2] = 0x02;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[3] = 0x03;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[4] = 0x00;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[5] = 0x00;

  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[0] = 0x00;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[1] = 0x02;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[2] = 0x02;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[3] = 0x03;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[4] = 0x00;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[5] = 0x00;

  ret = ux_device_stack_class_register(_ux_system_slave_class_cdc_ecm_name, ux_device_class_cdc_ecm_entry, 1, USBD_RNDIS_INTERFACE_INDEX, &cdc_ecm_parameter);
  return ret;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _RNDIS_get_IP_proprties(void)
{
  if (ivar.rndis_ip_addr_assign_method == IP_ADDRESS_ASSIGNMENT_METHOD_PRECONFIGURED_DHCP_SERVER)
  {
    Str_to_IP_v4("192.168.3.1",(uint8_t *)&rndis_net_props.ip_address);
    Str_to_IP_v4("255.255.255.0",(uint8_t *)&rndis_net_props.network_mask);
    Str_to_IP_v4("192.168.3.2",(uint8_t *)&rndis_net_props.gateway_address);
    Str_to_IP_v4("255.255.255.0",(uint8_t *)&rndis_net_props.dhcp_subnet_mask);
    Str_to_IP_v4("192.168.3.2",(uint8_t *)&rndis_net_props.dhcp_dns_ip);
    Str_to_IP_v4("192.168.3.2",(uint8_t *)&rndis_net_props.dhcp_start_ip);
    rndis_net_props.dhcp_end_ip = rndis_net_props.dhcp_start_ip + DHCP_SERVER_ADDITIONAL_ADDR_NUM;
  }
  else if (ivar.rndis_ip_addr_assign_method == IP_ADDRESS_ASSIGNMENT_METHOD_WINDOWS_HOME_NETWORK)
  {
    Str_to_IP_v4("192.168.137.2",(uint8_t *)&rndis_net_props.ip_address);
    Str_to_IP_v4("255.255.255.0",(uint8_t *)&rndis_net_props.network_mask);
    Str_to_IP_v4("192.168.137.1",(uint8_t *)&rndis_net_props.gateway_address);
    Str_to_IP_v4("255.255.255.0",(uint8_t *)&rndis_net_props.dhcp_subnet_mask);
    Str_to_IP_v4("192.168.137.1",(uint8_t *)&rndis_net_props.dhcp_dns_ip);
    Str_to_IP_v4("192.168.137.1",(uint8_t *)&rndis_net_props.dhcp_start_ip);
    rndis_net_props.dhcp_end_ip = rndis_net_props.dhcp_start_ip + DHCP_SERVER_ADDITIONAL_ADDR_NUM;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param mac_str
  \param max_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t RNDIS_Get_MAC(char* mac_str, uint32_t max_str_len)
{
  mac_str[0] = 0;

  uint8_t *m = (uint8_t*)rndis_parameter.ux_slave_class_rndis_parameter_local_node_id;
  // К этому моменту уже была выполнена функция _ux_network_driver_activate
  snprintf(mac_str, max_str_len, "%02X:%02X:%02X:%02X:%02X:%02X", m[0],m[1],m[2],m[3],m[4],m[5]);
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param ip_str
  \param max_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t RNDIS_Get_MASK_IP(char* ip_str, char* mask_str,uint32_t max_str_len)
{
  ULONG                    ip_address;
  ULONG                    network_mask;

  ip_str[0] = 0;
  mask_str[0] = 0;

  if (RNDIS_network_active_flag() == 0) return RES_ERROR;
  nx_ip_address_get(rndis_ip_ptr,&ip_address,&network_mask);
  snprintf(ip_str, max_str_len, "%03d.%03d.%03d.%03d", IPADDR(ip_address));
  snprintf(mask_str, max_str_len, "%03d.%03d.%03d.%03d", IPADDR(network_mask));
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param gate_str
  \param max_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t RNDIS_Get_Gateway_IP(char* gate_str,uint32_t max_str_len)
{
  ULONG                    ip_address;

  gate_str[0] = 0;
  if (RNDIS_network_active_flag() == 0) return RES_ERROR;

  nx_ip_gateway_address_get(rndis_ip_ptr,&ip_address);
  snprintf(gate_str, max_str_len, "%03d.%03d.%03d.%03d", IPADDR(ip_address));
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _RNDIS_create_DHCP_server(void)
{
  UINT status;
  UINT addresses_added;

  nx_ip_address_set(rndis_ip_ptr, rndis_net_props.ip_address, rndis_net_props.dhcp_subnet_mask);

  status = nx_dhcp_server_create(&rndis_dhcp_server,
       rndis_ip_ptr,
       &rndis_dhcp_server_stack_memory[0],
       RNDIS_DHCP_SERVER_STACK_SIZE,
       "RNDIS DHCP Server",
       &net_packet_pool);

  if (NX_SUCCESS != status)
  {
    APPLOG("RNDIS. Failed to create RNDIS DHCP server. Error %d", status);
    return RES_ERROR;
  }
  rndis_dhcp_server_active = 1;

  status = nx_dhcp_create_server_ip_address_list(&rndis_dhcp_server, 0, rndis_net_props.dhcp_start_ip, rndis_net_props.dhcp_end_ip,&addresses_added);
  if (NX_SUCCESS != status)
  {
    APPLOG("RNDIS. Failed to create RNDIS DHCP server address list. Error %d", status);
    return RES_ERROR;
  }
  else
  {
    APPLOG("RNDIS.DHCP server address list created.  %d addresses added", addresses_added);
  }

  status = nx_dhcp_set_interface_network_parameters(&rndis_dhcp_server, 0,rndis_net_props.dhcp_subnet_mask, rndis_net_props.ip_address, rndis_net_props.dhcp_dns_ip);
  if (NX_SUCCESS != status)
  {
    APPLOG("RNDIS. Failed to set RNDIS DHCP server net pareameters. Error %d", status);
    return RES_ERROR;
  }

  status = nx_dhcp_server_start(&rndis_dhcp_server);
  if (NX_SUCCESS != status)
  {
    APPLOG("RNDIS. Failed to start RNDIS DHCP server. Error %d", status);
    return RES_ERROR;
  }
  else
  {
    APPLOG("RNDIS DHCP server started");
    APPLOG("RNDIS host IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d Gateway: %03d.%03d.%03d.%03d", IPADDR(rndis_net_props.ip_address), IPADDR(rndis_net_props.dhcp_subnet_mask), IPADDR(rndis_net_props.gateway_address));
  }
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая при изменении статуса соединения
  У интерфейса USB не вызывается

  \param ip_ptr
  \param interface_index
  \param link_up  - 0 - соединение отключено, 1 - соединение включено
-----------------------------------------------------------------------------------------------------*/
static void _RNDIS_link_status_change_callback(NX_IP *ip_ptr, UINT interface_index, UINT link_up)
{

}

/*-----------------------------------------------------------------------------------------------------
  Вызывается из задачи main_thread_entry

  Вся нициализация в этой процедуре должна быть выполнена до того как активизируется RNDIS, иначе соединения не образуется
  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t RNDIS_init_network_stack(NX_IP **ip_p_ptr)
{
  UINT status;

  rndis_ip_ptr                   = App_malloc(sizeof(NX_IP));
  rndis_ip_stack_memory          = App_malloc(RNDIS_IP_STACK_SIZE);
  rndis_ip_arp_cache_memory      = App_malloc(NX_ARP_CACHE_SIZE);
  rndis_dhcp_server_stack_memory = App_malloc(RNDIS_DHCP_SERVER_STACK_SIZE);

  if ((rndis_ip_ptr == TX_NULL) ||(rndis_ip_stack_memory == TX_NULL) || (rndis_ip_arp_cache_memory==TX_NULL) || (rndis_dhcp_server_stack_memory== TX_NULL)) goto _err;

  ux_network_driver_init();  // Запускаем сетевой драйвер чере USB

  _RNDIS_get_IP_proprties();

  /* Create an IP instance. */
  status = nx_ip_create(rndis_ip_ptr,
       "RNDIS IP Instance",
       rndis_net_props.ip_address,
       rndis_net_props.network_mask,
       &net_packet_pool,
       _ux_network_driver_entry,
       &rndis_ip_stack_memory[0],
       RNDIS_IP_STACK_SIZE,
       3);
  if (NX_SUCCESS != status)
  {
    APPLOG("RNDIS. IP creation error %04X", status)
    goto _err;
  }
  APPLOG("RNDIS. IP created successfully");

  // Инициализация структуры и назначение callback-ов связанных с протоколом ARP
  status |= nx_ip_gateway_address_set(rndis_ip_ptr, rndis_net_props.gateway_address);
  status |= nx_arp_enable(rndis_ip_ptr, rndis_ip_arp_cache_memory, NX_ARP_CACHE_SIZE);
  status |= nx_tcp_enable(rndis_ip_ptr);
  status |= nx_udp_enable(rndis_ip_ptr);
  status |= nx_icmp_enable(rndis_ip_ptr);
  //status |= nx_igmp_enable(wifi_ap_ip_ptr);       // Включение этого протокола нарушает работу
  status |= nx_ip_fragment_enable(rndis_ip_ptr);
  if (status != NX_SUCCESS) goto _err2;

  APPLOG("RNDIS. ARP, TCP, UDP, ICMP, IGMP, IP frag. started successfully.");

  // Установка callback, который вызывается в цепочке _nx_ip_thread_entry -> _nx_ip_deferred_link_status_process после события NX_IP_LINK_STATUS_EVENT
  // Данное событие отправляет функция _nx_ip_driver_link_status_event
  // Переменная linkup передаваемая в callback функция получает свое значение во время выполнения _nx_ip_deferred_link_status_process из функции драйвера
  status = nx_ip_link_status_change_notify_set(rndis_ip_ptr,_RNDIS_link_status_change_callback);
  if (NX_SUCCESS != status)
  {
    APPLOG("RNDIS. notify set error %04X", status);
  }

  *ip_p_ptr = rndis_ip_ptr;
  return RES_OK;
_err2:
  nx_ip_delete(rndis_ip_ptr);
_err:
  App_free(rndis_ip_ptr);
  App_free(rndis_ip_stack_memory);
  App_free(rndis_ip_arp_cache_memory);
  App_free(rndis_dhcp_server_stack_memory);
  return RES_ERROR;

}


/*-----------------------------------------------------------------------------------------------------
  Вызывается из задачи Task_Net

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t RNDIS_start_network(void)
{

  if (rndis_network_active == 1) return RES_OK;

  APPLOG("RNDIS. Start network");

  if (ivar.rndis_ip_addr_assign_method == IP_ADDRESS_ASSIGNMENT_METHOD_PRECONFIGURED_DHCP_SERVER)
  {
    _RNDIS_create_DHCP_server();
  }
  TELNET_server_create(rndis_ip_ptr,&telnet_server, "RNDIS_Telnet");

  Net_FTP_server_create(rndis_ip_ptr, "RNDIS_FTP_server");

  MATLAB_connection_server_create(rndis_ip_ptr);

  {
    ULONG ip_address;
    ULONG network_mask;
    nx_ip_address_get(rndis_ip_ptr,&ip_address,&network_mask);
    APPLOG("RNDIS interface IP: %03d.%03d.%03d.%03d Mask: %03d.%03d.%03d.%03d", IPADDR(ip_address), IPADDR(network_mask));
  }

  Send_Net_task_event(EVT_NET_IP_READY); // Передаем сообщение основной задаче о готовности сетевого стека к работе

  rndis_network_active  =1;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t RNDIS_stop_network(void)
{
  UINT status;

  if (rndis_network_active == 0) return RES_OK;

  APPLOG("RNDIS. Stop network");

  // Удалять ip не надо, иначе при следующем подключении по USB не возникнет соедиенения
  //  res = nx_ip_delete(&rndis_ip);
  //  APPLOG("RNDIS. IP delete result: %d", res);


  TELNET_server_delete(&telnet_server);

  if (rndis_dhcp_server_active == 1)
  {
    status = nx_dhcp_server_delete(&rndis_dhcp_server);
    APPLOG("RNDIS. DHCP server delete result: %d", status);
    rndis_dhcp_server_active = 0;
  }

  Net_FTP_server_delete();

  MATLAB_connection_server_delete();

  rndis_network_active  =0;

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Следим и поддерживаем соединение по  RNDIS


  \param void
-----------------------------------------------------------------------------------------------------*/
void RNDIS_network_controller(void)
{
//  static uint32_t sof_cnt_prev = 0;
//  uint32_t  sof_cnt_curr = g_sof_cnt;
//
//  // Проверяем наличие соединения по USB и в случае его отсутствия разраваем соединение RNDIS
//  if (sof_cnt_curr == sof_cnt_prev)
//  {
//    if (Is_RNDIS_interface_active())
//    {
//      RNDIS_instance_deactivate_callback(0);
//    }
//  }
//  sof_cnt_prev = sof_cnt_curr;

  if ((Is_RNDIS_interface_active() == 1) && (RNDIS_network_active_flag() == 0))
  {
    RNDIS_start_network();
  }
  if ((Is_RNDIS_interface_active() == 0) && (RNDIS_network_active_flag() == 1))
  {
    RNDIS_stop_network();
  }
}
