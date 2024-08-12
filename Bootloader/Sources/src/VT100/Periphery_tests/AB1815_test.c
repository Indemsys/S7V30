// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-01-28
// 17:10:20
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void   _Print_AB1815_test_header(void)
{
  GET_MCBL;
  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  AB1815 test ===\n\r");
  MPRINTF("ESC - exit, [S] - read chip ID, [R] - read all registers, [I] - Init chip for work\n\r");
  MPRINTF("            [W] - write register                                                  \n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");

}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _AB1815_read_chip_ID(void)
{
  uint16_t  chip_id;

  GET_MCBL;

  if (AB1815_read_ID(&chip_id)==RES_OK)
  {
    MPRINTF("AB1815 CHIP ID = %04X. ", chip_id);
    if (chip_id == AB1815_CHIP_ID)
    {
      MPRINTF(" Correct!\n\r");
    }
    else
    {
      MPRINTF(" Incorrect!\n\r");
    }
  }
  else
  {
    MPRINTF("AB1815 CHIP ID reading error.");
  }

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _AB1815_Read_all_registers(void)
{
  uint8_t   reg;
  uint32_t  n;

  GET_MCBL;

  n = AB1815_get_registers_count();
  for (uint32_t i=0; i < n; i++)
  {
    const T_AB1815_reg_descr *dscr = AB1815_get_register_descr(i);

    if (AB1815_read_register(dscr->reg_addr,&reg) != RES_OK)
    {
      MPRINTF("Read register %04X error.\n\r", dscr->reg_addr);
      goto EXIT_ON_ERROR;
    }

    MPRINTF("%s(%04X) = %02X\n\r", dscr->reg_name, dscr->reg_addr, reg);

  }

  MPRINTF("\n\r");

  T_AB1815_buf *p_ab1815_buf;
  n = 0;
  if (AB1815_read_RAM(&p_ab1815_buf) == RES_OK)
  {
    MPRINTF("AB1815 RAM content:\n\r");
    do
    {
      for (uint32_t i=0; i < 8; i++)
      {
        MPRINTF(" %02X", p_ab1815_buf->data[n]);
        n++;
        if (n >= AB1815_NVRAM_SZ) break;
      }
      MPRINTF("\n\r");
    } while (n < AB1815_NVRAM_SZ);
  }

  MPRINTF("\n\r");
  return RES_OK;

EXIT_ON_ERROR:

  MPRINTF("\n\r");
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _AB1815_Init_chip_for_work(void)
{
  T_AB1815_init_res *p_ires;
  GET_MCBL;

  if (AB1815_init() == RES_OK)
  {
    MPRINTF("AB1815 initialising done.\n\r\n\r");

    p_ires = AB1815_get_init_res();

    MPRINTF("CHIP IP = %d, LOT NUM.=%d \n\r", p_ires->rtc_idn, p_ires->rtc_lotn);

    if (p_ires->status & BIT(7))
    {
      MPRINTF("Years register rolls over from 99 to 00\n\r");
    }
    if (p_ires->status & BIT(6))
    {
      MPRINTF("System switches to the VBAT Power state\n\r");
    }
    if (p_ires->status & BIT(5))
    {
      MPRINTF("Watchdog Timer is enabled and is triggered\n\r");
    }
    if (p_ires->status & BIT(4))
    {
      MPRINTF("Battery voltage VBAT crosses the reference voltage selected by BREF in the direction selected by BPOL\n\r");
    }
    if (p_ires->status & BIT(3))
    {
      MPRINTF("Countdown Timer is enabled and reaches zero.\n\r");
    }
    if (p_ires->status & BIT(2))
    {
      MPRINTF("Alarm function is enabled and all selected Alarm registers match their respective counters.\n\r");
    }
    if (p_ires->status & BIT(1))
    {
      MPRINTF("External trigger is detected on the WDI pin.\n\r");
    }
    if (p_ires->status & BIT(0))
    {
      MPRINTF("External trigger is detected on the EXTI pin.\n\r");
    }

    if (p_ires->osc_status & BIT(4))
    {
      MPRINTF("RC Oscillator is selected to drive the internal clocks.\n\r");
    }
    else
    {
      MPRINTF("Crystal Oscillator is selected to drive the internal clocks.\n\r");
    }
    if (p_ires->osc_status & BIT(1))
    {
      MPRINTF("Oscillator Failure. This bit is set on a power on reset, when both the system and battery voltages have dropped below acceptable levels.\n\r");
    }
    if (p_ires->osc_status & BIT(0))
    {
      MPRINTF("Autocalibration Failure occurs.\n\r");
    }
    if (p_ires->data_crc_fault == 0)
    {
      MPRINTF("RTC RAM CRC write succeeded.\n\r");
    }
    else
    {
      MPRINTF("RTC RAM CRC write error occurred.\n\r");
    }

    return RES_OK;
  }
  else
  {
    MPRINTF("AB1815 initialising error.\n\r\n\r");
    return RES_ERROR;
  }
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _AB1815_Write_register(void)
{
  GET_MCBL;
  char    in_str[32];

  uint8_t  b;
  uint32_t reg_addr = AB1815_HUNDREDTHS;
  uint32_t reg_val = 0;


  do
  {

    VT100_set_cursor_pos(10, 0);

    if (AB1815_read_register((uint8_t)reg_addr,&b) != RES_OK)
    {
      MPRINTF("Register %02X reading error.\n\r", reg_addr);
      break;
    }
    reg_val = b;
    MPRINTF("Register %02X (%s) = %02X\n\r", reg_addr, AB1815_get_register_name_by_addr(reg_addr), reg_val);

    if (VT100_edit_string_in_pos(in_str, 31, 11, "") != RES_OK) break;

    if (sscanf(in_str, "%02X %02X",&reg_addr,&reg_val) == 2)
    {
      if (AB1815_write_register((uint8_t)reg_addr, (uint8_t)reg_val) != RES_OK)
      {
        MPRINTF("Register %02X writing error.\n\r", reg_addr);
        Wait_ms(2000);
        break;
      }
    }
    else
    {
      MPRINTF("Wrong arguments.\n\r");
    }


  } while (1);

  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Do_AB1815_test(uint8_t keycode)
{
  uint8_t             b;
  GET_MCBL;

  _Print_AB1815_test_header();

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
          _AB1815_read_chip_ID();
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_AB1815_test_header();
          break;

        case 'R':
        case 'r':
          _AB1815_Read_all_registers();
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_AB1815_test_header();
          break;

        case 'I':
        case 'i':
          _AB1815_Init_chip_for_work();
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_AB1815_test_header();
          break;

        case 'W':
        case 'w':
          _AB1815_Write_register();
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_AB1815_test_header();
          break;

        case VT100_ESC:
          return;

        default:
          _Print_AB1815_test_header();
          break;
        }
      }
    }
  } while (1);
}



