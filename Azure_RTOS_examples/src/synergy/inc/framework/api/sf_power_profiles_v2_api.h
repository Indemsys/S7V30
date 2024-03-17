/***********************************************************************************************************************
 * Copyright [2015-2017] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : sf_power_profiles_v2_api.h
 * Description  : The Power Profiles Framework allows applications to control power consumption by defining different
 *                profiles that can be applied dynamically at runtime.
 **********************************************************************************************************************/

#ifndef SF_POWER_PROFILES_V2_API_H
#define SF_POWER_PROFILES_V2_API_H

/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_POWER_PROFILES_V2_API Power Profiles V2 Framework Interface
 * @brief Power Profiles Framework Interface
 *
 * @section SF_POWER_PROFILES_V2_API_SUMMARY Summary
 * This framework allows an application to apply power profiles at runtime. There are 2 types of profiles: Run and
 * Low Power. Applying a Run profile will change things like the system clock and IOPORT settings. The MCU will
 * continue to run during this process and will not be put into a low power mode.
 * Applying a Low Power profile will put the MCU into a low power mode. Which low power mode is used is specified
 * by the LPMv2 instance used. IOPORT settings can also be specified which will be applied before entering the low
 * power mode and after waking up.
 *
 * The Deep Software Standby low power mode, available on some MCUs, will reset the MCU when the woken up. In this
 * case the callback will not be called and the IOPORT configuration will not be applied after waking up.
 *
 * This framework can be used with, or without, an RTOS.
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * Framework Power Profiles Interface description: @ref FrameworkPowerProfilesInterface
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_lpmv2_api.h"
#if (1 == BSP_CFG_RTOS)
    #include "tx_api.h"
#endif
#include "r_ioport_api.h"
#include "r_cgc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Version of the API defined in this file */
#define SF_POWER_PROFILES_V2_API_VERSION_MAJOR (2U)
#define SF_POWER_PROFILES_V2_API_VERSION_MINOR (2U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Options for the callback events. */
typedef enum e_sf_power_profiles_v2_event
{
    SF_POWER_PROFILES_V2_EVENT_PRE_LOW_POWER,   ///< Callback just before entering low power mode
    SF_POWER_PROFILES_V2_EVENT_POST_LOW_POWER   ///< Callback just after exiting the low power mode
} sf_power_profiles_v2_event_t;

/** Power profiles callback arguments definitions  */
typedef struct st_sf_power_profiles_v2_callback_args
{
    sf_power_profiles_v2_event_t   event;        ///< Power profiles callback event
    void                         * p_context;    ///< Placeholder for user data
} sf_power_profiles_v2_callback_args_t;

/** Common control block. DO NOT INITIALIZE.  Initialization occurs when SF_POWER_PROFILES_V2_Open is called */
typedef struct st_sf_power_profiles_v2_ctrl
{
    uint32_t            open;        ///< Used by driver to check if pointer to control block is valid
#if (1 == BSP_CFG_RTOS)
    TX_MUTEX            mutex;       ///< Mutex used to protect access to lower level driver hardware registers
#endif /* (1 == BSP_CFG_RTOS) */
} sf_power_profiles_v2_ctrl_t;

/** Initialization configuration */
typedef struct st_sf_power_profiles_v2_cfg
{
    /** Pointer to additional settings (not currently in use) */
    void                                const * p_extend;
} sf_power_profiles_v2_cfg_t;

/** Run profile configuration */
typedef struct st_sf_power_profiles_v2_run_cfg
{
    /** Pointer to IOPORT settings */
    ioport_cfg_t                        const * p_ioport_pin_tbl;
    /** Pointer to a CGC configuration */
    cgc_clocks_cfg_t                    const * p_clock_cfg;
    /** Pointer to additional settings */
    void                                const * p_extend;
} sf_power_profiles_v2_run_cfg_t;

/** Low Power profile configuration */
typedef struct st_sf_power_profiles_v2_low_power_cfg
{
    /** Pointer to IOPORT settings to apply after exiting the low power mode   */
    ioport_cfg_t                        const * p_ioport_pin_tbl_exit;
    /** Pointer to IOPORT settings to apply before entering low power mode */
    ioport_cfg_t                        const * p_ioport_pin_tbl_enter;
    /** Pointer to an LPMv2 instance */
    lpmv2_instance_t                    const * p_lower_lvl_lpm;
    /** Callback function */
    void                                     (* p_callback)(sf_power_profiles_v2_callback_args_t * p_args);
    /** Placeholder for user data */
    void                                      * p_context;
    /** Pointer to additional settings  */
    void                                const * p_extend;
} sf_power_profiles_v2_low_power_cfg_t;

/** Framework Power Profiles v2 API structure.  Implementations will use the following API. */
typedef struct st_sf_power_profiles_v2_api
{
    /** Initializes the framework.
     * @par Implemented as
     *  - SF_POWER_PROFILES_V2_Open()
     *
     * @param[in,out] p_ctrl   Pointer to a structure allocated by user. Elements initialized here.
     * @param[in]     p_cfg    Pointer to configuration structure. Elements of the structure must be set by user.
     */
    ssp_err_t (* open)(sf_power_profiles_v2_ctrl_t       * const p_ctrl,
                       sf_power_profiles_v2_cfg_t  const * const p_cfg);

    /** Applies a Run profile.
     * @par Implemented as
     *  - SF_POWER_PROFILES_V2_RunApply()
     *
     * @param[in]   p_ctrl     Pointer to control block set in ::SF_POWER_PROFILES_V2_Open.
     * @param[in]   p_cfg    Pointer to configuration structure. Elements of the structure must be set by user.
     */
    ssp_err_t (* runApply)(sf_power_profiles_v2_ctrl_t          * const p_ctrl,
                           sf_power_profiles_v2_run_cfg_t const * const p_cfg);

    /** Applies a Low Power profile.
     * @par Implemented as
     *  - SF_POWER_PROFILES_V2_LowPowerApply()
     *
     * @param[in]   p_ctrl   Pointer to control block set in ::SF_POWER_PROFILES_V2_Open.
     * @param[in]   p_cfg    Pointer to configuration structure. Elements of the structure must be set by user.
     */
    ssp_err_t (* lowPowerApply)(sf_power_profiles_v2_ctrl_t                * const p_ctrl,
                                sf_power_profiles_v2_low_power_cfg_t const * const p_cfg);

    /** Closes the framework.
     * @par Implemented as
     *  - SF_POWER_PROFILES_V2_Close()
     *
     * @param[in]    p_ctrl   Pointer to control block set in ::SF_POWER_PROFILES_V2_Open.
     */
    ssp_err_t (* close)(sf_power_profiles_v2_ctrl_t * const p_ctrl);

    /** Gets version and stores it in provided pointer p_version.
     * @par Implemented as
     *  - SF_POWER_PROFILES_V2_VersionGet()
     *
     * @param[out]  p_version  Code and API version used.
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

} sf_power_profiles_v2_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_power_profiles_v2_instance
{
    sf_power_profiles_v2_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_power_profiles_v2_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    sf_power_profiles_v2_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sf_power_profiles_v2_instance_t;

/*******************************************************************************************************************//**
 * @} (end defgroup SF_POWER_PROFILES_V2_API)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_POWER_PROFILES_V2_API_H */
