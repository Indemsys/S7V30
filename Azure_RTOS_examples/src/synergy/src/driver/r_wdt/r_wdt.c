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
 * File Name    : r_wdt.c
 * Description  : Watchdog Timer (WDT) HAL API implementation.
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_wdt_api.h"
#include "r_wdt_private.h"
#include "r_wdt_private_api.h"
#include "hw/hw_wdt_private.h"
#include "bsp_api.h"
#include "bsp_cfg.h"
#include "r_cgc.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define WDT_OPEN                (0X00574454ULL)

/** Macro for error logger. */
#ifndef WDT_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define WDT_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_wdt_version)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void     wdt_nmi_internal_callback (bsp_grp_irq_t irq);

static uint32_t wdt_clock_divider_get (wdt_clock_division_t division);

#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
static ssp_err_t wdt_parameter_checking (wdt_ctrl_t * const p_api_ctrl, wdt_cfg_t const * const p_cfg);
#endif

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "wdt";
#endif
#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug.  This pragma suppresses the warnings in this 
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_wdt_version =
{
    .api_version_minor  = WDT_API_VERSION_MINOR,
    .api_version_major  = WDT_API_VERSION_MAJOR,
    .code_version_major = WDT_CODE_VERSION_MAJOR,
    .code_version_minor = WDT_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

static const uint8_t wdtcr_timeout[] =
{
    0xFFU,                                   ///< WDTCR value for WDT_TIMEOUT_128 (not supported by WDT).
    0xFFU,                                   ///< WDTCR value for WDT_TIMEOUT_512 (not supported by WDT).
    0x00U,                                   ///< WDTCR value for WDT_TIMEOUT_1024.
    0xFFU,                                   ///< WDTCR value for WDT_TIMEOUT_2048 (not supported by WDT).
    0x01U,                                   ///< WDTCR value for WDT_TIMEOUT_4096.
    0x02U,                                   ///< WDTCR value for WDT_TIMEOUT_8192.
    0x03U,                                   ///< WDTCR value for WDT_TIMEOUT_16384.
};

/* Convert WDT/IWDT timeout value to an integer */
static const uint32_t wdt_timeout[] =
{
    128U,
    512U,
    1024U,
    2048U,
    4096U,
    8192U,
    16384U
};

/** Global pointer to control structure for use by the NMI callback.  */
static wdt_instance_ctrl_t * gp_wdt_ctrl = NULL;

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
/** Watchdog implementation of WDT Driver  */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const wdt_api_t g_wdt_on_wdt =
{
    .open        = R_WDT_Open,
    .cfgGet      = R_WDT_CfgGet,
    .refresh     = R_WDT_Refresh,
    .statusGet   = R_WDT_StatusGet,
    .statusClear = R_WDT_StatusClear,
    .counterGet  = R_WDT_CounterGet,
    .timeoutGet  = R_WDT_TimeoutGet,
    .versionGet  = R_WDT_VersionGet,
};

/*******************************************************************************************************************//**
 * @addtogroup WDT WDT
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Configure the WDT in register start mode. In auto-start_mode the NMI callback can be registered. Implements
 * wdt_api_t::open.
 *
 * This function should only be called once as WDT configuration registers can only be written to once so subsequent
 * calls will have no effect.
 *
 * @retval SSP_SUCCESS              WDT successfully configured.
 * @retval SSP_ERR_ASSERTION        Null Pointer(s).
 * @retval SSP_ERR_INVALID_ARGUMENT    One or more configuration options is invalid.
 * @retval SSP_ERR_INVALID_MODE     An attempt to open the WDT in register-start mode when the OFS0 register is
 *                                  configured for auto-start mode. Or to open the WDT in auto-start mode when
 *                                  the OSF0 is configured for register start mode.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 *
 * @note This function is reentrant.
 *       In auto-start mode the only valid configuration option is for registering the callback for the NMI ISR if
 *       NMI output has been selected.
 **********************************************************************************************************************/
ssp_err_t R_WDT_Open (wdt_ctrl_t * const p_api_ctrl, wdt_cfg_t const * const p_cfg)
{
    wdt_instance_ctrl_t * p_ctrl = (wdt_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err;

    /** g_wdt_version is accessed by the ASSERT macro only and so compiler toolchain can issue a
     *  warning that it is not accessed. The code below eliminates this warning and also ensures this data
     *  structure is not optimised away. */
    SSP_PARAMETER_NOT_USED(g_wdt_version);
    /** Eliminate toolchain warning when NMI output is not being used.  */
    SSP_PARAMETER_NOT_USED(wdt_nmi_internal_callback);
#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    /** Check validity of the parameters */
    err = wdt_parameter_checking (p_ctrl, p_cfg);
    WDT_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_WDT;
    fmi_feature_info_t info = {0};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    WDT_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = info.ptr;

    /** Check the expected start mode matches the OSF0 configuration. */
    WDT_ERROR_RETURN((((p_cfg->start_mode == WDT_START_MODE_REGISTER) &&                                           \
                       ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) == WDT_PRV_OSF0_AUTO_START_MASK)) || \
                      ((p_cfg->start_mode == WDT_START_MODE_AUTO) &&
                       ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) == 0))), \
                     SSP_ERR_INVALID_MODE);

    /** Lock the IWDT Hardware Resource */
    WDT_ERROR_RETURN(SSP_SUCCESS == R_BSP_HardwareLock(&ssp_feature), SSP_ERR_HW_LOCKED);

    p_ctrl->wdt_open   = 0U;
    p_ctrl->p_callback = NULL;

    /** Initialize global pointer to WDT for NMI callback use.  */
    gp_wdt_ctrl = p_ctrl;

    /** Configuration only valid when WDT operating in register-start mode. */
