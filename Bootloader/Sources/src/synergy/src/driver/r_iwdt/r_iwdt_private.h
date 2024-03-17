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
 * File Name    : r_iwdt_private.h
 * Description  : Independent Watchdog Timer (IWDT) private macros and typedefs.
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @addtogroup IWDT
 * @{
 **********************************************************************************************************************/

#ifndef R_IWDT_PRIVATE_H
#define R_IWDT_PRIVATE_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "hw/hw_iwdt_private.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define IWDT_PRV_OSF0_AUTO_START_MASK    (0x00000002U)
#define IWDT_PRV_OSF0_NMI_REQUEST_MASK   (0x00001000U)
#define IWDT_PRV_OFS0_TIMEOUT_MASK       (0x0000000CU)
#define IWDT_PRV_OFS0_CLOCK_DIVIDER_MASK (0x000000F0U)
#define IWDT_PRV_OFS0_WINDOW_END_MASK    (0x00000300U)
#define IWDT_PRV_OFS0_WINDOW_START_MASK  (0x00000C00U)
#define IWDT_PRV_OFS0_RESET_CONTROL_MASK (0x00001000U)
#define IWDT_PRV_OFS0_STOP_CONTROL_MASK  (0x00004000U)
#define IWDT_PRV_IWDTSR_COUNTER_MASK     (0x3FFFU)
#define IWDT_PRV_IWDTSR_FLAGS_MASK       (0x0000C000U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
#endif /* R_IWDT_PRIVATE_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup IWDT)
 **********************************************************************************************************************/
