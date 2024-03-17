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
 * File Name    : r_lpmv2.c
 * Description  : LPMV2 API implementation
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "bsp_api.h"

#if defined(BSP_MCU_GROUP_S7G2)

#include "r_lpmv2_s7g2.h"
#include "r_lpmv2_private_api.h"
#include "./hw/hw_lpmv2_s7g2.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#if (0 != BSP_CFG_ERROR_LOG)
    const char g_lpmv2_module_name[] = "r_lpmv2";
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

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
/** Version data structure used by error logger macro. */
/*LDRA_INSPECTED 27 D This structure cannot be static because it is externed in hw_lpmv2_s7g2.c when error logging is
 * used (BSP_CFG_ERROR_LOG == 1). */
const ssp_version_t g_lpmv2_version =
{
    .api_version_major  = LPMV2_API_VERSION_MAJOR,
    .api_version_minor  = LPMV2_API_VERSION_MINOR,
    .code_version_major = LPMV2_CODE_VERSION_MAJOR,
    .code_version_minor = LPMV2_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const lpmv2_api_t g_lpmv2_on_lpmv2 =
{
    .versionGet                         = R_LPMV2_VersionGet,
    .init                               = R_LPMV2_Init,
    .lowPowerCfg                        = R_LPMV2_LowPowerConfigure,
    .lowPowerModeEnter                  = R_LPMV2_LowPowerModeEnter,
    .clearIOKeep                        = R_LPMV2_ClearIOKeep,
};

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup LPMV2_S7G2
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      Get the driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS              Successful close.
 * @retval     SSP_ERR_INVALID_POINTER  p_version is NULL.
 *
 **********************************************************************************************************************/
ssp_err_t R_LPMV2_VersionGet (ssp_version_t * const p_version)
{
#if LPMV2_CFG_PARAM_CHECKING_ENABLE
    LPMV2_ERROR_RETURN(NULL != p_version, SSP_ERR_INVALID_POINTER);
#endif

    p_version->version_id = g_lpmv2_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Perform any necessary initialization
 *
 * @retval SSP_SUCCESS              LPMV2 Software lock initialized
 *
 **********************************************************************************************************************/
ssp_err_t R_LPMV2_Init(void)
{
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Configure a low power mode
 *
 * NOTE: This function does not enter the low power mode, it only configures parameters of the mode. Execution of
 *       the WFI instruction is what causes the low power mode to be entered.
 *
 * @retval SSP_SUCCESS              Low power mode successfully applied
 * @retval SSP_ERR_INVALID_POINTER  p_cfg is NULL
 * @retval SSP_ERR_IN_USE           Lock was not acquired
 * @retval SSP_ERR_INVALID_HW_CONDITION    Operating mode change transition was detected (OPCMTSF, SOPCMTSF bits)
 **********************************************************************************************************************/
ssp_err_t R_LPMV2_LowPowerConfigure (lpmv2_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

#if LPMV2_CFG_PARAM_CHECKING_ENABLE
    LPMV2_ERROR_RETURN(NULL != p_cfg, SSP_ERR_INVALID_POINTER);
    err = HW_LPMV2_MCUSpecificLowPowerCheck(p_cfg);
    LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /** Get hardware lock */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 1U;
    ssp_feature.id = SSP_IP_LPM;
    err = R_BSP_HardwareLock(&ssp_feature);
    LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Unlock LPMV2 registers */
    HW_LPMV2_RegisterUnLock();

    /** Check for ongoing operating mode transition (OPCMTSF, SOPCMTSF) */
    bool stable = HW_LPMV2_WaitForOperatingModeFlags();

    /** Configure MCU specific settings related to low power modes */
    if(true == stable)
    {
        HW_LPMV2_MCUSpecificConfigure(p_cfg);
    }

    /** Lock LPMV2 registers */
    HW_LPMV2_RegisterLock();

    /** Release hardware lock. */
    R_BSP_HardwareUnlock(&ssp_feature);

    /*SSP_LDRA_EXECUTION_INSPECTED */
    /* Cannot test, based on operating power control mode hardware flags */
    LPMV2_ERROR_RETURN(true == stable, SSP_ERR_INVALID_HW_CONDITION);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Enter low power mode (sleep/standby/deep standby) using WFI macro.
 *
 * Function will return after waking from low power mode.
 *
 * @retval  SSP_SUCCESS                     Successful.
 * @retval  SSP_ERR_INVALID_HW_CONDITION    HOCO was unstable during attempt to revert operating mode.
 **********************************************************************************************************************/
ssp_err_t R_LPMV2_LowPowerModeEnter (void)
{
    ssp_err_t err = SSP_SUCCESS;

    /** Get hardware lock */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 1U;
    ssp_feature.id = SSP_IP_LPM;
    err = R_BSP_HardwareLock(&ssp_feature);
    LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Check for ongoing operating mode transition (OPCMTSF, SOPCMTSF) */
    bool stable = HW_LPMV2_WaitForOperatingModeFlags();

    /** Enter low power mode */
    if(true == stable)
    {
        err = HW_LPMV2_LowPowerModeEnter();
    }

    /** Release hardware lock. */
    R_BSP_HardwareUnlock(&ssp_feature);

    LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);

    /*SSP_LDRA_EXECUTION_INSPECTED */
    /* Cannot test, based on operating power control mode hardware flags */
    LPMV2_ERROR_RETURN(true == stable, SSP_ERR_INVALID_HW_CONDITION);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Clear the IOKEEP bit after deep software stand by
 *
 * @retval  SSP_SUCCESS                     DSPBYCR_b.IOKEEP bit cleared Successfully.
 * @retval  SSP_ERR_UNSUPPORTED    Deep stand by mode not supported on this MCU.
 **********************************************************************************************************************/
ssp_err_t R_LPMV2_ClearIOKeep(void)
{
    bsp_feature_lpmv2_t lpmv2_feature = {0U};
    R_BSP_FeatureLPMV2Get(&lpmv2_feature);
    LPMV2_ERROR_RETURN(lpmv2_feature.has_dssby, SSP_ERR_UNSUPPORTED)
    HW_LPMV2_RegisterUnLock();
    HW_LPMV2_ClearIOKeep();
    HW_LPMV2_RegisterLock();
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup LPMV2_S7G2)
 **********************************************************************************************************************/

 #endif
