#ifndef CYBT_PLATFORM_UTIL_H
  #define CYBT_PLATFORM_UTIL_H

  #include <stdbool.h>

  #ifdef __cplusplus
extern "C"
{
  #endif

  /**
   * Get current status of sleep mode.
   *
   * @returns  true : bt sleep mode is already enabled
   *           false: bt sleep mode is disabled
   *
   */
  bool cybt_platform_get_sleep_mode_status(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

