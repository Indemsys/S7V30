/***********************************************************************************************************************
 * Copyright [2016] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_lpmv2_api.h
 * Description  : LPMV2 interface
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup LPMV2_API Low Power Modes V2 Interface
 * @brief Interface for accessing low power modes.
 *
 * @section LPMV2_API_SUMMARY Summary
 * This section defines the API for the LPMV2 (Low Power Mode) Driver.
 * The LPMV2 Driver provides functions for controlling power consumption by configuring
 * and transitioning to a low power mode.
 * The LPMV2 driver supports configuration of MCU low power modes using the LPMV2 hardware
 * peripheral. The LPMV2 driver supports low power modes deep standby, standby, sleep, and snooze.
 *
 * @note Not all low power modes are available on all MCUs.
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * LPMV2 Interface description: HAL LPMV2 Interface
 *
 * @{
 **********************************************************************************************************************/
#ifndef DRV_LPMV2_API_H
#define DRV_LPMV2_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define LPMV2_API_VERSION_MAJOR (2U)
#define LPMV2_API_VERSION_MINOR (4U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Low power modes */
typedef enum e_lpmv2_low_power_mode
{
    LPMV2_LOW_POWER_MODE_SLEEP,             ///< Sleep mode
    LPMV2_LOW_POWER_MODE_STANDBY,           ///< Software Standby mode
    LPMV2_LOW_POWER_MODE_STANDBY_SNOOZE,    ///< Software Standby mode with Snooze mode enabled
    LPMV2_LOW_POWER_MODE_DEEP,              ///< Deep Software Standby mode
} lpmv2_low_power_mode_t;

/** User configuration structure, used in open function */
typedef struct st_lpmv2_cfg
{
    /** Low Power Mode */
    lpmv2_low_power_mode_t              low_power_mode;
    /** MCU Specific configuration */
    void                  const *       p_extend;
} lpmv2_cfg_t;

/** lpmv2 driver structure. General lpmv2 functions implemented at the HAL layer will follow this API. */
typedef struct st_lpmv2_api
{
    /** Initialization function
     * @par Implemented as
     * - R_LPMV2_Init()
     **/
    ssp_err_t (* init)(void);

    /** Configure a low power mode.
     * @par Implemented as
     * - R_LPMV2_LowPowerConfigure()
     *
     * @param[in]   p_cfg   Pointer to configuration structure. All elements of this structure must be set by user.
     **/
    ssp_err_t (* lowPowerCfg)(lpmv2_cfg_t const * const p_cfg);

    /** Enter low power mode (sleep/standby/deep standby) using WFI macro.
     *  Function will return after waking from low power mode.
     * @par Implemented as
     * - R_LPMV2_LowPowerModeEnter()
     **/
    ssp_err_t (* lowPowerModeEnter)(void);

    /** Get the driver version based on compile time macros.
     * @par Implemented as
     * - R_LPMV2_VersionGet()
     *
     * @param[out]  p_version  Code and API version used.
     **/
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

    /** Clear the IOKEEP bit after deep software standby.
     * * @par Implemented as
     * - R_LPMV2_ClearIOKeep()
    **/
    ssp_err_t (* clearIOKeep)(void);

} lpmv2_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_lpmv2_instance
{
    lpmv2_cfg_t const * const p_cfg;     ///< Pointer to the configuration structure for this instance
    lpmv2_api_t const * const p_api;     ///< Pointer to the API structure for this instance
} lpmv2_instance_t;

#endif /* DRV_LPMV2_API_H */

/*******************************************************************************************************************//**
 * @} (end defgroup LPMV2_API)
 **********************************************************************************************************************/
