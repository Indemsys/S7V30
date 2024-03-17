// >>>>> 2020-02-20 18:11:00
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
 * File Name    : sf_power_profiles_v2.c
 * Description  : ThreadX aware general power profile driver functions.
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include <string.h>
#include "sf_power_profiles_v2_cfg.h"
#include "sf_power_profiles_v2.h"
#include "sf_power_profiles_v2_private_api.h"
#include "r_lpmv2_api.h"
#include "r_ioport.h"
#include "r_cgc.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* "PPM2" in ASCII, used to identify general PPMV2 control block */
#define SF_POWER_PROFILES_V2_OPEN (0x50504D32U)
#define SF_POWER_PROFILES_V2_CLOSED (0U)

/* Macro for error logger. */
#ifndef SF_POWER_PROFILES_V2_ERROR_RETURN
    /*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
    #define SF_POWER_PROFILES_V2_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &s_sf_power_profiles_v2_version)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

#if SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t open_cfg_check(sf_power_profiles_v2_cfg_t const * const p_cfg);
static ssp_err_t open_ctrl_check(sf_power_profiles_v2_ctrl_t * const p_ctrl);
static ssp_err_t low_power_cfg_check(sf_power_profiles_v2_low_power_cfg_t const * const p_cfg);
static ssp_err_t low_power_ctrl_check(sf_power_profiles_v2_ctrl_t * const p_ctrl);
#endif /* SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE */
static ssp_err_t clock_config_apply(cgc_clocks_cfg_t const * const p_clock_cfg);
static ssp_err_t low_power_config_apply(sf_power_profiles_v2_low_power_cfg_t const * const p_cfg);
static ssp_err_t ioport_cfg_apply(ioport_cfg_t const * p_ioport_pin_tbl);
static void pre_low_power_callback_handle(sf_power_profiles_v2_low_power_cfg_t const * const p_cfg);
static void post_low_power_callback_handle(sf_power_profiles_v2_low_power_cfg_t const * const p_cfg);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/* Version data structure used by error logger macro. */
static const ssp_version_t s_sf_power_profiles_v2_version =
{
    .api_version_major  = SF_POWER_PROFILES_V2_API_VERSION_MAJOR,
    .api_version_minor  = SF_POWER_PROFILES_V2_API_VERSION_MINOR,
    .code_version_major = SF_POWER_PROFILES_V2_CODE_VERSION_MAJOR,
    .code_version_minor = SF_POWER_PROFILES_V2_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/* Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sf_power_profiles_v2";
#endif

extern const lpmv2_api_t g_lpmv2_on_lpmv2;

/***********************************************************************************************************************
 * Implementation of Interface
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_power_profiles_v2_api_t g_sf_power_profiles_v2_on_sf_power_profiles_v2 =
{
    .open          = SF_POWER_PROFILES_V2_Open,
    .runApply      = SF_POWER_PROFILES_V2_RunApply,
    .lowPowerApply = SF_POWER_PROFILES_V2_LowPowerApply,
    .close         = SF_POWER_PROFILES_V2_Close,
    .versionGet    = SF_POWER_PROFILES_V2_VersionGet
};

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @addtogroup SF_POWER_PROFILES_V2 Power Profiles Framework V2
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Configures the Power Profiles framework and opens any required HAL layer drivers that will be used.
 *
 * The SF_POWER_PROFILES_V2_Open function initializes the critical data structures and variables.
 *
 * @retval SSP_SUCCESS                  Initialization was successful.
 * @retval SSP_ERR_ASSERTION            One of the following parameters may be NULL: p_ctrl, p_api, or p_cfg.
 *                                      See HAL driver for other possible causes.
 * @retval SSP_ERR_IN_USE               Power profiles framework is already open.
 * @retval SSP_ERR_INTERNAL             Unable to obtain mutex.
 *                                      Unable to release mutex.
 * 
 * @return                              See @ref Common_Error_Codes for other possible return codes or causes.
 **********************************************************************************************************************/
ssp_err_t SF_POWER_PROFILES_V2_Open (sf_power_profiles_v2_ctrl_t * const p_ctrl, sf_power_profiles_v2_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

#if SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE
    err = open_ctrl_check(p_ctrl);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);
    err = open_cfg_check(p_cfg);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);