#if ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) == WDT_PRV_OSF0_AUTO_START_MASK)
    /** Register-start mode. */

    /** Register callback with BSP NMI ISR. */
    if (p_cfg->reset_control == WDT_RESET_CONTROL_NMI)
    {
#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
        if(p_cfg->p_callback == NULL)
        {
            /** Release the Hardware lock */
            R_BSP_HardwareUnlock(&ssp_feature);
            return SSP_ERR_ASSERTION;
        }
#endif
        R_BSP_GroupIrqWrite(BSP_GRP_IRQ_WDT_ERROR, wdt_nmi_internal_callback);
        p_ctrl->p_callback = p_cfg->p_callback;
        p_ctrl->p_context  = p_cfg->p_context;
        /** Enable the WDT underflow/refresh error interrupt (will generate an NMI).  */
        ssp_feature.id = SSP_IP_ICU;
        g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
        R_ICU_Type * p_icu_reg = (R_ICU_Type *) info.ptr;
        HW_WDT_InterruptEnable(p_icu_reg);
    }

    HW_WDT_WDTCRWrite(p_ctrl->p_reg, (uint16_t) ((((uint16_t) wdtcr_timeout[p_cfg->timeout] | (uint16_t) p_cfg->clock_division) | \
                                   ((uint16_t) p_cfg->window_start | (uint16_t) p_cfg->window_end))));
    HW_WDT_WDTRCRWrite(p_ctrl->p_reg, p_cfg->reset_control);
    HW_WDT_WDTCSTPRWrite(p_ctrl->p_reg, p_cfg->stop_control);

    p_ctrl->wdt_open = WDT_OPEN;
    if (true == p_cfg->autostart)
    {
        /** Start the timer by performing a refresh. */
        R_WDT_Refresh(p_ctrl);
    }

#else /* if ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) == WDT_PRV_OSF0_AUTO_START_MASK) */
    /** Auto-start mode. */
    /** Check for NMI output mode. */
#if (((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_NMI_REQUEST_MASK)) == 0)
    /** NMI output mode. */
