// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.05.19
// 22:10:04
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"




/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Print_SDRAM_test_header(void)
{
  GET_MCBL;
  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  SDRAM test ===\n\r");
  MPRINTF("ESC - exit, [S] - runing counter filling test, [C] - constants filling test\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint64_t Fill_SDRAM(uint32_t val)
{
  T_sys_timestump    t1, t2;

  uint32_t *addr;
  Get_hw_timestump(&t1);
  for (addr = (uint32_t *)SDRAM_BEGIN; addr  < (uint32_t *)SDRAM_END; addr++)
  {
    *addr = val;
  }
  Get_hw_timestump(&t2);
  return Hw_timestump_diff64_us(&t1,&t2);
}

/*-----------------------------------------------------------------------------------------------------


  \param raw
  \param constv
-----------------------------------------------------------------------------------------------------*/
static void Fill_and_check_by_const(uint32_t raw, uint32_t constv)
{
  uint8_t   b;
  uint32_t *addr;
  uint32_t  cnt;
  uint32_t  err_cnt = 0;
  uint64_t  td;
  GET_MCBL;

  VT100_set_cursor_pos(raw,1);
  MPRINTF(VT100_CLR_LINE"Filling by %08X...",constv);
  td = Fill_SDRAM(constv);
  VT100_set_cursor_pos(raw,1);
  MPRINTF(VT100_CLR_LINE"Checking by %08X...",constv);
  for (addr = (uint32_t *)SDRAM_BEGIN; addr  < (uint32_t *)SDRAM_END; addr++)
  {
    uint32_t v1;
    v1 =*addr;
    if (v1 != constv)
    {
      err_cnt++;
      MPRINTF(VT100_CLR_LINE"Memory error by address: %08X  - readed %08X but must be %08X (errors count=%d)", (uint32_t)addr, v1, constv, err_cnt);
      if (WAIT_CHAR(&b,  ms_to_ticks(100000)) == RES_OK)
      {
        switch (b)
        {
        case VT100_ESC:
          return;
        }
      }
    }
    cnt++;
    if (cnt > 1024 * 64)
    {
      VT100_set_cursor_pos(raw,1);
      MPRINTF(VT100_CLR_LINE"Read from address: %08X", (uint32_t)addr);
      cnt = 0;
    }
  }

  VT100_set_cursor_pos(raw,1);
  MPRINTF(VT100_CLR_LINE"Last address: %08X", (uint32_t)addr);

  VT100_set_cursor_pos(raw+1,1);
  if (err_cnt == 0)
  {
    MPRINTF(VT100_CLR_LINE"Filling by %08X SDRAM test done. All Ok!  (filling time = %lld, filling speed = %d MB/s )", constv, td,(SDRAM_END - SDRAM_BEGIN + 1) / td);
  }
  else
  {
    MPRINTF(VT100_CLR_LINE"SDRAM test error! Errors count= %d ", err_cnt);
  }
  WAIT_CHAR(&b,  ms_to_ticks(10000));
  VT100_set_cursor_pos(raw,1);
  MPRINTF(VT100_CLR_LINE);
  VT100_set_cursor_pos(raw+1,1);
  MPRINTF(VT100_CLR_LINE);

}
/*-----------------------------------------------------------------------------------------------------
  Тест SDRAM заполнением константами


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Test_SDRAM_fill_constants(uint32_t raw)
{
  GET_MCBL;

  VT100_set_cursor_pos(raw,1);
  MPRINTF(VT100_CLR_LINE"Test by filling constants", SDRAM_BEGIN);

  Fill_and_check_by_const(raw+1, 0);
  Fill_and_check_by_const(raw+1, 0xFFFFFFFF);
  Fill_and_check_by_const(raw+1, 0x5555AAAA);
  Fill_and_check_by_const(raw+1, 0xAAAA5555);

}

/*-----------------------------------------------------------------------------------------------------
  Тест SDRAM заполнением случайным числом

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Test_SDRAM_fill_runing_counter(uint32_t raw)
{
  uint8_t   b;
  uint32_t *addr;
  uint32_t  run_cnt;
  uint32_t  cnt;
  uint32_t  err_cnt = 0;
  GET_MCBL;

  VT100_set_cursor_pos(raw,1);
  MPRINTF(VT100_CLR_LINE"Test by filling from runing counter", SDRAM_BEGIN);

  // Заполняем SDRAM
  run_cnt = 0;
  cnt = 0;
  VT100_set_cursor_pos(raw+1,1);
  MPRINTF(VT100_CLR_LINE"Write to address: %08X", SDRAM_BEGIN);
  for (addr = (uint32_t *)SDRAM_BEGIN; addr  < (uint32_t *)SDRAM_END; addr++)
  {
    *addr = run_cnt++;
    cnt++;
    if (cnt > 1024 * 64)
    {
      VT100_set_cursor_pos(raw+1,1);
      MPRINTF(VT100_CLR_LINE"Write to address: %08X", (uint32_t)addr);
      cnt = 0;
    }
  }

  // Читаем SDRAM и сравниваем
  run_cnt = 0;
  cnt = 0;
  VT100_set_cursor_pos(raw+1,1);
  MPRINTF(VT100_CLR_LINE"Read from address: %08X", SDRAM_BEGIN);
  for (addr = (uint32_t *)SDRAM_BEGIN; addr  < (uint32_t *)SDRAM_END; addr++)
  {
    uint32_t v1, v2;
    v1 =*addr;
    v2 = run_cnt++;
    if (v1 != v2)
    {
      err_cnt++;
      MPRINTF("\r\nMemory error by address: %08X  - readed %08X but must be %08X (errors count=%d)", (uint32_t)addr, v1, v2, err_cnt);
      if (WAIT_CHAR(&b,  ms_to_ticks(100000)) == RES_OK)
      {
        switch (b)
        {
        case VT100_ESC:
          return;
        }
      }
    }
    cnt++;
    if (cnt > 1024 * 64)
    {
      VT100_set_cursor_pos(raw+1,1);
      MPRINTF(VT100_CLR_LINE"Read from address: %08X", (uint32_t)addr);
      cnt = 0;
    }
  }
  VT100_set_cursor_pos(raw+1,1);
  MPRINTF(VT100_CLR_LINE"Last address: %08X", (uint32_t)addr);
  VT100_set_cursor_pos(raw+2,1);

  if (err_cnt == 0)
  {
    MPRINTF(VT100_CLR_LINE"SDRAM test done. All Ok!");
  }
  else
  {
    MPRINTF(VT100_CLR_LINE"SDRAM test error! Errors count= %d ", err_cnt);
  }
}
/*-----------------------------------------------------------------------------------------------------
  Тестирование чипа MT48LC16M16A2B4-6A (32 МБайта)
   0x90000000... 0x92000000

  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_SDRAM_test(uint8_t keycode)
{
  uint8_t             b;
  GET_MCBL;

  Print_SDRAM_test_header();

  do
  {
    if (WAIT_CHAR(&b,  ms_to_ticks(100)) == RES_OK)
    {
      if (b > 0)
      {
        switch (b)
        {
        case 'S':
        case 's':
          Test_SDRAM_fill_runing_counter(6);
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          Print_SDRAM_test_header();
          break;
        case 'C':
        case 'c':
          Test_SDRAM_fill_constants(6);
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          Print_SDRAM_test_header();
          break;

        case VT100_ESC:
          return;

        default:
          Print_SDRAM_test_header();
          break;
        }
      }
    }
  } while (1);

}


