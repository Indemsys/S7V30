// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-07
// 10:43:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "whd.h"
#include   "whd_int.h"
#include   "whd_wifi_api.h"
#include   "whd_types.h"

extern  whd_driver_t        g_whd_driver;
extern  whd_interface_t     WIFI_get_whd_interface(void);
extern  whd_init_config_t* WIFI_get_config(void);

static void Do_Copy_Settings_to_File(uint8_t keycode);
static void Do_Show_WiFi_driver_diagnostic(uint8_t keycode);

const T_VT100_Menu_item MENU_BASE_ITEMS[] =
{
  { '1', Do_Show_WiFi_driver_diagnostic  , 0 },
  { '2', Do_Copy_Settings_to_File       , 0 },
  { 'R', 0, 0 },
  { 'M', 0,(void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_APPLICATION       =
{
  "APP diagnostic",
  "\033[5C <1> - WiFi driver diagnostic\r\n"
  "\033[5C <2> - Copy settings to a file\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_BASE_ITEMS,
};


/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void Do_Copy_Settings_to_File(uint8_t keycode)
{
  static const char *file_name = "_Settings.JSON";
  uint32_t res;
  GET_MCBL;

  ivar.en_formated_settings = 1;
  ivar.en_compress_settins  = 0;
  res = Save_settings_to(&wvar_inst, MEDIA_TYPE_FILE, (char *)file_name, APPLICATION_PARAMS);
  if (res == RES_OK)
  {
    MPRINTF(VT100_CLR_LINE"Settings saved to file %s successfully!\n\r\n\r", file_name);
  }
  else
  {
    MPRINTF(VT100_CLR_LINE"Settings don't saved. An error has occurred!\n\r\n\r");
  }
  Wait_ms(2000);
}

#define TMP_BUFS_SZ     256
#define WL_NUMCHANNELS  64

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show_AP_associations(void)
{
  GET_MCBL;
  char               *buf;
  uint32_t            res;
  whd_maclist_t      *ml;
  uint32_t            max_associations  =0;

  MPRINTF("\r\n");

  res = whd_wifi_ap_get_max_assoc(WIFI_get_whd_interface(),&max_associations);
  if (res == WHD_SUCCESS)
  {
    MPRINTF(VT100_CLR_LINE" Access Point max associations number = %d\r\n", max_associations);

    buf = App_malloc(TMP_BUFS_SZ);
    if (buf != NULL)
    {
      ml = (whd_maclist_t *)buf;
      ml->count = max_associations;
      MPRINTF(VT100_CLR_LINE" Associated clients: \r\n");
      res = whd_wifi_get_associated_client_list(WIFI_get_whd_interface(), buf, TMP_BUFS_SZ);
      if (res == WHD_SUCCESS)
      {
        if (ml->count == 0)
        {
          MPRINTF(VT100_CLR_LINE" No associated clients.\r\n");
        }
        else
        {
          for (uint32_t i=0; i < ml->count; i++)
          {
            int32_t rssi;
            MPRINTF(VT100_CLR_LINE" Station MAC Address : %02X:%02X:%02X:%02X:%02X:%02X", ml->mac_list[i].octet[0], ml->mac_list[i].octet[1], ml->mac_list[i].octet[2], ml->mac_list[i].octet[3], ml->mac_list[i].octet[4], ml->mac_list[i].octet[5]);
            res = whd_wifi_get_ap_client_rssi(WIFI_get_whd_interface(),&rssi,&(ml->mac_list[i]));
            if (res == WHD_SUCCESS)
            {
              MPRINTF("  RSSI=%d dBm\r\n",rssi);
            }
            else
            {
              MPRINTF("\r\n");
            }
          }
        }
      }
      else
      {
        MPRINTF(VT100_CLR_LINE" Request error %08X.\r\n", res);
      }
      App_free(buf);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Show_channel_list(void)
{
  GET_MCBL;
  uint32_t            res;
  whd_list_t         *channel_list;

  channel_list = App_malloc(WL_NUMCHANNELS * 4+ 4);
  if (channel_list != NULL)
  {
    MPRINTF(" Available channels list :");
    channel_list->count = WL_NUMCHANNELS;
    res = whd_wifi_get_channels(WIFI_get_whd_interface(),channel_list);
    if (res == WHD_SUCCESS)
    {
      for (uint32_t i=0; i < channel_list->count; i++)
      {
        MPRINTF(" %d", channel_list->element[i]);
      }
    }
    App_free(channel_list);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show_WHD_statistic(void)
{
  GET_MCBL;

  MPRINTF("\n\r");
  MPRINTF(VT100_CLR_LINE" WHD layer statistic\r\n");
  MPRINTF(VT100_CLR_LINE" ----------------\r\n");
  MPRINTF(VT100_CLR_LINE" tx_total    = %d\r\n", g_whd_driver->whd_stats.tx_total);
  MPRINTF(VT100_CLR_LINE" rx_total    = %d\r\n", g_whd_driver->whd_stats.rx_total);
  MPRINTF(VT100_CLR_LINE" tx_no_mem   = %d\r\n", g_whd_driver->whd_stats.tx_no_mem);
  MPRINTF(VT100_CLR_LINE" rx_no_mem   = %d\r\n", g_whd_driver->whd_stats.rx_no_mem);
  MPRINTF(VT100_CLR_LINE" tx_fail     = %d\r\n", g_whd_driver->whd_stats.tx_fail);
  MPRINTF(VT100_CLR_LINE" no_credit   = %d\r\n", g_whd_driver->whd_stats.no_credit);
  MPRINTF(VT100_CLR_LINE" flow_control= %d\r\n", g_whd_driver->whd_stats.flow_control);


}
/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Show_WiFi_driver_diagnostic(void)
{
  whd_mac_t           mac;
  uint32_t            res;
  char               *version;
  whd_init_config_t  *cfg_ptr;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  WIFI driver diagnostic ===\n\r");
  MPRINTF(" ESC - exit\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");

  if (WiFi_module_initialized_flag() == 0)
  {
    MPRINTF(VT100_CLR_LINE" WIFI module not initialized!\r\n");
    return;
  }

  version =  App_malloc(200);
  if (version != NULL)
  {
    res = whd_wifi_get_wifi_version(WIFI_get_whd_interface(),version, 200);
    if (res == WHD_SUCCESS)
    {
      if (strlen(version) > 1)
      {
        version[strlen(version)-1] = 0;
        MPRINTF(VT100_CLR_LINE"Firmware version   : %s\r\n",version);
      }
    }
    App_free(version);
  }

  MPRINTF("\n\r");
  whd_wifi_get_mac_address(WIFI_get_whd_interface(),&mac);
  MPRINTF(VT100_CLR_LINE"Module MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac.octet[0], mac.octet[1], mac.octet[2], mac.octet[3], mac.octet[4], mac.octet[5]);
  MPRINTF("\n\r");

  switch (wifi_composition)
  {
  case WIFI_ONLY_AP:
    cfg_ptr = WIFI_get_config();
    MPRINTF(VT100_CLR_LINE"WiFi mode          : Access Point. Channel = %d, SSID = %s, CNTRY = %08X\r\n", ivar.wifi_ap_channel, ivar.wifi_ap_ssid, (uint32_t)cfg_ptr->country);
    _Show_channel_list();
    _Show_AP_associations();
    break;
  case WIFI_ONLY_STA:
    MPRINTF(VT100_CLR_LINE"WiFi mode          : Station\r\n");
    break;
  case WIFI_STA_AND_AP:
    MPRINTF(VT100_CLR_LINE"WiFi mode          : Station & Access Point\r\n");
    break;
  default:
    MPRINTF(VT100_CLR_LINE"WiFi mode          : Unknown\r\n");
    break;
  }

  _Show_WHD_statistic();
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void Do_Show_WiFi_driver_diagnostic(uint8_t keycode)
{
  uint8_t                  b;
  GET_MCBL;
  _Show_WiFi_driver_diagnostic();

  do
  {
    if (WAIT_CHAR(&b,  ms_to_ticks(1000)) == RES_OK)
    {

      switch (b)
      {
      case VT100_ESC:
        return;
      }
    }
    _Show_WiFi_driver_diagnostic();

  } while (1);

}