#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    if(p_cfg->p_callback == NULL)
    {
        /** Release the Hardware lock */
        R_BSP_HardwareUnlock(&ssp_feature);
        return SSP_ERR_ASSERTION;
    }
#endif
    R_BSP_GroupIrqWrite(BSP_GRP_IRQ_WDT_ERROR, wdt_nmi_internal_callback);
    p_ctrl->p_callback = p_cfg->p_callback;
    p_ctrl->p_context  = p_cfg->p_context;
    /** Enable the WDT underflow/refresh error interrupt (will generate an NMI).  */
    ssp_feature.id = SSP_IP_ICU;
    g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    R_ICU_Type * p_icu_reg = (R_ICU_Type *) info.ptr;
    HW_WDT_InterruptEnable(p_icu_reg);
#endif /* if (((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_NMI_REQUEST_MASK)) == 0) */

    p_ctrl->wdt_open = WDT_OPEN;
#endif ///< End of #if ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) = WDT_PRV_OSF0_AUTO_START_MASK).

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_Open
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Read the configuration of the WDT in both register-start and auto-start modes. Implements
 * wdt_api_t::cfgGet.
 *
 *
 * @retval SSP_SUCCESS              WDT successfully configured.
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 * @retval SSP_ERR_NOT_OPEN         Instance control block is not initialized.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_WDT_CfgGet (wdt_ctrl_t * const p_api_ctrl, wdt_cfg_t * const p_cfg)
{
    wdt_instance_ctrl_t * p_ctrl = (wdt_instance_ctrl_t *) p_api_ctrl;

    #if ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) == WDT_PRV_OSF0_AUTO_START_MASK)
    uint16_t wdtcr;
#endif
    uint32_t  index;
    uint8_t  timeout;

#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_cfg != NULL);
    SSP_ASSERT(p_ctrl != NULL);
    WDT_ERROR_RETURN(WDT_OPEN == p_ctrl->wdt_open, SSP_ERR_NOT_OPEN);
#endif

#if ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) == WDT_PRV_OSF0_AUTO_START_MASK)
    /** Register-start mode. */
    p_cfg->start_mode = WDT_START_MODE_REGISTER;

    wdtcr             = HW_WDT_WDTCRRead(p_ctrl->p_reg);
    timeout           = (wdtcr & WDT_PRV_WDTCR_TIMEOUT_MASK);

    /** Get timeout value from WDTCR register. */
    for (index = 0; index < (sizeof(wdtcr_timeout)); index++)
    {
        if (wdtcr_timeout[index] == timeout)
        {
            p_cfg->timeout = (wdt_timeout_t) index;
        }
    }

    p_cfg->clock_division = (wdt_clock_division_t) (wdtcr & WDT_PRV_WDTCR_CLOCK_DIVIDER_MASK);
    p_cfg->window_end     = (wdt_window_end_t) (wdtcr & WDT_PRV_WDTCR_WINDOW_END_MASK);
    p_cfg->window_start   = (wdt_window_start_t) (wdtcr & WDT_PRV_WDTCR_WINDOW_START_MASK);
    p_cfg->reset_control  = (wdt_reset_control_t) HW_WDT_WDTRCRRead(p_ctrl->p_reg);
    p_cfg->stop_control   = (wdt_stop_control_t) HW_WDT_WDTCSTPRRead(p_ctrl->p_reg);
#else /* if ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) == WDT_PRV_OSF0_AUTO_START_MASK) */
    /** Auto-start mode. */
    p_cfg->start_mode = WDT_START_MODE_AUTO;

    timeout           = (uint8_t)((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OFS0_TIMEOUT_MASK) >> 18);
    /** Get timeout value from WDTCR register. */
    for (index = 0u; index < (sizeof(wdtcr_timeout)); index++)
    {
        if (wdtcr_timeout[index] == timeout)
        {
            p_cfg->timeout = (wdt_timeout_t) index;
        }
    }

    p_cfg->clock_division = (wdt_clock_division_t) ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OFS0_CLOCK_DIVIDER_MASK) >> 16);
    p_cfg->window_end     = (wdt_window_end_t) ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OFS0_WINDOW_END_MASK) >> 16);
    p_cfg->window_start   = (wdt_window_start_t) ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OFS0_WINDOW_START_MASK) >> 14);
    p_cfg->reset_control  = (wdt_reset_control_t) ((BSP_CFG_ROM_REG_OFS0 >> 21) & WDT_PRV_OFS0_RESET_CONTROL_MASK);
    p_cfg->stop_control   = (wdt_stop_control_t) ((BSP_CFG_ROM_REG_OFS0 >> 23) & WDT_PRV_OFS0_STOP_CONTROL_MASK);
