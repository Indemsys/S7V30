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
 * File Name    : hw_wdt_common.h
 * Description  : WDT HAL low-level functions to interface with hardware registers.
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @addtogroup WDT
 * @{
 **********************************************************************************************************************/
#ifndef HW_WDT_COMMON_H
#define HW_WDT_COMMON_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Writes H'00 then H'FF to WDTRR register
 *
 **********************************************************************************************************************/
__STATIC_INLINE void HW_WDT_Refresh (R_WDT_Type * p_wdt_reg)
{
    p_wdt_reg->WDTRR = 0x00;
    p_wdt_reg->WDTRR = 0xFF;
}

/*******************************************************************************************************************//**
 * Writes configuration data to WDTCR.
 * @param[in] config   Value to write to WDTCR
 **********************************************************************************************************************/
__STATIC_INLINE void HW_WDT_WDTCRWrite (R_WDT_Type * p_wdt_reg, uint16_t cfg)
{
    p_wdt_reg->WDTCR = cfg;
}

/*******************************************************************************************************************//**
 * Reads configuration from WDTCR.
 * @param[out] WDTCR value
 **********************************************************************************************************************/
__STATIC_INLINE uint16_t HW_WDT_WDTCRRead (R_WDT_Type * p_wdt_reg)
{
    return p_wdt_reg->WDTCR;
}

/*******************************************************************************************************************//**
 * Writes reset request configuration value to WDTRCR.
 * @param[in] reset_request   Value to write to WDTRCR
 **********************************************************************************************************************/
__STATIC_INLINE void HW_WDT_WDTRCRWrite (R_WDT_Type * p_wdt_reg, wdt_reset_control_t reset_request)
{
    p_wdt_reg->WDTRCR = reset_request;
}

/*******************************************************************************************************************//**
 * Reads reset mode from WDTRCR.
 * @param[out] WDTRCR value
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_WDT_WDTRCRRead (R_WDT_Type * p_wdt_reg)
{
    return p_wdt_reg->WDTRCR;
}

/*******************************************************************************************************************//**
 * Writes stop control configuration value to WDTCSTPR.
 * @param[in] stop_value   Value to write to WDTCSTPR
 **********************************************************************************************************************/
__STATIC_INLINE void HW_WDT_WDTCSTPRWrite (R_WDT_Type * p_wdt_reg, wdt_stop_control_t stop_value)
{
    p_wdt_reg->WDTCSTPR = stop_value;
}

/*******************************************************************************************************************//**
 * Reads stop mode from WDTCSTPR.
 * @param[out] WDTCSTPR value
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_WDT_WDTCSTPRRead (R_WDT_Type * p_wdt_reg)
{
    return p_wdt_reg->WDTCSTPR;
}

/*******************************************************************************************************************//**
 * Reads WDTSR register.
 * @retval Value of WDTSR (WDT StatusRegister)
 **********************************************************************************************************************/
__STATIC_INLINE uint16_t HW_WDT_WDTSRRead (R_WDT_Type * p_wdt_reg)
{
    return p_wdt_reg->WDTSR;
}

/*******************************************************************************************************************//**
 * Writes to WDTSR register.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_WDT_WDTSRWrite (R_WDT_Type * p_wdt_reg, uint16_t value)
{
    p_wdt_reg->WDTSR = value;
}

/*******************************************************************************************************************//**
 * Enables WDT underflow/refresh error interrupt. This interrupt will generate an NMI.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_WDT_InterruptEnable (R_ICU_Type * p_icu_reg)
{
    p_icu_reg->NMIER_b.WDTEN = 1;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif ///< HW_WDT_COMMON_H

/*******************************************************************************************************************//**
 * @} (end addtogroup WDT)
 **********************************************************************************************************************/
