// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-03-05
// 12:07:22
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"

extern VOID  _nx_sntp_client_utility_fraction_to_usecs(ULONG fraction, ULONG *usecs);

#define   DEMO_SNTP_UPDATE_EVENT  BIT(0)
static UINT _Leap_second_handler(NX_SNTP_CLIENT *client_ptr, UINT leap_indicator);
static UINT _Kiss_of_death_handler(NX_SNTP_CLIENT *client_ptr, UINT KOD_code);
static VOID _Time_update_callback(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time);

NX_SNTP_CLIENT          sntp_client BSP_PLACE_IN_SECTION_V2(".stack.SNTP_client")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);


TX_EVENT_FLAGS_GROUP    sntp_flags;
static NX_IP            *snmp_ip_ptr;


typedef struct
{
    char const *address;
    uint8_t     disabled;
} T_sntp_servers;

T_sntp_servers sntp_servers[] =
{
  {(char *)ivar.time_server_1 , 0},
  {(char *)ivar.time_server_2 , 0},
  {(char *)ivar.time_server_3 , 0},
};

#define SNTP_SERVERS_NUM (sizeof(sntp_servers)/sizeof(sntp_servers[0]))

static uint32_t     serv_indx;
static uint32_t     sntp_timestump;
static ULONG        sntp_address;
static uint32_t     sntp_KOD_code;
static uint8_t      sntp_time_received;