#endif /* if ((BSP_CFG_ROM_REG_OFS0 & WDT_PRV_OSF0_AUTO_START_MASK) == WDT_PRV_OSF0_AUTO_START_MASK) */

    p_cfg->p_callback = p_ctrl->p_callback;
    p_cfg->p_context  = p_ctrl->p_context;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_ConfigGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Read timeout information for the watchdog timer. Implements wdt_api_t::timeoutGet.
 *
 *
 * @retval SSP_SUCCESS              WDT timeout value successfully read.
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 * @retval SSP_ERR_ABORTED          Invalid clock divider for this watchdog
 *
 * @note This function is reentrant.
 *       This function must not be called before calling R_WDT_Open().
 **********************************************************************************************************************/
ssp_err_t R_WDT_TimeoutGet (wdt_ctrl_t * const p_api_ctrl, wdt_timeout_values_t * const p_timeout)
{
    wdt_instance_ctrl_t * p_ctrl = (wdt_instance_ctrl_t *) p_api_ctrl;

#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_timeout != NULL);
    SSP_ASSERT(p_ctrl != NULL);
#endif

    ssp_err_t err;
    wdt_cfg_t cfg;
    uint32_t  frequency;
    uint32_t  divider;

    /* Initial values to eliminate MISRA-C:2012 R.9.1 warning. */
    cfg.clock_division 	= WDT_CLOCK_DIVISION_8192;
    cfg.timeout 		= WDT_TIMEOUT_16384;
    frequency			= 60000000u;

    /** Read the configuration of the WDT */
    err = R_WDT_CfgGet(p_ctrl, &cfg);
    /* The following line of code is unreachable in the current implementation */
    /* Error checking is kept for future use and for code maintenance purpose */
    WDT_ERROR_RETURN(err == SSP_SUCCESS, err);

    p_timeout->timeout_clocks = wdt_timeout[cfg.timeout];

    /** Get the frequency of the clock supplying the WDT */
    err = g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &frequency);
    /* The following line of code is unreachable in the current implementation */
    /* Error checking is kept for future use and for code maintenance purpose */
    WDT_ERROR_RETURN(err == SSP_SUCCESS, err);

    divider = wdt_clock_divider_get(cfg.clock_division);
    WDT_ERROR_RETURN(divider != 0xFFFFFFFFul, SSP_ERR_ABORTED);

    frequency                     = frequency / divider;

    p_timeout->clock_frequency_hz = frequency;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_TimeoutGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Refresh the watchdog timer. Implements wdt_api_t::refresh.
 *
 * In addition to refreshing the watchdog counter this function can be used, in register start mode to start the
 * counter.
 *
 * @retval SSP_SUCCESS              WDT successfully refreshed.
 * @retval SSP_ERR_NOT_OPEN         Instance control block is not initialized.
 *
 * @note This function is reentrant.
 *       This function only returns SSP_SUCCESS. If the refresh fails due to being performed outside of the
 *       permitted refresh period the device will either reset or trigger an NMI ISR to run.
 *       This function must not be called before calling R_WDT_Open().
 **********************************************************************************************************************/
