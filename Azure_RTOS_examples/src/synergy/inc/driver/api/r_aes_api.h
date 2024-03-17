/***********************************************************************************************************************
 * Copyright [2015-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * Unless otherwise expressly agreed in writing between Renesas and you: 1) you may not use, copy, modify, distribute,
 * display, or perform the contents; 2) you may not use any name or mark of Renesas for advertising or publicity
 * purposes or in connection with your use of the contents; 3) RENESAS MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE
 * SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF CONTRACT OR TORT, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents included in this file may
 * be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : r_aes_api.h
 * Description  : AES_Interface
 ***********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *           29.01.2015 AES_Interface  1.00    Initial Release.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Crypto_API
 * @defgroup AES_API AES Interface
 *
 * @brief AES encryption and decryption APIs
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_AES_API_H
#define DRV_AES_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_crypto_api.h"
#include "r_aes_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define AES_API_VERSION_MAJOR (1U)
#define AES_API_VERSION_MINOR (0U)

#define DRV_AES_CONTEXT_BUFFER_SIZE (64)

/** Return Wrapped AES-XTS secret key size in bytes for a 128-bit AES XTS Mode Key */
#define AES_XTS_128_WRAPPPED_SECRET_KEY_SIZE_BYTES  (52)

/** Return AES-XTS secret key size in bytes for a 256-bit AES XTS Mode Key */
#define AES_XTS_256_WRAPPPED_SECRET_KEY_SIZE_BYTES  (84)

/** Return Wrapped AES secret key size in bytes for a 128-bit AES Key */
#define AES128_WRAPPPED_SECRET_KEY_SIZE_BYTES  (36)

/** Return Wrapped AES secret key size in bytes for a 192-bit AES Key */
#define AES192_WRAPPPED_SECRET_KEY_SIZE_BYTES  (52)

/** Return Wrapped AES secret key size in bytes for a 256-bit AES Key */
#define AES256_WRAPPPED_SECRET_KEY_SIZE_BYTES  (52)

/** Return AES secret key size in bytes for a 128-bit AES Key */
#define AES128_SECRET_KEY_SIZE_BYTES  (16)

/** Return AES secret key size in bytes for a 192-bit AES Key */
#define AES192_SECRET_KEY_SIZE_BYTES  (24)

/** Return AES secret key size in bytes for a 256-bit AES Key */
#define AES256_SECRET_KEY_SIZE_BYTES  (32)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** AES Interface control structure */
typedef struct st_aes_ctrl
{
    crypto_ctrl_t       * p_crypto_ctrl;  //!< pointer to crypto engine control structure
    crypto_api_t const  * p_crypto_api;   //!< pointer to crypto engine API
    uint32_t            work_buffer[DRV_AES_CONTEXT_BUFFER_SIZE];  //!< used for storing context/state of the Cipher
                                          //!< Examples: AES-GCM mode uses this for storing authentication tag etc.
} aes_ctrl_t;

/** AES Interface configuration structure. User must fill in these values before invoking the open() function */
typedef struct st_aes_cfg
{
    crypto_api_t const  * p_crypto_api;   //!< pointer to crypto engine api
} aes_cfg_t;

