// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-08-02
// 16:24:05
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

NX_DHCP                       net_dhcp_client BSP_PLACE_IN_SECTION_V2(".stack.dhcp_client") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
NX_DHCP_SERVER                net_dhcp_server BSP_PLACE_IN_SECTION_V2(".stack.dhcp_server")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
uint8_t                       net_dhcp_server_stack[DHCP_SERVER_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.dhcp_server_stack")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);

#define CASE_RETURN_STR(enum_val)          case enum_val: return #enum_val;

/*-----------------------------------------------------------------------------------------------------


  \param state

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* DHCP_client_state_str(UCHAR state)
{
  switch (state)
  {
    CASE_RETURN_STR(NX_DHCP_STATE_NOT_STARTED);
    CASE_RETURN_STR(NX_DHCP_STATE_BOOT);
    CASE_RETURN_STR(NX_DHCP_STATE_INIT);
    CASE_RETURN_STR(NX_DHCP_STATE_SELECTING);
    CASE_RETURN_STR(NX_DHCP_STATE_REQUESTING);
    CASE_RETURN_STR(NX_DHCP_STATE_BOUND);
    CASE_RETURN_STR(NX_DHCP_STATE_RENEWING);
    CASE_RETURN_STR(NX_DHCP_STATE_REBINDING);
    CASE_RETURN_STR(NX_DHCP_STATE_FORCERENEW);
    CASE_RETURN_STR(NX_DHCP_STATE_ADDRESS_PROBING);
  }
  return "UNKNOWN";
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return NX_DHCP*
-----------------------------------------------------------------------------------------------------*/
NX_DHCP* Net_get_dhcp_client_ptr(void)
{
  return &net_dhcp_client;
}


/*-----------------------------------------------------------------------------------------------------
  Создаем DHCP сервер
  Пул выделяемых адресов начинается сразу после адреса самого сервера

  \param serv_name
  \param dc

  \return NX_DHCP_SERVER*
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_DHCP_server_create(NX_IP *ip_ptr)
{
  uint32_t             res;
  uint32_t             err_line;
  UINT                 added_addr_num;

  uint32_t             ip_address;
  uint32_t             start_ip_address;
  uint32_t             end_ip_address;
  uint32_t             subnet_mask;
  uint32_t             gateway_address;


  if (ivar.ap_en_dhcp_server == 0) return RES_ERROR;


  res = nx_dhcp_server_create(&net_dhcp_server,
                              ip_ptr,
                              net_dhcp_server_stack,
                              DHCP_SERVER_STACK_SIZE,
                              "DHCP_server",
                              &net_packet_pool);

  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  // Конфигурируем пул для интерфейса RNDIS
  if ((ivar.en_rndis_dhcp_server != 0) && (ivar.usb_mode == USB_MODE_RNDIS))
  {
    Str_to_IP_v4((char const *)ivar.usb_default_ip_addr, (uint8_t *)&ip_address);
    // Пулу адресов назначаем только один элемент. Поскольку к точку доступа разрешаем подключиться только одному клиенту
    start_ip_address = ip_address + 1;
    end_ip_address   = start_ip_address+10;
    subnet_mask      = 0xFFFFFF00;
    gateway_address  = ip_address;

    res = nx_dhcp_create_server_ip_address_list(&net_dhcp_server, NET_USB_INTF_NUM , start_ip_address, end_ip_address ,&added_addr_num);
    if (res != NX_SUCCESS)
    {
      err_line = __LINE__;
      goto _err;
    }

    res = nx_dhcp_set_interface_network_parameters(&net_dhcp_server, NET_USB_INTF_NUM ,subnet_mask, gateway_address, gateway_address);
    if (res != NX_SUCCESS)
    {
      err_line = __LINE__;
      goto _err;
    }
  }

  // Конфигурируем пул для WiFi точки доступа
  if (ivar.ap_en_dhcp_server != 0)
  {
    Str_to_IP_v4((char const *)ivar.ap_default_ip_addr, (uint8_t *)&ip_address);
    // Пулу адресов назначаем только один элемент. Поскольку к точку доступа разрешаем подключиться только одному клиенту
    start_ip_address = ip_address + 1;
    end_ip_address   = start_ip_address+10;
    subnet_mask      = 0xFFFFFF00;
    gateway_address  = ip_address;

    res = nx_dhcp_create_server_ip_address_list(&net_dhcp_server, WIFI_AP_INTF_NUM , start_ip_address, end_ip_address ,&added_addr_num);
    if (res != NX_SUCCESS)
    {
      err_line = __LINE__;
      goto _err;
    }

    res = nx_dhcp_set_interface_network_parameters(&net_dhcp_server, WIFI_AP_INTF_NUM ,subnet_mask, gateway_address, gateway_address);
    if (res != NX_SUCCESS)
    {
      err_line = __LINE__;
      goto _err;
    }
  }


  res = nx_dhcp_server_start(&net_dhcp_server);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  NETLOG("DHCP server created");
  return RES_OK;
_err:
  NETLOG("DHCP server. Creation error in line %d. Result=%04X" , err_line, res);
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_DHCP_server_restart(void)
{
  uint32_t res;
  nx_dhcp_server_stop(&net_dhcp_server);

  res = nx_dhcp_server_start(&net_dhcp_server);
  if (res != NX_SUCCESS)
  {
    NETLOG("DHCP server starting error %04X", res);
  }
  return res;
}
/*-----------------------------------------------------------------------------------------------------


  \param dhcp_ptr
  \param new_state
-----------------------------------------------------------------------------------------------------*/
static void  Net_dhcp_state_change_notify(NX_DHCP *dhcp_ptr, UCHAR new_state)
{
  NETLOG("DHCP client state chaged to %s (%d)", DHCP_client_state_str(new_state), new_state);

}

/*-----------------------------------------------------------------------------------------------------


  \param cln_conf

  \return NX_DHCP*
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_DHCP_client_create(NX_IP *ip_ptr)
{
  uint32_t       res;
  uint32_t       err_line;

  res = nx_dhcp_create(&net_dhcp_client, ip_ptr,(CHAR *)ivar.this_host_name);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  res = nx_dhcp_packet_pool_set(&net_dhcp_client,&net_packet_pool);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  res = nx_dhcp_state_change_notify(&net_dhcp_client, Net_dhcp_state_change_notify);
  if (res != NX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  res = nx_dhcp_interface_enable(Net_get_dhcp_client_ptr(), WIFI_STA_INTF_NUM);
  if (res != TX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  if ((ivar.en_ecm_host_dhcp_client != 0) && (ivar.usb_mode == USB_MODE_HOST_ECM))
  {
    res = nx_dhcp_interface_enable(Net_get_dhcp_client_ptr(), NET_USB_INTF_NUM);
    if (res != TX_SUCCESS)
    {
      err_line = __LINE__;
      goto _err;
    }
  }

  NETLOG("DHCP client created");
  return RES_OK;
_err:
  NETLOG("DHCP client: Creation error in line %d. Result=%04X", err_line, res);
  return RES_ERROR;

}

