﻿// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-01-27
// 11:47:55 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Print_ISM330_test_header(void)
{
  GET_MCBL;
  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  ISM330 test ===\n\r");
  MPRINTF("ESC - exit, [S] - run test\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _ISM330_Test(void)
{
  uint8_t   b;

  GET_MCBL;


  if (ISM330_open() != SSP_SUCCESS)
  {
    MPRINTF("Connection to ISM330 error.\n\r");
    return RES_ERROR;
  }

  if (ISM330_read_register(ISM330_WHO_AM_I,&b) != SSP_SUCCESS)
  {
    MPRINTF("Read ISM330_WHO_AM_I error.\n\r");
    goto EXIT_ON_ERROR;
  }

  MPRINTF("ISM330 CHIP ID = %02X. ", b);

  if (b == ISM330_CHIP_ID)
  {
    MPRINTF(" Correct!\n\r");
  }
  else
  {
    MPRINTF(" Incorrect!\n\r");
  }

  ISM330_close();
  return RES_OK;

EXIT_ON_ERROR:


  ISM330_close();
  return RES_ERROR;

}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_ISM330_test(uint8_t keycode)
{
  uint8_t             b;
  GET_MCBL;

  _Print_ISM330_test_header();

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
          _ISM330_Test();
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_ISM330_test_header();
          break;

        case VT100_ESC:
          return;

        default:
          _Print_ISM330_test_header();
          break;
        }
      }
    }
  } while (1);
}