#else
    SSP_PARAMETER_NOT_USED(p_cfg);
#endif /* SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE */

#if (1 == BSP_CFG_RTOS)
    UINT tx_err;
    /* Create a mutex and initialize the HAL layer */
    tx_err = tx_mutex_create(&p_ctrl->mutex, NULL, TX_INHERIT);
    SF_POWER_PROFILES_V2_ERROR_RETURN(TX_SUCCESS == tx_err, SSP_ERR_INTERNAL);

#endif /* (1 == BSP_CFG_RTOS) */

    /** Initialize the LPM HAL driver. */
    err = g_lpmv2_on_lpmv2.init();

#if (1 == BSP_CFG_RTOS)
    /* Cannot test, unable to invoke error */
    /*If any of the HAL layer initializations failed, then delete the mutex and exit the function with the error code */
    if(SSP_SUCCESS != err)
    {
        /* Return code not being checked here since an error has already occurred and that will be returned. */
        (void)tx_mutex_delete(&p_ctrl->mutex);
        SSP_ERROR_LOG(err, &g_module_name[0], &s_sf_power_profiles_v2_version);
        return err;
    }
#else
    /* Cannot test, unable to invoke error */
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif /* (1 == BSP_CFG_RTOS) */

    /** Mark control block open so other tasks know it is valid. */
    p_ctrl->open = SF_POWER_PROFILES_V2_OPEN;

    return SSP_SUCCESS;
} /* End of function SF_POWER_PROFILES_V2_Open */

/*******************************************************************************************************************//**
 * @brief Applies a Run profile
 *
 * The SF_POWER_PROFILES_V2_RunApply function will:
 * - Apply an IO port configuration, if supplied
 * - Apply a clock configuration
 *
 * @retval SSP_SUCCESS                  Initialization was successful.
 * @retval SSP_ERR_ASSERTION            One of the following parameters may be NULL: p_ctrl, p_api, or p_cfg.
 *                                      See HAL driver for other possible causes.
 * @retval SSP_ERR_INVALID_ARGUMENT     Clock configuration is invalid.
 * @retval SSP_ERR_NOT_OPEN             Power profiles framework is not open.
 * @retval SSP_ERR_IN_USE               Unable to obtain mutex.
 * @retval SSP_ERR_INTERNAL             Unable to release mutex.
 * @retval SSP_ERR_INVALID_HW_CONDITION Incompatible system clock configuration.
 * 
 * @return                              See @ref Common_Error_Codes, r_ioport, or r_cgc driver for other possible
 *                                      return codes or causes.
 **********************************************************************************************************************/
ssp_err_t SF_POWER_PROFILES_V2_RunApply (sf_power_profiles_v2_ctrl_t * const p_ctrl, sf_power_profiles_v2_run_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

#if SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_cfg->p_clock_cfg);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SF_POWER_PROFILES_V2_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#else
#if (1 != BSP_CFG_RTOS)
    SSP_PARAMETER_NOT_USED(p_ctrl);
#endif
#endif /* SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE */

#if (1 == BSP_CFG_RTOS)
    /* Get mutex, stop timer, then release mutex */
    UINT tx_err = 0;
    tx_err = tx_mutex_get(&p_ctrl->mutex, TX_NO_WAIT);
    SF_POWER_PROFILES_V2_ERROR_RETURN(TX_SUCCESS == tx_err, SSP_ERR_IN_USE);
#endif /* (1 == BSP_CFG_RTOS) */

    /** Apply the ioport configuration. */
    /* Cannot test, unable to invoke error */
    err = ioport_cfg_apply(p_cfg->p_ioport_pin_tbl);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Set the clock config, this also sets the operating mode based on the clock speed. */
    err = clock_config_apply(p_cfg->p_clock_cfg);

#if (1 == BSP_CFG_RTOS)
    /* If any of the above steps failed, then return the mutex and exit the function with the error code */
    /* tx error is not needed to be checked since tx_mutex_get was successful */
    tx_mutex_put(&p_ctrl->mutex);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);

