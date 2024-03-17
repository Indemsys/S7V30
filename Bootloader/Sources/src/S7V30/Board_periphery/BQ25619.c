// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.06.01
// 15:54:02
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"

const i2c_cfg_t g_sf_i2c_bq25619_i2c_cfg =
{
  .channel             = 2,
  .rate                = I2C_RATE_STANDARD,
  .slave               = 0x6A,
  .addr_mode           = I2C_ADDR_MODE_7BIT,
  .sda_delay           = (300),
  .p_transfer_tx       = &DTC_I2C2_TX_transfer_instance,
  .p_transfer_rx       = &DTC_I2C2_RX_transfer_instance,
  .p_callback          = (NULL),
  .p_context           = &I2C2_interface_instance,
  .rxi_ipl             = 12,
  .txi_ipl             = 12,
  .tei_ipl             = 12,
  .eri_ipl             = 12,
  .p_extend            = &I2C2_extended_configuration,
};

sf_i2c_instance_ctrl_t g_sf_i2c_bq25619_ctrl =
{
  .p_lower_lvl_ctrl =&I2C2_control_block,
};

const sf_i2c_cfg_t g_sf_i2c_bq25619_cfg =
{
  .p_bus                    = (sf_i2c_bus_t *)&I2C2_bus,
  .p_lower_lvl_cfg          =&g_sf_i2c_bq25619_i2c_cfg,
};


/* Instance structure to use this module. */
const sf_i2c_instance_t g_sf_i2c_bq25619 =
{
  .p_ctrl        =&g_sf_i2c_bq25619_ctrl,
  .p_cfg         =&g_sf_i2c_bq25619_cfg,
  .p_api         =&g_sf_i2c_on_sf_i2c
};


uint32_t        BQ25619_open_err = 0;


#define     PSEL_PIN     R_PFS->P807PFS_b.PODR



static uint8_t rv[13];


const T_bq25619_reg_descr bq25619_regs[]=
{
  {  BQ25619_INPUT_SOURCE_CONTROL                         , "Input Source Control Register                     ",&rv[0]  },  // Input Source Control Register
  {  BQ25619_POWER_ON_CONFIGURATION                       , "Power-On Configuration Register                   ",&rv[1]  },  // Power-On Configuration Register
  {  BQ25619_CHARGE_CURRENT_CONTROL                       , "Charge Current Control Register                   ",&rv[2]  },  // Charge Current Control Register
  {  BQ25619_PRE_CHARGE_TERMINATION_CURRENT_CONTROL       , "Pre-Charge/Termination Current Control Register   ",&rv[3]  },  // Pre-Charge/Termination Current Control Register
  {  BQ25619_CHARGE_VOLTAGE_CONTROL                       , "Charge Voltage Control Register                   ",&rv[4]  },  // Charge Voltage Control Register
  {  BQ25619_CHARGE_TERMINATION_TIMER_CONTROL             , "Charge Termination/Timer Control Register         ",&rv[5]  },  // Charge Termination/Timer Control Register
  {  BQ25619_BOOST_VOLTAGE_THERMAL_REGULATION_CONTROL     , "Boost Voltage/Thermal Regulation Control Register ",&rv[6]  },  // Boost Voltage/Thermal Regulation Control Register
  {  BQ25619_MISC_OPERATION_CONTROL                       , "Misc Operation Control Register                   ",&rv[7]  },  // Misc Operation Control Register
  {  BQ25619_SYSTEM_STATUS0                               , "System Status Register 0                          ",&rv[8]  },  // System Status Register 0
  {  BQ25619_SYSTEM_STATUS1                               , "System Status Register 1                          ",&rv[9]  },  // System Status Register 1
  {  BQ25619_SYSTEM_STATUS2                               , "System Status Register 2                          ",&rv[10] },  // System Status Register 2
  {  BQ25619_PART_INFORMATION                             , "Part information                                  ",&rv[11] },  // Part information
  {  BQ25619_CHARGER_CONTROL4                             , "Charger Control 4 Register                        ",&rv[12] },  // Charger Control 4 Register
};

