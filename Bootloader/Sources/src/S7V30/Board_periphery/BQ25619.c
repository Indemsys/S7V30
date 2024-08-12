// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.06.01
// 15:54:02
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



static uint32_t        BQ25619_opened = 0;


static uint8_t  rv[13];


const T_bq25619_reg_descr bq25619_regs[] =
{
  {  BQ25619_INPUT_CURRENT_LIMIT                          , "Input current limit       ",&rv[0]  },  // Input Source Control Register
  {  BQ25619_CHARGER_CONTROL_0                            , "Charger control 0         ",&rv[1]  },  // Power-On Configuration Register
  {  BQ25619_CHARGE_CURRENT_LIMIT                         , "Charge current limit      ",&rv[2]  },  // Charge Current Control Register
  {  BQ25619_PRE_CHARGE_TERMINATION_CURRENT_LIMIT         , "Precharge term. cur. lim. ",&rv[3]  },  // Pre-Charge/Termination Current Control Register
  {  BQ25619_BATTERY_VOLTAGE_LIMIT                        , "Battery voltage limit     ",&rv[4]  },  // Charge Voltage Control Register
  {  BQ25619_CHARGER_CONTROL_1                            , "Charger control_1         ",&rv[5]  },  // Charge Termination/Timer Control Register
  {  BQ25619_CHARGER_CONTROL_2                            , "Charger control_2         ",&rv[6]  },  // Boost Voltage/Thermal Regulation Control Register
  {  BQ25619_CHARGER_CONTROL_3                            , "Charger control_3         ",&rv[7]  },  // Misc Operation Control Register
  {  BQ25619_SYSTEM_STATUS0                               , "System status0            ",&rv[8]  },  // System Status Register 0
  {  BQ25619_SYSTEM_STATUS1                               , "System status1            ",&rv[9]  },  // System Status Register 1
  {  BQ25619_SYSTEM_STATUS2                               , "System status2            ",&rv[10] },  // System Status Register 2
  {  BQ25619_PART_INFORMATION                             , "Part information          ",&rv[11] },  // Part information
  {  BQ25619_CHARGER_CONTROL4                             , "Charger control4          ",&rv[12] },  // Charger Control 4 Register
};

#define BQ25619_VERIFY_RESULT( x )  { ssp_err_t vres; vres = (x); if ( vres != SSP_SUCCESS ) { goto EXIT_ON_ERROR; } }

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Is_BQ25619_opened(void)
{
  return BQ25619_opened;
}

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
ssp_err_t  BQ25619_open(void)
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

  if (res == SSP_SUCCESS)
  {
    BQ25619_opened = 1;
  }
  else
  {
    BQ25619_opened = 0;
  }
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
  res = g_sf_i2c_bq25619.p_api->write(g_sf_i2c_bq25619.p_ctrl, buf, sz + 1, false , 10); // Записываем адрес
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
  for (uint32_t i = 0; i < n; i++)
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


  \param status

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t BQ25619_get_charge_state(uint8_t status)
{
  return (status >> 3) & 3;
}

