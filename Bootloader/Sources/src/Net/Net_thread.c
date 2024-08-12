// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-11-07
// 15:40:27
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"

extern VOID    _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);

#define THREAD_NET_STACK_SIZE     (1024*4)
static uint8_t thread_net_stack[THREAD_NET_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.net_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD              net_thread;
static TX_EVENT_FLAGS_GROUP   net_flag;

#define IP_INTERFACES_STACK_SIZE  (2048)
NX_IP                         net_ip                                 BSP_PLACE_IN_SECTION_V2(".stack.net_ip") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
uint8_t                       net_arp_cache[NX_ARP_CACHE_SIZE]       BSP_PLACE_IN_SECTION_V2(".stack.net_arp_cache")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static uint8_t                net_ip_stack[IP_INTERFACES_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.net_ip_stack")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);


static void Thread_net(ULONG initial_input);

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return NX_IP*
-----------------------------------------------------------------------------------------------------*/
NX_IP *Net_get_ip_ptr(void)
{
  return &net_ip;
}


/*-----------------------------------------------------------------------------------------------------

  \param msg

  \return UINT
-----------------------------------------------------------------------------------------------------*/
uint32_t  Send_net_event(uint32_t event_flag)
{
  return  tx_event_flags_set(&net_flag, event_flag, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param event_flags
  \param timeout_ms

  \return uint32_t Возвращает битовоую маску обнаруженнызсобытий
-----------------------------------------------------------------------------------------------------*/
uint32_t Wait_net_event(uint32_t event_flags, uint32_t *actual_flags, uint32_t timeout_ms)
{
  if (timeout_ms == TX_NO_WAIT)
  {
    return tx_event_flags_get(&net_flag, event_flags, TX_OR_CLEAR, (ULONG *)actual_flags,  TX_NO_WAIT);
  }
  else
  {
    return tx_event_flags_get(&net_flag, event_flags, TX_OR_CLEAR, (ULONG *)actual_flags,  ms_to_ticks(timeout_ms));
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Net_create_IP_instance(void)
{
  UINT        res = 0;
  uint32_t    err_line;
  uint32_t    ip_address    = 0;
  uint32_t    network_mask  = 0xFFFFFF00;
  uint32_t    gateway_address;
  ULONG       ret_val;

  // При создании IP передаваемый адрес и маска сети назначаются интефейсу с индексом 0 и ему присваивается имя "PRI"
  res = nx_ip_create(&net_ip,
                     "net_ip",
                     ip_address,
                     network_mask,
                     &net_packet_pool,
                     _nx_ram_network_driver,
                     net_ip_stack,
                     IP_INTERFACES_STACK_SIZE,
                     IP_INTERFACE_PRIORITY);

  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }
  // Интерфейс 0 не используем
  nx_ip_driver_interface_direct_command(&net_ip, NX_LINK_DISABLE, 0,&ret_val);

  // Здесь присоединяем драйвера интерфейсов. Индексы интерфейсам будут назначаться в порядке возрастания начиная с 1
  // У интерфейсов должны быть разные IP адреса, иначе присоединение драйвера не произойдет
  Str_to_IP_v4((char const *)ivar.ap_default_ip_addr,  (uint8_t *)&ip_address);
  Str_to_IP_v4((char const *)ivar.ap_default_net_mask,  (uint8_t *)&network_mask);
  NETLOG("WIFI AP  IP address: %03d.%03d.%03d.%03d", IPADDR(ip_address));
  res = nx_ip_interface_attach(&net_ip, "WIFI_AP" , ip_address, network_mask, WHD_AP_NetXDuo_driver_entry);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err2;
  }
  // Передаем команду напрямую в дравер о том что он выключен
  nx_ip_driver_interface_direct_command(&net_ip, NX_LINK_DISABLE, WIFI_AP_INTF_NUM,&ret_val);


  Str_to_IP_v4(WIFI_STA_get_def_ip(),  (uint8_t *)&ip_address);
  Str_to_IP_v4(WIFI_STA_get_def_mask(),(uint8_t *)&network_mask);
  NETLOG("WIFI STA IP address: %03d.%03d.%03d.%03d", IPADDR(ip_address));
  res = nx_ip_interface_attach(&net_ip, "WIFI_STA", ip_address, network_mask, WHD_STA_NetXDuo_driver_entry);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err2;
  }
  // Передаем команду напрямую в дравер о том что он выключен
  nx_ip_driver_interface_direct_command(&net_ip, NX_LINK_DISABLE, WIFI_STA_INTF_NUM,&ret_val);

  if ((ivar.usb_mode == USB_MODE_RNDIS) || (ivar.usb_mode == USB_MODE_HOST_ECM))
  {
    Str_to_IP_v4((char const *)ivar.usb_default_ip_addr,  (uint8_t *)&ip_address);
    Str_to_IP_v4((char const *)ivar.usb_default_net_mask,  (uint8_t *)&network_mask);
    NETLOG("USB      IP address: %03d.%03d.%03d.%03d", IPADDR(ip_address));
    res = nx_ip_interface_attach(&net_ip, "USB"     , ip_address, network_mask, _ux_network_driver_entry);
    if (res != NX_SUCCESS)
    {
      err_line = __LINE__;
      goto _err2;
    }
    // Передаем команду напрямую в дравер о том что он выключен
    nx_ip_driver_interface_direct_command(&net_ip, NX_LINK_DISABLE, NET_USB_INTF_NUM,&ret_val);
  }

  // На старте адресом гейтвея назначаем адрес точки достпа
  Str_to_IP_v4((char const *)ivar.ap_default_ip_addr,(uint8_t *)&gateway_address);
  nx_ip_gateway_address_set(&net_ip, gateway_address);

  res = nx_arp_enable(&net_ip, net_arp_cache, NX_ARP_CACHE_SIZE);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err2;
  }
  res = nx_tcp_enable(&net_ip);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err2;
  }
  res = nx_udp_enable(&net_ip);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err2;
  }
  res = nx_icmp_enable(&net_ip);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err2;
  }
  //status |= nx_igmp_enable(wifi_ap_ip_ptr);       // Включение этого протокола нарушает работу FTP
  res = nx_ip_fragment_enable(&net_ip);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err2;
  }

  NETLOG("IP interface created");
  return RES_OK;

