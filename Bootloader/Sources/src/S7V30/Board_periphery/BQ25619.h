#ifndef SYNERGY_BQ25619_H
  #define SYNERGY_BQ25619_H


  #define  BQ25619_OPEN_TIMEOUT                              100

  #define  BQ25619_INPUT_CURRENT_LIMIT                       0x00
  #define  BQ25619_CHARGER_CONTROL_0                         0x01
  #define  BQ25619_CHARGE_CURRENT_LIMIT                      0x02
  #define  BQ25619_PRE_CHARGE_TERMINATION_CURRENT_LIMIT      0x03
  #define  BQ25619_BATTERY_VOLTAGE_LIMIT                     0x04
  #define  BQ25619_CHARGER_CONTROL_1                         0x05
  #define  BQ25619_CHARGER_CONTROL_2                         0x06
  #define  BQ25619_CHARGER_CONTROL_3                         0x07
  #define  BQ25619_SYSTEM_STATUS0                            0x08
  #define  BQ25619_SYSTEM_STATUS1                            0x09
  #define  BQ25619_SYSTEM_STATUS2                            0x0A
  #define  BQ25619_PART_INFORMATION                          0x0B
  #define  BQ25619_CHARGER_CONTROL4                          0x0C


  #define  BQ25619_PG_STAT              BIT(2)  // Power Good status: 0 – Power Not Good, 1 – Power Good

  #define  BQ25619_IN_LIMIT_100MA       0
  #define  BQ25619_IN_LIMIT_200MA       1
  #define  BQ25619_IN_LIMIT_300MA       2
  #define  BQ25619_IN_LIMIT_400MA       3
  #define  BQ25619_IN_LIMIT_500MA       4
  #define  BQ25619_IN_LIMIT_600MA       5
  #define  BQ25619_IN_LIMIT_700MA       6
  #define  BQ25619_IN_LIMIT_800MA       7
  #define  BQ25619_IN_LIMIT_900MA       8
  #define  BQ25619_IN_LIMIT_1000MA      9
  #define  BQ25619_IN_LIMIT_1100MA      10
  #define  BQ25619_IN_LIMIT_1200MA      11
  #define  BQ25619_IN_LIMIT_1300MA      12
  #define  BQ25619_IN_LIMIT_1400MA      13
  #define  BQ25619_IN_LIMIT_1500MA      14
  #define  BQ25619_IN_LIMIT_1600MA      15
  #define  BQ25619_IN_LIMIT_1700MA      16
  #define  BQ25619_IN_LIMIT_1800MA      17
  #define  BQ25619_IN_LIMIT_1900MA      18
  #define  BQ25619_IN_LIMIT_2000MA      19
  #define  BQ25619_IN_LIMIT_2100MA      20
  #define  BQ25619_IN_LIMIT_2200MA      21
  #define  BQ25619_IN_LIMIT_2300MA      22
  #define  BQ25619_IN_LIMIT_2400MA      23
  #define  BQ25619_IN_LIMIT_2500MA      24
  #define  BQ25619_IN_LIMIT_2600MA      25
  #define  BQ25619_IN_LIMIT_2700MA      26
  #define  BQ25619_IN_LIMIT_2800MA      27
  #define  BQ25619_IN_LIMIT_2900MA      28
  #define  BQ25619_IN_LIMIT_3000MA      29
  #define  BQ25619_IN_LIMIT_3100MA      30
  #define  BQ25619_IN_LIMIT_3200MA      31


  #define  BQ25619_VBUS_NO_INPUT        0
  #define  BQ25619_VBUS_USB             1
  #define  BQ25619_VBUS_ADAPTER         3
  #define  BQ25619_VBUS_BOOST           7

  #define  CONV_TO_CURR_LIMIT(X)       (((X & 0x1F) +1) * 100)
  #define  FAST_CHARGE_CURR_LIMIT(X)   ((X & 0x3F) *20)
  #define  CURR_TO_FAST_CHARGE_LIM(X)  ((X / 20) & 0x3F)


typedef struct
{
    uint8_t      reg_addr;
    char  const  *reg_name;
    uint8_t      *reg_val;

} T_bq25619_reg_descr;


uint32_t             BQ25619_reg_count(void);
T_bq25619_reg_descr const* BQ25619_get_reg_descr(uint8_t indx);

uint8_t     Is_BQ25619_opened(void);
ssp_err_t   BQ25619_open(void);
ssp_err_t   BQ25619_lock(uint32_t timeout);
ssp_err_t   BQ25619_unlock(void);
uint32_t    BQ25619_close(void);

