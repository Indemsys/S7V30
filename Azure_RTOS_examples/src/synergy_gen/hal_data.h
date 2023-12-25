/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
  #define HAL_DATA_H_
  #include <stdint.h>
  #include "bsp_api.h"
  #include "common_data.h"
  #include "sf_power_profiles_v2.h"
  #include "r_flash_hp.h"
  #include "r_flash_api.h"
  #include "r_dmac.h"
  #include "r_transfer_api.h"
  #include "r_sdmmc.h"
  #include "r_sdmmc_api.h"
  #include "r_gpt.h"
  #include "r_timer_api.h"
  #include "r_adc.h"
  #include "r_adc_api.h"
  #include "r_rtc.h"
  #include "r_rtc_api.h"
  #ifdef __cplusplus
extern "C"
{
  #endif

#if POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Standby
  extern const ioport_cfg_t NULL;
#endif

#if POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Standby
  extern const ioport_cfg_t NULL;
#endif

  extern const flash_instance_t g_flash0;
  extern const rtc_instance_t   g_rtc0;

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* HAL_DATA_H_ */