/** AES_Interface SCE functions implemented at the HAL layer will follow this API. */
typedef struct st_aes_api
{
    /** AES module open function. Must be called before performing any encrypt/decrypt operations.
     *
     * @param[in,out] p_ctrl pointer to control structure for the AES interface. Must be declared by user.
     *                       Elements are set here.
     * @param[in]     p_cfg  pointer to control structure for the AES configuration. All elements of this structure
     *                       must be set by user.
     *
     */
    uint32_t (* open)(aes_ctrl_t * const p_ctrl, aes_cfg_t const * const p_cfg);

    /** @brief Generate an AES key for encrypt / decrypt operations
     *
     * @param[in,out] p_ctrl    pointer to control structure for the AES interface.
     * @param[in]     num_words number of words in buffer `p_key`
     * @param[out]    p_key     pointer to key buffer. Generated key will be stored at this location.
     */
    uint32_t (* createKey)(aes_ctrl_t * const p_ctrl, uint32_t num_words, uint32_t *p_key);

    /** @brief AES encryption.
     *
     * Encrypt input data with ECB mode using a 128-bit AES key
     * @param[in]  *p_key      pointer to the AES plain-text key
     * @param[in]  *p_iv       is a pointer to initialization vector. For ECB mode this parameter is unused. NULL value is acceptable.
     * @param[in]   num_words  data buffer size in words. Each word is 4-bytes. multiples of 4
     * @param[in]  *p_source   input data buffer
     * @param[out] *p_dest     output data buffer
     */
    uint32_t (* encrypt)(aes_ctrl_t * const p_ctrl, const uint32_t * p_key, uint32_t * p_iv, uint32_t num_words,
                         uint32_t * p_source, uint32_t * p_dest);

    /** @brief Add additional authentication data (called before starting an encryption or decryption operation)
     * @param[in] *p_key     pointer to the AES plain-text key
     * @param[in] *p_iv      unused for ECB mode
     * @param[in]  num_words data buffer size in words. Each word is 4-bytes. multiples of 4
     * @param[in] *p_source  input data buffer
     */
    uint32_t (* addAdditionalAuthenticationData)(aes_ctrl_t * const p_ctrl, const uint32_t * p_key, uint32_t * p_iv,
                                                 uint32_t num_words, uint32_t * p_source);

    /** @brief AES final encryption using the chaining mode and padding mode specified
     *  in the aes.open() function call.
     */
    uint32_t (* encryptFinal)(aes_ctrl_t * const p_ctrl, const uint32_t * p_key, uint32_t * p_iv,
                              uint32_t input_num_words, uint32_t * p_source, uint32_t output_num_words,
                              uint32_t * p_dest);

    /** @brief AES Decryption.
     *
     * Decrypt input data with ECB mode using a 128-bit AES key
     * @param[in] *p_key 128-bit plain key
     * @param[in] *p_iv is a pointer to initialization vector. For ECB mode this parameter is unused. NULL value is acceptable.
     * @param[in] num_words Size in words of `p_source` and `p_dest` data buffers. Each word is 4-bytes.
     *                      Must be multiples of 4 words.
     * @param[in] *p_source input data buffer
     * @param[out] *p_dest output data buffer
     */
    uint32_t (* decrypt)(aes_ctrl_t * const p_ctrl, const uint32_t * p_key, uint32_t * p_iv, uint32_t imaxcnt,
                         uint32_t * p_source, uint32_t * p_dest);

    /** @brief set parameter specific to the mode
     *
     * @param[in] p_ctrl pointer to the control structure
     * @param[in] num_words number of words in `p_source` buffer. This must be atleast 4 words
     * @param[in] p_source pointer to authentication tag data buffer, must be of size 4 words.
     */
    uint32_t (* setGcmTag)(aes_ctrl_t * const p_ctrl, uint32_t num_words, uint32_t * p_source);

    /** @brief Get authentication tag data.
     *
     * @param[in] p_ctrl pointer to the control structure
     * @param[in] num_words number of words in `p_dest` buffer. This must be atleast 4 words
     * @param[in] p_dest pointer to data buffer, must be of size 4 words.
     */
    uint32_t (* getGcmTag)(aes_ctrl_t * const p_ctrl, uint32_t num_words, uint32_t * p_dest);

    /** Close the AES module.
     *  @param[in] p_ctrl pointer to the control structure
     */
    uint32_t (* close)(aes_ctrl_t * const p_ctrl);

    /** @brief AES zero padding encryption using the chaining mode and padding mode specified.
     *         Implementation for GCM mode only
     *         API usage -
     *         1. Provide any Add Authentication Data (AAD): set p_dest = NULL
     *         2. Encryption: set p_source to input data and p_dest will return encrypted data
     *         3. Get/Compute Tag: set p_source = NULL
     *
     * @param[in] p_ctrl pointer to the control structure
     * @param[in]  *p_key      pointer to the AES plain-text key, the buffer size should be equal to the keylength
     * @param[in]  *p_iv       the buffer size must be 16 bytes
     * @param[in]   num_bytes  data buffer size in bytes.
     * @param[in]  *p_source   input data buffer, computes tag when set to NULL.
     * @param[out] *p_dest     output data buffer, adds authentication data when set to NULL.
     *
     * @note this function is not thread safe.
     */
    uint32_t (* zeroPaddingEncrypt)(aes_ctrl_t * const p_ctrl, const uint32_t * p_key, uint32_t * p_iv, uint32_t num_bytes,
                         uint32_t * p_source, uint32_t * p_dest);

    /** @brief AES zero padding decryption using the chaining mode and padding mode specified.
     *         Implementation for GCM mode only
     *         API usage -
     *         1. Set expected tag value using the setGcmTag() function
     *         2. Provide any Add Authentication Data (AAD), invoke this API using p_dest = NULL
     *         2. Decryption: set p_source to input encrypted data, decrypted data will be returned in p_dest
     *         3. Verify the tag, invoke this API using p_source = NULL and p_dest = NULL,
     *            the return value indicates authentication tag verification status.
     *
     * @param[in] p_ctrl pointer to the control structure
     * @param[in]  *p_key      pointer to the AES plain-text key, the buffer size should be equal to the keylength
     * @param[in]  *p_iv       the buffer size must be 16 bytes
     * @param[in]   num_bytes  data buffer size in bytes.
     * @param[in]  *p_source   input data buffer, computes tag when set to NULL.
     * @param[out] *p_dest     output data buffer, adds authentication data when set to NULL.
     *
     * @note this function is not thread safe.
     */
    uint32_t (* zeroPaddingDecrypt)(aes_ctrl_t * const p_ctrl, const uint32_t * p_key, uint32_t * p_iv, uint32_t num_bytes,
                         uint32_t * p_source, uint32_t * p_dest);

    /** Gets version and stores it in provided pointer p_version.
     * @param[out]    p_version  Code and API version used.
     */
    uint32_t (* versionGet)(ssp_version_t * const p_version);

} aes_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_aes_instance
{
    aes_ctrl_t              * p_ctrl;         ///< Pointer to the control structure for this instance
    aes_cfg_t         const * p_cfg;          ///< Pointer to the configuration structure for this instance
    aes_api_t         const * p_api;          ///< Pointer to the API structure for this instance
} aes_instance_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
extern const crypto_api_t g_sce_crypto_api;