#else
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif /* (1 == BSP_CFG_RTOS) */

    return SSP_SUCCESS;
} /* End of function SF_POWER_PROFILES_V2_RunApply */

/******************************************************************************************************************//**
 * @brief Applies a Low Power profile.
 *
 * The SF_POWER_PROFILES_V2_LowPowerApply function will:
 * - Apply a LPMv2 configuration to prepare for low power
 * - Apply an IO port configuration to prepare for low power, if supplied
 * - Notify application that low power is about to be entered
 * - Enter low power mode
 * - When low power mode is exited, apply an IO port configuration for wake up, if supplied
 * - Notify application that wake up has occurred
 *
 * @retval SSP_SUCCESS              Entered and exited low power mode successfully.
 * @retval SSP_ERR_ASSERTION        p_ctrl or p_ctrl->p_api is NULL.
 * @retval SSP_ERR_NOT_OPEN         Power profiles framework is not open.
 * @retval SSP_ERR_UNSUPPORTED      This function is not supported by one of the HAL drivers, r_lpmv2, r_ioport.
 * @retval SSP_ERR_INVALID_MODE     r_lpmv2 mode is not LPMV2_LOW_POWER_MODE_SLEEP but r_lmv2 p_extend is NULL.
 * @retval SSP_ERR_IN_USE           Unable to obtain mutex.
 * @retval SSP_ERR_INTERNAL         Unable to release mutex.
 * 
 * @return                          See @ref Common_Error_Codes, r_ioport, or r_lpmv2 drivers for other possible
 *                                  return codes or causes.
 *********************************************************************************************************************/
ssp_err_t SF_POWER_PROFILES_V2_LowPowerApply (sf_power_profiles_v2_ctrl_t * const p_ctrl,
                                              sf_power_profiles_v2_low_power_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_ERR_ASSERTION;

#if SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE
    err = low_power_ctrl_check(p_ctrl);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);
    err = low_power_cfg_check(p_cfg);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);
#else
#if (1 != BSP_CFG_RTOS)
    SSP_PARAMETER_NOT_USED(p_ctrl);
#endif
#endif /* SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE */

    /* PREPARE FOR LOW POWER MODE MODE */

#if (1 == BSP_CFG_RTOS)
    /* Get mutex, stop timer, then release mutex */
    SF_POWER_PROFILES_V2_ERROR_RETURN(TX_SUCCESS == tx_mutex_get(&p_ctrl->mutex, TX_NO_WAIT), SSP_ERR_IN_USE);
#endif /* (1 == BSP_CFG_RTOS) */

    /** Apply the LPM configuration. */
    err = low_power_config_apply(p_cfg);
    if(SSP_SUCCESS == err)
    {
        /** Apply the pre-low power IOPORT configuration. */
        /* Cannot test, unable to invoke error */
        err = ioport_cfg_apply(p_cfg->p_ioport_pin_tbl_enter);
        if(SSP_SUCCESS == err)
        {
            /** Notify application. */
            pre_low_power_callback_handle(p_cfg);
            /** Enter low power mode. */
            /* Cannot test, based on hardware flags */
            err = g_lpmv2_on_lpmv2.lowPowerModeEnter();
            if(SSP_SUCCESS == err)
            {
                /* AWAKE */
                /** Apply the post-low power IOPORT configuration. */
                /* Cannot test, unable to invoke error */
                err = ioport_cfg_apply(p_cfg->p_ioport_pin_tbl_exit);
                if(SSP_SUCCESS == err)
                {
                    /** Notify application. */
                    post_low_power_callback_handle(p_cfg);
                }
            }
        }
    }


#if (1 == BSP_CFG_RTOS)
    /* Return the mutex and exit */
    /* tx error is not needed to be checked since tx_mutex_get was successful */
    tx_mutex_put(&p_ctrl->mutex);
    if(SSP_SUCCESS != err)
    {
        SSP_ERROR_LOG(err, &g_module_name[0], &s_sf_power_profiles_v2_version);
        return err;
    }
#else
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif
    return err;
} /* End of function SF_POWER_PROFILES_V2_LowPowerApply */

