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
 * File Name    : hw_lpmv2_s7g2.c
 * Description  : LPMV2 S7G2 hardware implementation.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "bsp_api.h"

#if defined(BSP_MCU_GROUP_S7G2)

#include "r_lpmv2_s7g2.h"
#include "hw_lpmv2_s7g2.h"
#include "r_cgc.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* From user's manual and discussions with hardware group, 
 * using the maximum is safe for all MCUs, will be updated and restored in LPMV2 when entering
 * low power mode on S7 and S5 MCUs (lowPowerModeEnter())
 */
#define SW_STANDBY_HOCOWTR_HSTS     (0x2U)
#define SW_STANDBY_STCONR           (0x0U)
#define SW_STANDBY_WAKE_STCONR      (0x3U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if LPMV2_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t check_deep_standby_config(lpmv2_mcu_cfg_t * p_lpmv2_s7g2_cfg);
static ssp_err_t check_standby_config(lpmv2_mcu_cfg_t * p_lpmv2_s7g2_cfg);
static bool check_low_power_mode(lpmv2_low_power_mode_t low_power_mode);
static ssp_err_t check_snooze_config(lpmv2_snooze_dtc_t dtc_state_in_snooze,
                                     lpmv2_snooze_request_t snooze_request_source);
static bool check_output_port_config(lpmv2_output_port_enable_t output_port_enable);
static bool check_deep_standby_cancel_sources(lpmv2_deep_standby_cancel_source_bits_t deep_standby_cancel_source);
static bool check_deep_standby_cancel_edges(lpmv2_deep_standby_cancel_edge_bits_t deep_standby_cancel_edge);
static bool check_deep_standby_cancel_sources_edges(lpmv2_deep_standby_cancel_source_bits_t deep_standby_cancel_source,
                                                    lpmv2_deep_standby_cancel_edge_bits_t deep_standby_cancel_edge);
static ssp_err_t check_power_supply_state(lpmv2_power_supply_t power_supply_state);
static ssp_err_t check_io_port_state(lpmv2_io_port_t io_port_state);
static bool check_wake_sources(lpmv2_standby_wake_source_bits_t standby_wake_sources);
#endif /* LPMV2_CFG_PARAM_CHECKING_ENABLE */

