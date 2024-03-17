/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
  #define COMMON_DATA_H_
  #include <stdint.h>
  #include "bsp_api.h"
  #include "r_crypto_api.h"
  #include "r_aes_api.h"
  #include "r_rsa_api.h"
  #include "r_ecc_api.h"
  #include "r_hash_api.h"
  #include "r_trng_api.h"
  #include "r_sce.h"
  #include "r_trng_api.h"
  #include "sf_crypto.h"
  #include "sf_crypto_api.h"
  #include "nx_crypto_sce_config.h"
  #include "nx_secure_tls.h"
  #include "nx_secure_tls_api.h"
  #include "r_dtc.h"
  #include "r_transfer_api.h"
  #include "r_sci_spi.h"
  #include "r_spi_api.h"
  #include "r_cgc_api.h"
  #include "r_spi_api.h"
  #include "sf_spi.h"
  #include "sf_spi_api.h"
  #include "fx_api.h"
  #include "r_dmac.h"
  #include "r_transfer_api.h"
  #include "r_sdmmc.h"
  #include "r_sdmmc_api.h"
  #include "sf_block_media_sdmmc.h"
  #include "sf_block_media_api.h"
  #include "sf_el_fx.h"
  #include "r_riic.h"
  #include "r_i2c_api.h"
  #include "r_i2c_api.h"
  #include "sf_i2c.h"
  #include "sf_i2c_api.h"
  #include "r_cgc.h"
  #include "r_cgc_api.h"
  #include "r_lpmv2_s7g2.h"
  #include "r_lpmv2_api.h"
  #include "r_lpmv2_s7g2.h"
  #include "r_lpmv2_api.h"
  #include "r_lpmv2_s7g2.h"
  #include "r_lpmv2_api.h"
  #include "sf_power_profiles_v2.h"
  #include "r_cgc.h"
  #include "r_cgc_api.h"
  #include "r_elc.h"
  #include "r_elc_api.h"
  #include "r_ioport.h"
  #include "r_ioport_api.h"
  #include "r_fmi.h"
  #include "r_fmi_api.h"
  #include "ux_api.h"
  #include "ux_device_class_storage.h"
  #include "ux_device_class_cdc_acm.h"
  #include "ux_dcd_synergy.h"
  #include "sf_el_ux_dcd_hs_cfg.h"
  #include "sf_crypto_hash.h"
  #include "sf_crypto_hash_api.h"

  #ifdef __cplusplus
extern "C"
{
  #endif

  extern const cgc_instance_t         g_cgc;
  extern const elc_instance_t         g_elc;
  extern const fmi_instance_t         g_fmi;

  extern sf_crypto_hash_instance_t    g_sf_crypto_hash_sha256;
  extern sf_crypto_hash_instance_t    g_sf_crypto_hash_md5;

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* COMMON_DATA_H_ */
