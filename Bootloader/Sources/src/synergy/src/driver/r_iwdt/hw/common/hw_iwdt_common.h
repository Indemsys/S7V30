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
 * File Name    : hw_iwdt_common.h
 * Description  : Independent Watchdog Timer (IWDT) HW API header file.
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @addtogroup IWDT
 * @{
 **********************************************************************************************************************/

#ifndef HW_IWDT_COMMON_H
#define HW_IWDT_COMMON_H

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
 * Writes H'00 then H'FF to IWDTRR register
 *
 **********************************************************************************************************************/
__STATIC_INLINE void HW_IWDT_Refresh (R_IWDT_Type * p_iwdt_reg)
{
    p_iwdt_reg->IWDTRR = 0x00;
    p_iwdt_reg->IWDTRR = 0xFF;
}

/*******************************************************************************************************************//**
 * Reads status value from IWDTSR
 * @retval Value of IWDTSR (IWDT StatusRegister)
 **********************************************************************************************************************/
__STATIC_INLINE uint16_t HW_IWDT_IWDTSRRead (R_IWDT_Type * p_iwdt_reg)
{
    return p_iwdt_reg->IWDTSR;
}

/*******************************************************************************************************************//**
 * Writes to WDTCSTPR register
 * @param[in] value   Value to write to IWDTSR
 **********************************************************************************************************************/
__STATIC_INLINE void HW_IWDT_IWDTSRWrite (R_IWDT_Type * p_iwdt_reg, uint16_t value)
{
    p_iwdt_reg->IWDTSR = value;
}

/*******************************************************************************************************************//**
 * Enables IWDT underflow/refresh error interrupt. This interrupt will generate an NMI.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_IWDT_InterruptEnable (R_ICU_Type * p_icu_reg)
{
    p_icu_reg->NMIER_b.IWDTEN = 1;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_IWDT_COMMON_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup IWDT)
 **********************************************************************************************************************/