static bool revert_operating_mode(uint32_t opccr_prewfi, uint32_t sopccr_prewfi);
static ssp_err_t check_snooze_trigger_and_clocks(cgc_clock_t clock_source);
static ssp_err_t set_oscillator_stop_detect(uint32_t ostdcr_ostde);
static void set_hocowtr_hsts(uint32_t hocowtr_hsts);
static void pre_wfi_deep_standby(void);
static ssp_err_t pre_wfi_standby(void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

#if (0 != BSP_CFG_ERROR_LOG)
    extern const char g_lpmv2_module_name[];
    extern const ssp_version_t g_lpmv2_version;
#endif

/*******************************************************************************************************************//**
 * @addtogroup LPMV2_S7G2
 * @{
 **********************************************************************************************************************/

#if LPMV2_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief Verifies all MCU specific settings related to low power modes
 *
 * @param p_cfg the MCU specific configuration
 *
 * @return SSP_SUCCESS              configuration is valid
 * @return SSP_ERR_INVALID_MODE     invalid low power mode
 * @return SSP_ERR_INVALID_POINTER  NULL p_extend when low power mode is not Sleep
 * @return SSP_ERR_INVALID_ARGUMENT invalid snooze entry source
 * @return SSP_ERR_INVALID_ARGUMENT invalid snooze end sources
 * @return SSP_ERR_INVALID_MODE     invalid DTC option for snooze mode
 * @return SSP_ERR_INVALID_MODE     invalid deep standby end sources
 * @return SSP_ERR_INVALID_MODE     invalid deep standby end sources edges
 * @return SSP_ERR_INVALID_MODE     invalid power supply option for deep standby
 * @return SSP_ERR_INVALID_MODE     invalid IO port option for deep standby
 * @return SSP_ERR_INVALID_MODE     invalid output port state setting for standby or deep standby
 * @return SSP_ERR_INVALID_MODE     invalid sources for wake from standby mode
 *
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
ssp_err_t HW_LPMV2_MCUSpecificLowPowerCheck(lpmv2_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

    LPMV2_ERROR_RETURN(true == check_low_power_mode(p_cfg->low_power_mode),
                       SSP_ERR_INVALID_MODE);

    if(LPMV2_LOW_POWER_MODE_SLEEP != p_cfg->low_power_mode)
    {
        LPMV2_ERROR_RETURN(NULL != p_cfg->p_extend, SSP_ERR_INVALID_POINTER);

        lpmv2_mcu_cfg_t * p_lpmv2_s7g2_cfg = (lpmv2_mcu_cfg_t * )(p_cfg->p_extend);

        if(LPMV2_LOW_POWER_MODE_STANDBY_SNOOZE == p_cfg->low_power_mode)
        {
            err = check_snooze_config(p_lpmv2_s7g2_cfg->dtc_state_in_snooze,
                                                p_lpmv2_s7g2_cfg->snooze_request_source);
            LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);
        }
        else if(LPMV2_LOW_POWER_MODE_DEEP == p_cfg->low_power_mode)
        {
            err = check_deep_standby_config(p_lpmv2_s7g2_cfg);
            LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);
        }

        err = check_standby_config(p_lpmv2_s7g2_cfg);
        LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Verifies MCU specific settings related to deep standby. Internal function, do not use directly.
 *
 * @param p_lpmv2_s7g2_cfg the MCU specific configuration
 *
 * @return SSP_SUCCESS              configuration is valid
 * @return SSP_ERR_INVALID_MODE     invalid power supply option for deep standby
 * @return SSP_ERR_INVALID_MODE     invalid IO port option for deep standby
 * @return SSP_ERR_INVALID_MODE     invalid deep standby end sources
 * @return SSP_ERR_INVALID_MODE     invalid deep standby end sources edges
 *
 **********************************************************************************************************************/
static ssp_err_t check_deep_standby_config(lpmv2_mcu_cfg_t * p_lpmv2_s7g2_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

    LPMV2_ERROR_RETURN(true ==
        check_deep_standby_cancel_sources(p_lpmv2_s7g2_cfg->deep_standby_cancel_source),
        SSP_ERR_INVALID_MODE);

    LPMV2_ERROR_RETURN(true ==
        check_deep_standby_cancel_edges(p_lpmv2_s7g2_cfg->deep_standby_cancel_edge),
        SSP_ERR_INVALID_MODE);

    LPMV2_ERROR_RETURN(true == check_deep_standby_cancel_sources_edges(
                            p_lpmv2_s7g2_cfg->deep_standby_cancel_source,
                            p_lpmv2_s7g2_cfg->deep_standby_cancel_edge),
                       SSP_ERR_INVALID_MODE);

    err = check_power_supply_state(p_lpmv2_s7g2_cfg->power_supply_state);
    LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);

    err = check_io_port_state(p_lpmv2_s7g2_cfg->io_port_state);
    LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Verifies MCU specific settings related to standby. Internal function, do not use directly.
 *
 * @param p_lpmv2_s7g2_cfg the MCU specific configuration
 *
 * @return SSP_SUCCESS              configuration is valid
 * @return SSP_ERR_INVALID_MODE     invalid power supply option for standby
 * @return SSP_ERR_INVALID_MODE     invalid IO port option for standby
 * @return SSP_ERR_INVALID_MODE     invalid standby end sources
 * @return SSP_ERR_INVALID_MODE     invalid standby end sources edges
 *
 **********************************************************************************************************************/
static ssp_err_t check_standby_config(lpmv2_mcu_cfg_t * p_lpmv2_s7g2_cfg)
{
    LPMV2_ERROR_RETURN(true == check_wake_sources(p_lpmv2_s7g2_cfg->standby_wake_sources), SSP_ERR_INVALID_MODE);

    LPMV2_ERROR_RETURN(true == check_output_port_config(p_lpmv2_s7g2_cfg->output_port_enable), SSP_ERR_INVALID_MODE);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Verifies the requested low power mode.  Internal function, do not use directly.
 *
 * @param low_power_mode the requested low power mode
 *
 * @return true     valid low power mode
 * @return false    invalid low power mode
 *
 **********************************************************************************************************************/
static bool check_low_power_mode(lpmv2_low_power_mode_t low_power_mode)
{
    if((LPMV2_LOW_POWER_MODE_SLEEP          == low_power_mode)  ||
       (LPMV2_LOW_POWER_MODE_STANDBY        == low_power_mode)  ||
       (LPMV2_LOW_POWER_MODE_STANDBY_SNOOZE == low_power_mode)  ||
       (LPMV2_LOW_POWER_MODE_DEEP           == low_power_mode)
       )
    {
        return true;
    }

    return false;
}

/*******************************************************************************************************************//**
 * @brief Verifies MCU specific settings related to snooze mode. Internal function, do not use directly.
 *
 * @param dtc_state_in_snooze   DTC enabled or disabled in snooze mode
 * @param snooze_request_source Snoozed request source
 *
 * @return SSP_SUCCESS              configuration is valid
 * @return SSP_ERR_INVALID_MODE     invalid DTC option for snooze mode
 * @return SSP_ERR_INVALID_ARGUMENT invalid snooze entry source
 *
 *
 **********************************************************************************************************************/
static ssp_err_t check_snooze_config(lpmv2_snooze_dtc_t dtc_state_in_snooze,
                                     lpmv2_snooze_request_t snooze_request_source)
{
    LPMV2_ERROR_RETURN(0U == ((uint32_t)dtc_state_in_snooze & (uint32_t)~(uint32_t)1U), SSP_ERR_INVALID_MODE);
    LPMV2_ERROR_RETURN(0U == ((uint32_t)snooze_request_source & ~HW_LPMV2_SNOOZE_REQUEST_MASK), SSP_ERR_INVALID_ARGUMENT);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Verifies MCU specific settings related to output ports. Internal function, do not use directly.
 *
 * @param output_port_enable   output port mode
 *
 * @return true     output port mode is valid
 * @return false    invalid output port mode
 *
 **********************************************************************************************************************/
static bool check_output_port_config(lpmv2_output_port_enable_t output_port_enable)
{
    bool valid = false;

    if((LPMV2_OUTPUT_PORT_ENABLE_HIGH_IMPEDANCE == output_port_enable) ||
       (LPMV2_OUTPUT_PORT_ENABLE_RETAIN         == output_port_enable)
       )
    {
        valid = true;
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief Verifies the deep standby cancel sources. Internal function, do not use directly.
 *
 * @param deep_standby_cancel_source   deep standby cancel sources
 *
 * @return true     sources are valid
 * @return false    invalid sources
 *
 **********************************************************************************************************************/
static bool check_deep_standby_cancel_sources(lpmv2_deep_standby_cancel_source_bits_t deep_standby_cancel_source)
{
    bool valid = false;

    if(0U == (uint32_t)(~HW_LPMV2_DEEP_STANDBY_CANCEL_SOURCE_MASK & deep_standby_cancel_source))
    {
        valid = true;
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief Verifies the deep standby cancel edges. Internal function, do not use directly.
 *
 * @param deep_standby_cancel_source   deep standby cancel edges
 *
 * @return true     edges are valid
 * @return false    invalid edges
 *
 **********************************************************************************************************************/
static bool check_deep_standby_cancel_edges(lpmv2_deep_standby_cancel_edge_bits_t deep_standby_cancel_edge)
{
    bool valid = false;

    if(0U == (uint32_t)(~HW_LPMV2_DEEP_STANDBY_CANCEL_EDGE_MASK & deep_standby_cancel_edge))
    {
        valid = true;
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief Compares deep standby cancel edges and sources. Internal function, do not use directly.
 *
 * @param deep_standby_cancel_source   deep standby cancel edges
 * @param deep_standby_cancel_source   deep standby cancel sources
 *
 * @return true     sources and edges are valid
 * @return false    invalid edges with respect to the requested sources
 *
 **********************************************************************************************************************/
static bool check_deep_standby_cancel_sources_edges(lpmv2_deep_standby_cancel_source_bits_t deep_standby_cancel_source,
                                                    lpmv2_deep_standby_cancel_edge_bits_t deep_standby_cancel_edge)
{
    bool valid = false;

    if(0U == (~deep_standby_cancel_source & (lpmv2_deep_standby_cancel_source_t)deep_standby_cancel_edge))
    {
        valid = true;
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief Verifies MCU specific settings related to power supply state. Internal function, do not use directly.
 *
 * @param power_supply_state    power supply state in standby and deep standby
 *
 * @return SSP_SUCCESS              power supply state is valid
 * @return SSP_ERR_INVALID_MODE     power supply state is invalid
 *
 **********************************************************************************************************************/
static ssp_err_t check_power_supply_state(lpmv2_power_supply_t power_supply_state)
{

    LPMV2_ERROR_RETURN((LPMV2_POWER_SUPPLY_DEEPCUT0 == power_supply_state) ||
                       (LPMV2_POWER_SUPPLY_DEEPCUT1 == power_supply_state) ||
                       (LPMV2_POWER_SUPPLY_DEEPCUT3 == power_supply_state),
                       SSP_ERR_INVALID_MODE);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Verifies MCU specific settings related to io port state. Internal function, do not use directly.
 *
 * @param io_supply_state    io port state in standby and deep standby
 *
 * @return SSP_SUCCESS              io port state is valid
 * @return SSP_ERR_INVALID_MODE     io port state is invalid
 *
 **********************************************************************************************************************/
static ssp_err_t check_io_port_state(lpmv2_io_port_t io_port_state)
{
    LPMV2_ERROR_RETURN((LPMV2_IO_PORT_RESET     == io_port_state) ||
                       (LPMV2_IO_PORT_NO_CHANGE == io_port_state),
                       SSP_ERR_INVALID_MODE);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Check wake up sources. Internal function, do not use directly.
 *
 * @param standby_wake_sources   standby wake sources
 *
 * @return true     sources are valid
 * @return false    invalid sources
 *
 **********************************************************************************************************************/
static bool check_wake_sources(lpmv2_standby_wake_source_bits_t standby_wake_sources)
{
    bool valid = false;

    /* All reserved bits in WUPEN are 0 after reset. All reserved bits should be written as 0. */
    if(0U == ((uint32_t)standby_wake_sources & ~HW_LPMV2_STANDBY_WAKE_SOURCE_MASK))
    {
        valid = true;
    }

    return valid;
}
#endif /* LPMV2_CFG_PARAM_CHECKING_ENABLE */

/*******************************************************************************************************************//**
 * @brief Configures all MCU specific settings related to low power modes
 *
 * @param p_cfg the MCU specific configuration
 *
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
void HW_LPMV2_MCUSpecificConfigure(lpmv2_cfg_t const * const p_cfg)
{
    R_SYSTEM->SBYCR &= (uint16_t)~SBYCR_SSBY_MASK;

    R_SYSTEM->SNZCR &= (uint8_t)~SNZCR_SNZE_MASK;

    HW_LPMV2_DPSBYClear();

    if((LPMV2_LOW_POWER_MODE_SLEEP != p_cfg->low_power_mode) &&
       (NULL != p_cfg->p_extend))
    {
        lpmv2_mcu_cfg_t * p_lpmv2_s7g2_cfg = (lpmv2_mcu_cfg_t * )(p_cfg->p_extend);

        if(LPMV2_LOW_POWER_MODE_DEEP == p_cfg->low_power_mode)
        {
            HW_LPMV2_DPSBYSet();

            R_SYSTEM->DSPBYCR_b.DEEPCUT = DPSBYCR_DEEPCUT_MASK & p_lpmv2_s7g2_cfg->power_supply_state;

            HW_LPMV2_DPSIER0Set((uint8_t)(0xFFU & (uint8_t)(p_lpmv2_s7g2_cfg->deep_standby_cancel_source       )));
            HW_LPMV2_DPSIER1Set((uint8_t)(0xFFU & (uint8_t)(p_lpmv2_s7g2_cfg->deep_standby_cancel_source >> 8U )));
            HW_LPMV2_DPSIER2Set((uint8_t)(0xFFU & (uint8_t)(p_lpmv2_s7g2_cfg->deep_standby_cancel_source >> 16U)));
            HW_LPMV2_DPSIER3Set((uint8_t)(0xFFU & (uint8_t)(p_lpmv2_s7g2_cfg->deep_standby_cancel_source >> 24U)));

            HW_LPMV2_DPSIEGR0Set((uint8_t)(0xFFU & (uint8_t)(p_lpmv2_s7g2_cfg->deep_standby_cancel_edge       )));
            HW_LPMV2_DPSIEGR1Set((uint8_t)(0xFFU & (uint8_t)(p_lpmv2_s7g2_cfg->deep_standby_cancel_edge >> 8U )));
            HW_LPMV2_DPSIEGR2Set((uint8_t)(0xFFU & (uint8_t)(p_lpmv2_s7g2_cfg->deep_standby_cancel_edge >> 16U)));

            R_SYSTEM->DSPBYCR_b.IOKEEP = DPSBYCR_IOKEEP_MASK & p_lpmv2_s7g2_cfg->io_port_state;
        }
        else if(LPMV2_LOW_POWER_MODE_STANDBY_SNOOZE == p_cfg->low_power_mode)
        {
            HW_LPMV2_SnoozeConfigure(p_lpmv2_s7g2_cfg->dtc_state_in_snooze,
                                     p_lpmv2_s7g2_cfg->snooze_request_source,
                                     p_lpmv2_s7g2_cfg->snooze_end_sources,
                                     p_lpmv2_s7g2_cfg->snooze_cancel_sources);
            R_SYSTEM->SNZCR |= SNZCR_SNZE_MASK;
        }

        R_SYSTEM->SBYCR_b.OPE = SBYCR_OPE_MASK & p_lpmv2_s7g2_cfg->output_port_enable;

        HW_LPMV2_WUPENSet(p_lpmv2_s7g2_cfg->standby_wake_sources);

        R_SYSTEM->SBYCR |= SBYCR_SSBY_MASK;
    }
}

/*******************************************************************************************************************//**
 * @brief Perform pre-WFI execution tasks, enter low power mode, Perform post-WFI execution tasks
 *
 * @note This function will unlock and lock registers as needed
 *
 * @return  SSP_SUCCESS                     Successfully entered and woke from low power mode.
 * @retval  SSP_ERR_INVALID_HW_CONDITION    Operating power control stable flags were unstable during attempt 
 *                                          to revert the operating power control mode.
 * @retval  SSP_ERR_INVALID_MODE            HOCO was not system clock when using snooze mode with SCI0/RXD0.
 * @retval  SSP_ERR_INVALID_MODE            HOCO was not stable when using snooze mode with SCI0/RXD0.
 * @retval  SSP_ERR_INVALID_MODE            MOCO was running when using snooze mode with SCI0/RXD0.
 * @retval  SSP_ERR_INVALID_MODE            MAIN OSCILLATOR was running when using snooze mode with SCI0/RXD0.
 * @retval  SSP_ERR_INVALID_MODE            PLL was running when using snooze mode with SCI0/RXD0.
 * @retval  SSP_ERR_INVALID_MODE            Unable to disable ocillator stop detect when using standby or deep standby.
 **********************************************************************************************************************/
ssp_err_t HW_LPMV2_LowPowerModeEnter (void)
{
    uint32_t ostdcr_ostde_prewfi = 0U;
    uint32_t opccr_prewfi = 0U;
    uint32_t sopccr_prewfi = 0U;
    ssp_err_t ssp_err = SSP_SUCCESS;
    uint8_t hocowtr_hsts = 0U;

    if(1U == R_SYSTEM->SBYCR_b.SSBY)
    {
        /* Save oscillator stop detect state. */
        ostdcr_ostde_prewfi = R_SYSTEM->OSTDCR_b.OSTDE;

        /* S7G2 and S5D9 Only:
         * When transitioning from Software Standby mode to Normal or Snooze mode,
         * the settings in the OPCCR.OPCM[1:0] and SOPCCR.SOPCM bits are as follows,
         * regardless of their settings before entering Software Standby mode:
         *      OPCCR.OPCM[1:0] = 00b (High-speed mode)
         *      SOPCCR.SOPCM = 0b (not Subosc-speed mode).
         */
        opccr_prewfi = R_SYSTEM->OPCCR_b.OPCM;
        sopccr_prewfi = R_SYSTEM->SOPCCR_b.SOPCM;

        /* HOCO as system clock checked in check_snooze_trigger_and_clocks */
        /* Save HOCOWTCR_b.HSTS */
        hocowtr_hsts = R_SYSTEM->HOCOWTCR_b.HSTS;

        /* Execute pre-wfi standby tasks */
        ssp_err = pre_wfi_standby();
        LPMV2_ERROR_RETURN(SSP_SUCCESS == ssp_err, ssp_err);

        /* Execute pre-wfi deep standby tasks */
        pre_wfi_deep_standby();
    }

    /**
     * DSB should be last instruction executed before WFI
     * infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHICBGB.html
     */
     /*LDRA_INSPECTED 496 S Function call with no prior declaration. : __DSB MISRA-C:2012 R.17.3  */
    __DSB();

    __WFI();

    if(1U == R_SYSTEM->SBYCR_b.SSBY)
    {
        /* S7G2 and S5D9 Only:
         * When transitioning from Software Standby mode to Normal or Snooze mode,
         * the settings in the OPCCR.OPCM[1:0] and SOPCCR.SOPCM bits are as follows,
         * regardless of their settings before entering Software Standby mode:
         *      OPCCR.OPCM[1:0] = 00b (High-speed mode)
         *      SOPCCR.SOPCM = 0b (not Subosc-speed mode).
         */
        bool stable = revert_operating_mode(opccr_prewfi, sopccr_prewfi);

        ssp_err = set_oscillator_stop_detect(ostdcr_ostde_prewfi);

        /* Restore HOCOWTCR_b.HSTS if system clock is HOCO and SCI0 is the snooze trigger */
        if((1U == R_SYSTEM->SNZCR_b.SNZE) && (0U == R_SYSTEM->SNZREQCR))
        {
            set_hocowtr_hsts(hocowtr_hsts);
        }

        /*SSP_LDRA_EXECUTION_INSPECTED */
        /* Cannot test, based on operating power control mode hardware flags */
        LPMV2_ERROR_RETURN(true == stable, SSP_ERR_INVALID_HW_CONDITION);

        /*SSP_LDRA_EXECUTION_INSPECTED */
        /* Cannot test, based on operating power control mode hardware flags */
        LPMV2_ERROR_RETURN(SSP_SUCCESS == ssp_err, ssp_err);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Configure and enable snooze mode
 * This function enables the triggers that cause transition from standby mode to snooze mode.
 *
 * @param[in]  dtc_state_in_snooze  Enable/disable DTC operation
 * @param[in]  request_source  condition that can trigger entry in to snooze mode
 * @param[in]  end_sources  sources that can cause an exit from snooze to standby mode
 * @param[in]  cancel_sources  source that can cause an exit from snooze to normal mode
 *
 * @note Snooze should be disabled before calling this function, and enabled after
 *
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
void HW_LPMV2_SnoozeConfigure (lpmv2_snooze_dtc_t dtc_state_in_snooze,
                               lpmv2_snooze_request_t request_source,
                               lpmv2_snooze_end_bits_t end_sources,
                               lpmv2_snooze_cancel_t cancel_sources)
{
    /* Configure RXD0 falling edge detect */
    if(LPMV2_SNOOZE_REQUEST_RXD0_FALLING == request_source)
    {
        R_SYSTEM->SNZCR |= SNZCR_RXDREQEN_MASK;
        /* Clear all other the request conditions that can trigger entry in to snooze mode */
        HW_LPMV2_SNZREQCRSet((uint32_t)0U);
    }
    else
    {
        R_SYSTEM->SNZCR &= (uint8_t)~SNZCR_RXDREQEN_MASK;
        /* Set the request condition that can trigger entry in to snooze mode */
        HW_LPMV2_SNZREQCRSet((uint32_t)request_source);
    }
    R_SYSTEM->SNZCR_b.SNZDTCEN = SNZCR_SNZDTCEN_MASK & (uint32_t)dtc_state_in_snooze; /* Enable/disable DTC operation */
    HW_LPMV2_SELSR0Set(cancel_sources);  /* Set the source that can cause an exit from snooze to normal mode */
    HW_LPMV2_SNZEDCRSet((uint8_t)end_sources);  /* Set all sources that can cause an exit from snooze to standby mode */
}

/*******************************************************************************************************************//**
 * @brief  Wait for opccr and sopccr transition flags to clear. Internal function, do not use directly.
 *
 * @retval true     transition flags cleared
 * @retval false    transition flags not cleared
 *
 **********************************************************************************************************************/
bool HW_LPMV2_WaitForOperatingModeFlags(void)
{
    bool stable = false;
    int32_t timeout = 0xFFFF;
    /*SSP_LDRA_EXECUTION_INSPECTED */
    /* Cannot test, based on operating power control mode hardware flags */
    while(((0U != R_SYSTEM->OPCCR_b.OPCMTSF) || (0U != R_SYSTEM->SOPCCR_b.SOPCMTSF)) && (0 < timeout))
    {
        /* Wait for transition flags to clear */
        timeout--;
    }
    if(0 < timeout)
    {
        stable = true;
    }

    return stable;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Revert operating power control mode after wake from standby mode. Internal function, do not use directly.
 * @param[in]  opccr_prewfi   settings in the OPCCR.OPCM[1:0] bits
 * @param[in]  sopccr_prewfi  settings in the SOPCCR.SOPCM bits
 * @retval  bool  True if the operation was successful and transition flags are cleared, False if the transition
 *                flags are not cleared
 **********************************************************************************************************************/
static bool revert_operating_mode(uint32_t opccr_prewfi, uint32_t sopccr_prewfi)
{
    bool stable = HW_LPMV2_WaitForOperatingModeFlags();
    if(true == stable)
    {
        HW_LPMV2_RegisterUnLock();
        R_SYSTEM->OPCCR_b.OPCM = 0x3U & opccr_prewfi;
        R_SYSTEM->SOPCCR_b.SOPCM = 0x1U & sopccr_prewfi;
        HW_LPMV2_RegisterLock();
    }
    return stable;
}

/*******************************************************************************************************************//**
 * @brief  If snooze is enabled, and the snooze trigger is RXD0/DRX0, check a bunch of caveats.
 * @param[in]  clock_source  system clock
 * @retval  SSP_SUCCESS  if system clock is HOCO and is stable
 * @retval  SSP_ERR_INVALID_MODE  if the clock source is other than HOCO or if HOCO is not stable
 *
 **********************************************************************************************************************/
static ssp_err_t check_snooze_trigger_and_clocks(cgc_clock_t clock_source)
{
    LPMV2_ERROR_RETURN(CGC_CLOCK_HOCO == clock_source, SSP_ERR_INVALID_MODE);
    LPMV2_ERROR_RETURN(SSP_ERR_STABILIZED == g_cgc_on_cgc.clockCheck(CGC_CLOCK_HOCO), SSP_ERR_INVALID_MODE);
    LPMV2_ERROR_RETURN(SSP_ERR_CLOCK_INACTIVE == g_cgc_on_cgc.clockCheck(CGC_CLOCK_MOCO), SSP_ERR_INVALID_MODE);
    LPMV2_ERROR_RETURN(SSP_ERR_CLOCK_INACTIVE == g_cgc_on_cgc.clockCheck(CGC_CLOCK_MAIN_OSC), SSP_ERR_INVALID_MODE);
    LPMV2_ERROR_RETURN(SSP_ERR_CLOCK_INACTIVE == g_cgc_on_cgc.clockCheck(CGC_CLOCK_PLL), SSP_ERR_INVALID_MODE);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Set Oscillator stop detect
 * @param[in]  ostdcr_ostde  oscillator stop detect state
 * @retval  SSP_SUCCESS  if the OSTDE bit configured properly in OSTDCR register
 * @retval  SSP_ERR_INVALID_MODE  if OSTDE is not configured properly in OSTDCR register
 *
 **********************************************************************************************************************/
static ssp_err_t set_oscillator_stop_detect(uint32_t ostdcr_ostde)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_CGC);
    R_SYSTEM->OSTDCR_b.OSTDE = 0x1U & ostdcr_ostde;
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_CGC);
    LPMV2_ERROR_RETURN(ostdcr_ostde == R_SYSTEM->OSTDCR_b.OSTDE, SSP_ERR_INVALID_MODE);
    return SSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * @brief   Set HOCO wait register
 * @param[in]  hocowtr_hsts  state of HOCOWTCR_b.HSTS bit
 *
 **********************************************************************************************************************/
static void set_hocowtr_hsts(uint32_t hocowtr_hsts)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_CGC);
    /* Set HOCOWTCR_b.HSTS */
    R_SYSTEM->HOCOWTCR_b.HSTS = 0x7U & hocowtr_hsts;
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_CGC);
}

/* Execute pre-WFI deep standby tasks */
static void pre_wfi_deep_standby(void)
{
    if(1U == R_SYSTEM->DSPBYCR_b.DPSBY)
    {
        /* Clear the DOCDF flag to 0 before entering Deep Software Standby mode. */
        HW_LPMV2_DOCDFClear();

        /*
         * The flags in the DPSIFRn register set to 1 when the associated cancel request specified in DPSIEGRn is
         * generated. Each flag can be set to 1 when a cancel request is generated in any mode, not only in
         * Deep Software Standby mode, or when the setting in DPSIERn is changed.
         * Clear DPSIFRn to 00h before entering Deep Software Standby mode.
         */
        HW_LPMV2_RegisterUnLock();
        /* Dummy read is required of each register before clearing flags
         * to meet timing requirements. */
        HW_LPMV2_DPSIFR0Get();
        HW_LPMV2_DPSIFR0Set(0U);

        HW_LPMV2_DPSIFR1Get();
        HW_LPMV2_DPSIFR1Set(0U);

        HW_LPMV2_DPSIFR2Get();
        HW_LPMV2_DPSIFR2Set(0U);

        HW_LPMV2_DPSIFR3Get();
        HW_LPMV2_DPSIFR3Set(0U);

        HW_LPMV2_RegisterLock();
    }
}

/* Execute pre-WFI standby tasks */
static ssp_err_t pre_wfi_standby(void)
{
    ssp_err_t ssp_err = SSP_SUCCESS;
    cgc_clock_t clock_source = (cgc_clock_t)(0U - 1U);
    cgc_system_clock_cfg_t clock_cfg =
    {
        .pclka_div = CGC_SYS_CLOCK_DIV_1,
        .pclkb_div = CGC_SYS_CLOCK_DIV_1,
        .pclkc_div = CGC_SYS_CLOCK_DIV_1,
        .pclkd_div = CGC_SYS_CLOCK_DIV_1,
        .bclk_div = CGC_SYS_CLOCK_DIV_1,
        .fclk_div = CGC_SYS_CLOCK_DIV_1,
        .iclk_div = CGC_SYS_CLOCK_DIV_1,
    };

    /* If oscillator stop is enabled, disable. */
    ssp_err = set_oscillator_stop_detect(0U);
    LPMV2_ERROR_RETURN(SSP_SUCCESS == ssp_err, ssp_err);

    /* Get system clock */
    g_cgc_on_cgc.systemClockGet(&clock_source, &clock_cfg);

    if(0U == R_SYSTEM->DSPBYCR_b.DPSBY)
    {
        if(CGC_CLOCK_HOCO == clock_source)
        {
            HW_LPMV2_RegisterUnLock();
            HW_LPMV2_STCONR_SSTBYSet(SW_STANDBY_STCONR);
            HW_LPMV2_RegisterLock();
        }
        else
        {
            HW_LPMV2_RegisterUnLock();
            HW_LPMV2_STCONR_SSTBYSet(SW_STANDBY_WAKE_STCONR);
            HW_LPMV2_RegisterLock();
        }

        /* Set HOCOWTCR_b.HSTS if system clock is HOCO and SCI0 is the snooze trigger */
        if((1U == R_SYSTEM->SNZCR_b.SNZE) && (0U == R_SYSTEM->SNZREQCR))
        {
            /* If snooze is enabled, and the snooze trigger is RXD0/DRX0, check a bunch of caveats */
            ssp_err = check_snooze_trigger_and_clocks(clock_source);
            LPMV2_ERROR_RETURN(SSP_SUCCESS == ssp_err, ssp_err);
            set_hocowtr_hsts(SW_STANDBY_HOCOWTR_HSTS);
        }
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup LPMV2_S7G2)
 **********************************************************************************************************************/
#endif
