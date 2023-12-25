#ifndef S7V30_MAX17262_H
  #define S7V30_MAX17262_H


  #define    MAX17262_CHIP_ID              0x4039

  #define    MAX17262_OPEN_TIMEOUT         100

  #define    MAX17262_STATUS                0
  #define    MAX17262_VALRTTH               1
  #define    MAX17262_TALRTTH               2
  #define    MAX17262_SALRTTH               3
  #define    MAX17262_ATRATE                4
  #define    MAX17262_REPCAP                5
  #define    MAX17262_REPSOC                6
  #define    MAX17262_AGE                   7
  #define    MAX17262_TEMP                  8
  #define    MAX17262_VCELL                 9
  #define    MAX17262_CURRENT               10
  #define    MAX17262_AVGCURRENT            11
  #define    MAX17262_QRESIDUAL             12
  #define    MAX17262_MIXSOC                13
  #define    MAX17262_AVSOC                 14
  #define    MAX17262_MIXCAP                15
  #define    MAX17262_FULLCAPREP            16
  #define    MAX17262_TTE                   17
  #define    MAX17262_QRTABLE00             18
  #define    MAX17262_FULLSOCTHR            19
  #define    MAX17262_RCELL                 20
  #define    MAX17262_AVGTA                 22
  #define    MAX17262_CYCLES                23
  #define    MAX17262_DESIGNCAP             24
  #define    MAX17262_AVGVCELL              25
  #define    MAX17262_MAXMINTEMP            26
  #define    MAX17262_MAXMINVOLT            27
  #define    MAX17262_MAXMINCURR            28
  #define    MAX17262_CONFIG                29
  #define    MAX17262_ICHGTERM              30
  #define    MAX17262_AVCAP                 31
  #define    MAX17262_TTF                   32
  #define    MAX17262_DEVNAME               33
  #define    MAX17262_QRTABLE10             34
  #define    MAX17262_FULLCAPNOM            35
  #define    MAX17262_AIN                   39
  #define    MAX17262_LEARNCFG              40
  #define    MAX17262_FILTERCFG             41
  #define    MAX17262_RELAXCFG              42
  #define    MAX17262_MISCCFG               43
  #define    MAX17262_TGAIN                 44
  #define    MAX17262_TOFF                  45
  #define    MAX17262_CGAIN                 46
  #define    MAX17262_COFF                  47
  #define    MAX17262_QRTABLE20             50
  #define    MAX17262_DIETEMP               52
  #define    MAX17262_FULLCAP               53
  #define    MAX17262_RCOMP0                56
  #define    MAX17262_TEMPCO                57
  #define    MAX17262_VEMPTY                58
  #define    MAX17262_FSTAT                 61
  #define    MAX17262_TIMER                 62
  #define    MAX17262_SHDNTIMER             63
  #define    MAX17262_QRTABLE30             66
  #define    MAX17262_RGAIN                 67
  #define    MAX17262_DQACC                 69
  #define    MAX17262_DPACC                 70
  #define    MAX17262_CONVGCFG              73
  #define    MAX17262_VFREMCAP              74
  #define    MAX17262_QH                    77
  #define    MAX17262_STATUS2               176
  #define    MAX17262_POWER                 177
  #define    MAX17262_ID_USERMEM2           178
  #define    MAX17262_AVGPOWER              179
  #define    MAX17262_IALRTTH               180
  #define    MAX17262_TTFCFG                181
  #define    MAX17262_CVMIXCAP              182
  #define    MAX17262_CVHALFTIME            183
  #define    MAX17262_CGTEMPCO              184
  #define    MAX17262_CURVE                 185
  #define    MAX17262_HIBCFG                186
  #define    MAX17262_CONFIG2               187
  #define    MAX17262_VRIPPLE               188
  #define    MAX17262_RIPPLECFG             189
  #define    MAX17262_TIMERH                190
  #define    MAX17262_RSENSE_USERMEM3       208
  #define    MAX17262_SCOCVLIM              209
  #define    MAX17262_VGAIN                 210
  #define    MAX17262_SOCHOLD               211
  #define    MAX17262_MAXPEAKPOWER          212
  #define    MAX17262_SUSPEAKPOWER          213
  #define    MAX17262_PACKRESISTANCE        214
  #define    MAX17262_SYSRESISTANCE         215
  #define    MAX17262_MINSYSVOLTAGE         216
  #define    MAX17262_MPPCURRENT            217
  #define    MAX17262_SPPCURRENT            218
  #define    MAX17262_MODELCFG              219
  #define    MAX17262_ATQRESIDUAL           220
  #define    MAX17262_ATTTE                 221
  #define    MAX17262_ATAVSOC               222
  #define    MAX17262_ATAVCAP               223

typedef struct
{
    uint16_t      reg_addr;
    char  const  *reg_name;

} T_MAX17262_reg_descr;


uint32_t                     MAX17262_open(uint32_t timeout);
uint32_t                     MAX17262_close(void);
uint32_t                     MAX17262_reg_count(void);
T_MAX17262_reg_descr const*  MAX17262_get_reg_descr(uint8_t indx);

ssp_err_t                    MAX17262_write_reg(uint8_t addr, uint16_t val);
ssp_err_t                    MAX17262_read_reg(uint8_t addr, uint16_t *val);

uint32_t                     MAX17262_read_reg_transaction(uint8_t addr, uint16_t *val);
uint32_t                     MAX17262_write_reg_transaction(uint8_t addr, uint16_t val);

#endif
































































































































































































































































































































