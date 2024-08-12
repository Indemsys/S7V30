// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-01-09
// 15:50:11
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"

#define    WHD_RTT_LOG_STR_SZ    (256)
static char                       whd_rtt_log_str[WHD_RTT_LOG_STR_SZ];


#define CASE_RETURN_STRING(val) case val: return # val;

/*-----------------------------------------------------------------------------------------------------


  \param name
  \param line_num
  \param severity
  \param fmt_ptr
-----------------------------------------------------------------------------------------------------*/
void Send_RTT_log(const char *fmt_ptr, ...)
{
  unsigned int     n;
  va_list          ap;

  va_start(ap, fmt_ptr);

  __disable_interrupt();
  n = vsnprintf(whd_rtt_log_str, WHD_RTT_LOG_STR_SZ, (const char *)fmt_ptr, ap);
  SEGGER_RTT_Write(RTT_LOG_CH, whd_rtt_log_str, n);
  __enable_interrupt();

  va_end(ap);
}


/*-----------------------------------------------------------------------------------------------------


  \param val

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* WHD_event_to_string(uint32_t val)
{
  whd_event_num_t ev = (whd_event_num_t)val;

  switch (ev)
  {
    CASE_RETURN_STRING(WLC_E_NONE)                             // = 0x7FFFFFFE,
    CASE_RETURN_STRING(WLC_E_SET_SSID)                         // = 0,
    CASE_RETURN_STRING(WLC_E_JOIN)                             // = 1,
    CASE_RETURN_STRING(WLC_E_START)                            // = 2,
    CASE_RETURN_STRING(WLC_E_AUTH)                             // = 3,
    CASE_RETURN_STRING(WLC_E_AUTH_IND)                         // = 4,
    CASE_RETURN_STRING(WLC_E_DEAUTH)                           // = 5,
    CASE_RETURN_STRING(WLC_E_DEAUTH_IND)                       // = 6,
    CASE_RETURN_STRING(WLC_E_ASSOC)                            // = 7,
    CASE_RETURN_STRING(WLC_E_ASSOC_IND)                        // = 8,
    CASE_RETURN_STRING(WLC_E_REASSOC)                          // = 9,
    CASE_RETURN_STRING(WLC_E_REASSOC_IND)                      // = 10,
    CASE_RETURN_STRING(WLC_E_DISASSOC)                         // = 11,
    CASE_RETURN_STRING(WLC_E_DISASSOC_IND)                     // = 12,
    CASE_RETURN_STRING(WLC_E_QUIET_START)                      // = 13,
    CASE_RETURN_STRING(WLC_E_QUIET_END)                        // = 14,
    CASE_RETURN_STRING(WLC_E_BEACON_RX)                        // = 15,
    CASE_RETURN_STRING(WLC_E_LINK)                             // = 16,
    CASE_RETURN_STRING(WLC_E_MIC_ERROR)                        // = 17,
    CASE_RETURN_STRING(WLC_E_NDIS_LINK)                        // = 18,
    CASE_RETURN_STRING(WLC_E_ROAM)                             // = 19,
    CASE_RETURN_STRING(WLC_E_TXFAIL)                           // = 20,
    CASE_RETURN_STRING(WLC_E_PMKID_CACHE)                      // = 21,
    CASE_RETURN_STRING(WLC_E_RETROGRADE_TSF)                   // = 22,
    CASE_RETURN_STRING(WLC_E_PRUNE)                            // = 23,
    CASE_RETURN_STRING(WLC_E_AUTOAUTH)                         // = 24,
    CASE_RETURN_STRING(WLC_E_EAPOL_MSG)                        // = 25,
    CASE_RETURN_STRING(WLC_E_SCAN_COMPLETE)                    // = 26,
    CASE_RETURN_STRING(WLC_E_ADDTS_IND)                        // = 27,
    CASE_RETURN_STRING(WLC_E_DELTS_IND)                        // = 28,
    CASE_RETURN_STRING(WLC_E_BCNSENT_IND)                      // = 29,
    CASE_RETURN_STRING(WLC_E_BCNRX_MSG)                        // = 30,
    CASE_RETURN_STRING(WLC_E_BCNLOST_MSG)                      // = 31,
    CASE_RETURN_STRING(WLC_E_ROAM_PREP)                        // = 32,
    CASE_RETURN_STRING(WLC_E_PFN_NET_FOUND)                    // = 33,
    CASE_RETURN_STRING(WLC_E_PFN_NET_LOST)                     // = 34,
    CASE_RETURN_STRING(WLC_E_RESET_COMPLETE)                   // = 35,
    CASE_RETURN_STRING(WLC_E_JOIN_START)                       // = 36,
    CASE_RETURN_STRING(WLC_E_ROAM_START)                       // = 37,
    CASE_RETURN_STRING(WLC_E_ASSOC_START)                      // = 38,
    CASE_RETURN_STRING(WLC_E_IBSS_ASSOC)                       // = 39,
    CASE_RETURN_STRING(WLC_E_RADIO)                            // = 40,
    CASE_RETURN_STRING(WLC_E_PSM_WATCHDOG)                     // = 41,
    CASE_RETURN_STRING(WLC_E_CCX_ASSOC_START)                  // = 42,
    CASE_RETURN_STRING(WLC_E_CCX_ASSOC_ABORT)                  // = 43,
    CASE_RETURN_STRING(WLC_E_PROBREQ_MSG)                      // = 44,
    CASE_RETURN_STRING(WLC_E_SCAN_CONFIRM_IND)                 // = 45,
    CASE_RETURN_STRING(WLC_E_PSK_SUP)                          // = 46,
    CASE_RETURN_STRING(WLC_E_COUNTRY_CODE_CHANGED)             // = 47,
    CASE_RETURN_STRING(WLC_E_EXCEEDED_MEDIUM_TIME)             // = 48,
    CASE_RETURN_STRING(WLC_E_ICV_ERROR)                        // = 49,
    CASE_RETURN_STRING(WLC_E_UNICAST_DECODE_ERROR)             // = 50,
    CASE_RETURN_STRING(WLC_E_MULTICAST_DECODE_ERROR)           // = 51,
    CASE_RETURN_STRING(WLC_E_TRACE)                            // = 52,
    CASE_RETURN_STRING(WLC_E_BTA_HCI_EVENT)                    // = 53,
    CASE_RETURN_STRING(WLC_E_IF)                               // = 54,
    CASE_RETURN_STRING(WLC_E_P2P_DISC_LISTEN_COMPLETE)         // = 55,
    CASE_RETURN_STRING(WLC_E_RSSI)                             // = 56,
    CASE_RETURN_STRING(WLC_E_PFN_BEST_BATCHING)                // = 57,
    CASE_RETURN_STRING(WLC_E_EXTLOG_MSG)                       // = 58,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME)                     // = 59,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME_COMPLETE)            // = 60,
    CASE_RETURN_STRING(WLC_E_PRE_ASSOC_IND)                    // = 61,
    CASE_RETURN_STRING(WLC_E_PRE_REASSOC_IND)                  // = 62,
    CASE_RETURN_STRING(WLC_E_CHANNEL_ADOPTED)                  // = 63,
    CASE_RETURN_STRING(WLC_E_AP_STARTED)                       // = 64,
    CASE_RETURN_STRING(WLC_E_DFS_AP_STOP)                      // = 65,
    CASE_RETURN_STRING(WLC_E_DFS_AP_RESUME)                    // = 66,
    CASE_RETURN_STRING(WLC_E_WAI_STA_EVENT)                    // = 67,
    CASE_RETURN_STRING(WLC_E_WAI_MSG)                          // = 68,
    CASE_RETURN_STRING(WLC_E_ESCAN_RESULT)                     // = 69,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE)   // = 70,
    CASE_RETURN_STRING(WLC_E_PROBRESP_MSG)                     // = 71,
    CASE_RETURN_STRING(WLC_E_P2P_PROBREQ_MSG)                  // = 72,
    CASE_RETURN_STRING(WLC_E_DCS_REQUEST)                      // = 73,
    CASE_RETURN_STRING(WLC_E_FIFO_CREDIT_MAP)                  // = 74,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME_RX)                  // = 75,
    CASE_RETURN_STRING(WLC_E_WAKE_EVENT)                       // = 76,
    CASE_RETURN_STRING(WLC_E_RM_COMPLETE)                      // = 77,
    CASE_RETURN_STRING(WLC_E_HTSFSYNC)                         // = 78,
    CASE_RETURN_STRING(WLC_E_OVERLAY_REQ)                      // = 79,
    CASE_RETURN_STRING(WLC_E_CSA_COMPLETE_IND)                 // = 80,
    CASE_RETURN_STRING(WLC_E_EXCESS_PM_WAKE_EVENT)             // = 81,
    CASE_RETURN_STRING(WLC_E_PFN_SCAN_NONE)                    // = 82,
    CASE_RETURN_STRING(WLC_E_PFN_SCAN_ALLGONE)                 // = 83,
    CASE_RETURN_STRING(WLC_E_GTK_PLUMBED)                      // = 84,
    CASE_RETURN_STRING(WLC_E_ASSOC_IND_NDIS)                   // = 85,
    CASE_RETURN_STRING(WLC_E_REASSOC_IND_NDIS)                 // = 86,
    CASE_RETURN_STRING(WLC_E_ASSOC_REQ_IE)                     // = 87,
    CASE_RETURN_STRING(WLC_E_ASSOC_RESP_IE)                    // = 88,
    CASE_RETURN_STRING(WLC_E_ASSOC_RECREATED)                  // = 89,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME_RX_NDIS)             // = 90,
    CASE_RETURN_STRING(WLC_E_AUTH_REQ)                         // = 91,
    CASE_RETURN_STRING(WLC_E_MESH_DHCP_SUCCESS)                // = 92,
    CASE_RETURN_STRING(WLC_E_SPEEDY_RECREATE_FAIL)             // = 93,
    CASE_RETURN_STRING(WLC_E_NATIVE)                           // = 94,
    CASE_RETURN_STRING(WLC_E_PKTDELAY_IND)                     // = 95,
    CASE_RETURN_STRING(WLC_E_AWDL_AW)                          // = 96,
    CASE_RETURN_STRING(WLC_E_AWDL_ROLE)                        // = 97,
    CASE_RETURN_STRING(WLC_E_AWDL_EVENT)                       // = 98,
    CASE_RETURN_STRING(WLC_E_NIC_AF_TXS)                       // = 99,
    CASE_RETURN_STRING(WLC_E_NAN)                              // = 100,
    CASE_RETURN_STRING(WLC_E_BEACON_FRAME_RX)                  // = 101,
    CASE_RETURN_STRING(WLC_E_SERVICE_FOUND)                    // = 102,
    CASE_RETURN_STRING(WLC_E_GAS_FRAGMENT_RX)                  // = 103,
    CASE_RETURN_STRING(WLC_E_GAS_COMPLETE)                     // = 104,
    CASE_RETURN_STRING(WLC_E_P2PO_ADD_DEVICE)                  // = 105,
    CASE_RETURN_STRING(WLC_E_P2PO_DEL_DEVICE)                  // = 106,
    CASE_RETURN_STRING(WLC_E_WNM_STA_SLEEP)                    // = 107,
    CASE_RETURN_STRING(WLC_E_TXFAIL_THRESH)                    // = 108,
    CASE_RETURN_STRING(WLC_E_PROXD)                            // = 109,
    CASE_RETURN_STRING(WLC_E_MESH_PAIRED)                      // = 110,
    CASE_RETURN_STRING(WLC_E_AWDL_RX_PRB_RESP)                 // = 111,
    CASE_RETURN_STRING(WLC_E_AWDL_RX_ACT_FRAME)                // = 112,
    CASE_RETURN_STRING(WLC_E_AWDL_WOWL_NULLPKT)                // = 113,
    CASE_RETURN_STRING(WLC_E_AWDL_PHYCAL_STATUS)               // = 114,
    CASE_RETURN_STRING(WLC_E_AWDL_OOB_AF_STATUS)               // = 115,
    CASE_RETURN_STRING(WLC_E_AWDL_SCAN_STATUS)                 // = 116,
    CASE_RETURN_STRING(WLC_E_AWDL_AW_START)                    // = 117,
    CASE_RETURN_STRING(WLC_E_AWDL_AW_END)                      // = 118,
    CASE_RETURN_STRING(WLC_E_AWDL_AW_EXT)                      // = 119,
    CASE_RETURN_STRING(WLC_E_AWDL_PEER_CACHE_CONTROL)          // = 120,
    CASE_RETURN_STRING(WLC_E_CSA_START_IND)                    // = 121,
    CASE_RETURN_STRING(WLC_E_CSA_DONE_IND)                     // = 122,
    CASE_RETURN_STRING(WLC_E_CSA_FAILURE_IND)                  // = 123,
    CASE_RETURN_STRING(WLC_E_CCA_CHAN_QUAL)                    // = 124,
    CASE_RETURN_STRING(WLC_E_BSSID)                            // = 125,
    CASE_RETURN_STRING(WLC_E_TX_STAT_ERROR)                    // = 126,
    CASE_RETURN_STRING(WLC_E_BCMC_CREDIT_SUPPORT)              // = 127,
    CASE_RETURN_STRING(WLC_E_PSTA_PRIMARY_INTF_IND)            // = 128,
    CASE_RETURN_STRING(WLC_E_BT_WIFI_HANDOVER_REQ)             // = 130,
    CASE_RETURN_STRING(WLC_E_SPW_TXINHIBIT)                    // = 131,
    CASE_RETURN_STRING(WLC_E_FBT_AUTH_REQ_IND)                 // = 132,
    CASE_RETURN_STRING(WLC_E_RSSI_LQM)                         // = 133,
    CASE_RETURN_STRING(WLC_E_PFN_GSCAN_FULL_RESULT)            // = 134,
    CASE_RETURN_STRING(WLC_E_PFN_SWC)                          // = 135,
    CASE_RETURN_STRING(WLC_E_AUTHORIZED)                       // = 136,
    CASE_RETURN_STRING(WLC_E_PROBREQ_MSG_RX)                   // = 137,
    CASE_RETURN_STRING(WLC_E_PFN_SCAN_COMPLETE)                // = 138,
    CASE_RETURN_STRING(WLC_E_RMC_EVENT)                        // = 139,
    CASE_RETURN_STRING(WLC_E_DPSTA_INTF_IND)                   // = 140,
    CASE_RETURN_STRING(WLC_E_RRM)                              // = 141,
    CASE_RETURN_STRING(WLC_E_ULP)                              // = 146,
    CASE_RETURN_STRING(WLC_E_TKO)                              // = 151,
    CASE_RETURN_STRING(WLC_E_EXT_AUTH_REQ)                     // = 187,
    CASE_RETURN_STRING(WLC_E_EXT_AUTH_FRAME_RX)                // = 188,
    CASE_RETURN_STRING(WLC_E_MGMT_FRAME_TXSTATUS)              // = 189,
    CASE_RETURN_STRING(WLC_E_LAST)                             // = 190,
  default:
    return "Unknown";
  }
}





