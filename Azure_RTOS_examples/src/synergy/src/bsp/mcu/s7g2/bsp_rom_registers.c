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
/***********************************************************************************************************************
* File Name    : bsp_rom_registers.c
* Description  : Defines MCU registers that are in ROM (e.g. OFS) and must be set at compile-time. All registers
*                can be set using bsp_cfg.h.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup BSP_MCU_S7G2
 * @defgroup BSP_MCU_ROM_REGISTER_S7G2 ROM Registers
 *
 * Defines MCU registers that are in ROM (e.g. OFS) and must be set at compile-time. All registers can be set
 * using bsp_cfg.h.
 *
 * @{
 **********************************************************************************************************************/



/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "bsp_api.h"

#if defined(BSP_MCU_GROUP_S7G2)

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/** OR in the HOCO frequency setting from bsp_clock_cfg.h with the OFS1 setting from bsp_cfg.h. */
#define BSP_ROM_REG_OFS1_SETTING        (((uint32_t)BSP_CFG_ROM_REG_OFS1 & 0xFFFFF9FFU) | ((uint32_t)BSP_CFG_HOCO_FREQUENCY << 9))

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
***********************************************************************************************************************/
 
/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
/** ROM registers defined here. Secure MPU functionality has been removed from the S7 UM so those registers are now
 * set to 0xFFFFFFFF. */
/*LDRA_INSPECTED 57 D - This global is being initialized at it's declaration below. */
BSP_DONT_REMOVE static const uint32_t g_bsp_rom_registers[] BSP_PLACE_IN_SECTION_V2(BSP_SECTION_ROM_REGISTERS) =
{
    (uint32_t)BSP_CFG_ROM_REG_OFS0,
    (uint32_t)BSP_ROM_REG_OFS1_SETTING,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU,
    0xFFFFFFFFU
};


/** ID code definitions defined here. */
/*LDRA_INSPECTED 57 D - This global is being initialized at it's declaration below. */
BSP_DONT_REMOVE static const uint32_t g_bsp_id_codes[] BSP_PLACE_IN_SECTION_V2(BSP_SECTION_ID_CODE_1) =
{
    BSP_CFG_ID_CODE_LONG_1,         /* 0x40120050 */
    BSP_CFG_ID_CODE_LONG_2,         /* 0x40120054 */
    BSP_CFG_ID_CODE_LONG_3,         /* 0x40120058 */
    BSP_CFG_ID_CODE_LONG_4          /* 0x4012005c */
};

#endif


/** @} (end defgroup BSP_MCU_ROM_REGISTER_S7G2) */
