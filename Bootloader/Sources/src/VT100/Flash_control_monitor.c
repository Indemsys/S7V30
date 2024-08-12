// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020-01-31
// 11:14:21
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Format_buf_to_str(uint8_t *buf, uint32_t sz, char *str)
{
  char s[4];
  str[0] = 0;
  for (uint32_t i = 0; i < sz; i++)
  {
    sprintf(s, "%02X ", buf[i]);
    strcat(str, s);
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param pb

  \return const char*
-----------------------------------------------------------------------------------------------------*/
static const char* _Get_protection_state_description(uint8_t pb, uint8_t ID_clear_flag)
{
  if (ID_clear_flag) return "Protection disabled";

  switch (pb & 0xC0)
  {
  case 0xC0:
    return "Protection activated. All erase  enabled. Matching ID enabled.  JTAG/SWD enabled";
  case 0x80:
    return "Protection activated. All erase disabled. Matching ID enabled.  JTAG/SWD enabled";
  case 0x00:
    return "Protection activated. All erase disabled. Matching ID disabled. JTAG/SWD disabled";
  }
  return "Protection statius undefined";
}

/*-----------------------------------------------------------------------------------------------------


  \param ID_buf

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Is_all_bytes_in_buf_FF(uint8_t *ID_buf)
{
  for (uint32_t i = 0; i < 16; i++)
  {
    if (ID_buf[i] != 0xFF)
    {
      return 0;
    }
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Print_header(uint8_t  *ID_buf, uint8_t  *new_ID_buf)
{
  GET_MCBL;

  uint8_t  str[64];
  uint8_t  pb;
  uint8_t  prot_type;
  uint8_t  clear_id;
  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  Flash protection control ===\n\r");
  MPRINTF("ESC - exit, [E] - edit new ID \n\r");
  MPRINTF("[0] - Unprotect, [1] - Protect. 'All erase' enabled, [2] - Protect. 'All erase' disabled, [3] - Full protect. SWD/JTAG disabled\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");

  _Format_buf_to_str(ID_buf, 16, (char *)str);
  MPRINTF("Current ID = %s\n\r", str);

  pb = ID_buf[15];
  clear_id = _Is_all_bytes_in_buf_FF(ID_buf);

  if (clear_id)
  {
    prot_type = 0;
  }
  else
  {
    if ((pb & 0xC0) == 0xC0)
    {
      prot_type = 1;
    }
    else if ((pb & 0xC0) == 0x80)
    {
      prot_type = 2;
    }
    else if ((pb & 0xC0) == 0x00)
    {
      prot_type = 3;
    }
    else
    {
      prot_type = 3;
    }
  }

  MPRINTF("Protection type = %d - %s \n\r", prot_type, _Get_protection_state_description(pb,clear_id));
  MPRINTF("\n\r");
  _Format_buf_to_str(new_ID_buf, 16, (char *)str);
  MPRINTF("New     ID = %s\n\r", str);

  MPRINTF("----------------------------------------------------------------------\n\r\n\r");
}

/*-----------------------------------------------------------------------------------------------------


  \param protection_byte_mask
  \param ID_buf
  \param new_ID_buf
-----------------------------------------------------------------------------------------------------*/
static void _Program_OSIS(flash_id_code_mode_t flash_protection_cmd, uint8_t *ID_buf,  uint8_t *new_ID_buf)
{
  GET_MCBL;
  uint8_t    b;
  ssp_err_t  res;

  MPRINTF("\n\r\n\r\n\r");


  MPRINTF("Press Y to start\n\r");
  MPRINTF("\n\r");

  if (WAIT_CHAR(&b, 10000) == RES_OK)
  {
    if ((b == 'Y') || (b == 'y'))
    {
      new_ID_buf[15] = new_ID_buf[15] & 0x3F; // Очищаем верхние два бита. Они будут установлены в нужное состояние в самом драйвере Flash
      res = Set_Flash_protection(new_ID_buf, flash_protection_cmd);
      if (res == SSP_SUCCESS)
      {
        MPRINTF("Operation done OK!\n\r");
      }
      else
      {
        MPRINTF("Operation FAIL. Error %d!\n\r", res);
      }
      Read_OSIS_to_buf(ID_buf, 16);
      memcpy(new_ID_buf, ID_buf, 16);
      Wait_ms(2000);
      return;
    }
  }
  MPRINTF("Operation canceled\n\r");
  Wait_ms(2000);
}


/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_Flash_Protection_control(uint8_t keycode)
{
  GET_MCBL;
  uint8_t              b;
  uint8_t              ID_buf[16];
  uint8_t              new_ID_buf[16];
  char                 edit_str[61];
  uint32_t             ed[16];
  int32_t              res;

  Read_OSIS_to_buf(ID_buf, 16);
  memcpy(new_ID_buf, ID_buf, 16);
  _Print_header(ID_buf, new_ID_buf);

  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case '0':
        for (uint32_t i = 0; i < 16; i++) new_ID_buf[i] = 0xFF;
        _Program_OSIS(FLASH_ID_CODE_MODE_UNLOCKED, ID_buf, new_ID_buf);
        _Print_header(ID_buf, new_ID_buf);
        break;
      case '1':
        {
          if (_Is_all_bytes_in_buf_FF(new_ID_buf))
          {
            MPRINTF("\n\r\n\rProtection of level 0 imposible! All bytes = 0xFF\n\r");
            Wait_ms(2000);
            _Print_header(ID_buf, new_ID_buf);
            break;
          }
          _Program_OSIS(FLASH_ID_CODE_MODE_LOCKED_WITH_ALL_ERASE_SUPPORT, ID_buf, new_ID_buf);
          _Print_header(ID_buf, new_ID_buf);
        }
        break;
      case '2':
        _Program_OSIS(FLASH_ID_CODE_MODE_LOCKED, ID_buf, new_ID_buf);
        _Print_header(ID_buf, new_ID_buf);
        break;
      case '3':
        _Program_OSIS(FLASH_ID_CODE_MODE_FULL_LOCKED, ID_buf, new_ID_buf);
        _Print_header(ID_buf, new_ID_buf);
        break;


      case 'E':
      case 'e':
        {
          MPRINTF(VT100_CURSOR_ON);
          edit_str[0] = 0;
          res = VT100_edit_string(edit_str, 60, edit_str);
          MPRINTF(VT100_CURSOR_OFF);
          MPRINTF("\r");
          MPRINTF(VT100_CLR_LINE);

          if (res == RES_OK)
          {
            for (uint32_t i = 0; i < 16; i++) ed[i] = 0xFF;
            uint32_t n = sscanf(edit_str,"%X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X",&ed[0],&ed[1],&ed[2],&ed[3],&ed[4],&ed[5],&ed[6],&ed[7],&ed[8],&ed[9],&ed[10],&ed[11],&ed[12],&ed[13],&ed[14],&ed[15]);
            for (uint32_t i = 0; i < n; i++) new_ID_buf[i] = (uint8_t)ed[i];
          }
          _Print_header(ID_buf, new_ID_buf);
        }
        break;

      case VT100_ESC:
        return;
      }
    }

  }while (1);




}


