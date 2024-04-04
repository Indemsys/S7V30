// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020.09.01
// 11:20:28
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "sdcard.h"

extern sdmmc_instance_ctrl_t      sd_card_ctrl;
extern sdmmc_cfg_t                sd_card_cfg;

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint8_t SD_fill_lock_struct_with_password(T_Lock_Card_Data_Structure *p_lcds)
{
  uint32_t sz;
  memset(p_lcds, 0, sizeof(T_Lock_Card_Data_Structure));

  sz = strlen((char const *)ivar.sd_card_password);
  if (sz > MAX_SD_PASS_LEN) sz = MAX_SD_PASS_LEN;
  p_lcds->pwds_len = sz;
  memcpy(p_lcds->pass_data, ivar.sd_card_password, sz);

  return sz + 2;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_get_card_locked_satus(uint8_t *p_lock_stat)
{

  sdmmc_priv_card_status_t response = {0U};

  if (!(s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEND_STATUS, sd_card_ctrl.status.sdhi_rca << 16)))
  {
    return SSP_ERR_CARD_INIT_FAILED;
  }
  response.status = sd_card_ctrl.p_reg->SD_RSP10;
  *p_lock_stat = response.r1.card_is_locked;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param op

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const*  Get_sd_password_op_str(uint32_t op)
{
  switch (op)
  {

  case SD_SET_PASSWORD  :
    return "SET PASSWORD";
  case SD_UNLOCK        :
    return "UNLOCK";
  case SD_CLEAR_PASSWORD:
    return "CLEAR PASSWORD";
  case SD_ERASE_ALL     :
    return "EARASE ALL";
  default:
    return "UNKNOWN";
  }
}

/*-----------------------------------------------------------------------------------------------------
  Операции с паролем SD карты

  \param operation
  \param pass_buff
  \param pass_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_password_operations(uint8_t operation, char *pass_buff, uint8_t pass_len, sdmmc_priv_card_status_t  *p_response)
{
  T_Lock_Card_Data_Structure  lcds;
  uint8_t                     len;
  uint8_t                     lock_stat;
  ssp_err_t                   res;


  memset(&lcds, 0, sizeof(lcds));

  if ((pass_len == 0) || (pass_len > MAX_SD_PASS_LEN))
  {
    // Неверный размер пароля
    return RES_ERROR;
  }

  if (SD_get_card_locked_satus(&lock_stat) != RES_OK)
  {
    // Не удалось прочитать состояние установки пароля
    return RES_ERROR;
  }
  T_sd_unlock_status *p_st = s7_Get_sd_status();

  memset(&lcds, 0, sizeof(lcds));

  if (operation == SD_SET_PASSWORD)
  {
    if (lock_stat != 0)
    {
      // Карта заблокирована. Операцию выполнить невозможно
      return RES_ERROR;
    }
  }
  else if  (operation == SD_UNLOCK)
  {
    if (lock_stat == 0)
    {
      // Карта разблокирована. Операцию выполнять не нужно
      return RES_OK;
    }
  }
  else if  (operation == SD_CLEAR_PASSWORD)
  {
    if ((lock_stat == 0) && (p_st->pass_exist==0))
    {
      // Карта разблокирована и пароль не установлен
      return RES_OK;
    }
  }


  if (operation == SD_SET_PASSWORD)
  {
    // Установка нового пароля должна сопровождаться старым и новым паролем

    lcds.flags.set_pwd     = 1; // Устанавливаем пароль
    lcds.flags.clr_pwd     = 0; // Не стираем пароль
    lcds.flags.lock_unlock = 0; // Не закрываем карту
    lcds.flags.erase       = 0; // Не стираем всю карту

    if (p_st->pass_exist)
    {
      lcds.pwds_len = strlen((char*)ivar.sd_card_password);
      memcpy(lcds.pass_data, ivar.sd_card_password, lcds.pwds_len);  // Записать старый пароль
      memcpy(&lcds.pass_data[lcds.pwds_len], pass_buff, pass_len);   // Записать новый пароль
      lcds.pwds_len  += pass_len;
    }
    else
    {
      lcds.pwds_len  = pass_len;
      memcpy(lcds.pass_data, pass_buff, pass_len);   // Записать новый пароль
    }

    len = lcds.pwds_len + 2;   // Увеличиваем длину с учетом двух байт заголовка структуры T_Lock_Card_Data_Structure
    if ((len & 1) != 0) len++; // Длина блока всегда должна быть четной. Это требуется по спецификации в режиме DDR50 на принимаемую длину пароля это не влияет
    if (s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SET_BLOCKLEN,len) != true) return RES_ERROR;

    res = S7_sdmmc_command_w_transfer(&sd_card_ctrl, SDMMC_CMD_LOCK_UNLOCK,0, (uint8_t *)&lcds, len);
    DELAY_ms(10);

    s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEND_STATUS, sd_card_ctrl.status.sdhi_rca << 16);
    p_response->status = sd_card_ctrl.p_reg->SD_RSP10;

    if ((res == SSP_SUCCESS) && (p_response->r1.lock_unlock_failed == 0))
    {
      s7_Set_pass_exist();
      return RES_OK;
    }

  }
  else if  (operation == SD_UNLOCK)
  {

    lcds.flags.set_pwd     = 0; // Не устанавливаем пароль
    lcds.flags.clr_pwd     = 0; // Не стираем пароль
    lcds.flags.lock_unlock = 0; // Не закрываем карту (Открываем, если была закрыта)
    lcds.flags.erase       = 0; // Не стираем всю карту

    len = pass_len + 2;        // Увеличиваем длину с учетом двух байт заголовка структуры T_Lock_Card_Data_Structure
    if ((len & 1) != 0) len++; // Длина блока всегда должна быть четной. Это требуется по спецификации в режиме DDR50 на принимаемую длину пароля это не влияет
    if (s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SET_BLOCKLEN,len) != true) return RES_ERROR;

    lcds.pwds_len = pass_len;
    memcpy(lcds.pass_data, pass_buff, pass_len);
    res = S7_sdmmc_command_w_transfer(&sd_card_ctrl, SDMMC_CMD_LOCK_UNLOCK,0, (uint8_t *)&lcds, len);
    DELAY_ms(10);

    s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEND_STATUS, sd_card_ctrl.status.sdhi_rca << 16);
    p_response->status = sd_card_ctrl.p_reg->SD_RSP10;

    if ((res == SSP_SUCCESS) && (p_response->r1.lock_unlock_failed == 0)) return RES_OK;

  }
  else if  (operation == SD_CLEAR_PASSWORD)
  {

    lcds.flags.set_pwd     = 0; // Не устанавливаем пароль
    lcds.flags.clr_pwd     = 1; // Стираем пароль
    lcds.flags.lock_unlock = 0; // Не закрываем карту
    lcds.flags.erase       = 0; // Не стираем всю карту

    lcds.pwds_len = pass_len;
    memcpy(lcds.pass_data, pass_buff, pass_len);

    len = pass_len + 2;        // Увеличиваем длину с учетом двух байт заголовка структуры T_Lock_Card_Data_Structure
    if ((len & 1) != 0) len++; // Длина блока всегда должна быть четной. Это требуется по спецификации в режиме DDR50 на принимаемую длину пароля это не влияет
    if (s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SET_BLOCKLEN,len) != true) return RES_ERROR;

    res = S7_sdmmc_command_w_transfer(&sd_card_ctrl, SDMMC_CMD_LOCK_UNLOCK,0, (uint8_t *)&lcds, len);
    DELAY_ms(10);

    s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEND_STATUS, sd_card_ctrl.status.sdhi_rca << 16);
    p_response->status = sd_card_ctrl.p_reg->SD_RSP10;

    if ((res == SSP_SUCCESS) && (p_response->r1.lock_unlock_failed == 0))
    {
      s7_Set_pass_clear();
      return RES_OK;
    }
  }
  else if  (operation == SD_ERASE_ALL)
  {

    lcds.flags.set_pwd     = 0; // Не устанавливаем пароль
    lcds.flags.clr_pwd     = 0; // Не стираем пароль
    lcds.flags.lock_unlock = 0; // Не закрываем карту
    lcds.flags.erase       = 1; // Стираем всю карту

    if (s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SET_BLOCKLEN,1) != true) return RES_ERROR;

    res = S7_sdmmc_command_w_transfer(&sd_card_ctrl, SDMMC_CMD_LOCK_UNLOCK,0, (uint8_t *)&lcds, 1);
    DELAY_ms(10);

    s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEND_STATUS, sd_card_ctrl.status.sdhi_rca << 16);
    p_response->status = sd_card_ctrl.p_reg->SD_RSP10;

    if ((res == SSP_SUCCESS) && (p_response->r1.lock_unlock_failed == 0)) return RES_OK;
  }

  return RES_ERROR;
}




/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_card_open(void)
{
  ssp_err_t res;
  res = S7_sdmmc_Open(&sd_card_ctrl,&sd_card_cfg);
  if (res != SSP_SUCCESS) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_card_close(void)
{
  ssp_err_t res;
  res = S7_sdmmc_Close(&sd_card_ctrl);
  if (res != SSP_SUCCESS) return RES_ERROR;
  return RES_OK;
}

