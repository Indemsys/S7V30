// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-07-02
// 15:23:56
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


#define  DEF_MON_PASSWORD "123456789"
#define  PASS_STR_MAX_LEN  32


/*-------------------------------------------------------------------------------------------------------------
  Процедура разрешения доступа к терминалу
 -------------------------------------------------------------------------------------------------------------*/
uint32_t Access_control(void)
{
  uint32_t       indx;
  uint8_t        b;
  GET_MCBL;
  char           pass_buf[PASS_STR_MAX_LEN + 1];
  char           pass_orig[PASS_STR_MAX_LEN + 1];
  uint32_t       sz;

  do
  {
    if (Is_flash_protection_type_3() == 0)
    {
      sz = strlen(DEF_MON_PASSWORD);
      strcpy(pass_orig, DEF_MON_PASSWORD);
      MPRINTF(VT100_CLEAR_AND_HOME);
      MPRINTF("Enter string %s: ", DEF_MON_PASSWORD);
    }
    else
    {
      sz = Monitor_pass_SIZE;
      if (sz > PASS_STR_MAX_LEN) sz = PASS_STR_MAX_LEN;
      strncpy(pass_orig, (char *)Monitor_pass, sz);
      pass_orig[sz] = 0;
      MPRINTF(VT100_CLEAR_AND_HOME);
      MPRINTF("Enter password: ");
    }

    indx = 0;
    while (1)
    {
      if (WAIT_CHAR(&b,  ms_to_ticks(100000)) == RES_OK)
      {
        if (b == VT100_CR) break;
        if (b == VT100_ESC) break;
        pass_buf[indx] = b;
        indx++;
        MPRINTF("*");
        if (indx > 32) return RES_ERROR;
      }
      else return RES_ERROR;
    }
    if (b == VT100_ESC) continue;

    pass_buf[indx] = 0;

    if (strncmp(pass_orig, pass_buf, sz) == 0) return RES_OK;

  }while (1);

}


/*-------------------------------------------------------------------------------------------------------------
  Ввод кода для доступа к специальному меню
-------------------------------------------------------------------------------------------------------------*/
int Enter_special_code(void)
{
  char str[32];
  GET_MCBL;

  if (mcbl->g_access_to_spec_menu != 0)
  {
    return RES_OK;
  }
  if (Is_flash_protection_disabled()) return RES_OK;

  MPRINTF(VT100_CLEAR_AND_HOME"Enter access code>");
  if (VT100_get_string(str, 31) == RES_OK)
  {
    if (memcmp(str, Engnr_menu_pass, Engnr_menu_pass_SIZE) == 0)
    {
      mcbl->g_access_to_spec_menu = 1;
      return RES_OK;
    }
  }

  return RES_ERROR;
}
