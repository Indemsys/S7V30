// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-29
// 14:27:21
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


#define           THREAD_WIFI_MAN_STACK_SIZE (1024*2)
static uint8_t    thread_wifi_man_stack[THREAD_WIFI_MAN_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.wifi_man_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD  wifi_man_thread;

#define            WHD_STACK_SIZE   (1024*5) // Размер должен быть больше или равен MINIMUM_WHD_STACK_SIZE
static uint8_t     whd_thread_stack[WHD_STACK_SIZE]  BSP_PLACE_IN_SECTION_V2(".stack.whd_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);

#define MAX_SDIO_INIT_ATTEMPT_NUM  5
#define MAX_WIFI_INIT_ATTEMPT_NUM  3


whd_init_config_t       g_whd_init_cfg;
whd_resource_source_t   g_whd_resource;
whd_buffer_funcs_t      g_whd_buff_funcs;
whd_netif_funcs_t       g_whd_net_funcs;
whd_sdio_config_t       g_whd_sdio_cfg;
cyhal_sdio_t            g_sdhc_obj;
whd_driver_t            g_whd_driver;
whd_interface_t         g_prim_whd_intf_ptr = NULL;  // Primary  WHD interface  for STA
whd_interface_t         g_secd_whd_intf_ptr = NULL;  // Secondary WHD interface for AP

whd_mac_t               primary_mac_addr;
whd_mac_t               secondary_mac_addr;

extern const sdmmc_instance_t g_sdio1;

const whd_event_num_t events_list[] =
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
  //  WLC_E_TX_STAT_ERROR, не встречалось
  WLC_E_PFN_SWC,
  WLC_E_AUTHORIZED,
  WLC_E_AUTH_REQ,
  WLC_E_ASSOC_REQ_IE,
  WLC_E_ASSOC_RESP_IE,
  WLC_E_ASSOC_RECREATED,
  WLC_E_ASSOC_IND,
  WLC_E_DISASSOC_IND,
  WLC_E_WAI_STA_EVENT,
  //  WLC_E_IF,            не встречалось
  //  WLC_E_PRUNE,         не встречалось
  WLC_E_DISASSOC,
  WLC_E_PSK_SUP,
  //  WLC_E_TXFAIL, Это событие может происходить слишком часто
  WLC_E_NONE  //  В конце этого списка всегда должно идти WLC_E_NONE
};

// Массив со всеми возможными событиями
//const whd_event_num_t events_list[] =
//{
//  WLC_E_SET_SSID,                         // = 0,
//  WLC_E_JOIN,                             // = 1,
//  WLC_E_START,                            // = 2,
//  WLC_E_AUTH,                             // = 3,
//  WLC_E_AUTH_IND,                         // = 4,
//  WLC_E_DEAUTH,                           // = 5,
//  WLC_E_DEAUTH_IND,                       // = 6,
//  WLC_E_ASSOC,                            // = 7,
//  WLC_E_ASSOC_IND,                        // = 8,
//  WLC_E_REASSOC,                          // = 9,
//  WLC_E_REASSOC_IND,                      // = 10,
//  WLC_E_DISASSOC,                         // = 11,
//  WLC_E_DISASSOC_IND,                     // = 12,
//  WLC_E_QUIET_START,                      // = 13,
//  WLC_E_QUIET_END,                        // = 14,
//  WLC_E_BEACON_RX,                        // = 15,
//  WLC_E_LINK,                             // = 16,
//  WLC_E_MIC_ERROR,                        // = 17,
//  WLC_E_NDIS_LINK,                        // = 18,
//  WLC_E_ROAM,                             // = 19,
//  WLC_E_TXFAIL,                           // = 20,
//  WLC_E_PMKID_CACHE,                      // = 21,
//  WLC_E_RETROGRADE_TSF,                   // = 22,
//  WLC_E_PRUNE,                            // = 23,
//  WLC_E_AUTOAUTH,                         // = 24,
//  WLC_E_EAPOL_MSG,                        // = 25,
//  WLC_E_SCAN_COMPLETE,                    // = 26,
//  WLC_E_ADDTS_IND,                        // = 27,
//  WLC_E_DELTS_IND,                        // = 28,
//  WLC_E_BCNSENT_IND,                      // = 29,
//  WLC_E_BCNRX_MSG,                        // = 30,
//  WLC_E_BCNLOST_MSG,                      // = 31,
//  WLC_E_ROAM_PREP,                        // = 32,
//  WLC_E_PFN_NET_FOUND,                    // = 33,
//  WLC_E_PFN_NET_LOST,                     // = 34,
//  WLC_E_RESET_COMPLETE,                   // = 35,
//  WLC_E_JOIN_START,                       // = 36,
//  WLC_E_ROAM_START,                       // = 37,
//  WLC_E_ASSOC_START,                      // = 38,
//  WLC_E_IBSS_ASSOC,                       // = 39,
//  WLC_E_RADIO,                            // = 40,
//  WLC_E_PSM_WATCHDOG,                     // = 41,
//  WLC_E_CCX_ASSOC_START,                  // = 42,
//  WLC_E_CCX_ASSOC_ABORT,                  // = 43,
//  WLC_E_PROBREQ_MSG,                      // = 44, Событие поступает слишком часто
//  WLC_E_SCAN_CONFIRM_IND,                 // = 45,
//  WLC_E_PSK_SUP,                          // = 46,
//  WLC_E_COUNTRY_CODE_CHANGED,             // = 47,
//  WLC_E_EXCEEDED_MEDIUM_TIME,             // = 48,
//  WLC_E_ICV_ERROR,                        // = 49,
//  WLC_E_UNICAST_DECODE_ERROR,             // = 50,
//  WLC_E_MULTICAST_DECODE_ERROR,           // = 51,
//  WLC_E_TRACE,                            // = 52,
//  WLC_E_BTA_HCI_EVENT,                    // = 53,
//  WLC_E_IF,                               // = 54,
//  WLC_E_P2P_DISC_LISTEN_COMPLETE,         // = 55,
//  WLC_E_RSSI,                             // = 56,
//  WLC_E_PFN_BEST_BATCHING,                // = 57,
//  WLC_E_EXTLOG_MSG,                       // = 58,
//  WLC_E_ACTION_FRAME,                     // = 59,
//  WLC_E_ACTION_FRAME_COMPLETE,            // = 60,
//  WLC_E_PRE_ASSOC_IND,                    // = 61,
//  WLC_E_PRE_REASSOC_IND,                  // = 62,
//  WLC_E_CHANNEL_ADOPTED,                  // = 63,
//  WLC_E_AP_STARTED,                       // = 64,
//  WLC_E_DFS_AP_STOP,                      // = 65,
//  WLC_E_DFS_AP_RESUME,                    // = 66,
//  WLC_E_WAI_STA_EVENT,                    // = 67,
//  WLC_E_WAI_MSG,                          // = 68,
//  WLC_E_ESCAN_RESULT,                     // = 69,
//  WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE,   // = 70,
//  WLC_E_PROBRESP_MSG,                     // = 71,
//  WLC_E_P2P_PROBREQ_MSG,                  // = 72,
//  WLC_E_DCS_REQUEST,                      // = 73,
//  WLC_E_FIFO_CREDIT_MAP,                  // = 74,
//  WLC_E_ACTION_FRAME_RX,                  // = 75,
//  WLC_E_WAKE_EVENT,                       // = 76,
//  WLC_E_RM_COMPLETE,                      // = 77,
//  WLC_E_HTSFSYNC,                         // = 78,
//  WLC_E_OVERLAY_REQ,                      // = 79,
//  WLC_E_CSA_COMPLETE_IND,                 // = 80,
//  WLC_E_EXCESS_PM_WAKE_EVENT,             // = 81,
//  WLC_E_PFN_SCAN_NONE,                    // = 82,
//  WLC_E_PFN_SCAN_ALLGONE,                 // = 83,
//  WLC_E_GTK_PLUMBED,                      // = 84,
//  WLC_E_ASSOC_IND_NDIS,                   // = 85,
//  WLC_E_REASSOC_IND_NDIS,                 // = 86,
//  WLC_E_ASSOC_REQ_IE,                     // = 87,
//  WLC_E_ASSOC_RESP_IE,                    // = 88,
//  WLC_E_ASSOC_RECREATED,                  // = 89,
//  WLC_E_ACTION_FRAME_RX_NDIS,             // = 90,
//  WLC_E_AUTH_REQ,                         // = 91,
//  WLC_E_MESH_DHCP_SUCCESS,                // = 92,
//  WLC_E_SPEEDY_RECREATE_FAIL,             // = 93,
//  WLC_E_NATIVE,                           // = 94,
//  WLC_E_PKTDELAY_IND,                     // = 95,
//  WLC_E_AWDL_AW,                          // = 96,
//  WLC_E_AWDL_ROLE,                        // = 97,
//  WLC_E_AWDL_EVENT,                       // = 98,
//  WLC_E_NIC_AF_TXS,                       // = 99,
//  WLC_E_NAN,                              // = 100,
//  WLC_E_BEACON_FRAME_RX,                  // = 101,
//  WLC_E_SERVICE_FOUND,                    // = 102,
//  WLC_E_GAS_FRAGMENT_RX,                  // = 103,
//  WLC_E_GAS_COMPLETE,                     // = 104,
//  WLC_E_P2PO_ADD_DEVICE,                  // = 105,
//  WLC_E_P2PO_DEL_DEVICE,                  // = 106,
//  WLC_E_WNM_STA_SLEEP,                    // = 107,
//  WLC_E_TXFAIL_THRESH,                    // = 108,
//  WLC_E_PROXD,                            // = 109,
//  WLC_E_MESH_PAIRED,                      // = 110,
//  WLC_E_AWDL_RX_PRB_RESP,                 // = 111,
//  WLC_E_AWDL_RX_ACT_FRAME,                // = 112,
//  WLC_E_AWDL_WOWL_NULLPKT,                // = 113,
//  WLC_E_AWDL_PHYCAL_STATUS,               // = 114,
//  WLC_E_AWDL_OOB_AF_STATUS,               // = 115,
//  WLC_E_AWDL_SCAN_STATUS,                 // = 116,
//  WLC_E_AWDL_AW_START,                    // = 117,
//  WLC_E_AWDL_AW_END,                      // = 118,
//  WLC_E_AWDL_AW_EXT,                      // = 119,
//  WLC_E_AWDL_PEER_CACHE_CONTROL,          // = 120,
//  WLC_E_CSA_START_IND,                    // = 121,
//  WLC_E_CSA_DONE_IND,                     // = 122,
//  WLC_E_CSA_FAILURE_IND,                  // = 123,
//  WLC_E_CCA_CHAN_QUAL,                    // = 124,
//  WLC_E_BSSID,                            // = 125,
//  WLC_E_TX_STAT_ERROR,                    // = 126,
//  WLC_E_BCMC_CREDIT_SUPPORT,              // = 127,
//  WLC_E_PSTA_PRIMARY_INTF_IND,            // = 128,
//  WLC_E_BT_WIFI_HANDOVER_REQ,             // = 130,
//  WLC_E_SPW_TXINHIBIT,                    // = 131,
//  WLC_E_FBT_AUTH_REQ_IND,                 // = 132,
//  WLC_E_RSSI_LQM,                         // = 133,
//  WLC_E_PFN_GSCAN_FULL_RESULT,            // = 134,
//  WLC_E_PFN_SWC,                          // = 135,
//  WLC_E_AUTHORIZED,                       // = 136,
//  WLC_E_PROBREQ_MSG_RX,                   // = 137, Событие поступает слишком часто
//  WLC_E_PFN_SCAN_COMPLETE,                // = 138,
//  WLC_E_RMC_EVENT,                        // = 139,
//  WLC_E_DPSTA_INTF_IND,                   // = 140,
//  WLC_E_RRM,                              // = 141,
//  WLC_E_ULP,                              // = 146,
//  WLC_E_TKO,                              // = 151,
//  WLC_E_EXT_AUTH_REQ,                     // = 187,
//  WLC_E_EXT_AUTH_FRAME_RX,                // = 188,
//  WLC_E_MGMT_FRAME_TXSTATUS,              // = 189,
//  WLC_E_LAST,                             // = 190,
//  WLC_E_NONE  //  В конце этого списка всегда должно идти WLC_E_NONE
//};

static uint16_t          whd_sta_event_index;
static uint16_t          whd_ap_event_index;
static T_wifi_scan_cbl   scbl;
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Set_whd_interface_MAC_address(whd_mac_t *mac_addr, uint8_t offs)
{
  mac_addr->octet[0] = 0x74;
  mac_addr->octet[1] = 0x90;
  mac_addr->octet[2] = 0x50;
  mac_addr->octet[3] = g_cpu_id[1];
  mac_addr->octet[4] = g_cpu_id[2];
  mac_addr->octet[5] = g_cpu_id[3] + offs;
}



/*-----------------------------------------------------------------------------------------------------


  \param void

  \return whd_interface_t
-----------------------------------------------------------------------------------------------------*/
whd_interface_t WIFI_get_whd_interface(void)
{
  return g_prim_whd_intf_ptr;
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


  \param void

  \return T_wifi_scan_cbl*
-----------------------------------------------------------------------------------------------------*/
T_wifi_scan_cbl* WIFI_get_scan_cbl(void)
{
  return &scbl;
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
        version[strlen(version) - 1] = 0;
        NETLOG("WiFi fw: %s",version);
      }
    }
    App_free(version);
  }
  NETLOG("WiFi Access Point Channel = %d, SSID = %s, CNTRY = %08X", ivar.wifi_ap_channel, ivar.wifi_ap_ssid, (uint32_t)g_whd_init_cfg.country);

}

