#ifndef S7V30_AB1815_H
  #define S7V30_AB1815_H


#define    AB1815_SPEED                 1000000
#define    AB1815_OPEN_TIMEOUT          100


#define    AB1815_CHIP_ID               0x1815

#define    AB1815_HUNDREDTHS            0x00
#define    AB1815_SECONDS               0x01
#define    AB1815_MINUTES               0x02
#define    AB1815_HOURS                 0x03
#define    AB1815_DATE                  0x04
#define    AB1815_MONTHS                0x05
#define    AB1815_YEARS                 0x06
#define    AB1815_WEEKDAYS              0x07
#define    AB1815_HUNDREDTHS_ALARM      0x08
#define    AB1815_SECONDS_ALARM         0x09
#define    AB1815_MINUTES_ALARM         0x0A
#define    AB1815_HOURS_ALARM_24        0x0B
#define    AB1815_HOURS_ALARM_12        0x0B
#define    AB1815_DATE_ALARM            0x0C
#define    AB1815_MONTHS_ALARM          0x0D
#define    AB1815_WEEKDAYS_ALARM        0x0E
#define    AB1815_STATUS                0x0F
#define    AB1815_CONTROL1              0x10
#define    AB1815_CONTROL2              0x11
#define    AB1815_INTMASK               0x12
#define    AB1815_SQW                   0x13
#define    AB1815_CAL_XT                0x14
#define    AB1815_CAL_RC_               0x15
#define    AB1815_CAL_RC_LOW            0x16
#define    AB1815_SLEEP_CONTROL         0x17
#define    AB1815_TIMER_CONTROL         0x18
#define    AB1815_TIMER                 0x19
#define    AB1815_TIMER_INITIAL         0x1A
#define    AB1815_WDT                   0x1B
#define    AB1815_OSC_CONTROL           0x1C
#define    AB1815_OSC_STATUS            0x1D
#define    AB1815_RESERVED1             0x1E
#define    AB1815_CONFIGURATION_KEY     0x1F
#define    AB1815_TRICKLE               0x20
#define    AB1815_BREF_CONTROL          0x21
#define    AB1815_RESERVED2             0x22
#define    AB1815_RESERVED3             0x23
#define    AB1815_RESERVED4             0x24
#define    AB1815_RESERVED5             0x25
#define    AB1815_AFCTRL                0x26
#define    AB1815_BATMODE_I_O           0x27
#define    AB1815_ID0                   0x28
#define    AB1815_ID1                   0x29
#define    AB1815_ID2                   0x2A
#define    AB1815_ID3                   0x2B
#define    AB1815_ID4                   0x2C
#define    AB1815_ID5                   0x2D
#define    AB1815_ID6                   0x2E
#define    AB1815_ASTAT                 0x2F
#define    AB1815_OCTRL                 0x30
#define    AB1815_EXTENSION_ADDRESS     0x3F
#define    AB1815_RAM_NORMAL            0x40
#define    AB1815_RAM_ALTERNATE         0x80


#define    AB1815_NVRAM_SZ              64
#define    MAX_AB1815_BUF_SZ            AB1815_NVRAM_SZ


#define    NVRAM_CRC_OFFS               (AB1815_NVRAM_SZ-2)

#define    SET_AB1815_MODE   {SCI8_SPI_set_mode(AB1815_SPEED, 0 , 1);}



typedef __packed  struct
{
  uint8_t addr;
  uint8_t data[MAX_AB1815_BUF_SZ];

} T_AB1815_buf;


typedef struct
{
  uint16_t      reg_addr;
  char  const  *reg_name;

} T_AB1815_reg_descr;

typedef struct
{
  uint16_t        chip_id;
  uint32_t        rtc_lotn;
  uint32_t        rtc_idn;
  uint8_t         status;
  uint8_t         osc_status;
  uint32_t        err_code;
  uint8_t         alarm_flag;
  uint8_t         data_crc_fault;

} T_AB1815_init_res;

uint32_t                  AB1815_get_registers_count(void);
const T_AB1815_reg_descr* AB1815_get_register_descr(uint32_t indx);
T_AB1815_init_res*        AB1815_get_init_res(void);
uint32_t                  AB1815_init(void);
uint32_t                  AB1815_reg_count(void);
T_AB1815_reg_descr const* AB1815_get_reg_descr(uint8_t indx);
char  const*              AB1815_get_register_name_by_addr(uint8_t addr);

uint32_t                  AB1815_read_date_time( rtc_time_t  *p_time);
uint32_t                  AB1815_write_date_time(rtc_time_t  *p_time);
uint32_t                  AB1815_write_alarm_date_time(rtc_time_t  *p_time);
uint32_t                  AB1815_restore_data_itegrity(void);
uint32_t                  AB1815_read_alarm_flag(uint8_t *p_flag);
uint32_t                  AB1815_read_ID(uint16_t *p_id);
uint32_t                  AB1815_read_RAM(T_AB1815_buf **p_ab1815_buf);
uint32_t                  AB1815_read_register(uint16_t reg_addr, uint8_t *p_reg);
uint32_t                  AB1815_write_register(uint16_t reg_addr, uint8_t reg);


#endif



