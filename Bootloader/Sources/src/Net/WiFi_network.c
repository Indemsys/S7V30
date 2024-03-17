// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-29
// 14:27:21
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



#define                       WHD_STACK_SIZE   (1024*5) // Размер должен быть больше или равен MINIMUM_WHD_STACK_SIZE
static ULONG whd_thread_stack[WHD_STACK_SIZE]  BSP_PLACE_IN_SECTION_V2(".stack.whd_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);

whd_init_config_t       g_whd_init_cfg;
whd_resource_source_t   g_whd_resource;
whd_buffer_funcs_t      g_whd_buff_funcs;
whd_netif_funcs_t       g_whd_net_funcs;
whd_sdio_config_t       g_whd_sdio_cfg;
cyhal_sdio_t            g_sdhc_obj;
whd_driver_t            g_whd_driver;
whd_interface_t         g_pri_ifp;
whd_interface_t         g_sec_ifp;
whd_ssid_t              ssid;

uint8_t                 wifi_composition = WIFI_ONLY_STA;
whd_mac_t               ap_mac_addr;

extern const sdmmc_instance_t g_sdio1;

static uint8_t   wifi_module_initialized;

const whd_event_num_t events_list[]=
{
  WLC_E_JOIN,
  WLC_E_START,
  WLC_E_ASSOC_START,
  WLC_E_AUTH,
  WLC_E_AUTH_IND,
  WLC_E_DEAUTH,
  WLC_E_DEAUTH_IND,
  WLC_E_LINK,
  WLC_E_AP_STARTED,
  WLC_E_RM_COMPLETE,
  WLC_E_TX_STAT_ERROR,
  WLC_E_PFN_SWC,
  WLC_E_AUTHORIZED,
  WLC_E_AUTH_REQ,
  WLC_E_ASSOC_REQ_IE,
  WLC_E_ASSOC_RESP_IE,
  WLC_E_ASSOC_RECREATED,
  WLC_E_WAI_STA_EVENT,
  WLC_E_IF,
  WLC_E_NONE
};

uint16_t  whd_event_index;


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t WiFi_module_initialized_flag(void)
{
  return wifi_module_initialized;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return whd_interface_t
-----------------------------------------------------------------------------------------------------*/
whd_interface_t WIFI_get_whd_interface(void)
{
  return g_pri_ifp;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return whd_init_config_t
-----------------------------------------------------------------------------------------------------*/
whd_init_config_t* WIFI_get_config(void)
{
  return  &g_whd_init_cfg;
}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void WIFI_log_module_info(void)
{
  uint32_t    res;
  char        *version;

  version =  App_malloc(200);
  if (version != NULL)
  {
    res = whd_wifi_get_wifi_version(WIFI_get_whd_interface(),version, 200);
    if (res == WHD_SUCCESS)
    {
      if (strlen(version) > 1)
      {
        version[strlen(version)-1] = 0;
        APPLOG("WiFi fw: %s",version);
      }
    }
    App_free(version);
  }
  switch (wifi_composition)
  {
  case WIFI_ONLY_AP:
    APPLOG("WiFi mode: Access Point. Channel = %d, SSID = %s, CNTRY = %08X", ivar.wifi_ap_channel, ivar.wifi_ap_ssid, (uint32_t)g_whd_init_cfg.country);
    break;
  case WIFI_ONLY_STA:
    APPLOG("WiFi mode: Station");
    break;
  case WIFI_STA_AND_AP:
    APPLOG("WiFi mode: Station & Access Point");
    break;
  default:
    APPLOG("WiFi mode: Unknown");
    break;
  }

}

/*-----------------------------------------------------------------------------------------------------
  Перехватчик событий при обмене с WiFi модулем

  Событие происходят при подключении клиента к точке доступа
  Event 4 (WLC_E_AUTH_IND) (WHD_event_handler 181)
  Event 136 (WLC_E_AUTHORIZED) (WHD_event_handler 181)



  События происходят при отключении клиента от точки доступа
  Event 6 (WLC_E_DEAUTH_IND) (WHD_event_handler 181)
  Event 67 (WLC_E_WAI_STA_EVENT) (WHD_event_handler 181)




  \param ifp
  \param event_header
  \param event_data
  \param handler_user_data

  \return void*
-----------------------------------------------------------------------------------------------------*/
static void* WHD_event_handler(whd_interface_t ifp, const whd_event_header_t *event_header, const uint8_t *event_data, void *handler_user_data)
{
  APPLOG("WIFI. Event: %d (%s) Reason: %d, Status: %d ", event_header->event_type, WHD_event_to_string(event_header->event_type), event_header->reason, event_header->status);

  if  (event_header->event_type == (uint32_t)WLC_E_LINK)
  {
    if (event_header->reason == 0)
    {
      // Произошло подключение устройства к AP
      if ((wifi_composition == WIFI_ONLY_STA) || (wifi_composition == WIFI_STA_AND_AP))
      {
        APPLOG("WIFI. AP Connected    %02X:%02X:%02X:%02X:%02X:%02X",event_header->addr.octet[0], event_header->addr.octet[1],event_header->addr.octet[2], event_header->addr.octet[3], event_header->addr.octet[4], event_header->addr.octet[5]);
      }
    }
    else if ((event_header->reason == 1) || (event_header->reason == 2))
    {
      // Произошло отключение устройства от AP
      if ((wifi_composition == WIFI_ONLY_STA) || (wifi_composition == WIFI_STA_AND_AP))
      {
        APPLOG("WIFI. AP Disconnected %02X:%02X:%02X:%02X:%02X:%02X",event_header->addr.octet[0], event_header->addr.octet[1],event_header->addr.octet[2], event_header->addr.octet[3], event_header->addr.octet[4], event_header->addr.octet[5]);
      }
    }
  }
  if ((event_header->event_type == (uint32_t)WLC_E_WAI_STA_EVENT) || (event_header->event_type == (uint32_t)WLC_E_DEAUTH))
  {
    // Произошло отключение от устройства станции
    if ((wifi_composition == WIFI_ONLY_AP) || (wifi_composition == WIFI_STA_AND_AP))
    {
      APPLOG("WIFI. STA Disconnected   %02X:%02X:%02X:%02X:%02X:%02X",event_header->addr.octet[0], event_header->addr.octet[1],event_header->addr.octet[2], event_header->addr.octet[3], event_header->addr.octet[4], event_header->addr.octet[5]);
    }
  }
  if (event_header->event_type == (uint32_t)WLC_E_AUTHORIZED)
  {
    // Произошло подключение к устройству станции
    if ((wifi_composition == WIFI_ONLY_AP) || (wifi_composition == WIFI_STA_AND_AP))
    {
      APPLOG("WIFI. STA Connected      %02X:%02X:%02X:%02X:%02X:%02X",event_header->addr.octet[0], event_header->addr.octet[1],event_header->addr.octet[2], event_header->addr.octet[3], event_header->addr.octet[4], event_header->addr.octet[5]);
    }
  }

  return handler_user_data;
}

/*-----------------------------------------------------------------------------------------------------


  \param max_assoc

  \return wwd_result_t
-----------------------------------------------------------------------------------------------------*/
whd_result_t WHD_wifi_set_max_associations(uint32_t max_assoc)
{
  uint32_t     res;
  res = whd_wifi_set_iovar_buffer(g_pri_ifp, IOVAR_STR_MAX_ASSOC,&max_assoc, 4);
  return res;
}


/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t WIFI_init_module(void)
{
  uint32_t              res;
  whd_mac_t             macaddr;

  wifi_module_initialized = 0;

  WIFI_REG_ON = 1;
  Wait_ms(190);    // Задержка для WiFi модуля

  if (Init_SDIO1() != RES_OK)
  {
    Deinit_SDIO1();
    APPLOG("WIFI. Init_SDIO1 failed");
    return RES_ERROR;
  }

  // С кодами стран WHD_COUNTRY_POLAND и WHD_COUNTRY_LITHUANIA в режиме станции не происходит активизация модуля
  // Коды других стран не проверялись
  g_whd_init_cfg.country                                      = WHD_COUNTRY_UNITED_STATES;
  g_whd_init_cfg.thread_priority                              = 10;
  g_whd_init_cfg.thread_stack_size                            = WHD_STACK_SIZE;
  g_whd_init_cfg.thread_stack_start                           = whd_thread_stack;

  g_whd_resource.whd_resource_size                            = WHD_resource_size;
  g_whd_resource.whd_get_resource_block                       = WHD_get_resource_block;
  g_whd_resource.whd_get_resource_no_of_blocks                = WHD_get_resource_no_of_blocks;
  g_whd_resource.whd_get_resource_block_size                  = WHD_get_resource_block_size;
  g_whd_resource.whd_resource_read                            = WHD_resource_read;

  g_whd_buff_funcs.whd_host_buffer_get                        = WHD_host_buffer_get;
  g_whd_buff_funcs.whd_buffer_release                         = WHD_buffer_release;
  g_whd_buff_funcs.whd_buffer_get_current_piece_data_pointer  = WHD_buffer_get_current_piece_data_pointer;
  g_whd_buff_funcs.whd_buffer_get_current_piece_size          = WHD_buffer_get_current_piece_size;
  g_whd_buff_funcs.whd_buffer_set_size                        = WHD_buffer_set_size;
  g_whd_buff_funcs.whd_buffer_add_remove_at_front             = WHD_buffer_add_remove_at_front;

  g_whd_net_funcs.whd_network_process_ethernet_data           = WHD_network_process_ethernet_data;

  if (whd_init(&g_whd_driver,&g_whd_init_cfg,&g_whd_resource,&g_whd_buff_funcs,&g_whd_net_funcs) != WHD_SUCCESS)
  {
    Deinit_SDIO1();
    APPLOG("WIFI. whd_init failed");
    return RES_ERROR;
  }


  g_whd_sdio_cfg.high_speed_sdio_clock                        = WHD_TRUE;
  g_whd_sdio_cfg.sdio_1bit_mode                               = WHD_FALSE; //WHD_FALSE; // WHD_TRUE;
  g_whd_sdio_cfg.oob_config.host_oob_pin                      = 0;
  g_whd_sdio_cfg.oob_config.dev_gpio_sel                      = 0;
  g_whd_sdio_cfg.oob_config.is_falling_edge                   = WHD_FALSE;
  g_whd_sdio_cfg.oob_config.intr_priority                     = 0;
  g_whd_sdio_cfg.oob_config.drive_mode                        = CYHAL_GPIO_DRIVE_NONE;
  g_whd_sdio_cfg.oob_config.init_drive_state                  = WHD_FALSE;


  g_sdhc_obj = (void *)&g_sdio1;
  if (whd_bus_sdio_attach(g_whd_driver,&g_whd_sdio_cfg,&g_sdhc_obj) != WHD_SUCCESS)
  {
    whd_bus_sdio_detach(g_whd_driver);
    Deinit_SDIO1();
    APPLOG("WIFI. whd_bus_sdio_attach failed");
    return RES_ERROR;
  }

  // Первый интерфейс предназначен для организазии STA (Station)
  if (whd_wifi_on(g_whd_driver,&g_pri_ifp) != WHD_SUCCESS)
  {
    whd_wifi_off(g_pri_ifp);
    Deinit_SDIO1();
    APPLOG("WIFI. whd_wifi_on failed");
    return RES_ERROR;
  }


  if (wifi_composition == WIFI_STA_AND_AP)
  {
    // Второй интерфейс предназначен для организазии AP (Access Point)
    if (whd_add_secondary_interface(g_whd_driver,&ap_mac_addr,&g_sec_ifp) != WHD_SUCCESS)
    {
      whd_wifi_off(g_pri_ifp);
      Deinit_SDIO1();
      APPLOG("WIFI. whd_add_secondary_interface failed");
      return RES_ERROR;
    }
  }

  res = whd_wifi_set_event_handler(g_pri_ifp, (uint32_t *)events_list,WHD_event_handler,NULL,&whd_event_index);
  if (res != WHD_SUCCESS)
  {
    APPLOG("WIFI. whd_wifi_set_event_handler failed (%08X)");
  }

  if (wifi_composition == WIFI_ONLY_AP)
  {
    WHD_wifi_set_max_associations(1);
  }




  res = whd_wifi_get_mac_address(g_pri_ifp, &macaddr);
  if (res == WHD_SUCCESS)
  {
    memcpy(&(g_pri_ifp->mac_addr), &macaddr, sizeof(macaddr));
  }


  WIFI_log_module_info();
  APPLOG("WIFI module initialize successfully. MAC: %02X:%02X:%02X:%02X:%02X:%02X", g_pri_ifp->mac_addr.octet[0],g_pri_ifp->mac_addr.octet[1],g_pri_ifp->mac_addr.octet[2],g_pri_ifp->mac_addr.octet[3],g_pri_ifp->mac_addr.octet[4],g_pri_ifp->mac_addr.octet[5]);
  wifi_module_initialized = 1;
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t WIFI_get_network_composition(void)
{
  if ((ivar.en_wifi_ap) && (WIFI_STA_enabled_flag()))
  {
    // Получаем второй MAC адрес необходимый для второго интерфейса
    if (Str_to_MAC((const char *)ivar.ap_mac_addr,ap_mac_addr.octet) != RES_OK)
    {
      ivar.en_wifi_ap = 0;
      return WIFI_ONLY_STA;
    }
    return WIFI_STA_AND_AP;
  }
  else if (ivar.en_wifi_ap)
  {
    return WIFI_ONLY_AP;
  }
  else
  {
    return WIFI_ONLY_STA;
  }
}



/*-----------------------------------------------------------------------------------------------------


  \param ip_p_ptr возвращаемый указатель на IP используемый для основных сервисов

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_init_network_stack(NX_IP **ip_p_ptr)
{
  wifi_composition = WIFI_get_network_composition();

  if (WHD_open_resource_files()!= RES_OK) return RES_ERROR;
  if (WIFI_init_module() != RES_OK) return RES_ERROR;
  WHD_close_resource_files();

  switch (wifi_composition)
  {
  case WIFI_ONLY_AP:
    if (WIFI_AP_init_TCP_stack() != RES_OK) return RES_ERROR;
    *ip_p_ptr = wifi_ap_ip_ptr;
    break;
  case WIFI_ONLY_STA:
    if (WIFI_STA_init_TCP_stack() != RES_OK) return RES_ERROR;
    *ip_p_ptr = wifi_sta_ip_ptr;
    break;
  case WIFI_STA_AND_AP:
    if (WIFI_AP_init_TCP_stack() != RES_OK) return RES_ERROR;
    *ip_p_ptr = wifi_ap_ip_ptr;
    break;
  default:
    return RES_ERROR;
  }

  return RES_OK;
}