/*-----------------------------------------------------------------------------------------------------
  Перехватчик событий при обмене с WiFi модулем в интерфейсе станции
  Перехватываются только события перечисленные в массиве events_list
  Вызывается в контекте задачи с именем WHD
  по цепочке whd_thread_func -> whd_thread_receive_one_packet -> whd_sdpcm_process_rx_packet -> whd_process_bdc_event ->

  Коды event_header->reason объявлены в типе whd_event_reason_t в файле whd_events_int.h строка 363
  Коды event_header->status объявлены в типе whd_event_status_t в файле whd_events_int.h строка 255
  Коды приходят непосредственно из WiFi модуля с сообщением

  \param ifp
  \param event_header
  \param event_data
  \param handler_user_data - Пользовательские данные поступающие на вход. Эти же данные поступают и на выход. Сами данные порождаются при выполнении этой функции

  \return void* - Ссылка на пользовательские данные
-----------------------------------------------------------------------------------------------------*/
static void* WHD_STA_event_handler(whd_interface_t ifp, const whd_event_header_t *event_header, const uint8_t *event_data, void *handler_user_data)
{
  NETLOG("WIFI. STA Event: %d (%s) Reason: %d, Status: %d ", event_header->event_type, WHD_event_to_string(event_header->event_type), event_header->reason, event_header->status);

  if  ((event_header->event_type == (uint32_t)WLC_E_LINK) && (event_header->reason == 0))
  {
    // Произошло подключение устройства к AP
    if (ifp == g_prim_whd_intf_ptr)
    {
      WIFI_STA_save_remote_ap_addr((whd_mac_t *)&(event_header->addr));
      Send_net_event(NET_FLG_STA_CONNECTED);
      NETLOG("WIFI. Connection to remote AP      %02X:%02X:%02X:%02X:%02X:%02X",event_header->addr.octet[0], event_header->addr.octet[1],event_header->addr.octet[2], event_header->addr.octet[3], event_header->addr.octet[4], event_header->addr.octet[5]);
    }
  }
  if (((event_header->event_type == (uint32_t)WLC_E_LINK) && ((event_header->reason == 1) || (event_header->reason == 2)))
      ||  ((event_header->event_type == (uint32_t)WLC_E_DEAUTH_IND) && (event_header->reason == 7)))
  {
    // Произошло отключение устройства от AP
    if (ifp == g_prim_whd_intf_ptr)
    {
      Send_net_event(NET_FLG_STA_DISCONNECTED);
      NETLOG("WIFI. Disconnection from remote AP %02X:%02X:%02X:%02X:%02X:%02X",event_header->addr.octet[0], event_header->addr.octet[1],event_header->addr.octet[2], event_header->addr.octet[3], event_header->addr.octet[4], event_header->addr.octet[5]);
    }
  }
  return handler_user_data;
}

