/***********************************************************************************************************************
 * Copyright [2017-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : sf_crypto_hash_private_api.h
 * Description  : SSP Crypto HASH Framework module private API definition
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_HASH_PRIVATE_API_H
#define SF_CRYPTO_HASH_PRIVATE_API_H

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE! Use functions through Interface API structure instead.
 **********************************************************************************************************************/
ssp_err_t   SF_CRYPTO_HASH_Open (sf_crypto_hash_ctrl_t * const p_api_ctrl, sf_crypto_hash_cfg_t const * const p_cfg);

ssp_err_t   SF_CRYPTO_HASH_Close (sf_crypto_hash_ctrl_t * const p_api_ctrl);

ssp_err_t   SF_CRYPTO_HASH_MessageDigestInit (sf_crypto_hash_ctrl_t * const p_api_ctrl);

ssp_err_t   SF_CRYPTO_HASH_MessageDigestUpdate (sf_crypto_hash_ctrl_t * const p_api_ctrl,
                                                sf_crypto_data_handle_t const * const p_data_in);

ssp_err_t   SF_CRYPTO_HASH_MessageDigestFinal (sf_crypto_hash_ctrl_t * const p_api_ctrl,
                                               sf_crypto_data_handle_t * const p_msg_digest, uint32_t * p_size);

ssp_err_t   SF_CRYPTO_HASH_VersionGet (ssp_version_t * const p_version);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_HASH_PRIVATE_API_H */
