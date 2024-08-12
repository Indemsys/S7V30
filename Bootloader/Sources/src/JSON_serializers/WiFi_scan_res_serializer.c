// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-08-11
// 13:31:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



/*-----------------------------------------------------------------------------------------------------


  \param band

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* _get_band_str(whd_802_11_band_t band)
{
  switch ((int)band)
  {
    case WHD_802_11_BAND_5GHZ    : return "5 GHz  ";
    case WHD_802_11_BAND_2_4GHZ  : return "2.4 GHz";
    case WHD_802_11_BAND_6GHZ    : return "6 GHz  ";
  }
  return "UNKNOWN";
}

/*-----------------------------------------------------------------------------------------------------


  \param bss_type

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* _get_bss_type_str(whd_bss_type_t bss_type)
{
  switch ((int)bss_type)
  {
    case WHD_BSS_TYPE_INFRASTRUCTURE  : return "INFRASTRUCTURE";
    case WHD_BSS_TYPE_ADHOC           : return "ADHOC         ";
    case WHD_BSS_TYPE_ANY             : return "ANY           ";
    case WHD_BSS_TYPE_MESH            : return "MESH          ";
  }
  return "UNKNOWN";
}

/*-----------------------------------------------------------------------------------------------------


  \param security

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* _get_security_str(whd_security_t security)
{
  switch ((int)security)
  {
    case WHD_SECURITY_OPEN               : return "OPEN               ";
    case WHD_SECURITY_WEP_PSK            : return "WEP PSK            ";
    case WHD_SECURITY_WEP_SHARED         : return "WEP SHARED         ";
    case WHD_SECURITY_WPA_TKIP_PSK       : return "WPA TKIP PSK       ";
    case WHD_SECURITY_WPA_AES_PSK        : return "WPA AES PSK        ";
    case WHD_SECURITY_WPA_MIXED_PSK      : return "WPA MIXED PSK      ";
    case WHD_SECURITY_WPA2_AES_PSK       : return "WPA2 AES PSK       ";
    case WHD_SECURITY_WPA2_AES_PSK_SHA256: return "WPA2 AES PSK SHA256";
    case WHD_SECURITY_WPA2_TKIP_PSK      : return "WPA2 TKIP PSK      ";
    case WHD_SECURITY_WPA2_MIXED_PSK     : return "WPA2 MIXED PSK     ";
    case WHD_SECURITY_WPA2_FBT_PSK       : return "WPA2 FBT PSK       ";
    case WHD_SECURITY_WPA3_SAE           : return "WPA3 SAE           ";
    case WHD_SECURITY_WPA2_WPA_AES_PSK   : return "WPA2 WPA AES PSK   ";
    case WHD_SECURITY_WPA2_WPA_MIXED_PSK : return "WPA2 WPA MIXED PSK ";
    case WHD_SECURITY_WPA3_WPA2_PSK      : return "WPA3 WPA2 PSK      ";
    case WHD_SECURITY_WPA_TKIP_ENT       : return "WPA TKIP ENT       ";
    case WHD_SECURITY_WPA_AES_ENT        : return "WPA AES ENT        ";
    case WHD_SECURITY_WPA_MIXED_ENT      : return "WPA MIXED ENT      ";
    case WHD_SECURITY_WPA2_TKIP_ENT      : return "WPA2 TKIP ENT      ";
    case WHD_SECURITY_WPA2_AES_ENT       : return "WPA2 AES ENT       ";
    case WHD_SECURITY_WPA2_MIXED_ENT     : return "WPA2 MIXED ENT     ";
    case WHD_SECURITY_WPA2_FBT_ENT       : return "WPA2 FBT ENT       ";
    case WHD_SECURITY_IBSS_OPEN          : return "IBSS OPEN          ";
    case WHD_SECURITY_WPS_SECURE         : return "WPS SECURE         ";
  }
  return "UNKNOWN";
}


/*-----------------------------------------------------------------------------------------------------

  \param sz_ptr

  \return char*
-----------------------------------------------------------------------------------------------------*/
static void  _Print_WiFi_scan_results(T_wifi_scan_cbl *scbl_ptr, char *buff, uint32_t *buff_sz)
{
  char     str[SSID_NAME_SIZE+1];
  uint32_t offset = 0;
  whd_scan_result_t *p;

  offset += Print_to(buff, offset, "[");

  for (uint32_t i=0;i < scbl_ptr->scan_results_num;i++)
  {
    if (i>0) offset += Print_to(buff, offset, ",");
    offset += Print_to(buff, offset, "{");

    p = &scbl_ptr->scan_results[i];
    strncpy(str, (char*)p->SSID.value, p->SSID.length);
    str[p->SSID.length] = 0;
    offset += Print_to(buff, offset, "\"SSID\":\"%s\",",str);
    offset += Print_to(buff, offset, "\"BSID\":\"%02X:%02X:%02X:%02X:%02X:%02X\",",p->BSSID.octet[0],p->BSSID.octet[1],p->BSSID.octet[2],p->BSSID.octet[3],p->BSSID.octet[4],p->BSSID.octet[5] );
    offset += Print_to(buff, offset, "\"band\":\"%s\",",_get_band_str(p->band));
    offset += Print_to(buff, offset, "\"bss_type\":\"%s\",",_get_bss_type_str(p->bss_type));
    offset += Print_to(buff, offset, "\"security\":\"%s\",",_get_security_str(p->security));
    offset += Print_to(buff, offset, "\"max_data_rate\":%d,",p->max_data_rate);
    offset += Print_to(buff, offset, "\"signal_strength\":%d,",p->signal_strength);
    offset += Print_to(buff, offset, "\"channel\":%d,",p->channel);
    if (p->ccode[0] == 0) p->ccode[0] = '?';
    if (p->ccode[1] == 0) p->ccode[1] = '?';
    offset += Print_to(buff, offset, "\"ccode\":\"%c%c\",",p->ccode[0],p->ccode[1]);
    offset += Print_to(buff, offset, "\"flags\":\"0x%02X\"",p->flags);

    offset += Print_to(buff, offset, "}");
  }
  offset += Print_to(buff, offset, "]");
  *buff_sz = offset;
}

/*-----------------------------------------------------------------------------------------------------


  \param sz_ptr

  \return char*
-----------------------------------------------------------------------------------------------------*/
char*  WiFi_scan_results_serializer_to_buff(uint32_t *sz_ptr)
{
  uint32_t         buff_sz = 0;
  char            *buffer = NULL;
  T_wifi_scan_cbl *scbl_ptr = WIFI_get_scan_cbl();

  if (scbl_ptr->scan_in_busy) return NULL;
  scbl_ptr->scan_in_busy = 1;

  _Print_WiFi_scan_results(scbl_ptr, NULL,&buff_sz);

  buffer = App_malloc_pending(buff_sz + 1, MS_TO_TICKS(100));
  if (buffer == NULL)
  {
    scbl_ptr->scan_in_busy = 0;
    return NULL;
  }

  _Print_WiFi_scan_results(scbl_ptr, buffer, sz_ptr);

  scbl_ptr->scan_in_busy = 0;
  return buffer;

}
