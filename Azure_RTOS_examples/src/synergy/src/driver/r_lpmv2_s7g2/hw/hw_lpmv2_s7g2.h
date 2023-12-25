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
 * File Name    : hw_lpmv2_s7g2.h
 * Description  : LPMV2 S7G2 specific macros.
 **********************************************************************************************************************/

#ifndef HW_LPMV2_S7G2_H_
#define HW_LPMV2_S7G2_H_

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define HW_LPMV2_SNOOZE_REQUEST_MASK                0x7342FFFFU
#define HW_LPMV2_SNOOZE_END_MASK                    0x000000FFU
#define HW_LPMV2_STANDBY_WAKE_SOURCE_MASK           0xFF4FFFFFU

#define HW_LPMV2_DEEP_STANDBY_CANCEL_SOURCE_MASK    0x071FFFFFU
#define HW_LPMV2_DEEP_STANDBY_CANCEL_EDGE_MASK      0x0013FFFFU

#define DPSBYCR_DEEPCUT_MASK    (3U)
#define DPSBYCR_DPSBY_SHIFT     (7U)
#define DPSBYCR_DPSBY_MASK      (1U << DPSBYCR_DPSBY_SHIFT)
#define DPSBYCR_IOKEEP_MASK     (1U)

#define SNZCR_RXDREQEN_SHIFT    (0U)
#define SNZCR_RXDREQEN_MASK     (1U << SNZCR_RXDREQEN_SHIFT)
#define SNZCR_SNZDTCEN_MASK     (1U)
#define SNZCR_SNZE_SHIFT        (7U)
#define SNZCR_SNZE_MASK         (1U << SNZCR_SNZE_SHIFT)
#define SBYCR_SSBY_SHIFT        (15U)
#define SBYCR_SSBY_MASK         (1U << SBYCR_SSBY_SHIFT)
#define SBYCR_OPE_MASK          (1U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

#if LPMV2_CFG_PARAM_CHECKING_ENABLE
    ssp_err_t HW_LPMV2_MCUSpecificLowPowerCheck (lpmv2_cfg_t const * const p_cfg);
#endif /* LPMV2_CFG_PARAM_CHECKING_ENABLE */
void HW_LPMV2_MCUSpecificConfigure (lpmv2_cfg_t const * const p_cfg);
ssp_err_t HW_LPMV2_LowPowerModeEnter (void);
void HW_LPMV2_SnoozeConfigure (lpmv2_snooze_dtc_t  dtc_state_in_snooze,
                               lpmv2_snooze_request_t request_source,
                               lpmv2_snooze_end_bits_t end_sources,
                               lpmv2_snooze_cancel_t cancel_sources);
bool HW_LPMV2_WaitForOperatingModeFlags(void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      This function locks LPMV2 registers
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_RegisterLock (void)
{
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_OM_LPC_BATT);
}