#define BQ25619_VERIFY_RESULT( x )  { ssp_err_t vres; vres = (x); if ( vres != SSP_SUCCESS ) { goto EXIT_ON_ERROR; } }

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t BQ25619_reg_count(void)
{
  return sizeof(bq25619_regs) / sizeof(bq25619_regs[0]);
}

/*-----------------------------------------------------------------------------------------------------


  \param indx

  \return T_da7217_reg_descr
-----------------------------------------------------------------------------------------------------*/
T_bq25619_reg_descr const* BQ25619_get_reg_descr(uint8_t indx)
{
  if (indx >= BQ25619_reg_count()) indx = 0;
  return &bq25619_regs[indx];
}


/*-----------------------------------------------------------------------------------------------------
  Открытие коммуникационного канала с чипом BQ25619

  Функция использует глобальный объект  g_sf_i2c_bq25619
  Быть внимательным при вызове из разных задач!!!

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  BQ25619_open(void)
{
  uint32_t      attempt_cnt = 0;
  ssp_err_t     res = SSP_SUCCESS;
  do
  {
    res = g_sf_i2c_bq25619.p_api->open(g_sf_i2c_bq25619.p_ctrl, g_sf_i2c_bq25619.p_cfg);
    if (res == SSP_ERR_ALREADY_OPEN) res = SSP_SUCCESS;
    if (res == SSP_SUCCESS)  break;
    if (attempt_cnt > 5) break;
    Wait_ms(50);
    attempt_cnt++;
  } while (1);
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param timeout

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t  BQ25619_lock(uint32_t timeout)
{
  ssp_err_t           res = SSP_SUCCESS;
  res = g_sf_i2c_bq25619.p_api->lockWait(g_sf_i2c_bq25619.p_ctrl, timeout);
  return res;
}

/*-----------------------------------------------------------------------------------------------------



  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t  BQ25619_unlock(void)
{
  ssp_err_t           res = SSP_SUCCESS;
  res = g_sf_i2c_bq25619.p_api->unlock(g_sf_i2c_bq25619.p_ctrl);
  return res;
}



/*-----------------------------------------------------------------------------------------------------
  Закрытие коммуникационного канала с чипом BQ25619

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  BQ25619_close(void)
{
  return g_sf_i2c_bq25619.p_api->close(g_sf_i2c_bq25619.p_ctrl);
}

/*-----------------------------------------------------------------------------------------------------
  Запись буфера с данными в кодек
  Первый байт буффера должен быть свободным для размещения в нем адреса
  Размер буфера указывается без учета первого байта

  \param sz

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t BQ25619_write_buf(uint8_t addr, uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  buf[0] = addr;
  res = g_sf_i2c_bq25619.p_api->write(g_sf_i2c_bq25619.p_ctrl, buf, sz+1, false , 10); // Записываем адрес
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param buf
  \param sz

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t BQ25619_read_buf(uint8_t addr, uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  res = g_sf_i2c_bq25619.p_api->write(g_sf_i2c_bq25619.p_ctrl,&addr, 1, true , 10); // Записываем адрес
  if (res == SSP_SUCCESS)
  {
    res = g_sf_i2c_bq25619.p_api->read(g_sf_i2c_bq25619.p_ctrl, buf, sz, false , 10); // Читаем данные
  }
  return res;
}



/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param val

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t BQ25619_write_reg(uint8_t addr, uint8_t val)
{
  uint8_t buf[2];

  buf[1] = val;
  return BQ25619_write_buf(addr,buf,1);
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param val

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t BQ25619_read_reg(uint8_t addr, uint8_t *val)
{
  return BQ25619_read_buf(addr,val,1);
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
uint32_t BQ25619_read_all(void)
{
  ssp_err_t err = SSP_SUCCESS;

  uint32_t n = BQ25619_reg_count();
  for (uint32_t i=0; i < n; i++)
  {
    err = BQ25619_read_buf(bq25619_regs[i].reg_addr, bq25619_regs[i].reg_val, 1);
  }
  return err;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t BQ25619_get_system_status(void)
{
  return   *bq25619_regs[BQ25619_SYSTEM_STATUS0].reg_val;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Set_BQ25619_PSEL_HIGH(void)
{
  PSEL_PIN = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Set_BQ25619_PSEL_LOW(void)
{
  PSEL_PIN = 0;
}


/*-----------------------------------------------------------------------------------------------------
  Отключаем питание от аккумулятора

  \param addr
  \param val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  BQ25619_sw_off_accum(void)
{
  uint8_t reg;

  //
  reg = 0
       + LSHIFT(0, 7) // PFM_DIS      | PFM disable in both buck and boost mode. 0 – PFM enable (default). 1 – PFM disable
       + LSHIFT(0, 6) // WD_RST       | I2C Watchdog Timer Reset. Back to 0 after watchdog timer reset. 1 – Reset
       + LSHIFT(0, 5) // BST_CONFIG   | Boost mode enable. 0 – boost mode disable (default)
       + LSHIFT(0, 4) // CHG_CONFIG   | 1 - Charge Enable
       + LSHIFT(1, 3) // SYS_MIN[2]   | Minimum System Voltage Limit
       + LSHIFT(0, 2) // SYS_MIN[1]   | Default: 3.5 V (101)
       + LSHIFT(1, 1) // SYS_MIN[0]   |
       + LSHIFT(1, 0) // MIN_VBAT_SEL | Minimum battery voltage when exiting boost mode. 0 – 2.8 V BAT falling, 3 V rising (default)
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_POWER_ON_CONFIGURATION,reg));

  //
  reg = 0
       + LSHIFT(1, 7) // EN_HIZ     | 0 – Disable, 1 – Enable.  Отключение внешнего источника питания от чипа. Система остается питаеться только от аккумулятора.
       + LSHIFT(0, 6) // TS_IGNORE  | 1 – Ignore TS pin. Always consider TS is good to allow charging and boost mode.
       + LSHIFT(0, 5) // BATSNS_DIS | Select either BATSNS pin or BAT pin to regulate battery voltage. 0 – Enable BATSNS in battery CV regulation.
       + LSHIFT(1, 4) // IINDPM[4]  | Input current limit setting (maximum limit, not typical)
       + LSHIFT(0, 3) // IINDPM[3]  | Default: 2400 mA (10111)
       + LSHIFT(1, 2) // IINDPM[2]  |
       + LSHIFT(1, 1) // IINDPM[1]  |
       + LSHIFT(1, 0) // IINDPM[0]  |
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_INPUT_SOURCE_CONTROL,reg));

  //
  reg= 0
      + LSHIFT(1, 7) // IINDET_EN           | 1 – Force Input current limit detection when VBUS power is presence
      + LSHIFT(0, 6) // TMR2X_EN            | 1 – Safety timer slowed by 2X during input DPM or thermal regulation
      + LSHIFT(0, 5) // BATFET_DIS          | 1 – Turn off BATFET (Q4)
      + LSHIFT(1, 4) // BATFET_RST_WVBUS    | Start BATFET full system reset with or without adapter present. 0 – Start BATFET full system reset after adapter is removed from VBUS. (default)
      + LSHIFT(0, 3) // BATFET_DLY          | Delay from BATFET_DIS (REG07[5]) set to 1 to BATFET turn off during ship mode. 0 – Turn off BATFET immediately when BATFET_DIS bit is set
      + LSHIFT(1, 2) // BATFET_RST_EN       | Enable BATFET full system reset. 1 – Enable BATFET reset function when REG07[5] is also 1. (default)
      + LSHIFT(0, 1) // VINDPM_BAT_TRACK[1] | Sets VINDPM to track BAT voltage.
      + LSHIFT(0, 0) // VINDPM_BAT_TRACK[0] | 00 – Disable function (VINDPM set by register) (default)
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_MISC_OPERATION_CONTROL,reg));

  reg= 0
      + LSHIFT(1, 7) // IINDET_EN           | 1 – Force Input current limit detection when VBUS power is presence
      + LSHIFT(0, 6) // TMR2X_EN            | 1 – Safety timer slowed by 2X during input DPM or thermal regulation
      + LSHIFT(1, 5) // BATFET_DIS          | 1 – Turn off BATFET (Q4)
      + LSHIFT(1, 4) // BATFET_RST_WVBUS    | Start BATFET full system reset with or without adapter present. 0 – Start BATFET full system reset after adapter is removed from VBUS. (default)
      + LSHIFT(0, 3) // BATFET_DLY          | Delay from BATFET_DIS (REG07[5]) set to 1 to BATFET turn off during ship mode. 0 – Turn off BATFET immediately when BATFET_DIS bit is set
      + LSHIFT(1, 2) // BATFET_RST_EN       | Enable BATFET full system reset. 1 – Enable BATFET reset function when REG07[5] is also 1. (default)
      + LSHIFT(0, 1) // VINDPM_BAT_TRACK[1] | Sets VINDPM to track BAT voltage.
      + LSHIFT(0, 0) // VINDPM_BAT_TRACK[0] | 00 – Disable function (VINDPM set by register) (default)
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_MISC_OPERATION_CONTROL,reg));

  return RES_OK;
EXIT_ON_ERROR:
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------
  Отключаем питание от аккумулятора

  \param addr
  \param val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  BQ25619_sw_off_charger(void)
{
  uint8_t reg;

  //
  reg = 0
       + LSHIFT(0, 7) // PFM_DIS      | PFM disable in both buck and boost mode. 0 – PFM enable (default). 1 – PFM disable
       + LSHIFT(0, 6) // WD_RST       | I2C Watchdog Timer Reset. Back to 0 after watchdog timer reset. 1 – Reset
       + LSHIFT(0, 5) // BST_CONFIG   | Boost mode enable. 0 – boost mode disable (default)
       + LSHIFT(0, 4) // CHG_CONFIG   | 1 - Charge Enable
       + LSHIFT(1, 3) // SYS_MIN[2]   | Minimum System Voltage Limit
       + LSHIFT(0, 2) // SYS_MIN[1]   | Default: 3.5 V (101)
       + LSHIFT(1, 1) // SYS_MIN[0]   |
       + LSHIFT(1, 0) // MIN_VBAT_SEL | Minimum battery voltage when exiting boost mode. 0 – 2.8 V BAT falling, 3 V rising (default)
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_POWER_ON_CONFIGURATION,reg));

  //
  reg = 0
       + LSHIFT(1, 7) // EN_HIZ     | 0 – Disable, 1 – Enable.  Отключение внешнего источника питания от чипа. Система остается питаеться только от аккумулятора.
       + LSHIFT(0, 6) // TS_IGNORE  | 1 – Ignore TS pin. Always consider TS is good to allow charging and boost mode.
       + LSHIFT(0, 5) // BATSNS_DIS | Select either BATSNS pin or BAT pin to regulate battery voltage. 0 – Enable BATSNS in battery CV regulation.
       + LSHIFT(1, 4) // IINDPM[4]  | Input current limit setting (maximum limit, not typical)
       + LSHIFT(0, 3) // IINDPM[3]  | Default: 2400 mA (10111)
       + LSHIFT(1, 2) // IINDPM[2]  |
       + LSHIFT(1, 1) // IINDPM[1]  |
       + LSHIFT(1, 0) // IINDPM[0]  |
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_INPUT_SOURCE_CONTROL,reg));


  return RES_OK;
EXIT_ON_ERROR:
  return RES_ERROR;
}