/*******************************************************************************************************************//**
 * @brief Closes the framework
 *
 * @retval SSP_SUCCESS          Successful close.
 * @retval SSP_ERR_ASSERTION    p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN     Power profiles framework is not open.
 * @retval SSP_ERR_IN_USE       Unable to obtain mutex.
 * @retval SSP_ERR_INTERNAL     Unable to release mutex.
 **********************************************************************************************************************/
ssp_err_t SF_POWER_PROFILES_V2_Close (sf_power_profiles_v2_ctrl_t * const p_ctrl)
{
#if SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SF_POWER_PROFILES_V2_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif /* SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE */

    /** Clear information from control block so other functions know this block is closed. */
    p_ctrl->open = SF_POWER_PROFILES_V2_CLOSED;

#if (1 == BSP_CFG_RTOS)
    /* Delete RTOS services used */
    (void)tx_mutex_delete(&p_ctrl->mutex);
#endif /* (1 == BSP_CFG_RTOS) */

    return SSP_SUCCESS;
} /* End of function SF_POWER_PROFILES_V2_Close */

/******************************************************************************************************************//**
 * @brief Gets version and stores it in provided pointer p_version.
 *
 * @retval SSP_SUCCESS           Version returned successfully.
 * @retval SSP_ERR_ASSERTION     Parameter p_version was null.
 *********************************************************************************************************************/