ssp_err_t   BQ25619_write_buf(uint8_t addr, uint8_t *buf, uint32_t sz);
ssp_err_t   BQ25619_read_buf(uint8_t addr, uint8_t *buf, uint32_t sz);
ssp_err_t   BQ25619_write_reg(uint8_t addr, uint8_t val);
ssp_err_t   BQ25619_read_reg(uint8_t addr, uint8_t *val);
uint32_t    BQ25619_read_all(void);
uint8_t     BQ25619_get_system_status(void);


uint32_t    BQ25619_set_max_sys_voltage(void);
uint32_t    BQ25619_sw_off_accum(void);
uint32_t    BQ25619_disable_charger(void);
uint32_t    BQ25619_switch_off_accum(void);

uint32_t    BQ25619_get_charge_state(uint8_t status);
uint32_t    BQ25619_get_vbus_state(uint8_t status);
uint32_t    BQ25619_switch_off_vbus(void);

float       Get_BQ25619_ilim_val(uint8_t b);
float       Get_BQ25619_sysv_val(uint8_t b);
float       Get_BQ25619_charge_curr_lim_val(uint8_t b);
float       Get_BQ25619_precharge_curr_lim_val(uint8_t b);
float       Get_BQ25619_charge_term_curr_lim_val(uint8_t b);
float       Get_BQ25619_batt_volt_lim_val(uint8_t b);
float       Get_BQ25619_VINDPM_val(uint8_t b);

char const* Get_BQ25619_VBUS_status_str(uint8_t src);
char const* Get_BQ25619_charging_status_str(uint8_t b);
char const* Get_BQ25619_NTC_monitoring_str(uint8_t b);
char const* Get_BQ25619_HiZ_state_str(uint8_t b);
char const* Get_BQ25619_PFM_state_str(uint8_t b);
char const* Get_BQ25619_WDT_state_str(uint8_t b);
char const* Get_BQ25619_boost_state_str(uint8_t b);
char const* Get_BQ25619_charge_state_str(uint8_t b);
char const* Get_BQ25619_pgood_state_str(uint8_t b);
char const* Get_BQ25619_therm_state_str(uint8_t b);
char const* Get_BQ25619_vsys_state_str(uint8_t b);
char const* Get_BQ25619_wdt_fault_str(uint8_t b);
char const* Get_BQ25619_boost_fault_str(uint8_t b);
char const* Get_BQ25619_charge_fault_str(uint8_t b);
char const* Get_BQ25619_batt_fault_str(uint8_t b);
char const* Get_BQ25619_NTC_fault_str(uint8_t b);
char const* Get_BQ25619_VBUS_GD_str(uint8_t b);
char const* Get_BQ25619_VINDPM_STAT_str(uint8_t b);
char const* Get_BQ25619_IINDPM_STAT_str(uint8_t b);
char const* Get_BQ25619_BATSNS_STAT_str(uint8_t b);
char const* Get_BQ25619_TOPOFF_ACTIVE_str(uint8_t b);
char const* Get_BQ25619_ACOV_STAT_str(uint8_t b);
char const* Get_BQ25619_VINDPM_INT_MASK_str(uint8_t b);
char const* Get_BQ25619_IINDPM_INT_MASK_str(uint8_t b);
char const* Get_BQ25619_Q1_FULLON_str(uint8_t b);
char const* Get_BQ25619_TOPOFF_TIMER_str(uint8_t b);
char const* Get_BQ25619_VRECHG_str(uint8_t b);
char const* Get_BQ25619_EN_TERM_str(uint8_t b);
char const* Get_BQ25619_WATCHDOG_str(uint8_t b);
char const* Get_BQ25619_EN_TIMER_str(uint8_t b);
char const* Get_BQ25619_CHG_TIMER_str(uint8_t b);
char const* Get_BQ25619_TREG_str(uint8_t b);
char const* Get_BQ25619_JEITA_VSET_str(uint8_t b);
char const* Get_BQ25619_OVP_str(uint8_t b);
char const* Get_BQ25619_BOOSTV_str(uint8_t b);
char const* Get_BQ25619_IINDET_EN_str(uint8_t b);
char const* Get_BQ25619_TMR2X_EN_str(uint8_t b);
char const* Get_BQ25619_BATFET_DIS_str(uint8_t b);
char const* Get_BQ25619_BATFET_RST_WVBUS_str(uint8_t b);
char const* Get_BQ25619_BATFET_DLY_str(uint8_t b);
char const* Get_BQ25619_BATFET_RST_EN_str(uint8_t b);
char const* Get_BQ25619_VINDPM_BAT_TRACK_str(uint8_t b);

#endif



