// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020.09.02
// 9:18:17
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"


/*-----------------------------------------------------------------------------------------------------


  \param v

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t SD_card_get_transfer_speed(uint8_t v)
{
  uint32_t  tr = v & 0x7;
  uint32_t  tv = (v >> 3) & 0xF;
  uint32_t  s;

  switch (tr)
  {
  case 0:
    s = 10;
    break;
  case 1:
    s = 100;
    break;
  case 2:
    s = 1000;
    break;
  case 3:
    s = 10000;
    break;
  default:
    s = 100000;
    break;
  }

  switch (tv)
  {
  case 0  :
    return 0  * s;
  case 1  :
    return 10 * s;
  case 2  :
    return 12 * s;
  case 3  :
    return 13 * s;
  case 4  :
    return 15 * s;
  case 5  :
    return 20 * s;
  case 6  :
    return 25 * s;
  case 7  :
    return 30 * s;
  case 8  :
    return 35 * s;
  case 9  :
    return 40 * s;
  case 10 :
    return 45 * s;
  case 11 :
    return 50 * s;
  case 12 :
    return 55 * s;
  case 13 :
    return 60 * s;
  case 14 :
    return 70 * s;
  case 15 :
    return 80 * s;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param mult
  \param c_size
  \param bl_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t SD_csd1_card_get_size(uint32_t mult, uint32_t c_size, uint32_t bl_len)
{
  uint32_t m = 0;
  uint32_t bl = 0;

  switch (mult)
  {
  case 0 :
    m = 4;
    break;
  case 1 :
    m = 8;
    break;
  case 2 :
    m = 16;
    break;
  case 3 :
    m = 32;
    break;
  case 4 :
    m = 64;
    break;
  case 5 :
    m = 128;
    break;
  case 6 :
    m = 256;
    break;
  case 7 :
    m = 512;
    break;
  }

  switch (bl_len)
  {
  case 9  :
    bl = 512;
    break;
  case 10 :
    bl = 1024;
    break;
  case 11 :
    bl = 2048;
    break;
  }

  return m * (c_size + 1) * bl;
}

/*-----------------------------------------------------------------------------------------------------


  \param mult
  \param c_size
  \param bl_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint64_t SD_csd2_card_get_size(uint32_t c_size)
{
  return (c_size + 1) * 512ll * 1024ll;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Print_header(void)
{
  T_sd_unlock_status *p_st;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  SD card control ===\n\r");
  MPRINTF("ESC - exit,  [Q]- SD card CSD info, [X] - Clear password, [S] - Set password, [D] - Erase card, [F] - format card\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");

  p_st = s7_Get_sd_status();

  if (p_st->not_identified == 1)
  {
    MPRINTF("SD card not identified\r\n");
  }
  else
  {
    if (p_st->pass_exist)
    {
      MPRINTF("SD card password is set. ");
    }
    if (p_st->lock_detected == 0)
    {
      MPRINTF("SD card isn't locked\r\n");
    }
    else
    {
      MPRINTF("SD card is locked. ");
      if (p_st->unlock_executed != 0)
      {
        MPRINTF("On start SD was unlocked successfuly\r\n");
      }
      else
      {
        MPRINTF("On start SD card unlock was failed\r\n");
      }
    }
  }
  if (g_file_system_ready == 1)
  {
    MPRINTF("File system on SD card ready\r\n");
  }
  else
  {
    MPRINTF("File system on SD card fail!\r\n");
  }
  MPRINTF("----------------------------------------------------------------------\n\r\n\r");
}

/*-----------------------------------------------------------------------------------------------------


  \param p_csd_reg
  \param str_buf
  \param maxsz
-----------------------------------------------------------------------------------------------------*/
#define INS_STR_TO_BUF(fmt, ...) {sprintf(str, fmt, ##__VA_ARGS__); slen = strlen(str); if ((slen+sz)>= maxsz) goto exit_; strcpy(&str_buf[sz], str); sz += slen;}
uint32_t Get_card_csd_text(sdmmc_priv_csd_reg_t *p_csd_reg, char *str_buf, uint32_t maxsz)
{
  char      str[64];
  uint32_t  slen;
  uint32_t  sz = 0;


  if (p_csd_reg->csd_v1_b.csd_structure == 0)
  {
    INS_STR_TO_BUF("CSD Version 1.0\r\n");
    INS_STR_TO_BUF("file_format        = %d  \r\n",p_csd_reg->csd_v1_b.file_format);
    INS_STR_TO_BUF("tmp_write_protect  = %d  \r\n",p_csd_reg->csd_v1_b.tmp_write_protect);
    INS_STR_TO_BUF("perm_write_protect = %d  \r\n",p_csd_reg->csd_v1_b.perm_write_protect);
    INS_STR_TO_BUF("copy               = %d  \r\n",p_csd_reg->csd_v1_b.copy);
    INS_STR_TO_BUF("file_format_grp    = %d  \r\n",p_csd_reg->csd_v1_b.file_format_grp);
    INS_STR_TO_BUF("write_bl_partial   = %d  \r\n",p_csd_reg->csd_v1_b.write_bl_partial);
    INS_STR_TO_BUF("write_bl_len       = %d  \r\n",p_csd_reg->csd_v1_b.write_bl_len);
    INS_STR_TO_BUF("r2w_factor         = %d  \r\n",p_csd_reg->csd_v1_b.r2w_factor);
    INS_STR_TO_BUF("wp_grp_enable      = %d  \r\n",p_csd_reg->csd_v1_b.wp_grp_enable);
    INS_STR_TO_BUF("wp_grp_size        = %d  \r\n",p_csd_reg->csd_v1_b.wp_grp_size);
    INS_STR_TO_BUF("sector_size        = %d  \r\n",p_csd_reg->csd_v1_b.sector_size);
    INS_STR_TO_BUF("erase_blk_en       = %d  \r\n",p_csd_reg->csd_v1_b.erase_blk_en);
    INS_STR_TO_BUF("c_size_mult        = %d  \r\n",p_csd_reg->csd_v1_b.c_size_mult);
    INS_STR_TO_BUF("vdd_w_curr_max     = %d  \r\n",p_csd_reg->csd_v1_b.vdd_w_curr_max);
    INS_STR_TO_BUF("vdd_w_curr_min     = %d  \r\n",p_csd_reg->csd_v1_b.vdd_w_curr_min);
    INS_STR_TO_BUF("vdd_r_curr_max     = %d  \r\n",p_csd_reg->csd_v1_b.vdd_r_curr_max);
    INS_STR_TO_BUF("vdd_r_curr_min     = %d  \r\n",p_csd_reg->csd_v1_b.vdd_r_curr_min);
    INS_STR_TO_BUF("c_size             = %d(%d bytes)\r\n",p_csd_reg->csd_v1_b.c_size, SD_csd1_card_get_size(p_csd_reg->csd_v1_b.c_size_mult, p_csd_reg->csd_v1_b.c_size, p_csd_reg->csd_v1_b.read_bl_len));
    INS_STR_TO_BUF("dsr_imp            = %d  \r\n",p_csd_reg->csd_v1_b.dsr_imp);
    INS_STR_TO_BUF("read_blk_misalign  = %d  \r\n",p_csd_reg->csd_v1_b.read_blk_misalign);
    INS_STR_TO_BUF("write_blk_misalign = %d  \r\n",p_csd_reg->csd_v1_b.write_blk_misalign);
    INS_STR_TO_BUF("read_bl_partial    = %d  \r\n",p_csd_reg->csd_v1_b.read_bl_partial);
    INS_STR_TO_BUF("read_bl_len        = %d  \r\n",p_csd_reg->csd_v1_b.read_bl_len);
    INS_STR_TO_BUF("ccc                = %04X\r\n",p_csd_reg->csd_v1_b.ccc);
    INS_STR_TO_BUF("tran_speed         = %d(%d kbit/s) \r\n",p_csd_reg->csd_v1_b.tran_speed, SD_card_get_transfer_speed(p_csd_reg->csd_v1_b.tran_speed));
    INS_STR_TO_BUF("nsac               = %d  \r\n",p_csd_reg->csd_v1_b.nsac);
    INS_STR_TO_BUF("taac               = %d  \r\n",p_csd_reg->csd_v1_b.taac);
    INS_STR_TO_BUF("csd_structure      = %d  \r\n",p_csd_reg->csd_v1_b.csd_structure);
  }
  else if (p_csd_reg->csd_v2_b.csd_structure == 1)
  {
    INS_STR_TO_BUF("CSD Version 2.0\r\n");
    INS_STR_TO_BUF("file_format        = %d  \r\n",p_csd_reg->csd_v2_b.file_format);
    INS_STR_TO_BUF("tmp_write_protect  = %d  \r\n",p_csd_reg->csd_v2_b.tmp_write_protect);
    INS_STR_TO_BUF("perm_write_protect = %d  \r\n",p_csd_reg->csd_v2_b.perm_write_protect);
    INS_STR_TO_BUF("copy               = %d  \r\n",p_csd_reg->csd_v2_b.copy);
    INS_STR_TO_BUF("file_format_grp    = %d  \r\n",p_csd_reg->csd_v2_b.file_format_grp);
    INS_STR_TO_BUF("write_bl_partial   = %d  \r\n",p_csd_reg->csd_v2_b.write_bl_partial);
    INS_STR_TO_BUF("write_bl_len       = %d  \r\n",p_csd_reg->csd_v2_b.write_bl_len);
    INS_STR_TO_BUF("r2w_factor         = %d  \r\n",p_csd_reg->csd_v2_b.r2w_factor);
    INS_STR_TO_BUF("wp_grp_enable      = %d  \r\n",p_csd_reg->csd_v2_b.wp_grp_enable);
    INS_STR_TO_BUF("wp_grp_size        = %d  \r\n",p_csd_reg->csd_v2_b.wp_grp_size);
    INS_STR_TO_BUF("sector_size        = %d  \r\n",p_csd_reg->csd_v2_b.sector_size);
    INS_STR_TO_BUF("erase_blk_en       = %d  \r\n",p_csd_reg->csd_v2_b.erase_blk_en);
    INS_STR_TO_BUF("c_size             = %d(%lld bytes)\r\n",p_csd_reg->csd_v2_b.c_size, SD_csd2_card_get_size(p_csd_reg->csd_v2_b.c_size));
    INS_STR_TO_BUF("dsr_imp            = %d  \r\n",p_csd_reg->csd_v2_b.dsr_imp);
    INS_STR_TO_BUF("read_blk_misalign  = %d  \r\n",p_csd_reg->csd_v2_b.read_blk_misalign);
    INS_STR_TO_BUF("write_blk_misalign = %d  \r\n",p_csd_reg->csd_v2_b.write_blk_misalign);
    INS_STR_TO_BUF("read_bl_partial    = %d  \r\n",p_csd_reg->csd_v2_b.read_bl_partial);
    INS_STR_TO_BUF("read_bl_len        = %d  \r\n",p_csd_reg->csd_v2_b.read_bl_len);
    INS_STR_TO_BUF("ccc                = %d  \r\n",p_csd_reg->csd_v2_b.ccc);
    INS_STR_TO_BUF("tran_speed         = %d(%d kbit/s) \r\n",p_csd_reg->csd_v2_b.tran_speed, SD_card_get_transfer_speed(p_csd_reg->csd_v2_b.tran_speed));
    INS_STR_TO_BUF("nsac               = %d  \r\n",p_csd_reg->csd_v2_b.nsac);
    INS_STR_TO_BUF("taac               = %d  \r\n",p_csd_reg->csd_v2_b.taac);
    INS_STR_TO_BUF("csd_structure      = %d  \r\n",p_csd_reg->csd_v2_b.csd_structure);
  }

exit_:
  str_buf[sz] = 0;
  sz++;
  return sz;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_csd_reg
  \param str_buf
  \param maxsz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_response_text(sdmmc_priv_card_status_t  *p_response, char *str_buf, uint32_t maxsz)
{
  char      str[64];
  uint32_t  slen;
  uint32_t  sz = 0;

  INS_STR_TO_BUF("Ake_seq_error            = %d\r\n",p_response->r1.ake_seq_error);
  INS_STR_TO_BUF("App_cmd                  = %d\r\n",p_response->r1.app_cmd);
  INS_STR_TO_BUF("Ready_for_data           = %d\r\n",p_response->r1.ready_for_data);
  INS_STR_TO_BUF("SD_state_t current_state = %d\r\n",p_response->r1.current_state);
  INS_STR_TO_BUF("Erase_reset              = %d\r\n",p_response->r1.erase_reset);
  INS_STR_TO_BUF("Card_ecc_disable         = %d\r\n",p_response->r1.card_ecc_disable);
  INS_STR_TO_BUF("Wp_erase_skip            = %d\r\n",p_response->r1.wp_erase_skip);
  INS_STR_TO_BUF("CSD_overwrite            = %d\r\n",p_response->r1.csd_overwrite);
  INS_STR_TO_BUF("Error                    = %d\r\n",p_response->r1.error);
  INS_STR_TO_BUF("CC_error                 = %d\r\n",p_response->r1.cc_error);
  INS_STR_TO_BUF("Card_ecc_failed          = %d\r\n",p_response->r1.card_ecc_failed);
  INS_STR_TO_BUF("Illegal_command          = %d\r\n",p_response->r1.illegal_command);
  INS_STR_TO_BUF("Com_crc_error            = %d\r\n",p_response->r1.com_crc_error);
  INS_STR_TO_BUF("Lock_unlock_failed       = %d\r\n",p_response->r1.lock_unlock_failed);
  INS_STR_TO_BUF("Card_is_locked           = %d\r\n",p_response->r1.card_is_locked);
  INS_STR_TO_BUF("Wp_violation             = %d\r\n",p_response->r1.wp_violation);
  INS_STR_TO_BUF("Erase_param              = %d\r\n",p_response->r1.erase_param);
  INS_STR_TO_BUF("Erase_seq_error          = %d\r\n",p_response->r1.erase_seq_error);
  INS_STR_TO_BUF("Block_len_error          = %d\r\n",p_response->r1.block_len_error);
  INS_STR_TO_BUF("Address_error            = %d\r\n",p_response->r1.address_error);
  INS_STR_TO_BUF("Out_of_range             = %d\r\n",p_response->r1.out_of_range);

exit_:
  str_buf[sz] = 0;
  sz++;
  return sz;
}


/*-----------------------------------------------------------------------------------------------------


  \param p_response
-----------------------------------------------------------------------------------------------------*/
void Print_reponse(sdmmc_priv_card_status_t  *p_response)
{
  GET_MCBL;
  uint32_t sz = 0;

  MPRINTF("\r\nCard response:\r\n");
  char *str_buf = App_malloc(2048);
  if (str_buf != NULL)
  {
    sz = Get_response_text(p_response, str_buf, 1024);
    SEND_BUF(str_buf, sz);
    App_free(str_buf);
  }
  else
  {
    MPRINTF("\r\nMem allocation fault.\r\n");
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_SD_card_control(uint8_t keycode)
{
  GET_MCBL;
  uint8_t                    b;
  uint32_t                   res;
  sdmmc_priv_card_status_t   response = {0};
  T_sd_unlock_status        *p_st;
  sdmmc_priv_csd_reg_t      *p_csd_reg;

  _Print_header();

  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case 'Q':
      case 'q':
        {
          p_st      = s7_Get_sd_status();
          p_csd_reg = s7_Get_csd_reg();

          if (p_st->not_identified == 1)
          {
            MPRINTF("No SD card!\r\n");
          }
          else
          {
            MPRINTF("SD card info:\r\n");
            char *str_buf = App_malloc(2048);
            if (str_buf != NULL)
            {
              uint32_t sz = Get_card_csd_text(p_csd_reg,str_buf,1024);
              SEND_BUF(str_buf, sz);
              App_free(str_buf);
            }
            else
            {
              MPRINTF("\r\nMem allocation fault.\r\n");
            }
          }
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_header();
        }
        break;

      case 'X':
      case 'x':
        // Очистка пароля
        {
          p_st      = s7_Get_sd_status();
          if (p_st->not_identified == 1)
          {
            MPRINTF("No SD card!\r\n");
          }
          else
          {
            res = Set_SD_card_password(SD_CLEAR_PASSWORD, (char *)ivar.sd_card_password, strlen((char *)ivar.sd_card_password),&response);
            if (res == 0)
            {
              if (response.r1.card_is_locked)
              {
                MPRINTF("\r\nPassword clearing failed.\r\n");
              }
              else
              {
                MPRINTF("\r\nPassword cleared successfully.\r\n");
              }
              Print_reponse(&response);
            }
            else
            {
              MPRINTF("\r\nError during password clearing = %d\r\n", res);
            }
          }
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_header();
        }
        break;

      case 'S':
      case 's':
        // Установка  пароля
        {
          p_st      = s7_Get_sd_status();
          if (p_st->not_identified == 1)
          {
            MPRINTF("No SD card!\r\n");
          }
          else
          {
            res  = Set_SD_card_password(SD_SET_PASSWORD,(char *)ivar.sd_card_password, strlen((char *)ivar.sd_card_password),&response);
            if (res == 0)
            {
              MPRINTF("\r\nPassword set successfully.\r\n");
              Print_reponse(&response);
            }
            else
            {
              MPRINTF("\r\nError during password setting = %d\r\n", res);
            }
          }
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_header();
        }
        break;

      case 'D':
      case 'd':
        // Стирание всей карты
        {
          p_st      = s7_Get_sd_status();
          if (p_st->not_identified == 1)
          {
            MPRINTF("No SD card!\r\n");
          }
          else
          {
            res  = Set_SD_card_password(SD_ERASE_ALL,(char *)ivar.sd_card_password, strlen((char *)ivar.sd_card_password),&response);
            if (res == 0)
            {
              if (response.r1.card_is_locked)
              {
                MPRINTF("\r\nPassword erasing failed.\r\n");
              }
              else
              {
                MPRINTF("\r\nCard erased successfully.\r\n");
              }
              Print_reponse(&response);
            }
            else
            {
              MPRINTF("\r\nError during card erasing = %d\r\n", res);
            }
          }
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_header();
        }
        break;

      case 'F':
      case 'f':
        // Стирание всей карты
        {
          p_st      = s7_Get_sd_status();
          if (p_st->not_identified == 1)
          {
            MPRINTF("No SD card!\r\n");
          }
          else
          {
            MPRINTF("Formating. Please wait...!");
            // Форматируем карту
            UINT res = FS_format();
            if (res == FX_SUCCESS)
            {
              MPRINTF(VT100_CLR_LINE"SD card formated successfully!\r\n");
            }
            else
            {
              MPRINTF(VT100_CLR_LINE"SD card formating error %s\r\n", res);
            }
          }
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_header();
        }
        break;
      case VT100_ESC:
        return;
      }
    }

  }while (1);




}



