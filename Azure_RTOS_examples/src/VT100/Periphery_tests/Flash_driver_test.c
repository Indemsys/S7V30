// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020-01-30
// 12:04:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"

extern const flash_instance_t flash_cbl;

static void Flasher_test(void);


uint32_t  flash_block_start =  0;
uint32_t  flash_block_size  =  0;


static T_sys_timestump   t1;
static T_sys_timestump   t2;


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Get_t(void)
{
  Get_hw_timestump(&t1);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Show_t(void)
{
  GET_MCBL;
  uint32_t td;
  Get_hw_timestump(&t2);

  td = Hw_timestump_diff32_us(&t1,&t2);
  float    f = flash_block_size * 1000000.0f / (float)td;

  MPRINTF("Time = %d us, Speed = %0.1f byte/s\n\r", td, (double)f);
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_Flash_drivewr_test(uint8_t keycode)
{
  GET_MCBL;
  uint8_t   b;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  Flash driver test ===\n\r");
  MPRINTF("ESC - exit\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r\n\r");

  Flasher_test();

  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case VT100_ESC:
        return;
      }
    }

  }while (1);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _DataFlash_check_blank(void)
{
  GET_MCBL;
  flash_result_t flash_res;

  _Get_t();
  Set_flash_op_status(0);
  flash_cbl.p_api->blankCheck(flash_cbl.p_ctrl, flash_block_start, flash_block_size,&flash_res);
  uint32_t r= Wait_for_flash_op_end(ms_to_ticks(100));
  MPRINTF("---------------------------- CHECK BLANK      :");
  _Show_t();

  if (r != TX_SUCCESS)
  {
    MPRINTF("Blank check operation error.\n\r");
  }
  else
  {
    MPRINTF("Blank check operation done.\n\r");
  }

  MPRINTF("Flash operation status = %08X\n\r", Get_flash_op_status());

  if (Get_flash_op_status() & BIT_FLASH_EVENT_BLANK)
  {
    MPRINTF("Flash block blank.\n\r");
  }
  if (Get_flash_op_status() & BIT_FLASH_EVENT_NOT_BLANK)
  {
    MPRINTF("Flash block not blank.\n\r");
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _DataFlash_Erase(void)
{
  GET_MCBL;

  _Get_t();
  Set_flash_op_status(0);
  flash_cbl.p_api->erase(flash_cbl.p_ctrl, flash_block_start, 1);
  uint32_t r= Wait_for_flash_op_end(ms_to_ticks(100));
  MPRINTF("---------------------------- ERASE            :");
  _Show_t();

  if (r != TX_SUCCESS)
  {
    MPRINTF("Erase operation error.\n\r");
    return;
  }
  else
  {
    MPRINTF("Erase operation done.\n\r");
  }
  MPRINTF("Flash operation status = %08X\n\r", Get_flash_op_status());
}

/*-----------------------------------------------------------------------------------------------------


  \param block
-----------------------------------------------------------------------------------------------------*/
static void _DataFlash_write(uint32_t block)
{
  GET_MCBL;

  _Get_t();
  Set_flash_op_status(0);
  flash_cbl.p_api->write(flash_cbl.p_ctrl, block  , flash_block_start, flash_block_size);
  uint32_t r= Wait_for_flash_op_end(ms_to_ticks(100));
  MPRINTF("---------------------------- WRITE            :");
  _Show_t();

  if (r != TX_SUCCESS)
  {
    MPRINTF("Write operation error.\n\r");
    return;
  }
  else
  {
    MPRINTF("Write operation done.\n\r");
  }
  MPRINTF("Flash operation status = %08X\n\r", Get_flash_op_status());
}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _CodeFlash_check_blank(void)
{
  GET_MCBL;
  ssp_err_t      res;
  flash_result_t flash_res;
  UINT           prev_int_state;

  _Get_t();
  prev_int_state = tx_interrupt_control(TX_INT_DISABLE);
  res = flash_cbl.p_api->blankCheck(flash_cbl.p_ctrl, flash_block_start, flash_block_size,&flash_res);
  tx_interrupt_control(prev_int_state);
  MPRINTF("---------------------------- CHECK BLANK      :");
  _Show_t();

  if (res != SSP_SUCCESS)
  {
    MPRINTF("Function blankCheck error.\n\r");
  }

  if (flash_res == FLASH_RESULT_BLANK)
  {
    MPRINTF("Flash block blank.\n\r");
  }
  else if (flash_res == FLASH_RESULT_NOT_BLANK)
  {
    MPRINTF("Flash block not blank.\n\r");
  }
  else
  {
    MPRINTF("Flash block state unknown.\n\r");
  }
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _CodeFlash_Erase(void)
{
  GET_MCBL;
  ssp_err_t      res;
  UINT           prev_int_state;

  _Get_t();
  prev_int_state = tx_interrupt_control(TX_INT_DISABLE);
  res = flash_cbl.p_api->erase(flash_cbl.p_ctrl, flash_block_start, 1);
  tx_interrupt_control(prev_int_state);
  MPRINTF("---------------------------- ERASE            :");
  _Show_t();

  if (res != SSP_SUCCESS)
  {
    MPRINTF("Function Erase error.\n\r");
  }
  else
  {
    MPRINTF("Erase done.\n\r");
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param block
-----------------------------------------------------------------------------------------------------*/
static void _CodeFlash_write(uint32_t block)
{
  GET_MCBL;
  ssp_err_t      res;
  UINT           prev_int_state;

  _Get_t();
  prev_int_state = tx_interrupt_control(TX_INT_DISABLE);
  res = flash_cbl.p_api->write(flash_cbl.p_ctrl, block  , flash_block_start, flash_block_size);
  tx_interrupt_control(prev_int_state);
  MPRINTF("---------------------------- WRITE            :");
  _Show_t();

  if (res != SSP_SUCCESS)
  {
    MPRINTF("Function Write error.\n\r");
  }
  else
  {
    MPRINTF("Write done.\n\r");
  }
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
static void Flasher_test(void)
{
  GET_MCBL;

  uint8_t *block;



  if (Flash_driver_init() != RES_OK)
  {
    MPRINTF("Flasher_init error.\n\r");
    return;
  }


  if (flash_cbl.p_api->open(flash_cbl.p_ctrl,flash_cbl.p_cfg) != SSP_SUCCESS)
  {
    MPRINTF("Flash open error.\n\r");
    return;
  }
  else
  {
    MPRINTF("Driver opened.\n\r");
  }

  MPRINTF("\n\r\n\rData flash test.\n\r");
  flash_block_start =  DATA_FLASH_START;
  flash_block_size  =  DATA_FLASH_EBLOCK_SZ;
  block = App_malloc(flash_block_size);


  _DataFlash_check_blank();
  _DataFlash_Erase();
  _DataFlash_check_blank();


  for (uint32_t i=0; i < flash_block_size; i++) block[i] = i;
  _DataFlash_write((uint32_t)block);

  _DataFlash_check_blank();
  _DataFlash_Erase();
  _DataFlash_check_blank();

  for (uint32_t i=0; i < flash_block_size; i++) block[i] = 0xFF;
  _DataFlash_write((uint32_t)block);

  _DataFlash_check_blank();
  _DataFlash_Erase();
  _DataFlash_check_blank();

  for (uint32_t i=0; i < flash_block_size; i++) block[i] = 0x00;
  _DataFlash_write((uint32_t)block);

  _DataFlash_check_blank();
  _DataFlash_Erase();
  _DataFlash_check_blank();


  App_free(block);

  MPRINTF("\n\r\n\rCode flash test.\n\r");
  flash_block_start =  CODE_FLASH_END - CODE_FLASH_EBLOCK_SZ + 1;
  flash_block_size  =  CODE_FLASH_EBLOCK_SZ;
  block = App_malloc(flash_block_size);

  _CodeFlash_check_blank();
  _CodeFlash_Erase();
  _CodeFlash_check_blank();


  for (uint32_t i=0; i < flash_block_size; i++) block[i] = i;
  _CodeFlash_write((uint32_t)block);

  _CodeFlash_check_blank();
  _CodeFlash_Erase();
  _CodeFlash_check_blank();

  for (uint32_t i=0; i < flash_block_size; i++) block[i] = 0xFF;
  _CodeFlash_write((uint32_t)block);

  _CodeFlash_check_blank();
  _CodeFlash_Erase();
  _CodeFlash_check_blank();

  for (uint32_t i=0; i < flash_block_size; i++) block[i] = 0x00;
  _CodeFlash_write((uint32_t)block);

  _CodeFlash_check_blank();
  _CodeFlash_Erase();
  _CodeFlash_check_blank();

  App_free(block);

  flash_cbl.p_api->close(flash_cbl.p_ctrl);

}

