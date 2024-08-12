#include "App.h"
#include "BLE_main.h"

#ifdef ENABLE_BLE_RTT_LOG

/******************************************************************************
 *                           Variables Definitions
 ******************************************************************************/
cybt_platform_trace_cb_t trace_cb =
{
  .trace_level = {
    INITIAL_TRACE_LEVEL_MAIN,
    INITIAL_TRACE_LEVEL_SPIF,
    INITIAL_TRACE_LEVEL_HCITX_TASK,
    INITIAL_TRACE_LEVEL_HCIRX_TASK,
    INITIAL_TRACE_LEVEL_HCI_DRV,
    INITIAL_TRACE_LEVEL_HCI_LOG,
    INITIAL_TRACE_LEVEL_MEMORY,
    INITIAL_TRACE_LEVEL_PRM,
    INITIAL_TRACE_LEVEL_STACK,
    INITIAL_TRACE_LEVEL_APP
  }
};




  #define    BLE_RTT_LOG_STR_SZ    (256)
static char                       ble_rtt_log_str[BLE_RTT_LOG_STR_SZ];
/*-----------------------------------------------------------------------------------------------------


  \param fmt_ptr
-----------------------------------------------------------------------------------------------------*/
void BLE_RTT_LOGs(const char *fmt_ptr, ...)
{
  unsigned int     n;
  va_list          ap;

  va_start(ap, fmt_ptr);

  __disable_interrupt();
  n = vsnprintf(ble_rtt_log_str, BLE_RTT_LOG_STR_SZ, (const char *)fmt_ptr, ap);
  strcat(ble_rtt_log_str,"\r\n");
  SEGGER_RTT_Write(RTT_LOG_CH, ble_rtt_log_str, n + 2);
  __enable_interrupt();

  va_end(ap);
}


/******************************************************************************
 *                           Function Definitions
 ******************************************************************************/

/********************************************************************************
 **
 **    Function Name:   bt_platform_set_trace_level
 **
 **    Purpose:  Set trace level
 **
 **    Input Parameters:  id:    trace id
 **                       level: trace level
 **    Returns:
 **                      Nothing
 **
 *********************************************************************************/
void cybt_platform_set_trace_level(cybt_trace_id_t id, cybt_trace_level_t level)
{
  if (CYBT_TRACE_ID_MAX < level)
  {
    return;
  }

  if (CYBT_TRACE_ID_MAX > id)
  {
    trace_cb.trace_level[id] = level;
  }
  else if (CYBT_TRACE_ID_ALL == id)
  {
    uint8_t idx;

    for (idx = 0; idx < CYBT_TRACE_ID_MAX; idx++)
    {
      trace_cb.trace_level[idx] = level;
    }
  }
}
#endif