_err2:
  nx_ip_delete(&net_ip);
_err:
  NETLOG("IP interface. Creation error in line %d. Result=%04X", err_line, res);
  return RES_ERROR;
}



/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_net(ULONG initial_input)
{
  uint32_t    res;
  uint32_t    err_line;
  ULONG       actual_flags;

  NETLOG("Task NET started");

  res = Net_create_IP_instance();
  if (res != RES_OK)
  {
    err_line = __LINE__;
    goto _err;
  }

  if (ivar.en_wifi_module)
  {
    uint32_t actual_flags;
    res = Thread_WiFi_manager_create();
    if (res != RES_OK)
    {
      err_line = __LINE__;
      goto _err;
    }

    // Ожидаем окончания инициализации access point WiFi модуля
    Wait_wifi_event(NET_EVT_WIFI_AP_READY,&actual_flags, 5000);
  }

  Net_DHCP_client_create(&net_ip);
  Net_DHCP_server_create(&net_ip);
  Net_mDNS_server_create(&net_ip);
  Net_DNS_client_create(&net_ip);
  Net_TELNET_server_create(&net_ip);
  Net_FTP_server_create(&net_ip);
  Net_HTTP_server_create(&net_ip);
  Net_MQTT_client_create(&net_ip);
  Net_SNTP_client_create(&net_ip);
  Net_FreeMaster_server_create(&net_ip);
  #ifdef ENABLE_MATLAB_CONNECTION
  MATLAB_server_create(&net_ip);
  #endif

  Send_wifi_event(NET_EVT_WIFI_STA_DISJOINED);  // Посылаем событие по которому задача Thread_wifi_manager вызовет процедуру WiFi Join

  // Главный цикл задачи
  do
  {
    if (tx_event_flags_get(&net_flag, 0xFFFFFFFF, TX_OR_CLEAR,&actual_flags, 1) == TX_SUCCESS)
    {
      if (actual_flags & NET_FLG_RNDIS_STOP)
      {
        RNDIS_to_stop();
      }
      if (actual_flags & NET_FLG_RNDIS_START)
      {
        RNDIS_to_start();
      }
      if (actual_flags & NET_FLG_STA_CONNECTED)
      {
        WIFI_STA_accept_connection_to_AP();
      }
      if (actual_flags & NET_FLG_STA_DISCONNECTED)
      {
        WIFI_STA_accept_disconnection_from_AP();
      }
      if (actual_flags & NET_FLG_AP_CONNECTED)
      {
        WIFI_AP_accept_STA_connection();
      }
      if (actual_flags & NET_FLG_AP_DISCONNECTED)
      {
        WIFI_AP_accept_STA_disconnection();
      }
      if (actual_flags & NET_FLG_ECM_HOST_START)
      {
        ECM_HOST_accept_connection();
      }
      if (actual_flags & NET_FLG_ECM_HOST_STOP)
      {
        ECM_HOST_accept_disconnection();
      }
    }

    Net_MQTT_client_task(&net_ip);
    Net_SNTP_client_task(&net_ip);

  } while (1);


_err:
  NETLOG("Task NET error in line %d. Result=%04X", err_line, res);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_Net_create(void)
{
  uint32_t res;

  Init_Net();

  res = tx_event_flags_create(&net_flag, "net_ctl");
  if (res != TX_SUCCESS)
  {
    NETLOG("Net task creating error %04X.", res);
    return RES_ERROR;
  }

  res = tx_thread_create(&net_thread, "Net", Thread_net,
                         0,
                         (void *)thread_net_stack, // stack_start
                         THREAD_NET_STACK_SIZE,    // stack_size
                         THREAD_NET_MAN_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
                         THREAD_NET_MAN_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (res == TX_SUCCESS)
  {
    NETLOG("Task NET created");
    return RES_OK;
  }
  else
  {
    tx_event_flags_delete(&net_flag);
    NETLOG("Task NET creating error %04X.", res);
    return RES_ERROR;
  }
}

