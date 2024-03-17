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
 * File Name    : r_iwdt.c
 * Description  : Independent Watchdog Timer (IWDT) APIs.
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_wdt_api.h"
#include "r_iwdt_private.h"
#include "hw/hw_iwdt_private.h"
#include "r_iwdt_private_api.h"
#include "bsp_api.h"
#include "bsp_cfg.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define IWDT_OPEN                (0X49574454ULL)

/** Macro for error logger. */
#ifndef IWDT_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define IWDT_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_iwdt_version)
#endif

#ifdef MOCK_BUILD
#ifndef BSP_CFG_ROM_REG_OFS0
#define BSP_CFG_ROM_REG_OFS0 BSP_CFG_OFS0_REG_VALUE
#endif
#endif

/** Clock frequency of IWDT */
#define IWDT_CLOCK_FREQUENCY 15000UL

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void     iwdt_nmi_internal_callback (bsp_grp_irq_t irq);

static uint32_t wdt_clock_divider_get (wdt_clock_division_t division);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "iwdt";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_iwdt_version =
{
    .api_version_minor  = WDT_API_VERSION_MINOR,
    .api_version_major  = WDT_API_VERSION_MAJOR,
    .code_version_major = IWDT_CODE_VERSION_MAJOR,
    .code_version_minor = IWDT_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

static const uint8_t g_ofs0_timeout[] =
{
    0x00U,                           ///< IWDTCR value for WDT_TIMEOUT_128
    0x04U,                           ///< IWDTCR value for WDT_TIMEOUT_512
    0x08U,                           ///< IWDTCR value for WDT_TIMEOUT_1024
    0x0CU,                           ///< WDTCR value for WDT_TIMEOUT_2048
    0xFFU,                           ///< WDTCR value for WDT_TIMEOUT_4096 (not supported by IWDT)
    0xFFU,                           ///< WDTCR value for WDT_TIMEOUT_8192 (not supported by IWDT)
    0xFFU,                           ///< WDTCR value for WDT_TIMEOUT_16384 (not supported by IWDT)
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
static iwdt_instance_ctrl_t * gp_iwdt_ctrl = NULL;

/** Watchdog implementation of IWDT Driver  */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const wdt_api_t   g_wdt_on_iwdt =
{
    .open        = R_IWDT_Open,
    .cfgGet      = R_IWDT_CfgGet,
    .refresh     = R_IWDT_Refresh,
    .statusGet   = R_IWDT_StatusGet,
    .statusClear = R_IWDT_StatusClear,
    .counterGet  = R_IWDT_CounterGet,
    .timeoutGet  = R_IWDT_TimeoutGet,
    .versionGet  = R_IWDT_VersionGet,
};

/*******************************************************************************************************************//**
 * @addtogroup IWDT
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Register the IWDT NMI callback.
 *
 * @retval SSP_SUCCESS              IWDT NMI callback successfully configured.
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 * @retval SSP_ERR_INVALID_MODE     An attempt to open the IWDT when the OFS0 register is not
 *                                  configured for auto-start mode.
 * @retval SSP_ERR_HW_LOCKED		IWDT module has already been called.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 *
 * @note This function is not reentrant.
 **********************************************************************************************************************/
ssp_err_t R_IWDT_Open (wdt_ctrl_t * const p_api_ctrl, wdt_cfg_t const * const p_cfg)
{
    iwdt_instance_ctrl_t * p_ctrl = (iwdt_instance_ctrl_t *) p_api_ctrl;

    /** g_iwdt_version is accessed by the ASSERT macro only and so compiler toolchain can issue
     * a warning that it is not accessed. The code below eliminates this warning and also ensures these data
     * structures are not optimized away. */
    SSP_PARAMETER_NOT_USED(g_iwdt_version);
    SSP_PARAMETER_NOT_USED(p_cfg);
    /** Eliminate toolchain warning when NMI output is not being used.  */
    SSP_PARAMETER_NOT_USED(iwdt_nmi_internal_callback);

#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);

    /** Check the IWDT is enabled. */
    /* Cast used to ensure bit operation is performed on unsigned value */
    IWDT_ERROR_RETURN((((uint32_t) BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OSF0_AUTO_START_MASK) == 0), SSP_ERR_INVALID_MODE);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_IWDT;
    fmi_feature_info_t info = {0};
    ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    IWDT_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = info.ptr;

    /** Lock the IWDT Hardware Resource */
    IWDT_ERROR_RETURN(SSP_SUCCESS == R_BSP_HardwareLock(&ssp_feature), SSP_ERR_HW_LOCKED);

    p_ctrl->iwdt_open  = 0U;
    p_ctrl->p_callback = NULL;

    /** Initialize global pointer to WDT for NMI callback use. */
    gp_iwdt_ctrl = p_ctrl;

    /** Check for NMI output mode */
    /* Cast used to ensure bit operation is performed on unsigned value */
#if (((BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OSF0_NMI_REQUEST_MASK)) == 0)
    /** NMI output mode */
#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    if (NULL == p_cfg->p_callback)
    {
       /** Unlock the IWDT Hardware Resource before returning from this API */
       R_BSP_HardwareUnlock(&ssp_feature);
       return SSP_ERR_ASSERTION;
    }
#endif
    R_BSP_GroupIrqWrite(BSP_GRP_IRQ_IWDT_ERROR, iwdt_nmi_internal_callback);

    p_ctrl->p_callback = p_cfg->p_callback;
    p_ctrl->p_context  = p_cfg->p_context;
    /** Enable the IWDT underflow/refresh error interrupt (will generate an NMI).  */
    ssp_feature.id = SSP_IP_ICU;
    g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    R_ICU_Type * p_icu_reg = (R_ICU_Type *) info.ptr;
    HW_IWDT_InterruptEnable(p_icu_reg);
#endif /* if (((BSP_CFG_ROM_REG_OFS0 & IWDT_OSF0_NMI_REQUEST_MASK)) == 0) */

    p_ctrl->iwdt_open = IWDT_OPEN;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_IWDT_Open
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Read the configuration of the IWDT. Implements wdt_api_t::cfgGet.
 *
 *
 * @retval SSP_SUCCESS              IWDT configuration successfully read.
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 * @retval SSP_ERR_INVALID_ARGUMENT	One or more configuration options is invalid.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_IWDT_CfgGet (wdt_ctrl_t * const p_api_ctrl, wdt_cfg_t * const p_cfg)
{
    iwdt_instance_ctrl_t * p_ctrl = (iwdt_instance_ctrl_t *) p_api_ctrl;

    uint8_t index;
    uint8_t timeout;

#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_cfg != NULL);
    SSP_ASSERT(p_ctrl != NULL);
#endif

    /* Cast used to ensure bit operation is performed on unsigned value */
    timeout = (uint8_t)((uint32_t) BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OFS0_TIMEOUT_MASK);

    /** Get timeout value from OFS0 register. */
    /* Cast used to ensure result of sizeof operation is an unsigned 8-bit value to satisfy static analysis of code. */
    for (index = 0u; index < (uint8_t) (sizeof(g_ofs0_timeout)); index++)
    {
        if (g_ofs0_timeout[index] == timeout)
        {
            /* Cast to convert uint8_t to type wdt_timeout_t. wdt_timeout_t will fit in an 8-bit value. */
            p_cfg->timeout = (wdt_timeout_t) index;
        }
    }

    /* Cast used to ensure bit operation is performed on unsigned value */
    p_cfg->clock_division = (wdt_clock_division_t) ((uint32_t) BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OFS0_CLOCK_DIVIDER_MASK);
    /* Cast used to ensure bit operation is performed on unsigned value */
    p_cfg->window_end     = (wdt_window_end_t) ((uint32_t) BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OFS0_WINDOW_END_MASK);
    /* Cast used to ensure bit operation is performed on unsigned value */
    p_cfg->window_start   =
        (wdt_window_start_t) (((uint32_t) BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OFS0_WINDOW_START_MASK) << 2);
    /* Cast to convert stop control value in OFS0 to compatible type for wdt_stop_control_t. */
    p_cfg->stop_control  = (wdt_stop_control_t) ((BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OFS0_STOP_CONTROL_MASK) >> 7);
    /* Cast to convert reset control value in OFS0 to compatible type for wdt_reset_control_t. */
    p_cfg->reset_control = (wdt_reset_control_t) ((BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OFS0_RESET_CONTROL_MASK) >> 5);
    p_cfg->p_callback    = p_ctrl->p_callback;

    /* Cast used to ensure bit operation is performed on unsigned value */
    if (((uint32_t) BSP_CFG_ROM_REG_OFS0 & IWDT_PRV_OSF0_AUTO_START_MASK) == 0)
    {
        p_cfg->start_mode = WDT_START_MODE_AUTO;
    }
    else
    {
        p_cfg->start_mode = WDT_START_MODE_DISABLED;
    }

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_IWDT_ConfigGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Refresh the Independent Watchdog Timer. Implements wdt_api_t::refresh.
 *
 *
 * @retval SSP_SUCCESS              IWDT successfully refreshed.
 * @retval SSP_ERR_NOT_OPEN         The driver has not been opened. Perform R_IWDT_Open() first.
 *
 * @note This function is reentrant.
 * This function only returns SSP_SUCCESS. If the refresh fails due to being performed outside of the
 * permitted refresh period the device will either reset or trigger an NMI ISR to run.
 *
 **********************************************************************************************************************/
ssp_err_t R_IWDT_Refresh (wdt_ctrl_t * const p_api_ctrl)
{
    iwdt_instance_ctrl_t * p_ctrl = (iwdt_instance_ctrl_t *) p_api_ctrl;

#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl != NULL);
    IWDT_ERROR_RETURN((IWDT_OPEN == p_ctrl->iwdt_open), SSP_ERR_NOT_OPEN);
#endif

    HW_IWDT_Refresh(p_ctrl->p_reg);

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_IWDT_Refresh
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Read the IWDT status flags
 *
 * Indicates both status and error conditions.
 *
 * @retval SSP_SUCCESS              IWDT status successfully read.
 * @retval SSP_ERR_ASSERTION        Null pointer as a parameter.
 * @retval SSP_ERR_NOT_OPEN         The driver has not been opened. Perform R_IWDT_Open() first.
 *
 * @note This function is reentrant.
 * When the IWDT is configured to output a reset on underflow or refresh error reading the status and error flags
 * can be read after reset to establish if the IWDT caused the reset.
 * Reading the status and error flags in NMI output mode indicates whether the IWDT generated the NMI interrupt.
 **********************************************************************************************************************/
ssp_err_t R_IWDT_StatusGet (wdt_ctrl_t * const p_api_ctrl, wdt_status_t * const p_status)
{
    iwdt_instance_ctrl_t * p_ctrl = (iwdt_instance_ctrl_t *) p_api_ctrl;

    uint16_t status;

#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_status != NULL);
    SSP_ASSERT(p_ctrl != NULL);
    IWDT_ERROR_RETURN((IWDT_OPEN == p_ctrl->iwdt_open), SSP_ERR_NOT_OPEN);
#endif

    status    = HW_IWDT_IWDTSRRead(p_ctrl->p_reg);
    status    = status >> 14;
    /* Cast to convert status read from status register to type wdt_status_t. */
    *p_status = (wdt_status_t) status;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_IWDT_StatusGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Clear the IWDT status and error flags. Implements wdt_api_t::statusClear.
 *
 * @retval SSP_SUCCESS              IWDT flag(s) successfully cleared.
 * @retval SSP_ERR_ASSERTION        Null pointer as a parameter.
 * @retval SSP_ERR_NOT_OPEN         The driver has not been opened. Perform R_IWDT_Open() first.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_IWDT_StatusClear (wdt_ctrl_t * const p_api_ctrl, const wdt_status_t status)
{
    iwdt_instance_ctrl_t * p_ctrl = (iwdt_instance_ctrl_t *) p_api_ctrl;

    uint16_t value;
    uint16_t read_value;

#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl != NULL);
    IWDT_ERROR_RETURN((IWDT_OPEN == p_ctrl->iwdt_open), SSP_ERR_NOT_OPEN);
#endif

    /* Cast to ensure wdt_status_t is valid for writing as a 16-bit value to clear status bits. */
    value = (uint16_t) status;
    /** Write zero to clear flags */
    value = (uint16_t) ~value;
    value = (uint16_t) (value << 14);

    /* Read back status flags until required flag(s) cleared. */
	/* Flags cannot be cleared until after the clock cycle after they are set.  */
	do
	{
		HW_IWDT_IWDTSRWrite(p_ctrl->p_reg, value);
		read_value =  HW_IWDT_IWDTSRRead(p_ctrl->p_reg);
		/* Cast to ensure data is treated as unsigned. */
		read_value &= (uint16_t)((uint16_t)status << 14);
		read_value &= (uint16_t)IWDT_PRV_IWDTSR_FLAGS_MASK;
	} while (read_value);

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_IWDT_StatusClear
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Read the current count value of the IWDT. Implements wdt_api_t::counterGet.
 *
 * @retval SSP_SUCCESS          IWDT current count successfully read.
 * @retval SSP_ERR_ASSERTION    Null pointer passed as a parameter.
 * @retval SSP_ERR_NOT_OPEN     The driver has not been opened. Perform R_IWDT_Open() first.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_IWDT_CounterGet (wdt_ctrl_t * const p_api_ctrl, uint32_t * const p_count)
{
    iwdt_instance_ctrl_t * p_ctrl = (iwdt_instance_ctrl_t *) p_api_ctrl;

#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_count != NULL);
    SSP_ASSERT(p_ctrl != NULL);
    IWDT_ERROR_RETURN((IWDT_OPEN == p_ctrl->iwdt_open), SSP_ERR_NOT_OPEN);
#endif

    /* Cast used here as although IWDT counter is 14-bit the count value returned is 32-bit for future expansion. */
    (*p_count)  = (uint32_t) HW_IWDT_IWDTSRRead(p_ctrl->p_reg);
    /* Cast used here as although IWDT counter is 14-bit the count value returned is 32-bit for future expansion. */
    (*p_count) &= (uint32_t) IWDT_PRV_IWDTSR_COUNTER_MASK;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_IWDT_CounterGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Read timeout information for the watchdog timer. Implements wdt_api_t::timeoutGet.
 *
 *
 * @retval SSP_SUCCESS              WDT successfully refreshed.
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 * @retval SSP_ERR_ABORTED          Invalid clock divider for this watchdog
 *
 * @note This function is reentrant.
 *       This function must not be called before calling R_WDT_Open().
 **********************************************************************************************************************/
ssp_err_t R_IWDT_TimeoutGet (wdt_ctrl_t * const p_api_ctrl, wdt_timeout_values_t * const p_timeout)
{
    iwdt_instance_ctrl_t * p_ctrl = (iwdt_instance_ctrl_t *) p_api_ctrl;

#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_timeout != NULL);
    SSP_ASSERT(p_ctrl != NULL);
#endif

    wdt_cfg_t cfg;
    uint32_t  frequency;
    uint32_t  divider;

    /* Initial values to eliminate MISRA-C:2012 R.9.1 warning. */
    cfg.clock_division 	= WDT_CLOCK_DIVISION_256;
    cfg.timeout			= WDT_TIMEOUT_2048;

    /* Read the configuration of the watchdog */
    R_IWDT_CfgGet(p_ctrl, &cfg);

    p_timeout->timeout_clocks = wdt_timeout[cfg.timeout];

    /* Get the frequency of the clock supplying the watchdog */
    frequency = (uint32_t)IWDT_CLOCK_FREQUENCY;

    divider   = wdt_clock_divider_get(cfg.clock_division);
    IWDT_ERROR_RETURN(divider != 0xFFFFFFFFul, SSP_ERR_ABORTED);

    frequency                     = frequency / divider;

    p_timeout->clock_frequency_hz = frequency;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_WDT_TimeoutGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief   Return IWDT HAL driver version. Implements wdt_api_t::versionGet.
 *
 * @retval          SSP_SUCCESS			Call successful.
 * @retval      SSP_ERR_ASSERTION       Null pointer passed as a parameter.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_IWDT_VersionGet (ssp_version_t * const p_data)
{
#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_data != NULL);
#endif

    p_data->version_id = g_iwdt_version.version_id;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_IWDT_VersionGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @} (end addtogroup IWDT)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Internal NMI ISR callback which calls the user provided callback passing the context provided by the user.
 *
 * @param[in]	irq		IRQ which has triggered the NMI interrupt.
 *
 **********************************************************************************************************************/
static void iwdt_nmi_internal_callback (bsp_grp_irq_t irq)
{
    SSP_PARAMETER_NOT_USED(irq);

    /** Call user registered callback */
    if (NULL != gp_iwdt_ctrl)
    {
        if (NULL != gp_iwdt_ctrl->p_callback)
        {
            wdt_callback_args_t p_args;
            p_args.p_context = gp_iwdt_ctrl->p_context;
            gp_iwdt_ctrl->p_callback(&p_args);
        }
    }
}

/******************************************************************************
 * End of function iwdt_nmi_internal_callback
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Internal function to return timeout in terms of watchdog clocks from provided timeout setting.
 *
 * @param[in]   division	Watchdog division setting.
 *
 **********************************************************************************************************************/
static uint32_t wdt_clock_divider_get (wdt_clock_division_t division)
{
    uint32_t divider;

    switch (division)
    {
        case WDT_CLOCK_DIVISION_1:
            divider = (uint32_t)1ul;
            break;

        case WDT_CLOCK_DIVISION_16:
            divider = (uint32_t)16ul;
            break;

        case WDT_CLOCK_DIVISION_32:
            divider = (uint32_t)32ul;
            break;

        case WDT_CLOCK_DIVISION_64:
            divider = (uint32_t)64ul;
            break;

        case WDT_CLOCK_DIVISION_128:
            divider = (uint32_t)128ul;
            break;

        case WDT_CLOCK_DIVISION_256:
            divider = (uint32_t)256ul;
            break;

        default:
            divider = 0xFFFFFFFFul;
            break;
    }

    return divider;
}

/*******************************************************************************************************************//**
 * @} (end defgroup IWDT)
 **********************************************************************************************************************/
