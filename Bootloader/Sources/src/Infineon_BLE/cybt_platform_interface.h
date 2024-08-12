#ifndef CYBT_PLATFORM_INTERFACE_H
  #define CYBT_PLATFORM_INTERFACE_H

#include "BLE_main.h"




/******************************************************************************
 *                                Constants
 ******************************************************************************/
  #define  CYBT_TRACE_BUFFER_SIZE    (128)


  #ifdef __cplusplus
extern "C"
{
  #endif

  /*****************************************************************************
   *                           Function Declarations
   *****************************************************************************/

  /**
   * The first platform-port function to be invoked. Initialization for
   * everything (e.g. os components, peripheral driver, timer, etc.) can
   * be put inside this function.
   *
   * @returns  void
   */
  void cybt_platform_init(void);


  /**
   * The platform-port function which is used to de-initialization all
   * the components (e.g. os components, peripheral driver, timer, etc.) which
   * had been intialized for Bluetooth.
   * It will be invoked when BT stack shutdown API is called.
   *
   * @returns  void
   */
  void cybt_platform_deinit(void);


  /**
   * Get memory via OS malloc function.
   *
   * @param[in] req_size: the requested size of memory
   *
   * @returns the pointer of memory block
   */
  void* cybt_platform_malloc(uint32_t req_size);


  /**
   * Return memory to OS via OS free function.
   *
   * @param[in] p_mem_block: the pointer of memory block which was allocated
   *                          by cybt_platform_malloc() function.
   *
   * @return  void
   */
  void cybt_platform_free(void *p_mem_block);


  /**
   * Log printing function. It will be invoked whenever stack has log output.
   * In this function these logs can be forwarded to UART, log task, file system,
   * or somewhere else, depends on the implementation.
   *
   * @param[in] fmt_str :  output trace string
   *
   * @return  void
   */
  void Send_RTT_log(const char *fmt_str, ...);


  /**
   * This function is used by BT stack to get current tick count.
   *  The unit is micro-second.
   *
   * @return the current tick count in micro-second
   */
  uint64_t cybt_platform_get_tick_count_us(void);


  /**
   * This function is used by BT stack to set next timeout in absolute tick count in micro-second.
   *
   * @param[in] abs_tick_us_to_expire: absolute tick count in micro-second to be expired
   *
   * @return
   */
  void cybt_platform_set_next_timeout(uint64_t abs_tick_us_to_expire);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