/*-----------------------------------------------------------------------------------------------------
  Перемещаем индекс на следующий SNTP сервер

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t SNTP_select_next_server(void)
{
  uint32_t start_indx;
  start_indx = serv_indx;
  do
  {
    serv_indx++;
    if (serv_indx >= SNTP_SERVERS_NUM) serv_indx = 0;
    if (start_indx == serv_indx) return RES_ERROR;
    if (sntp_servers[serv_indx].disabled == 0) return RES_OK;
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void SNTP_exclude_current_server(void)
{
  sntp_servers[serv_indx].disabled = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void SNTP_enable_all_server(void)
{
  for (uint32_t i=0; i < SNTP_SERVERS_NUM; i++)
  {
    sntp_servers[i].disabled = 0;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Получить IP адрес сервера по индексу в массиве записей с URL

  \param indx

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t SNTP_get_server_address(uint8_t *ip_address_ptr)
{
  UINT    status;
  ULONG   host_ip_address;

  do
  {
    if (Str_to_IP_v4(sntp_servers[serv_indx].address,(uint8_t *)&host_ip_address) != RES_OK)
    {
      // Используем DNS для определения IP адреса
      status =  DNS_get_host_address((UCHAR *)sntp_servers[serv_indx].address,&host_ip_address, MS_TO_TICKS(2000));
      if (status == NX_SUCCESS)
      {
        APPLOG("SNTP. Selected server %s (%03d.%03d.%03d.%03d)",sntp_servers[serv_indx].address, IPADDR(host_ip_address));
        memcpy(ip_address_ptr,&host_ip_address, 4);
        return RES_OK;
      }
      else
      {
        APPLOG("SNTP. DNS request of %s failed. Error %04X",sntp_servers[serv_indx].address, status);
      }

      // Если неудача, то переходим на следующий адрес, а текущий исключаем из списка кандидатов
      SNTP_exclude_current_server();
      if (SNTP_select_next_server() != RES_OK) return RES_ERROR;
      continue;
    }
    else
    {
      APPLOG("SNTP. Selected server %s",sntp_servers[serv_indx].address);
      memcpy(ip_address_ptr,&host_ip_address, 4);
      return RES_OK;
    }
  } while (1);
}


/*-----------------------------------------------------------------------------------------------------
  Callback for application response to impending leap second

   This application defined handler for handling an impending leap second is not required by the SNTP Client.
   The default handler below only logs the event for every time stamp received with the leap indicator set.

  \param client_ptr
  \param indicator

  \return UINT
-----------------------------------------------------------------------------------------------------*/
static UINT _Leap_second_handler(NX_SNTP_CLIENT *client_ptr, UINT indicator)
{

  return NX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  Callback for application response to receiving Kiss of Death packet

   This application defined handler for handling a Kiss of Death packet is not required by the SNTP Client.
   A KOD handler should determine if the Client task should continue vs. abort sending/receiving time data
   from its current time server, and if aborting if it should remove the server from its active server list.

  \param client_ptr
  \param code

  \return UINT
-----------------------------------------------------------------------------------------------------*/
static UINT _Kiss_of_death_handler(NX_SNTP_CLIENT *client_ptr, UINT code)
{
  UINT    status = NX_SUCCESS;

  NX_PARAMETER_NOT_USED(client_ptr);

  sntp_KOD_code = code;

  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param time_update_ptr
  \param local_time
-----------------------------------------------------------------------------------------------------*/
static void _Time_update_callback(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time)
{
  tx_event_flags_set(&sntp_flags, DEMO_SNTP_UPDATE_EVENT, TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t SNTP_client_create(NX_IP  *ip_ptr)
{
  UINT     status;

  if (snmp_ip_ptr != NULL)    return RES_OK;
  if (ivar.en_sntp == 0)      return RES_ERROR;
  if (Is_DNS_created() == 0)  return RES_ERROR;


  status = SNTP_get_server_address((uint8_t *)&sntp_address);
  if (status != RES_OK)
  {
    APPLOG("SNTP. Unable to get server address: %d", status);
    return RES_ERROR;
  }

  snmp_ip_ptr = ip_ptr;

  status = tx_event_flags_create(&sntp_flags, "SNTP flags");
  if (status != TX_SUCCESS)
  {
    APPLOG("SNTP. Client event creation error %d",status);
    goto err1;
  }
  status =  nx_sntp_client_create(&sntp_client, ip_ptr, 0,&net_packet_pool,  _Leap_second_handler, _Kiss_of_death_handler, NULL); /* no random_number_generator callback */
  if (status != NX_SUCCESS)
  {
    APPLOG("SNTP. Client creation error %d",status);
    goto err;
  }

  nx_sntp_client_set_time_update_notify(&sntp_client, _Time_update_callback);

  status = nx_sntp_client_set_local_time(&sntp_client, 0,0);
  if (status != NX_SUCCESS)
  {
    APPLOG("SNTP. set local time error: %d", status);
    goto err;
  }

  status = nx_sntp_client_initialize_unicast(&sntp_client, sntp_address, ivar.sntp_poll_interval);
  if (status != NX_SUCCESS)
  {
    APPLOG("SNTP. inialising error: %d", status);
    goto err;
  }

  status = nx_sntp_client_run_unicast(&sntp_client);
  if (status != NX_SUCCESS)
  {
    APPLOG("SNTP. Run unicast error: %d", status);
    goto err;
  }

  APPLOG("SNTP. Client created");
  Get_system_ticks(&sntp_timestump);
  return RES_OK;

err:
  nx_sntp_client_delete(&sntp_client);
  tx_event_flags_delete(&sntp_flags);
err1:
  snmp_ip_ptr = NULL;
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param ip_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SNTP_client_delete(void)
{
  UINT     status;

  if (snmp_ip_ptr == 0) return RES_OK;

  status = nx_sntp_client_stop(&sntp_client);
  APPLOG("SNTP. Stop result: %d", status);
  status = nx_sntp_client_delete(&sntp_client);
  APPLOG("SNTP. Delete result: %d", status);
  status = tx_event_flags_delete(&sntp_flags);
  APPLOG("SNTP. Delete flags result: %d", status);
  snmp_ip_ptr = 0;
  sntp_time_received = 0;
  SNTP_enable_all_server();
  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Is_sntp_time_received(void)
{
  return sntp_time_received;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void SNTP_client_controller(void)
{
  ULONG     status;
  ULONG     events = 0;
  char      time_str[128];
  uint32_t  curr_time;

  NX_IP   *ip_ptr = NULL;

  if (snmp_ip_ptr == NULL)
  {
    switch (g_network_type)
    {
    case NET_BY_WIFI_STA:
      if (WIFI_STA_network_active_flag()) ip_ptr = wifi_sta_ip_ptr;
      break;
    case NET_BY_WIFI_AP:
      if (WIFI_AP_network_active_flag())  ip_ptr = wifi_ap_ip_ptr;
      break;
    case NET_BY_RNDIS:
      if (RNDIS_network_active_flag())    ip_ptr = rndis_ip_ptr;
      break;
    case NET_BY_ECM:
      if (ECM_Host_network_active_flag()) ip_ptr = ecm_host_ip_ptr;
      break;
    default:
      return;
    }
    if (SNTP_client_create(ip_ptr) != RES_OK) return;
  }


  if (tx_event_flags_get(&sntp_flags, DEMO_SNTP_UPDATE_EVENT, TX_OR_CLEAR,&events, TX_NO_WAIT) == TX_SUCCESS)
  {
    UINT         server_status;
    ULONG        seconds, milliseconds, microseconds, fraction;
    rtc_time_t   rt_time;

    if (events & DEMO_SNTP_UPDATE_EVENT)
    {
      status = nx_sntp_client_receiving_updates(&sntp_client,&server_status);
      if ((status == NX_SUCCESS) && (server_status != NX_FALSE))
      {
        status = nx_sntp_client_get_local_time_extended(&sntp_client,&seconds,&fraction, NX_NULL, 0);
        if (status == NX_SUCCESS)
        {
          _nx_sntp_client_utility_fraction_to_usecs(fraction,&microseconds);
          milliseconds =((microseconds + 500) / 1000);

          // Если это первый прием времени то вывести в лог
          if (sntp_time_received == 0)
          {
            sntp_time_received = 1;
            nx_sntp_client_utility_display_date_time(&sntp_client , time_str, 128); // Создать строку с датой временем
            APPLOG("SNTP sec, msec: %u %u" , seconds, milliseconds);
            APPLOG("SNTP resp: %s", time_str);
          }

          if (ivar.en_sntp_time_receiving)
          {
            Convert_NTP_to_UTC_time(seconds,&rt_time, ivar.utc_offset);
            APPLOG("Time : %04d.%02d.%02d %02d:%02d:%02d", rt_time.tm_year+1970, rt_time.tm_mon+1, rt_time.tm_mday, rt_time.tm_hour, rt_time.tm_min, rt_time.tm_sec);

            rt_time.tm_year = rt_time.tm_year + 1970 - 1900;
            // Обновить время в RTC устройства
            RTC_set_system_DateTime(&rt_time);
          }
          Get_system_ticks(&sntp_timestump);
        }
        else
        {
          APPLOG("SNTP. Get local time error %d", status);
        }
      }
      else
      {
        APPLOG("SNTP. Receiving updates error %d", status);
      }
    }
  }

  // Проверим как долго ждем ответа от сервера времени.
  // Если ждем слишком долго, то меняем сервер времени.
  Get_system_ticks(&curr_time);
  if ((Time_diff_seconds(sntp_timestump, curr_time) > (ivar.sntp_poll_interval + 10)) || (sntp_KOD_code != 0))
  {
    sntp_KOD_code = 0;
    sntp_timestump = curr_time;
    // Переходим на другой сервер
    SNTP_exclude_current_server();
    if (SNTP_select_next_server() == RES_OK)
    {
      status = nx_sntp_client_stop(&sntp_client);
      if (status == NX_SUCCESS)
      {
        if (SNTP_get_server_address((uint8_t *)&sntp_address) == RES_OK)
        {
          nx_sntp_client_initialize_unicast(&sntp_client, sntp_address, ivar.sntp_poll_interval);
          nx_sntp_client_run_unicast(&sntp_client);
        }
      }
    }
    else SNTP_client_delete();
  }


}