ssp_err_t R_WDT_Refresh (wdt_ctrl_t * const p_api_ctrl)
{
    wdt_instance_ctrl_t * p_ctrl = (wdt_instance_ctrl_t *) p_api_ctrl;

#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl != NULL);
    WDT_ERROR_RETURN(WDT_OPEN == p_ctrl->wdt_open, SSP_ERR_NOT_OPEN);
#endif

    /** Refresh the WDT Down counter */
    HW_WDT_Refresh(p_ctrl->p_reg);

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_RefreshSet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Read the WDT status flags. Implements wdt_api_t::statusGet.
 *
 * Indicates both status and error conditions.
 *
 * @retval SSP_SUCCESS              WDT status successfully read.
 * @retval SSP_ERR_ASSERTION        Null pointer as a parameter.
 * @retval SSP_ERR_NOT_OPEN         Instance control block is not initialized.
 *
 * @note This function is reentrant.
 *       When the WDT is configured to output a reset on underflow or refresh error reading the status and error flags
 *       serves no purpose as they will always indicate that no underflow has occurred and there is no refresh error.
 *       Reading the status and error flags is only valid when interrupt request output is enabled.
 **********************************************************************************************************************/
ssp_err_t R_WDT_StatusGet (wdt_ctrl_t * const p_api_ctrl, wdt_status_t * const p_status)
{
    wdt_instance_ctrl_t * p_ctrl = (wdt_instance_ctrl_t *) p_api_ctrl;

    uint16_t status;

#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_status != NULL);
    SSP_ASSERT(p_ctrl != NULL);
    WDT_ERROR_RETURN(WDT_OPEN == p_ctrl->wdt_open, SSP_ERR_NOT_OPEN);
#endif

    /** Read the WDT status */
    status    = HW_WDT_WDTSRRead(p_ctrl->p_reg);

    /** Get the value of refresh or underflow error flag */
    status    = status >> 14;
    *p_status = (wdt_status_t) status;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_StatusRead
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Clear the WDT status and error flags. Implements wdt_api_t::statusClear.
 *
 * @retval SSP_SUCCESS              WDT flag(s) successfully cleared.
 * @retval SSP_ERR_ASSERTION        Null pointer as a parameter.
 * @retval SSP_ERR_NOT_OPEN         Instance control block is not initialized.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_WDT_StatusClear (wdt_ctrl_t * const p_api_ctrl, const wdt_status_t status)
{
    wdt_instance_ctrl_t * p_ctrl = (wdt_instance_ctrl_t *) p_api_ctrl;

    uint16_t value;
    uint16_t read_value;

#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl != NULL);
    WDT_ERROR_RETURN(WDT_OPEN == p_ctrl->wdt_open, SSP_ERR_NOT_OPEN);
#endif

    /* Casts to uint16_t to ensure value is handled as unsigned. */
    value = (uint16_t) status;
    /** Write zero to clear flags. */
    value = (uint16_t) ~value;
    value = (uint16_t) (value << 14);

    /* Read back status flags until required flag(s) cleared. */
    /* Flags cannot be cleared until after the clock cycle after they are set.  */
    do
    {
    	HW_WDT_WDTSRWrite(p_ctrl->p_reg, value);
    	read_value =  HW_WDT_WDTSRRead(p_ctrl->p_reg);
        /* Isolate flags to clear. */
        read_value &= (uint16_t)((uint16_t)status << 14);
    } while (0U != read_value);

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_StatusClear
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Read the current count value of the WDT. Implements wdt_api_t::counterGet.
 *
 * @retval SSP_SUCCESS          WDT current count successfully read.
 * @retval SSP_ERR_ASSERTION    Null pointer passed as a parameter.
 * @retval SSP_ERR_NOT_OPEN     Instance control block is not initialized.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_WDT_CounterGet (wdt_ctrl_t * const p_api_ctrl, uint32_t * const p_count)
{
    wdt_instance_ctrl_t * p_ctrl = (wdt_instance_ctrl_t *) p_api_ctrl;

#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_count != NULL);
    SSP_ASSERT(p_ctrl != NULL);
    WDT_ERROR_RETURN(WDT_OPEN == p_ctrl->wdt_open, SSP_ERR_NOT_OPEN);
#endif
    SSP_PARAMETER_NOT_USED(p_ctrl);

     /** Read the WDT status */
    *p_count  = (uint32_t) HW_WDT_WDTSRRead(p_ctrl->p_reg);

    /** Get WDT down counter value */
    *p_count &= WDT_PRV_WDTSR_COUNTER_MASK;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_TimerRead
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Return WDT HAL driver version. Implements wdt_api_t::versionGet.
 *
 * @retval          SSP_SUCCESS                Version information successfully read.
 * @retval      SSP_ERR_ASSERTION       Null pointer passed as a parameter
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_WDT_VersionGet (ssp_version_t * const p_data)
{
#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_data != NULL);
#endif

    p_data->version_id = g_wdt_version.version_id;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_VersionGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @} (end addtogroup WDT)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Internal NMI ISR callback which calls the user provided callback passing the context provided by the user.
 *
 * @param[in]    irq        IRQ which has triggered the NMI interrupt.
 *
 **********************************************************************************************************************/
static void wdt_nmi_internal_callback (bsp_grp_irq_t irq)
{
    SSP_PARAMETER_NOT_USED(irq);

    /** Call user registered callback */
    if (NULL != gp_wdt_ctrl)
    {
        if (NULL != gp_wdt_ctrl->p_callback)
        {
            wdt_callback_args_t p_args;
            p_args.p_context = gp_wdt_ctrl->p_context;
            gp_wdt_ctrl->p_callback(&p_args);
        }
    }
}
/******************************************************************************
 * End of function wdt_nmi_internal_callback
 ******************************************************************************/
/*******************************************************************************************************************//**
 * @brief Get the WDT clock divider value.
 * @param[in]   division     Clock division.
 *
 **********************************************************************************************************************/
static uint32_t wdt_clock_divider_get (wdt_clock_division_t division)
{
    uint32_t divider;

    switch (division)
    {
        case WDT_CLOCK_DIVISION_1:
            divider = 1U;
            break;

        case WDT_CLOCK_DIVISION_4:
            divider = 4U;
            break;

        case WDT_CLOCK_DIVISION_64:
            divider = 64U;
            break;

        case WDT_CLOCK_DIVISION_128:
            divider = 128U;
            break;

        case WDT_CLOCK_DIVISION_512:
            divider = 512U;
            break;

        case WDT_CLOCK_DIVISION_2048:
            divider = 2048U;
            break;

        case WDT_CLOCK_DIVISION_8192:
            divider = 8192U;
            break;

        default:
            divider = 0xFFFFFFFFU;
            break;
    }

    return divider;
}


#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * @brief Parameter checking function for WDT Open
 *
 * @param[in]    p_ctrl        Pointer to instance control structure
 * @param[in]    p_cfg         Pointer to configuration structure
 *
 **********************************************************************************************************************/
static ssp_err_t wdt_parameter_checking (wdt_ctrl_t * const p_ctrl, wdt_cfg_t const * const p_cfg)
{
	/** Check that control and config structure pointers are valid. */
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);

    /** Check timeout parameter is supported by WDT. */
    WDT_ERROR_RETURN((p_cfg->timeout == WDT_TIMEOUT_1024) || (p_cfg->timeout == WDT_TIMEOUT_4096) || \
                     (p_cfg->timeout == WDT_TIMEOUT_8192) || (p_cfg->timeout == WDT_TIMEOUT_16384),
                     SSP_ERR_INVALID_ARGUMENT)

    return SSP_SUCCESS;
}
#endif