ssp_err_t SF_POWER_PROFILES_V2_VersionGet (ssp_version_t * const p_version)
{
#if SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_version);
#endif /* SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE */

    p_version->version_id = s_sf_power_profiles_v2_version.version_id;

    return SSP_SUCCESS;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

 /******************************************************************************************************************//**
 * @brief Internal function that checks and applies the clock configuration.
 *
 * @retval SSP_SUCCESS   Clock configuration applied successfully.
 * 
 * @return               See @ref Common_Error_Codes or r_cgc driver for other possible return codes or causes.
 *********************************************************************************************************************/
static ssp_err_t clock_config_apply(cgc_clocks_cfg_t const * const p_clock_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

    err = g_cgc_on_cgc.clocksCfg(p_clock_cfg);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

#if SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief      Verify config structure before use by the open function. Internal function, do not use directly.
 *
 * @param[in]   p_cfg               PPM V2 configuration structure
 * @retval      SSP_ERR_ASSERTION   One of the following parameters may be NULL: p_lower_lvl_lpm, p_api, or p_cfg.
 *
 **********************************************************************************************************************/
static ssp_err_t open_cfg_check(sf_power_profiles_v2_cfg_t const * const p_cfg)
{
    SSP_ASSERT(NULL != p_cfg);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Verify control structure before use by the open function. Internal function, do not use directly.
 *
 * @param[in]   p_ctrl              PPM V2 control structure
 * @retval      SSP_ERR_ASSERTION   p_ctrl is NULL
 * @retval      SSP_ERR_IN_USE      PPM V2 framework is already open
 *
 **********************************************************************************************************************/
static ssp_err_t open_ctrl_check(sf_power_profiles_v2_ctrl_t * const p_ctrl)
{
    SSP_ASSERT(NULL != p_ctrl);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SF_POWER_PROFILES_V2_OPEN != p_ctrl->open, SSP_ERR_IN_USE);
    return SSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief      Verify config structure before use by the low power function. Internal function, do not use directly.
 *
 * @param[in]   p_cfg                PPM V2 configuration structure
 * @retval      SSP_ERR_ASSERTION    One of the following parameters may be NULL: p_lower_lvl_lpm, p_api, or p_cfg.
 * @retval      SSP_ERR_INVALID_MODE The p_extend member can only be NULL when SLEEP mode is used
 *
 **********************************************************************************************************************/
static ssp_err_t low_power_cfg_check (sf_power_profiles_v2_low_power_cfg_t const * const p_cfg)
{
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_cfg->p_lower_lvl_lpm);
    SSP_ASSERT(NULL != p_cfg->p_lower_lvl_lpm->p_cfg);
    if(NULL == p_cfg->p_lower_lvl_lpm->p_cfg->p_extend)
    {
        SF_POWER_PROFILES_V2_ERROR_RETURN(LPMV2_LOW_POWER_MODE_SLEEP == p_cfg->p_lower_lvl_lpm->p_cfg->low_power_mode, SSP_ERR_INVALID_MODE);
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Verify control structure before use by the low power function. Internal function, do not use directly.
 *
 * @param[in]   p_ctrl              PPM V2 control structure
 * @retval      SSP_ERR_ASSERTION   p_ctrl is NULL
 * @retval      SSP_ERR_NOT_OPEN    PPM V2 framework has not been opened yet
 *
 **********************************************************************************************************************/
static ssp_err_t low_power_ctrl_check (sf_power_profiles_v2_ctrl_t * const p_ctrl)
{
    SSP_ASSERT(NULL != p_ctrl);
    SF_POWER_PROFILES_V2_ERROR_RETURN(SF_POWER_PROFILES_V2_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    return SSP_SUCCESS;
}
#endif /* SF_POWER_PROFILES_V2_CFG_PARAM_CHECKING_ENABLE */

/*******************************************************************************************************************//**
 * @brief       Apply the low power configuration using the lpm driver. Internal function, do not use directly.
 *              Returns mutex if an error occurs.
 *
 * @param[in]   p_cfg               PPM V2 config structure
 * @retval      SSP_SUCCESS         LPM configuration was successful.
 * 
 * @return                          See @ref Common_Error_Codes or r_lpmv2 driver for other possible return codes
 *                                  or causes.
 **********************************************************************************************************************/
static ssp_err_t low_power_config_apply(sf_power_profiles_v2_low_power_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

    /* Apply the LPM configuration */
    err = g_lpmv2_on_lpmv2.lowPowerCfg(p_cfg->p_lower_lvl_lpm->p_cfg);
    return err;
}

/*******************************************************************************************************************//**
 * @brief       Apply the IO Port configuration using the ioport driver. Internal function, do not use directly.
 *              Returns mutex if an error occurs.
 *
 * @param[in]   p_ioport_pin_tbl    Pointer to ioport settings
 * @retval      SSP_SUCCESS         IO Port configuration was successful.
 * 
 * @return                          See @ref Common_Error_Codes or r_ioport driver for other possible return codes
 *                                  or causes.
 **********************************************************************************************************************/
static ssp_err_t ioport_cfg_apply(ioport_cfg_t const * p_ioport_pin_tbl)
{
    ssp_err_t err = SSP_SUCCESS;

    if (p_ioport_pin_tbl != NULL)
    {
        /* Set the ioport pins per the supplied low power configuration */
        err = g_ioport_on_ioport.pinsCfg(p_ioport_pin_tbl);

    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief       Notify application using pre-low power callback. Internal function, do not use directly.
 *
 * @param[in]   p_cfg               PPM V2 config structure
 * @retval      none
 **********************************************************************************************************************/
static void pre_low_power_callback_handle (sf_power_profiles_v2_low_power_cfg_t const * const p_cfg)
{
    sf_power_profiles_v2_callback_args_t args;

    /* Issue the Pre-Low Power Callback */
    /* Populate the callback argument fields */
    args.event = SF_POWER_PROFILES_V2_EVENT_PRE_LOW_POWER;
    args.p_context = p_cfg->p_context;

    if (p_cfg->p_callback != NULL)
    {
        /* Call the user callback */
        p_cfg->p_callback(&args);
    }
}

/*******************************************************************************************************************//**
 * @brief       Notify application using post-low power callback. Internal function, do not use directly.
 *
 * @param[in]   p_cfg               PPM V2 config structure
 * @retval      none
 **********************************************************************************************************************/
static void post_low_power_callback_handle (sf_power_profiles_v2_low_power_cfg_t const * const p_cfg)
{
    sf_power_profiles_v2_callback_args_t args;

    /* Issue the Post-Low Power Callback */
    /* Populate the callback argument fields */
    args.event = SF_POWER_PROFILES_V2_EVENT_POST_LOW_POWER;
    args.p_context = p_cfg->p_context;

    if (p_cfg->p_callback != NULL)
    {
        /* Call the user callback */
        p_cfg->p_callback(&args);
    }
}

/*******************************************************************************************************************//**
 * @} (end addtogroup SF_POWER_PROFILES_V2)
 **********************************************************************************************************************/