/** AES interface available boards -
 * S7G2, S5D9, S5D5, S3A7, S3A3 and S3A6 - Chaining modes CBC, GCM, CTR, ECB, XTS for 128 & 256-bit
 * S7G2, S5D9, and S5D5            - Chaining modes CBC, GCM, CTR, ECB for 192-bit
*/
extern const aes_api_t    g_aes128ecb_on_sce;
extern const aes_api_t    g_aes128cbc_on_sce;
extern const aes_api_t    g_aes128ctr_on_sce;

extern const aes_api_t    g_aes256ecb_on_sce;
extern const aes_api_t    g_aes256cbc_on_sce;
extern const aes_api_t    g_aes256ctr_on_sce;

#if (defined(BSP_MCU_GROUP_S3A3) || defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S3A7) || defined(BSP_MCU_GROUP_S3A6) || \
     defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S3A1) || defined(BSP_MCU_GROUP_S5D3))
extern const aes_api_t    g_aes128gcm_on_sce;
extern const aes_api_t    g_aes128xts_on_sce;

extern const aes_api_t    g_aes256gcm_on_sce;
extern const aes_api_t    g_aes256xts_on_sce;

extern const aes_api_t    g_aes128gcm_on_sceHrk;
extern const aes_api_t    g_aes256gcm_on_sceHrk;
#endif

#if defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3)
extern const aes_api_t    g_aes192ecb_on_sce;
extern const aes_api_t    g_aes192cbc_on_sce;
extern const aes_api_t    g_aes192ctr_on_sce;
extern const aes_api_t    g_aes192gcm_on_sce;
#endif

/** HRK Supported global structure definitions */
#if (defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S3A7) || defined(BSP_MCU_GROUP_S3A6) || \
     defined(BSP_MCU_GROUP_S3A3) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S3A1) || defined(BSP_MCU_GROUP_S5D3))
extern const aes_api_t    g_aes128ecb_on_sceHrk;
extern const aes_api_t    g_aes128cbc_on_sceHrk;
extern const aes_api_t    g_aes128ctr_on_sceHrk;
extern const aes_api_t    g_aes128xts_on_sceHrk;
extern const aes_api_t    g_aes128gcm_on_sceHrk;

extern const aes_api_t    g_aes256ecb_on_sceHrk;
extern const aes_api_t    g_aes256cbc_on_sceHrk;
extern const aes_api_t    g_aes256ctr_on_sceHrk;
extern const aes_api_t    g_aes256xts_on_sceHrk;
extern const aes_api_t    g_aes256gcm_on_sceHrk;
#endif

#if defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3)
extern const aes_api_t    g_aes192ecb_on_sceHrk;
extern const aes_api_t    g_aes192cbc_on_sceHrk;
extern const aes_api_t    g_aes192ctr_on_sceHrk;
extern const aes_api_t    g_aes192gcm_on_sceHrk;
#endif

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_AES_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup AES_API)
 **********************************************************************************************************************/
