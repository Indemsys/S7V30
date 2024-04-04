// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-11-07
// 15:40:27
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "nxd_bsd.h"
#include   "WiFi_STA_network.h"

#define THREAD_NET_STACK_SIZE (1024*4)
#define THREAD_BSD_STACK_SIZE (1024*2)
static uint8_t thread_net_stack[THREAD_NET_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.net_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static uint8_t thread_bsd_stack[THREAD_BSD_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.bsd_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       net_thread;


uint8_t                       g_BSD_initialised;
T_network_type                g_network_type = NET_BY_WIFI_AP;

static TX_EVENT_FLAGS_GROUP   net_flag;

static void Thread_net(ULONG initial_input);


/*-----------------------------------------------------------------------------------------------------

  \param msg

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Send_Net_task_event(uint32_t event_flag)
{
  return  tx_event_flags_set(&net_flag, event_flag, TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param event_flags
  \param timeout_ms

  \return uint32_t Возвращает битовоую маску обнаруженнызсобытий
-----------------------------------------------------------------------------------------------------*/
uint32_t Wait_Net_task_event(uint32_t event_flags, uint32_t timeout_ms)
{
  ULONG actual_flags = 0;
  tx_event_flags_get(&net_flag, event_flags, TX_OR_CLEAR,&actual_flags,  ms_to_ticks(timeout_ms));
  return actual_flags;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Determine_network_type(void)
{
  if (WIFI_STA_enabled_flag())
  {
    g_network_type = NET_BY_WIFI_STA;
  }
  else if (ivar.en_wifi_ap)
  {
    g_network_type = NET_BY_WIFI_AP;
  }
  else if (ivar.usb_mode == USB_MODE_RNDIS)
  {
    g_network_type = NET_BY_RNDIS;
  }
  else if (ivar.usb_mode == USB_MODE_HOST_ECM)
  {
    g_network_type = NET_BY_ECM;
  }
  else
  {
    g_network_type = NET_NONE;
  }


}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return NX_IP*
-----------------------------------------------------------------------------------------------------*/
NX_IP* Get_net_ip(void)
{
  switch (g_network_type)
  {
  case NET_BY_WIFI_STA:
    return wifi_sta_ip_ptr;
  case NET_BY_WIFI_AP:
    return wifi_ap_ip_ptr;
  case NET_BY_RNDIS:
    return rndis_ip_ptr;
  case NET_BY_ECM:
    return ecm_host_ip_ptr;
  default:
    return NULL;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param ip_addr_str
  \param ip_addr_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_device_ip_addr(char *ip_addr_str, uint32_t ip_addr_str_len)
{
  ULONG ip_address;
  ULONG network_mask;
  NX_IP *ip_ptr;

  ip_ptr = Get_net_ip();
  if (ip_ptr == NULL)
  {
    return RES_ERROR;
  }

  if (ip_ptr->nx_ip_interface->nx_interface_link_up)
  {
    nx_ip_address_get(ip_ptr,&ip_address,&network_mask);
    snprintf(ip_addr_str, ip_addr_str_len, "%03d.%03d.%03d.%03d", IPADDR(ip_address));
    return RES_OK;
  }
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param ip_addr_str
  \param ip_addr_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_device_MAC_addr(char *mac_addr_str, uint32_t mac_addr_str_len)
{
  NX_IP *ip_ptr;

  ip_ptr = Get_net_ip();
  if (ip_ptr == NULL)
  {
    return RES_ERROR;
  }

  if (ip_ptr->nx_ip_interface->nx_interface_link_up)
  {
    uint8_t mac[6];
    mac[0] = (uint8_t)((ip_ptr->nx_ip_interface->nx_interface_physical_address_msw & 0x0000ff00)>> 8);
    mac[1] = (uint8_t)((ip_ptr->nx_ip_interface->nx_interface_physical_address_msw & 0x000000ff)>> 0);
    mac[2] = (uint8_t)((ip_ptr->nx_ip_interface->nx_interface_physical_address_lsw & 0xff000000)>> 24);
    mac[3] = (uint8_t)((ip_ptr->nx_ip_interface->nx_interface_physical_address_lsw & 0x00ff0000)>> 16);
    mac[4] = (uint8_t)((ip_ptr->nx_ip_interface->nx_interface_physical_address_lsw & 0x0000ff00)>> 8);
    mac[5] = (uint8_t)((ip_ptr->nx_ip_interface->nx_interface_physical_address_lsw & 0x000000ff)>> 0);
    snprintf(mac_addr_str, mac_addr_str_len, "%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    return RES_OK;
  }
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param ap_name_str
  \param ap_name_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_connected_SSID(char *ssid_str, uint32_t ssid_str_len)
{
  NX_IP *ip_ptr;

  if (g_network_type!= NET_BY_WIFI_STA) return RES_ERROR;

  ip_ptr = wifi_sta_ip_ptr;

  if (ip_ptr->nx_ip_interface->nx_interface_link_up)
  {
    snprintf(ssid_str, ssid_str_len, "%s",WIFI_STA_get_current_SSID());
    return RES_OK;
  }
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_net(ULONG initial_input)
{
  UINT      res;
  ULONG     flags;
  NX_IP     *ip_ptr;


  APPLOG("Start task Net");

  switch (g_network_type)
  {
  case NET_BY_WIFI_STA:
    res = WIFI_init_network_stack(&ip_ptr);
    break;
  case NET_BY_WIFI_AP:
    res = WIFI_init_network_stack(&ip_ptr);
    break;
  case NET_BY_RNDIS:
    res = RNDIS_init_network_stack(&ip_ptr);
    break;
  case NET_BY_ECM:
    res = ECM_Host_init_network_stack(&ip_ptr);
    break;
  }

  if (res != RES_OK) goto exit_thread;

  do
  {
    // Ожидаем сообщения 10 мс
    flags = Wait_Net_task_event(EVT_MQTT_MSG, 10);
    if (flags & EVT_MQTT_MSG)
    {
      // Отработка сообщения от MQTT брокера
      MQTTMC_messages_processor();
    }
    else
    {

      switch (g_network_type)
      {
      case NET_BY_WIFI_STA:
        WIFI_STA_network_controller();
        break;
      case NET_BY_WIFI_AP:
        WIFI_AP_network_controller();
        break;
      case NET_BY_RNDIS:
        RNDIS_network_controller();   // Старт или останов серсисов Telnet, FTP, MATLAB, DHCP, mDNS поверх канала RNDIS
        break;
      case NET_BY_ECM:
        ECM_Host_network_controller(); // Старт или останов серсисов Telnet, FTP, MATLAB, DHCP, mDNS поверх канала ECM
        break;
      }

      DNS_client_controller();
      HTTP_server_controller();
      MQTT_client_controller();
      SNTP_client_controller();
    }

  } while (1);

exit_thread:

   APPLOG("Task Net stopped.");
   do
   {
     Wait_ms(100);
   }
   while (1);


}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_Net_create(void)
{
  uint32_t res;

  if (g_network_type == NET_NONE) return;
  if (tx_event_flags_create(&net_flag, "Task_Net") != TX_SUCCESS) return;

  Init_Net();

  res = tx_thread_create(&net_thread, "Net", Thread_net,
                         0,
                         (void *)thread_net_stack, // stack_start
                         THREAD_NET_STACK_SIZE,   // stack_size
                         THREAD_NET_PRIORITY,     // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
                         THREAD_NET_PRIORITY,     // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  APPLOG("Net task creation result: %d", res);
}

