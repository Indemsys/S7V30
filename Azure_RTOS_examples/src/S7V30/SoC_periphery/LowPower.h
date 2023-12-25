#ifndef LOWPOWER_H
  #define LOWPOWER_H

#ifdef NULL
#define POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Standby (0)
#else
#define POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Standby (1)
#endif

#ifdef NULL
#define POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Standby (0)
#else
#define POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Standby (1)
#endif

#ifdef NULL
#define POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Standby (0)
#else
#define POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Standby (1)
#endif

#if POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Standby
  void NULL(sf_power_profiles_v2_callback_args_t *p_args);
#endif
  /** Power Profiles run structure */
  extern sf_power_profiles_v2_low_power_cfg_t g_sf_power_profiles_Standby;
#ifdef NULL
#define POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Sleep (0)
#else
#define POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Sleep (1)
#endif
#if POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Sleep
  extern const ioport_cfg_t NULL;
#endif
#ifdef NULL
#define POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Sleep (0)
#else
#define POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Sleep (1)
#endif
#if POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Sleep
  extern const ioport_cfg_t NULL;
#endif
#ifdef NULL
#define POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Sleep (0)
#else
#define POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Sleep (1)
#endif
#if POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Sleep
  void NULL(sf_power_profiles_v2_callback_args_t *p_args);
#endif
  /** Power Profiles run structure */
  extern sf_power_profiles_v2_low_power_cfg_t g_sf_power_profiles_Sleep;
#ifdef NULL
#define POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Deep_Standby (0)
#else
#define POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Deep_Standby (1)
#endif
#if POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_Deep_Standby
  extern const ioport_cfg_t NULL;
#endif
#ifdef NULL
#define POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Deep_Standby (0)
#else
#define POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Deep_Standby (1)
#endif
#if POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_Deep_Standby
  extern const ioport_cfg_t NULL;
#endif
#ifdef NULL
#define POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Deep_Standby (0)
#else
#define POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Deep_Standby (1)
#endif
#if POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_Deep_Standby
  void NULL(sf_power_profiles_v2_callback_args_t *p_args);
#endif


/** CGC Clocks */
extern const cgc_clocks_cfg_t g_cgc_cfg_240Mhz;
/** CGC Clocks */
extern const cgc_clocks_cfg_t g_cgc_cfg_24Mhz;
/** lpmv2 Instance */
extern const lpmv2_instance_t g_lpmv2_standby0;
/** lpmv2 Instance */
extern const lpmv2_instance_t g_lpmv2_sleep0;
/** lpmv2 Instance */

/** Power Profiles on Power Profiles instance */
extern sf_power_profiles_v2_instance_t g_sf_power_profiles_v2_common;

void sf_power_profiles_v2_init0(void);



#endif // LOWPOWER_H