/*-----------------------------------------------------------------------------------------------------
  Перехватчик событий при обмене с WiFi модулем в интерфейсе точки доступа
  Перехватываются только события перечисленные в массиве events_list
  Вызывается в контекте задачи с именем WHD
  по цепочке whd_thread_func -> whd_thread_receive_one_packet -> whd_sdpcm_process_rx_packet -> whd_process_bdc_event ->

  Событие происходят при удачном подключении к модулю в режиме точки доступа
  - Event: 4 (WLC_E_AUTH_IND) Reason: 0, Status: 0
  - Event: 8 (WLC_E_ASSOC_IND) Reason: 0, Status: 0  (Через 0.05 сек)
  - Event: 136 (WLC_E_AUTHORIZED) Reason: 0, Status: 136 (Через 0.9 сек)

  События происходят при отключении компьютера от модуля в режиме точки доступа
  - Event: 6 (WLC_E_DEAUTH_IND) Reason: 1, Status: 0
  - Event: 67 (WLC_E_WAI_STA_EVENT) Reason: 0, Status: 0

  События происходят при отключении мобильного телефона от модуля в режиме точки доступа
  - Event: 12 (WLC_E_DISASSOC_IND) Reason: 8, Status: 0
  - Event: 67 (WLC_E_WAI_STA_EVENT) Reason: 0, Status: 0

  При потери связи модуля в режиме точки доступа с присоединенным к нему клиентом
  - Event: 20 (WLC_E_TXFAIL) Reason: 0, Status: 2
  - Event: 12 (WLC_E_DISASSOC_IND) Reason: 8, Status: 0
  - Event: 67 (WLC_E_WAI_STA_EVENT) Reason: 0, Status: 0

  \param ifp
  \param event_header
  \param event_data
  \param handler_user_data - Пользовательские данные поступающие на вход. Эти же данные поступают и на выход. Сами данные порождаются при выполнении этой функции

  \return void* - Ссылка на пользовательские данные
-----------------------------------------------------------------------------------------------------*/
static void* WHD_AP_event_handler(whd_interface_t ifp, const whd_event_header_t *event_header, const uint8_t *event_data, void *handler_user_data)
{
  NETLOG("WIFI. AP Event: %d (%s) Reason: %d, Status: %d ", event_header->event_type, WHD_event_to_string(event_header->event_type), event_header->reason, event_header->status);

  if (event_header->event_type == (uint32_t)WLC_E_AUTHORIZED)
  {
    // Произошло подключение к устройству станции
    if (ifp == g_secd_whd_intf_ptr)
    {
      WIFI_AP_save_remote_sta_addr((whd_mac_t *)&(event_header->addr));
      Send_net_event(NET_FLG_AP_CONNECTED);
      NETLOG("WIFI. Remote STA connection        %02X:%02X:%02X:%02X:%02X:%02X",event_header->addr.octet[0], event_header->addr.octet[1],event_header->addr.octet[2], event_header->addr.octet[3], event_header->addr.octet[4], event_header->addr.octet[5]);
    }
  }
  if ((event_header->event_type == (uint32_t)WLC_E_WAI_STA_EVENT) || (event_header->event_type == (uint32_t)WLC_E_DEAUTH))
  {
    // Произошло отключение от устройства станции
    if (ifp == g_secd_whd_intf_ptr)
    {
      Send_net_event(NET_FLG_AP_DISCONNECTED);
      NETLOG("WIFI. Remote STA disconnection     %02X:%02X:%02X:%02X:%02X:%02X",event_header->addr.octet[0], event_header->addr.octet[1],event_header->addr.octet[2], event_header->addr.octet[3], event_header->addr.octet[4], event_header->addr.octet[5]);
    }
  }
  return handler_user_data;
}