/*-----------------------------------------------------------------------------------------------------


  \param status

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t BQ25619_get_vbus_state(uint8_t status)
{
  return (status >> 5) & 7;
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
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_0,reg));

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
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_INPUT_CURRENT_LIMIT,reg));

  //
  reg = 0
        + LSHIFT(1, 7) // IINDET_EN           | 1 – Force Input current limit detection when VBUS power is presence
        + LSHIFT(0, 6) // TMR2X_EN            | 1 – Safety timer slowed by 2X during input DPM or thermal regulation
        + LSHIFT(0, 5) // BATFET_DIS          | 1 – Turn off BATFET (Q4)
        + LSHIFT(1, 4) // BATFET_RST_WVBUS    | Start BATFET full system reset with or without adapter present. 0 – Start BATFET full system reset after adapter is removed from VBUS. (default)
        + LSHIFT(0, 3) // BATFET_DLY          | Delay from BATFET_DIS (REG07[5]) set to 1 to BATFET turn off during ship mode. 0 – Turn off BATFET immediately when BATFET_DIS bit is set
        + LSHIFT(1, 2) // BATFET_RST_EN       | Enable BATFET full system reset. 1 – Enable BATFET reset function when REG07[5] is also 1. (default)
        + LSHIFT(0, 1) // VINDPM_BAT_TRACK[1] | Sets VINDPM to track BAT voltage.
        + LSHIFT(0, 0) // VINDPM_BAT_TRACK[0] | 00 – Disable function (VINDPM set by register) (default)
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_3,reg));

  reg = 0
        + LSHIFT(1, 7) // IINDET_EN           | 1 – Force Input current limit detection when VBUS power is presence
        + LSHIFT(0, 6) // TMR2X_EN            | 1 – Safety timer slowed by 2X during input DPM or thermal regulation
        + LSHIFT(1, 5) // BATFET_DIS          | 1 – Turn off BATFET (Q4)
        + LSHIFT(1, 4) // BATFET_RST_WVBUS    | Start BATFET full system reset with or without adapter present. 0 – Start BATFET full system reset after adapter is removed from VBUS. (default)
        + LSHIFT(0, 3) // BATFET_DLY          | Delay from BATFET_DIS (REG07[5]) set to 1 to BATFET turn off during ship mode. 0 – Turn off BATFET immediately when BATFET_DIS bit is set
        + LSHIFT(1, 2) // BATFET_RST_EN       | Enable BATFET full system reset. 1 – Enable BATFET reset function when REG07[5] is also 1. (default)
        + LSHIFT(0, 1) // VINDPM_BAT_TRACK[1] | Sets VINDPM to track BAT voltage.
        + LSHIFT(0, 0) // VINDPM_BAT_TRACK[0] | 00 – Disable function (VINDPM set by register) (default)
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_3,reg));

  return RES_OK;
EXIT_ON_ERROR:
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  BQ25619_set_max_sys_voltage(void)
{
  uint8_t reg;

  // Запрещаем зарядку
  reg = 0
        + LSHIFT(0, 7) // PFM_DIS      | PFM disable in both buck and boost mode. 0 – PFM enable (default). 1 – PFM disable
        + LSHIFT(1, 6) // WD_RST       | I2C Watchdog Timer Reset. Back to 0 after watchdog timer reset. 1 – Reset
        + LSHIFT(0, 5) // BST_CONFIG   | Boost mode enable. 0 – boost mode disable (default)
        + LSHIFT(1, 4) // CHG_CONFIG   | 1 – Charge Enable
        + LSHIFT(1, 3) // SYS_MIN[2]   | Minimum System Voltage Limit 111 – 3.7 V
        + LSHIFT(1, 2) // SYS_MIN[1]   |
        + LSHIFT(1, 1) // SYS_MIN[0]   |
        + LSHIFT(1, 0) // MIN_VBAT_SEL | Minimum battery voltage when exiting boost mode. 0 – 2.8 V BAT falling, 3 V rising (default)
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_0,reg));
  return RES_OK;
EXIT_ON_ERROR:
  return RES_ERROR;

}

/*-----------------------------------------------------------------------------------------------------
  Принудительно выключаем зарядку работу зарядника аккумулятора

  \param addr
  \param val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  BQ25619_disable_charger(void)
{
  uint8_t reg;

  // Запрещаем зарядку
  reg = 0
        + LSHIFT(0, 7) // PFM_DIS      | PFM disable in both buck and boost mode. 0 – PFM enable (default). 1 – PFM disable
        + LSHIFT(0, 6) // WD_RST       | I2C Watchdog Timer Reset. Back to 0 after watchdog timer reset. 1 – Reset
        + LSHIFT(0, 5) // BST_CONFIG   | Boost mode enable. 0 – boost mode disable (default)
        + LSHIFT(0, 4) // CHG_CONFIG   | 0 – Charge Disable
        + LSHIFT(1, 3) // SYS_MIN[2]   | Minimum System Voltage Limit 111 – 3.7 V
        + LSHIFT(1, 2) // SYS_MIN[1]   |
        + LSHIFT(1, 1) // SYS_MIN[0]   |
        + LSHIFT(1, 0) // MIN_VBAT_SEL | Minimum battery voltage when exiting boost mode. 0 – 2.8 V BAT falling, 3 V rising (default)
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_0,reg));

  // Если просто запретить зарядку, то через некоторое время она снова включается, поэтому надо отключить акумулятор
  // Отключаем аккумулятор
  reg = 0
        + LSHIFT(0, 7) // IINDET_EN           | 0 – Not in input current limit detection. (default)
        + LSHIFT(0, 6) // TMR2X_EN            | 0 – Disable. Safety timer duration is set by REG05[2].
        + LSHIFT(1, 5) // BATFET_DIS          | 1 – Turn off Q4 after tBATFET_DLY delay time (REG07[3])
        + LSHIFT(0, 4) // BATFET_RST_WVBUS    | 0 – Start BATFET full system reset after adapter is removed from VBUS. (default)
        + LSHIFT(1, 3) // BATFET_DLY          | 0 – Turn off BATFET immediately when BATFET_DIS bit is set.
        + LSHIFT(1, 2) // BATFET_RST_EN       | 1 – Enable BATFET reset function when REG07[5] is also 1. (default)
        + LSHIFT(0, 1) // VINDPM_BAT_TRACK[1] | 00 – Disable function (VINDPM set by register) (default)
        + LSHIFT(0, 0) // VINDPM_BAT_TRACK[0] |
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_3,reg));


  return RES_OK;
EXIT_ON_ERROR:
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Отключаем аккумулятор таким образом чтобы низкий уровень сигнала QON мог его снова подключить

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t BQ25619_switch_off_accum(void)
{
  uint8_t reg;

  reg = 0
        + LSHIFT(0, 7) // IINDET_EN           | 0 – Not in input current limit detection. (default)
        + LSHIFT(0, 6) // TMR2X_EN            | 0 – Disable. Safety timer duration is set by REG05[2].
        + LSHIFT(0, 5) // BATFET_DIS          | 1 – Turn off Q4 after tBATFET_DLY delay time (REG07[3])
        + LSHIFT(1, 4) // BATFET_RST_WVBUS    | 0 – Start BATFET full system reset after adapter is removed from VBUS. (default)
        + LSHIFT(0, 3) // BATFET_DLY          | 0 – Turn off BATFET immediately when BATFET_DIS bit is set.
        + LSHIFT(0, 2) // BATFET_RST_EN       | 1 – Enable BATFET reset function when REG07[5] is also 1. (default)
        + LSHIFT(0, 1) // VINDPM_BAT_TRACK[1] | 00 – Disable function (VINDPM set by register) (default)
        + LSHIFT(0, 0) // VINDPM_BAT_TRACK[0] |
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_3,reg));

  // Отключаем аккумулятор
  reg = 0
        + LSHIFT(0, 7) // IINDET_EN           | 0 – Not in input current limit detection. (default)
        + LSHIFT(0, 6) // TMR2X_EN            | 0 – Disable. Safety timer duration is set by REG05[2].
        + LSHIFT(1, 5) // BATFET_DIS          | 1 – Turn off Q4 after tBATFET_DLY delay time (REG07[3])
        + LSHIFT(1, 4) // BATFET_RST_WVBUS    | 0 – Start BATFET full system reset after adapter is removed from VBUS. (default)
        + LSHIFT(0, 3) // BATFET_DLY          | 0 – Turn off BATFET immediately when BATFET_DIS bit is set.
        + LSHIFT(0, 2) // BATFET_RST_EN       | 1 – Enable BATFET reset function when REG07[5] is also 1. (default)
        + LSHIFT(0, 1) // VINDPM_BAT_TRACK[1] | 00 – Disable function (VINDPM set by register) (default)
        + LSHIFT(0, 0) // VINDPM_BAT_TRACK[0] |
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_3,reg));

    return RES_OK;
EXIT_ON_ERROR:
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Отключаем внешний источник и аккумулятор от шины SYS и остаемся в этом состоянии

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t BQ25619_switch_off_vbus(void)
{
  uint8_t reg;

  // Отключаем аккумулятор
  reg = 0
        + LSHIFT(0, 7) // IINDET_EN           | 0 – Not in input current limit detection. (default)
        + LSHIFT(0, 6) // TMR2X_EN            | 0 – Disable. Safety timer duration is set by REG05[2].
        + LSHIFT(1, 5) // BATFET_DIS          | 1 – Turn off Q4 after tBATFET_DLY delay time (REG07[3])
        + LSHIFT(0, 4) // BATFET_RST_WVBUS    | 0 – Start BATFET full system reset after adapter is removed from VBUS. (default)
        + LSHIFT(0, 3) // BATFET_DLY          | 0 – Turn off BATFET immediately when BATFET_DIS bit is set.
        + LSHIFT(1, 2) // BATFET_RST_EN       | 1 – Enable BATFET reset function when REG07[5] is also 1. (default)
        + LSHIFT(0, 1) // VINDPM_BAT_TRACK[1] | 00 – Disable function (VINDPM set by register) (default)
        + LSHIFT(0, 0) // VINDPM_BAT_TRACK[0] |
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_CHARGER_CONTROL_3,reg));

  // Отключаем адаптер
  reg = 0
        + LSHIFT(1, 7) // EN_HIZ     | 1 – Enable.  Отключение внешнего источника питания от чипа.
        + LSHIFT(0, 6) // TS_IGNORE  | 1 – Ignore TS pin. Always consider TS is good to allow charging and boost mode.
        + LSHIFT(0, 5) // BATSNS_DIS | Select either BATSNS pin or BAT pin to regulate battery voltage. 0 – Enable BATSNS in battery CV regulation.
        + LSHIFT(0, 4) // IINDPM[4]  | Input current limit setting (maximum limit, not typical)
        + LSHIFT(0, 3) // IINDPM[3]  | 100 mA (000000)
        + LSHIFT(0, 2) // IINDPM[2]  |
        + LSHIFT(0, 1) // IINDPM[1]  |
        + LSHIFT(0, 0) // IINDPM[0]  |
  ;
  BQ25619_VERIFY_RESULT(BQ25619_write_reg(BQ25619_INPUT_CURRENT_LIMIT,reg));



  return RES_OK;
EXIT_ON_ERROR:
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return float
-----------------------------------------------------------------------------------------------------*/
float Get_BQ25619_ilim_val(uint8_t b)
{
  return  (100.0f + 1600 * ((b >> 4) & 1) + 800 * ((b >> 3) & 1) + 400 * ((b >> 2) & 1) + 200 * ((b >> 1) & 1) + 100 * (b & 1));
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return float
-----------------------------------------------------------------------------------------------------*/
float Get_BQ25619_sysv_val(uint8_t b)
{
  static const float sys_voltages[] = {2.6f, 2.8f, 3.0f, 3.2f, 3.4f, 3.5f, 3.6f, 3.7f};
  return  sys_voltages[(b >> 1) & 7];
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return float
-----------------------------------------------------------------------------------------------------*/
float Get_BQ25619_charge_curr_lim_val(uint8_t b)
{
  return  (640.0f * ((b >> 5) & 1) + 320.0f * ((b >> 4) & 1) + 160.0f * ((b >> 3) & 1) + 80.0f * ((b >> 2) & 1) + 40.0f * ((b >> 1) & 1) + 20.0f * (b & 1));

}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return float
-----------------------------------------------------------------------------------------------------*/
float Get_BQ25619_precharge_curr_lim_val(uint8_t b)
{
  float f = 20.0f + 160.0f * ((b >> 7) & 1) + 80.0f * ((b >> 6) & 1) + 40.0f * ((b >> 5) & 1) + 20.0f * ((b >> 4) & 1);
  if (f > 260.0f) f = 260.0f;
  return f;
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return float
-----------------------------------------------------------------------------------------------------*/
float Get_BQ25619_charge_term_curr_lim_val(uint8_t b)
{
  float f =  20.0f + 160.0f * ((b >> 3) & 1) + 80.0f * ((b >> 2) & 1) + 40.0f * ((b >> 1) & 1) + 20.0f * ((b >> 0) & 1);
  if (f > 260.0f) f = 260.0f;
  return f;
}


/*-----------------------------------------------------------------------------------------------------


  \param b

  \return float
-----------------------------------------------------------------------------------------------------*/
float Get_BQ25619_batt_volt_lim_val(uint8_t b)
{
  static const float batt_voltages[] = {3.504f, 3.60f, 3.696f, 3.80f, 3.904f, 4.000f, 4.10f, 4.15f, 4.20f};
  float f = 0.0f;

  b = (b >> 3) & 0x1F;

  if (b < 9)
  {
    f = batt_voltages[b];
  }
  else
  {
    f = (b - 9)*0.01f + 4.30f;
  }

  return f;
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return float
-----------------------------------------------------------------------------------------------------*/
float Get_BQ25619_VINDPM_val(uint8_t b)
{
  float f =  3.9f + 0.8f * ((b >> 3) & 1) + 0.4f * ((b >> 2) & 1) + 0.2f * ((b >> 1) & 1) + 0.1f * ((b >> 0) & 1);
  return f;
}


/*-----------------------------------------------------------------------------------------------------


  \param src

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_VBUS_status_str(uint8_t b)
{
  uint8_t v = (b >> 5) & 0x07;
  switch (v)
  {
  case BQ25619_VBUS_NO_INPUT    :
    return "No input";
  case BQ25619_VBUS_USB :
    return "USB 500 mA";
  case BQ25619_VBUS_ADAPTER  :
    return "ADAPTER 2.4 A";
  case BQ25619_VBUS_BOOST      :
    return "Boost mode";
  }
  return "UNDEF   ";
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_charging_status_str(uint8_t b)
{
  uint8_t v = (b >> 3) & 0x03;
  switch (v)
  {
  case 0:
    return "Not charging";
  case 1:
    return "Precharge or trickle charge";
  case 2:
    return "Fast charging";
  case 3:
    return "Charge termination";
  }
  return "UNDEF   ";
}


/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_NTC_monitoring_str(uint8_t b)
{
  if (b & BIT(6))
  {
    return "NTC ignored";
  }
  else
  {
    return "NTC monitored";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_HiZ_state_str(uint8_t b)
{
  if (b & BIT(7))
  {
    return "HiZ mode enabled";
  }
  else
  {
    return "HiZ mode disabled";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_PFM_state_str(uint8_t b)
{
  if (b & BIT(7))
  {
    return "PFM disabled";
  }
  else
  {
    return "PFM enabled";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_WDT_state_str(uint8_t b)
{
  if (b & BIT(6))
  {
    return "I2C WDT reset";
  }
  else
  {
    return "I2C WDT normal";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_boost_state_str(uint8_t b)
{
  if (b & BIT(5))
  {
    return "Boost enabled";
  }
  else
  {
    return "Boost disabled";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_charge_state_str(uint8_t b)
{
  if (b & BIT(4))
  {
    return "Charging enabled";
  }
  else
  {
    return "Charging disabled";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_pgood_state_str(uint8_t b)
{
  if (b & BIT(2))
  {
    return "Power good";
  }
  else
  {
    return "Power not good";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_therm_state_str(uint8_t b)
{
  if (b & BIT(1))
  {
    return "In thermal reg.";
  }
  else
  {
    return "Not in thermal reg.";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_vsys_state_str(uint8_t b)
{
  if (b & BIT(0))
  {
    return "In SYS_MIN reg.";
  }
  else
  {
    return "Not in SYS_MIN reg.";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_wdt_fault_str(uint8_t b)
{
  if (b & BIT(7))
  {
    return "WDT fault";
  }
  else
  {
    return "WDT not fault";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_boost_fault_str(uint8_t b)
{
  if (b & BIT(6))
  {
    return "Boost fault";
  }
  else
  {
    return "Boost normal";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_charge_fault_str(uint8_t b)
{
  b = (b >> 4) & 3;
  switch (b)
  {
  case 0:
    return "Charge normal";
  case 1:
    return "Charger input fault";
  case 2:
    return "Charger thermal shutdown";
  case 3:
    return "Charge safety timer expiration";
  }
  return "-";
}


/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_batt_fault_str(uint8_t b)
{
  if (b & BIT(3))
  {
    return "Batt. overvoltage";
  }
  else
  {
    return "Batt. v. normal";
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_NTC_fault_str(uint8_t b)
{
  b = b & 7;
  switch (b)
  {
  case 0:
    return "NTC normal";
  case 2:
    return "NTC warm";
  case 3:
    return "NTC cool";
  case 5:
    return "NTC cold";
  case 6:
    return "NTC hot";
  }
  return "NTC ?";
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_VBUS_GD_str(uint8_t b)
{
  if (b & BIT(7))
  {
    return "VBUS_GD=1";
  }
  else
  {
    return "VBUS_GD=0";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_VINDPM_STAT_str(uint8_t b)
{
  if (b & BIT(6))
  {
    return "In VINDPM";
  }
  else
  {
    return "No VINDPM";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_IINDPM_STAT_str(uint8_t b)
{
  if (b & BIT(5))
  {
    return "In IINDPM";
  }
  else
  {
    return "No IINDPM";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_BATSNS_STAT_str(uint8_t b)
{
  if (b & BIT(4))
  {
    return "BATSNS wrong";
  }
  else
  {
    return "BATSNS good";
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_TOPOFF_ACTIVE_str(uint8_t b)
{
  if (b & BIT(3))
  {
    return "Tim. count.";
  }
  else
  {
    return "Tim. not count.";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_ACOV_STAT_str(uint8_t b)
{
  if (b & BIT(2))
  {
    return "In ACOV";
  }
  else
  {
    return "No ACOV";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_VINDPM_INT_MASK_str(uint8_t b)
{
  if (b & BIT(1))
  {
    return "No INT on VINDPM";
  }
  else
  {
    return "INT on VINDPM";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_IINDPM_INT_MASK_str(uint8_t b)
{
  if (b & BIT(1))
  {
    return "No INT on IINDPM";
  }
  else
  {
    return "INT on IINDPM";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_Q1_FULLON_str(uint8_t b)
{
  if (b & BIT(6))
  {
    return "Fully turn on Q1";
  }
  else
  {
    return "Partially turn on Q1";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param src

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_TOPOFF_TIMER_str(uint8_t b)
{
  uint8_t v = (b >> 1) & 0x03;
  switch (v)
  {
  case 0    :
    return "Top-off time disabled";
  case 1 :
    return "Top-off time 15 min";
  case 2  :
    return "Top-off time 30 min";
  case 3      :
    return "Top-off time 45 min";
  }
  return "-";
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_VRECHG_str(uint8_t b)
{
  if (b & BIT(0))
  {
    return "Battery recharge threshold = 210 mV";
  }
  else
  {
    return "Battery recharge threshold = 120 mV";
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_EN_TERM_str(uint8_t b)
{
  if (b & BIT(7))
  {
    return "Bat.charg. term. en.";
  }
  else
  {
    return "Bat.charg. termi. dis.";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_WATCHDOG_str(uint8_t b)
{
  uint8_t v = (b >> 4) & 0x03;
  switch (v)
  {
  case 0    :
    return "WDT timer disabled";
  case 1 :
    return "WDT timer 40s";
  case 2  :
    return "WDT timer 80s";
  case 3      :
    return "WDT timer 160s";
  }
  return "-";
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_EN_TIMER_str(uint8_t b)
{
  if (b & BIT(3))
  {
    return "Bat. charg. safety tmr en.";
  }
  else
  {
    return "Bat. charg. safety tmr dis.";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_CHG_TIMER_str(uint8_t b)
{
  if (b & BIT(2))
  {
    return "Safety tmr=20 hrs";
  }
  else
  {
    return "Safety tmr=10 hrs";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_TREG_str(uint8_t b)
{
  if (b & BIT(1))
  {
    return "T.reg. = 110 C";
  }
  else
  {
    return "T.reg. = 90 C";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_JEITA_VSET_str(uint8_t b)
{
  if (b & BIT(0))
  {
    return "JEITA_VSET=1";
  }
  else
  {
    return "JEITA_VSET=0";
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_OVP_str(uint8_t b)
{
  uint8_t v = (b >> 6) & 0x03;
  switch (v)
  {
  case 0    :
    return "Overvoltage threshold=5.85 V";
  case 1 :
    return "Overvoltage threshold=6.4 V";
  case 2  :
    return "Overvoltage threshold=11 V";
  case 3      :
    return "Overvoltage threshold=14.2 V";
  }
  return "-";
}
/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_BOOSTV_str(uint8_t b)
{
  uint8_t v = (b >> 4) & 0x03;
  switch (v)
  {
  case 0    :
    return "Boost voltage=4.6 V";
  case 1 :
    return "Boost voltage=4.75 V";
  case 2  :
    return "Boost voltage=5.0 V";
  case 3      :
    return "Boost voltage=5.15 V";
  }
  return "-";
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_IINDET_EN_str(uint8_t b)
{
  if (b & BIT(7))
  {
    return "IINDET_EN=1";
  }
  else
  {
    return "IINDET_EN=0";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_TMR2X_EN_str(uint8_t b)
{
  if (b & BIT(6))
  {
    return "TMR2X_EN=1";
  }
  else
  {
    return "TMR2X_EN=0";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_BATFET_DIS_str(uint8_t b)
{
  if (b & BIT(5))
  {
    return "BATFET_DIS=1";
  }
  else
  {
    return "BATFET_DIS=0";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_BATFET_RST_WVBUS_str(uint8_t b)
{
  if (b & BIT(4))
  {
    return "BATFET_RST_WVBUS=1";
  }
  else
  {
    return "BATFET_RST_WVBUS=0";
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_BATFET_DLY_str(uint8_t b)
{
  if (b & BIT(3))
  {
    return "BATFET_DLY=1";
  }
  else
  {
    return "BATFET_DLY=0";
  }
}
/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_BATFET_RST_EN_str(uint8_t b)
{
  if (b & BIT(2))
  {
    return "BATFET_RST_EN=1";
  }
  else
  {
    return "BATFET_RST_EN=0";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_VINDPM_BAT_TRACK_str(uint8_t b)
{
  uint8_t v = b & 0x03;
  switch (v)
  {
  case 0    :
    return "VINDPM set by register";
  case 1 :
    return "VINDPM=Vbat+200mV";
  case 2  :
    return "VINDPM=Vbat+250mV";
  case 3      :
    return "VINDPM=Vbat+300mV";
  }
  return "-";
}