/*******************************************************************************************************************//**
 * @brief      This function unlocks LPMV2 registers
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_RegisterUnLock (void)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_OM_LPC_BATT);
}

/*******************************************************************************************************************//**
 * @brief Set the value of the SNZREQCR register, sources to trigger entry to snooze mode from standby mode
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_SNZREQCRSet (uint32_t value)
{
    R_SYSTEM->SNZREQCR = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of SELSR0 register with the sources to trigger exit from snooze mode back to normal mode
 * @param  value    The event as the source of canceling snooze mode.
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_SELSR0Set (lpmv2_snooze_cancel_t value)
{
    R_ICU->SELSR0_b.SELS = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of the SNZEDCR register, sources to trigger exit from snooze mode back to standby mode
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_SNZEDCRSet (uint8_t value)
{
    R_SYSTEM->SNZEDCR = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of WUPEN register, sources for waking from standby mode
 * @param  value    The value to set
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_WUPENSet (uint32_t value)
{
    R_ICU->WUPEN = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIER0 register, deep standby cancel sources
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIER0Set (uint8_t value)
{
    R_SYSTEM->DPSIER0 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIER1 register, deep standby cancel sources
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIER1Set (uint8_t value)
{
    R_SYSTEM->DPSIER1 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIER2 register, deep standby cancel sources
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIER2Set (uint8_t value)
{
    R_SYSTEM->DPSIER2 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIER3 register, deep standby cancel sources
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIER3Set (uint8_t value)
{
    R_SYSTEM->DPSIER3 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIEGR0 register, deep standby cancel sources edges
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIEGR0Set (uint8_t value)
{
    R_SYSTEM->DPSIEGR0 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIEGR1 register, deep standby cancel sources edges
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIEGR1Set (uint8_t value)
{
    R_SYSTEM->DPSIEGR1 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIEGR2 register, deep standby cancel sources edges
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIEGR2Set (uint8_t value)
{
    R_SYSTEM->DPSIEGR2 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIFR0 register, deep standby cancel sources flags, write 0 to clear
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIFR0Set (uint8_t value)
{
    R_SYSTEM->DPSIFR0 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIFR1 register, deep standby cancel sources flags, write 0 to clear
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIFR1Set (uint8_t value)
{
    R_SYSTEM->DPSIFR1 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIFR2 register, deep standby cancel sources flags, write 0 to clear
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIFR2Set (uint8_t value)
{
    R_SYSTEM->DPSIFR2 = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of DPSIFR3 register, deep standby cancel sources flags, write 0 to clear
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSIFR3Set (uint8_t value)
{
    R_SYSTEM->DPSIFR3 = value;
}

/*******************************************************************************************************************//**
 * @brief   Get the value of the DPSIFR0 register.
 * @returns  The register value
 * @note    This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_LPMV2_DPSIFR0Get (void)
{
    return R_SYSTEM->DPSIFR0;
}

/*******************************************************************************************************************//**
 * @brief   Get the value of the DPSIFR1 register.
 * @returns  The register value
 * @note    This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_LPMV2_DPSIFR1Get (void)
{
    return R_SYSTEM->DPSIFR1;
}

/*******************************************************************************************************************//**
 * @brief   Get the value of the DPSIFR2 register.
 * @returns  The register value
 * @note    This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_LPMV2_DPSIFR2Get (void)
{
    return R_SYSTEM->DPSIFR2;
}

/*******************************************************************************************************************//**
 * @brief   Get the value of the DPSIFR3 register.
 * @returns  The register value
 * @note    This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_LPMV2_DPSIFR3Get (void)
{
    return R_SYSTEM->DPSIFR3;
}

/*******************************************************************************************************************//**
 * @brief Clear the DOCDF bit
 * @param  value    The value to set
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DOCDFClear (void)
{
    /*
     * From the S7G2 user's manual: 11.2.23 System Control OCD Control Register (SYOCDCR)
     * Clear the DOCDF flag to 0 before transitioning to Deep Software Standby mode.
     */
    R_SYSTEM->SYOCDCR_b.DOCDF = 0U;
}

/*******************************************************************************************************************//**
 * @brief Set DPSBY bit, Deep Standby mode enabled if SBYCR.SSBY = 1, no effect if SBYCR.SSBY = 0.
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSBYSet (void)
{
    R_SYSTEM->DSPBYCR |= DPSBYCR_DPSBY_MASK;
}

/*******************************************************************************************************************//**
 * @brief Clear DPSBY bit, Deep Standby mode disabled if SBYCR.SSBY = 1, no effect if SBYCR.SSBY = 0.
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_DPSBYClear (void)
{
    R_SYSTEM->DSPBYCR &= (uint8_t)~DPSBYCR_DPSBY_MASK;
}

/*******************************************************************************************************************//**
 * @brief Set STCONR_b.STCON bit, set to 0 if HOCO is system clock, else 3.
 * @param  stconr_sstby    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_STCONR_SSTBYSet (uint8_t stconr_sstby)
{
    R_SYSTEM->STCONR_b.STCON = 3U & stconr_sstby;
}

/*******************************************************************************************************************//**
 * @brief Clear the IOKEEP bit in DSPBYCR register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_ClearIOKeep()
{
    R_SYSTEM->DSPBYCR_b.IOKEEP = 0U;
}

#endif /* HW_LPMV2_S7G2_H_ */
