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
 * File Name    : r_wdt.h
 * Description  : Watchdog Timer (WDT) public APIs.
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup WDT WDT
 * @brief Driver for the Watchdog Timer (WDT).
 *
 * @section WDT_SUMMARY Summary
 * This module supports the Watchdog Timer (WDT). It implements the @ref WDT_API.
 * The WDT HAL APIs provide the ability to configure the operation of the WDT (when used in register start mode),
 * refresh the watchdog, read the timer value and read and clear status flags.
 * @{
 **********************************************************************************************************************/

#ifndef R_WDT_H
#define R_WDT_H

#include "bsp_api.h"

#include "r_wdt_cfg.h"
#include "r_wdt_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define WDT_CODE_VERSION_MAJOR (1U)
#define WDT_CODE_VERSION_MINOR (8U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** WDT control block. DO NOT INITIALIZE.  Initialization occurs when wdt_api_t::open is called. */
typedef struct st_wdt_instance_ctrl
{
    uint32_t    wdt_open;                              ///< Indicates whether the open() API has been successfully
                                                       ///< called.
    void const  * p_context;                           ///< Placeholder for user data.  Passed to the user callback in
                                                       ///< wdt_callback_args_t.
    R_WDT_Type  * p_reg;                               ///< Pointer to register base address
    void (* p_callback)(wdt_callback_args_t * p_args); ///< Callback provided when a WDT NMI ISR occurs.
} wdt_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const wdt_api_t g_wdt_on_wdt;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif ///< R_WDT_H

/*******************************************************************************************************************//**
 * @} (end addtogroup WDT)
 **********************************************************************************************************************/
