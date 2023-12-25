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
 * File Name    : r_iwdt.h
 * Description  : Independent Watchdog Timer (IWDT) public APIs.
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup IWDT IWDT
 * @brief Driver for the Independent Watchdog Timer (IWDT).
 *
 * @section IWDT_SUMMARY Summary
 * This module supports the Independent Watchdog Timer (IWDT). It implements the @ref WDT_API.
 * Extends WDT_API
 * @brief HAL layer drivers for interfacing with the Independent Watchdog Timer (IWDT) peripheral.
 *
 * The IWDT HAL APIs provide the ability to refresh the independent watchdog, read the timer value
 * and read and clear status flags. When used in NMI output mode the callback to be called by the NMI ISR can
 * be registered.
 * @{
 **********************************************************************************************************************/

#ifndef R_IWDT_H
#define R_IWDT_H

#include "bsp_api.h"

#include "r_iwdt_cfg.h"
#include "r_wdt_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define IWDT_CODE_VERSION_MAJOR (1U)
#define IWDT_CODE_VERSION_MINOR (7U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** WDT control block. DO NOT INITIALIZE.  Initialization occurs when wdt_api_t::open is called. */
typedef struct st_iwdt_instance_ctrl
{
    uint32_t      iwdt_open;                           ///< Indicates whether the open() API has been successfully
                                                       ///< called.
    void const  * p_context;                           ///< Placeholder for user data.  Passed to the user callback in
                                                       ///< wdt_callback_args_t.
    R_IWDT_Type * p_reg;                               ///< Pointer to register base address
    void (* p_callback)(wdt_callback_args_t * p_args); ///< Callback provided when a WDT NMI ISR occurs.
} iwdt_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const wdt_api_t g_wdt_on_iwdt;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_IWDT_H

/*******************************************************************************************************************//**
 * @} (end addtogroup IWDT)
 **********************************************************************************************************************/