/*-----------------------------------------------------------------------------------------------------


  \param max_assoc

  \return wwd_result_t
-----------------------------------------------------------------------------------------------------*/
whd_result_t WHD_wifi_set_AP_max_associations(uint32_t max_assoc)
{
  uint32_t     res;
  res = whd_wifi_set_iovar_buffer(g_prim_whd_intf_ptr, IOVAR_STR_MAX_ASSOC,&max_assoc, 4);
  return res;
}


/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WIFI_init_module(void)
{
  uint32_t              res;
  whd_mac_t             macaddr;
  uint32_t              err_line;

  // С кодами стран WHD_COUNTRY_POLAND и WHD_COUNTRY_LITHUANIA в режиме станции не происходит активизация модуля
  // Коды других стран не проверялись
  g_whd_init_cfg.country                                      = WHD_COUNTRY_UNITED_STATES;
  g_whd_init_cfg.thread_priority                              = WHD_TASK_PRIO;
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

  res = whd_init(&g_whd_driver,&g_whd_init_cfg,&g_whd_resource,&g_whd_buff_funcs,&g_whd_net_funcs);
  if (res != WHD_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }


  g_whd_sdio_cfg.high_speed_sdio_clock                        = WHD_TRUE;
  #ifdef SDIO1_1BIT_MODE
  g_whd_sdio_cfg.sdio_1bit_mode                               = WHD_TRUE;
  #else
  g_whd_sdio_cfg.sdio_1bit_mode                               = WHD_FALSE;
  #endif
  g_whd_sdio_cfg.oob_config.host_oob_pin                      = 0;
  g_whd_sdio_cfg.oob_config.dev_gpio_sel                      = 0;
  g_whd_sdio_cfg.oob_config.is_falling_edge                   = WHD_FALSE;
  g_whd_sdio_cfg.oob_config.intr_priority                     = 0;
  g_whd_sdio_cfg.oob_config.drive_mode                        = CYHAL_GPIO_DRIVE_NONE;
  g_whd_sdio_cfg.oob_config.init_drive_state                  = WHD_FALSE;


  g_sdhc_obj = (void *)&g_sdio1;
  res = whd_bus_sdio_attach(g_whd_driver,&g_whd_sdio_cfg,&g_sdhc_obj);
  if (res != WHD_SUCCESS)
  {
    whd_bus_sdio_detach(g_whd_driver);
    err_line = __LINE__;
    goto _err;
  }

  Set_whd_interface_MAC_address(&primary_mac_addr, 1);

  // Первый интерфейс предназначен для организации STA (Station) или AP (Access Point) если организуется только один интерфейс
  res = whd_wifi_on(g_whd_driver,&g_prim_whd_intf_ptr,&primary_mac_addr);
  if (res != WHD_SUCCESS)
  {
    whd_wifi_off(g_prim_whd_intf_ptr);
    err_line = __LINE__;
    goto _err;
  }

  // Если для первичного интерфейса установка MAC адреса в функции whd_wifi_on не работает
  // то здесь выполняем чтение MAC адреса непоспредсвенно из WiFi модуля и запись его в структуру интерфейса
  res = whd_wifi_get_mac_address(g_prim_whd_intf_ptr,&macaddr);
  if (res == WHD_SUCCESS)
  {
    memcpy(&(g_prim_whd_intf_ptr->mac_addr),&macaddr, sizeof(macaddr));
  }


  Set_whd_interface_MAC_address(&secondary_mac_addr, 2);
  // Если запускаются одновременно два интерфейса, то второй интерфейс предназначен только для организазии AP (Access Point)
  if (whd_add_secondary_interface(g_whd_driver,&secondary_mac_addr,&g_secd_whd_intf_ptr) != WHD_SUCCESS)
  {
    whd_wifi_off(g_prim_whd_intf_ptr);
    err_line = __LINE__;
    goto _err;
  }

  // Если для вторичного интерфейса установка MAC адреса в функции whd_wifi_on не работает
  // то здесь выполняем чтение MAC адреса непоспредсвенно из WiFi модуля и запись его в структуру интерфейса
  res = whd_wifi_get_mac_address(g_secd_whd_intf_ptr,&macaddr);
  if (res == WHD_SUCCESS)
  {
    memcpy(&(g_secd_whd_intf_ptr->mac_addr),&macaddr, sizeof(macaddr));
  }


  WHD_wifi_set_AP_max_associations(1);

  res = whd_wifi_set_event_handler(g_prim_whd_intf_ptr, (uint32_t *)events_list,WHD_STA_event_handler,NULL,&whd_sta_event_index);
  if (res != WHD_SUCCESS)
  {
    whd_wifi_off(g_prim_whd_intf_ptr);
    err_line = __LINE__;
    goto _err;
  }

  res = whd_wifi_set_event_handler(g_secd_whd_intf_ptr, (uint32_t *)events_list,WHD_AP_event_handler,NULL,&whd_ap_event_index);
  if (res != WHD_SUCCESS)
  {
    whd_wifi_off(g_prim_whd_intf_ptr);
    err_line = __LINE__;
    goto _err;
  }

  WIFI_log_module_info();
  NETLOG("WIFI initialized. MAC: %02X:%02X:%02X:%02X:%02X:%02X", g_prim_whd_intf_ptr->mac_addr.octet[0],g_prim_whd_intf_ptr->mac_addr.octet[1],g_prim_whd_intf_ptr->mac_addr.octet[2],g_prim_whd_intf_ptr->mac_addr.octet[3],g_prim_whd_intf_ptr->mac_addr.octet[4],g_prim_whd_intf_ptr->mac_addr.octet[5]);
  return RES_OK;

_err:

  NETLOG("WIFI initialization error in line %d. Result=%04X", err_line , res);
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param result_ptr
  \param user_data
  \param status
-----------------------------------------------------------------------------------------------------*/
static void WIFI_scan_callback(whd_scan_result_t **result_ptr, void *user_data, whd_scan_status_t status)
{
  whd_scan_result_t   *res_ptr;
  uint32_t             i;

  if (status == WHD_SCAN_COMPLETED_SUCCESSFULLY)
  {
    scbl.scan_in_busy = 0;
  }
  if (result_ptr != NULL)
  {
    res_ptr =  *result_ptr;

    for (i = 0; i < scbl.scan_results_num; i++)
    {
      if (memcmp(&scbl.scan_results[i].BSSID,&res_ptr->BSSID, sizeof(whd_mac_t)) == 0)
      {
        memcpy(&scbl.scan_results[i], res_ptr, sizeof(whd_scan_result_t));
        break;
      }
    }

    if (i == scbl.scan_results_num)
    {
      if (scbl.scan_results_num >= MAX_SCAN_RESULTS_RECORDS) return;
      scbl.scan_results_num++;
      // Новый обнаруженный SSID
      memcpy(&scbl.scan_results[i], res_ptr, sizeof(whd_scan_result_t));
    }

  }
}

/*-----------------------------------------------------------------------------------------------------
  Начало сканированя точек доступа

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t  WIFI_start_scan(void)
{
  uint32_t res;

  if (scbl.scan_in_busy) return WHD_SUCCESS;

  // Выделяем массив для хранения результатов сканирования
  scbl.scan_in_busy = 1;
  scbl.scan_results_num = 0;
  if (scbl.scan_results != NULL) App_free(scbl.scan_results);
  scbl.scan_results = App_malloc(sizeof(whd_scan_result_t) * MAX_SCAN_RESULTS_RECORDS);
  if (scbl.scan_results == NULL) return RES_ERROR;


  res = whd_wifi_scan(
                      g_prim_whd_intf_ptr,
                      WHD_SCAN_TYPE_ACTIVE,   //Specifies whether the scan should be Active, Passive or scan Prohibited channels
                      WHD_BSS_TYPE_ANY,       // Specifies whether the scan should search for Infrastructure networks (those using an Access Point), Ad-hoc networks, or both types.
                      NULL,                   // optional_ssid.  If this is non-Null, then the scan will only search for networks using the specified SSID.
                      NULL,                   // optional_mac . If this is non-Null, then the scan will only search for networks where the BSSID (MAC address of the Access Point) matches the specified MAC address.
                      NULL,                   // optional_channel_list. If this is non-Null, then the scan will only search for networks on the specified channels - array of channel numbers to search, terminated with a zero
                      NULL,                   // optional_extended_params. If this is non-Null, then the scan will obey the specifications about dwell times and number of probes.
                      WIFI_scan_callback,     // callback. The callback function which will receive and process the result data.
                      &scbl.scan_result,           // result_ptr. Pointer to a pointer to a result storage structure.
                      NULL                    // user_data. user specific data that will be passed directly to the callback function
                     );

  if (res == WHD_SUCCESS)
  {

    return RES_OK;
  }
  else
  {
    App_free(scbl.scan_results);
    scbl.scan_in_busy = 0;
    return RES_ERROR;
  }
}


/*-----------------------------------------------------------------------------------------------------
  Управление процессом инициализации Wi-Fi модуля, поиском точек доступа и проч.

  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_wifi_manager(ULONG initial_input)
{
  uint32_t cnt;
  uint32_t actual_flags;


  for (cnt = 0; cnt < MAX_SDIO_INIT_ATTEMPT_NUM; cnt++)
  {
    // Подаем питание на WiFi модуль с выдержкой паузы на установление
    WiFi_module_switch_ON();

    // Инициализируем интерфейс SDIO1 к кторому подключен WiFi модуль
    if (Init_SDIO1() == RES_OK) break;
    // Если неудача, то выключаем питание и снова повторяем попытку инциализации SDIO
    WiFi_module_switch_OFF();
    Wait_ms(1000);
  }

  if (cnt == MAX_SDIO_INIT_ATTEMPT_NUM)
  {
    NETLOG("Exhausted number of attempts to initialize SDIO1");
    ivar.en_wifi_module = 0;
    Request_save_nv_parameters(Get_mod_params_instance(), BOOTL_PARAMS);
    NETLOG("SDIO1 interface is disabled");
    return;
  }

  for (cnt = 0; cnt < MAX_WIFI_INIT_ATTEMPT_NUM; cnt++)
  {
    // Подготавливаем ресурсы для загрузки в модуль
    if (WHD_open_resource_files() == RES_OK)
    {
      if (WIFI_init_module() == RES_OK)
      {
        WHD_close_resource_files();
        break;
      }
      // Не удалость инициализировать модуль
      WHD_close_resource_files();
      Wait_ms(1000);
    }
    else
    {
      // Не удалость подготовить ресурсы
      Wait_ms(1000);
    }
  }

  if (cnt == MAX_WIFI_INIT_ATTEMPT_NUM)
  {
    NETLOG("Exhausted number of attempts to initialize WiFi module");
    return;
  }

  // Здесь модуль инициализирован
  // Организуем точку доступа
  WIFI_AP_start();

  Send_wifi_event(NET_EVT_WIFI_INIT_DONE);

  // Организуем станцию
  do
  {
    if (Wait_wifi_event(0xFFFFFFFF,&actual_flags, 10) == TX_SUCCESS)
    {
      if (actual_flags & NET_EVT_WIFI_STA_DISJOINED)
      {
        // Если поступил сигнал о разъединении то стразу попытаться соединиться вновь
        WIFI_STA_Join();
      }
      if (actual_flags & NET_EVT_START_WIFI_SCAN)
      {
        // Поступил сигнал о начале сканирования
        WIFI_start_scan();
      }
    }

    // Повторяем попытки подсоединения к точкам доступа в случае остсутствия связи
    if (WIFI_STA_disjoint_time_sec() > 120)
    {
      WIFI_STA_Join();
    }



  } while (1);


}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_WiFi_manager_create(void)
{
  uint32_t res;

  res = tx_thread_create(&wifi_man_thread, "WiFi_man", Thread_wifi_manager,
                         0,
                         (void *)thread_wifi_man_stack, // stack_start
                         THREAD_WIFI_MAN_STACK_SIZE,    // stack_size
                         THREAD_WIFI_MAN_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
                         THREAD_WIFI_MAN_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (res == TX_SUCCESS)
  {
    NETLOG("WiFi manager task created.");
    return RES_OK;
  }
  else
  {
    NETLOG("WiFi manager task creating error %04X.", res);
    return RES_ERROR;
  }
}

