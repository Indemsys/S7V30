#ifndef S7V30_ADS7028_H
  #define S7V30_ADS7028_H


  #define   EXT_REF_VOLTAGE                 (3.3f)                            // Уровень напряжения откносительно которого ведутся измерения АЦП
  #define   EXT_ADC_RANGE                   (4096.0f)
  #define   EXT_ADC_SCALE                   (EXT_REF_VOLTAGE/EXT_ADC_RANGE)  // Коэффициент пересчета напряжения на входе ADC в величину отсчета на выходе ADC


  #define  ADS728_SPEED                      15000000ul // На скорости 30 МГц у АЦП не проходит калибровка.



  #define  ADS7028_SYSTEM_STATUS             0x0
  #define  ADS7028_GENERAL_CFG               0x1
  #define  ADS7028_DATA_CFG                  0x2
  #define  ADS7028_OSR_CFG                   0x3
  #define  ADS7028_OPMODE_CFG                0x4
  #define  ADS7028_PIN_CFG                   0x5
  #define  ADS7028_GPIO_CFG                  0x7
  #define  ADS7028_GPO_DRIVE_CFG             0x9
  #define  ADS7028_GPO_VALUE                 0xB
  #define  ADS7028_GPI_VALUE                 0xD
  #define  ADS7028_ZCD_BLANKING_CFG          0xF
  #define  ADS7028_SEQUENCE_CFG              0x10
  #define  ADS7028_CHANNEL_SEL               0x11
  #define  ADS7028_AUTO_SEQ_CH_SEL           0x12
  #define  ADS7028_ALERT_CH_SEL              0x14
  #define  ADS7028_ALERT_MAP                 0x16
  #define  ADS7028_ALERT_PIN_CFG             0x17
  #define  ADS7028_EVENT_FLAG                0x18
  #define  ADS7028_EVENT_HIGH_FLAG           0x1A
  #define  ADS7028_EVENT_LOW_FLAG            0x1C
  #define  ADS7028_EVENT_RGN                 0x1E
  #define  ADS7028_HYSTERESIS_CH0            0x20
  #define  ADS7028_HIGH_TH_CH0               0x21
  #define  ADS7028_EVENT_COUNT_CH0           0x22
  #define  ADS7028_LOW_TH_CH0                0x23
  #define  ADS7028_HYSTERESIS_CH1            0x24
  #define  ADS7028_HIGH_TH_CH1               0x25
  #define  ADS7028_EVENT_COUNT_CH1           0x26
  #define  ADS7028_LOW_TH_CH1                0x27
  #define  ADS7028_HYSTERESIS_CH2            0x28
  #define  ADS7028_HIGH_TH_CH2               0x29
  #define  ADS7028_EVENT_COUNT_CH2           0x2A
  #define  ADS7028_LOW_TH_CH2                0x2B
  #define  ADS7028_HYSTERESIS_CH3            0x2C
  #define  ADS7028_HIGH_TH_CH3               0x2D
  #define  ADS7028_EVENT_COUNT_CH3           0x2E
  #define  ADS7028_LOW_TH_CH3                0x2F
  #define  ADS7028_HYSTERESIS_CH4            0x30
  #define  ADS7028_HIGH_TH_CH4               0x31
  #define  ADS7028_EVENT_COUNT_CH4           0x32
  #define  ADS7028_LOW_TH_CH4                0x33
  #define  ADS7028_HYSTERESIS_CH5            0x34
  #define  ADS7028_HIGH_TH_CH5               0x35
  #define  ADS7028_EVENT_COUNT_CH5           0x36
  #define  ADS7028_LOW_TH_CH5                0x37
  #define  ADS7028_HYSTERESIS_CH6            0x38
  #define  ADS7028_HIGH_TH_CH6               0x39
  #define  ADS7028_EVENT_COUNT_CH6           0x3A
  #define  ADS7028_LOW_TH_CH6                0x3B
  #define  ADS7028_HYSTERESIS_CH7            0x3C
  #define  ADS7028_HIGH_TH_CH7               0x3D
  #define  ADS7028_EVENT_COUNT_CH7           0x3E
  #define  ADS7028_LOW_TH_CH7                0x3F
  #define  ADS7028_MAX_CH0_LSB               0x60
  #define  ADS7028_MAX_CH0_MSB               0x61
  #define  ADS7028_MAX_CH1_LSB               0x62
  #define  ADS7028_MAX_CH1_MSB               0x63
  #define  ADS7028_MAX_CH2_LSB               0x64
  #define  ADS7028_MAX_CH2_MSB               0x65
  #define  ADS7028_MAX_CH3_LSB               0x66
  #define  ADS7028_MAX_CH3_MSB               0x67
  #define  ADS7028_MAX_CH4_LSB               0x68
  #define  ADS7028_MAX_CH4_MSB               0x69
  #define  ADS7028_MAX_CH5_LSB               0x6A
  #define  ADS7028_MAX_CH5_MSB               0x6B
  #define  ADS7028_MAX_CH6_LSB               0x6C
  #define  ADS7028_MAX_CH6_MSB               0x6D
  #define  ADS7028_MAX_CH7_LSB               0x6E
  #define  ADS7028_MAX_CH7_MSB               0x6F
  #define  ADS7028_MIN_CH0_LSB               0x80
  #define  ADS7028_MIN_CH0_MSB               0x81
  #define  ADS7028_MIN_CH1_LSB               0x82
  #define  ADS7028_MIN_CH1_MSB               0x83
  #define  ADS7028_MIN_CH2_LSB               0x84
  #define  ADS7028_MIN_CH2_MSB               0x85
  #define  ADS7028_MIN_CH3_LSB               0x86
  #define  ADS7028_MIN_CH3_MSB               0x87
  #define  ADS7028_MIN_CH4_LSB               0x88
  #define  ADS7028_MIN_CH4_MSB               0x89
  #define  ADS7028_MIN_CH5_LSB               0x8A
  #define  ADS7028_MIN_CH5_MSB               0x8B
  #define  ADS7028_MIN_CH6_LSB               0x8C
  #define  ADS7028_MIN_CH6_MSB               0x8D
  #define  ADS7028_MIN_CH7_LSB               0x8E
  #define  ADS7028_MIN_CH7_MSB               0x8F
  #define  ADS7028_RECENT_CH0_LSB            0xA0
  #define  ADS7028_RECENT_CH0_MSB            0xA1
  #define  ADS7028_RECENT_CH1_LSB            0xA2
  #define  ADS7028_RECENT_CH1_MSB            0xA3
  #define  ADS7028_RECENT_CH2_LSB            0xA4
  #define  ADS7028_RECENT_CH2_MSB            0xA5
  #define  ADS7028_RECENT_CH3_LSB            0xA6
  #define  ADS7028_RECENT_CH3_MSB            0xA7
  #define  ADS7028_RECENT_CH4_LSB            0xA8
  #define  ADS7028_RECENT_CH4_MSB            0xA9
  #define  ADS7028_RECENT_CH5_LSB            0xAA
  #define  ADS7028_RECENT_CH5_MSB            0xAB
  #define  ADS7028_RECENT_CH6_LSB            0xAC
  #define  ADS7028_RECENT_CH6_MSB            0xAD
  #define  ADS7028_RECENT_CH7_LSB            0xAE
  #define  ADS7028_RECENT_CH7_MSB            0xAF
  #define  ADS7028_RMS_CFG                   0xC0
  #define  ADS7028_RMS_LSB                   0xC1
  #define  ADS7028_RMS_MSB                   0xC2
  #define  ADS7028_GPO0_TRIG_EVENT_SEL       0xC3
  #define  ADS7028_GPO1_TRIG_EVENT_SEL       0xC5
  #define  ADS7028_GPO2_TRIG_EVENT_SEL       0xC7
  #define  ADS7028_GPO3_TRIG_EVENT_SEL       0xC9
  #define  ADS7028_GPO4_TRIG_EVENT_SEL       0xCB
  #define  ADS7028_GPO5_TRIG_EVENT_SEL       0xCD
  #define  ADS7028_GPO6_TRIG_EVENT_SEL       0xCF
  #define  ADS7028_GPO7_TRIG_EVENT_SEL       0xD1
  #define  ADS7028_GPO_VALUE_ZCD_CFG_CH0_CH3 0xE3
  #define  ADS7028_GPO_VALUE_ZCD_CFG_CH4_CH7 0xE4
  #define  ADS7028_GPO_ZCD_UPDATE_EN         0xE7
  #define  ADS7028_GPO_TRIGGER_CFG           0xE9
  #define  ADS7028_GPO_VALUE_TRIG            0xEB


  #define    SET_ADS7028_MODE   {SCI8_SPI_set_mode(ADS728_SPEED, 0 , 1);}

typedef struct
{
    uint16_t      reg_addr;
    char  const  *reg_name;

} T_ADS7028_reg_descr;



uint32_t    ADS7028_open(void);
uint32_t    ADS7028_close(void);


uint32_t    ADS7028_read(uint8_t n, uint8_t addr, uint8_t *val);
uint32_t    ADS7028_write(uint8_t n, uint8_t addr, uint8_t val);
uint32_t    ADS7028_set_channel_read_result(uint8_t n, uint8_t addr, uint8_t channel_sel, uint16_t *pval);
uint32_t    ADS7028_read_autoscan_result(uint8_t n, uint16_t *pval);

#endif




