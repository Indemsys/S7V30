#ifndef SYNERGY_SDMMC_H
  #define SYNERGY_SDMMC_H

#include "sdcard.h"

extern const sdmmc_api_t g_s7_sdmmc;

#define  MAX_SD_PASS_LEN 16

// Стркутура команды управления паролем
typedef __packed  struct
{
    struct {
      uint8_t set_pwd          : 1;
      uint8_t clr_pwd          : 1;
      uint8_t lock_unlock      : 1;
      uint8_t erase            : 1;
      uint8_t reserved         : 4;
    }
    flags;
    uint8_t  pwds_len;
    uint8_t  pass_data[MAX_SD_PASS_LEN*2]; // Массив должен вмещать старый и новый пароли

} T_Lock_Card_Data_Structure;


typedef struct
{
  uint8_t      pass_exist;      // Если 1 то на карте установлен пароль
  uint8_t      lock_detected;   // Если 1 то была обнаружена блокировка карты
  uint8_t      unlock_executed; // Если 1 то была выполнена разблокировка.
  uint8_t      not_identified;  // Если 1 то карта не была обнаружена.

} T_sd_unlock_status;

ssp_err_t               S7_sdmmc_Open(sdmmc_ctrl_t *const p_ctrl, sdmmc_cfg_t const *const p_cfg);
ssp_err_t               S7_sdmmc_Close(sdmmc_ctrl_t *const p_ctrl);
ssp_err_t               S7_sdmmc_Read(sdmmc_ctrl_t *const p_ctrl, uint8_t *const      p_dest, uint32_t const       start_sector, uint32_t const       sector_count);
ssp_err_t               S7_sdmmc_Write(sdmmc_ctrl_t *const  p_ctrl, uint8_t const *const p_source, uint32_t const        start_sector, uint32_t const        sector_count);
ssp_err_t               S7_sdmmc_Control(sdmmc_ctrl_t *const p_ctrl, ssp_command_t const command, void *p_data);
ssp_err_t               S7_sdmmc_ReadIo(sdmmc_ctrl_t *const p_ctrl, uint8_t *const  p_data, uint32_t const  function, uint32_t const  address);
ssp_err_t               S7_sdmmc_WriteIo(sdmmc_ctrl_t *const p_ctrl, uint8_t *const  p_data, uint32_t const  function, uint32_t const  address, sdmmc_io_write_mode_t const  read_after_write);
ssp_err_t               S7_sdmmc_ReadIoExt(sdmmc_ctrl_t *const p_ctrl, uint8_t *const p_dest, uint32_t const  function, uint32_t const  address, uint32_t *const  count, sdmmc_io_transfer_mode_t  transfer_mode, sdmmc_io_address_mode_t  address_mode);
ssp_err_t               S7_sdmmc_WriteIoExt(sdmmc_ctrl_t *const p_ctrl, uint8_t const *const p_source, uint32_t const  function, uint32_t const  address, uint32_t const  count, sdmmc_io_transfer_mode_t  transfer_mode, sdmmc_io_address_mode_t  address_mode);
ssp_err_t               S7_sdmmc_IoIntEnable(sdmmc_ctrl_t *const p_ctrl, bool enable);
ssp_err_t               S7_sdmmc_VersionGet(ssp_version_t *const p_version);
ssp_err_t               S7_sdmmc_InfoGet(sdmmc_ctrl_t *const p_ctrl,  sdmmc_info_t *const p_info);
ssp_err_t               S7_sdmmc_Erase(sdmmc_ctrl_t *const p_api_ctrl, uint32_t       const start_sector, uint32_t       const sector_count);


bool                    s7_sdmmc_command_send (sdmmc_instance_ctrl_t * p_ctrl, uint16_t command, uint32_t argument);
ssp_err_t               S7_sdmmc_command_w_transfer (sdmmc_ctrl_t * const p_api_ctrl, uint16_t command , uint32_t argument, uint8_t const * const p_source, uint32_t const bl_size);
T_sd_unlock_status     *s7_Get_sd_status(void);
sdmmc_priv_csd_reg_t   *s7_Get_csd_reg(void);
void                    s7_Set_pass_exist(void);
void                    s7_Set_pass_clear(void);
#endif


