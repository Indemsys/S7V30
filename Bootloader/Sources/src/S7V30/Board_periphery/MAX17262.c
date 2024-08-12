// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020.04.30
// 10:09:05
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

static uint32_t        MAX17262_opened = 0;

static uint16_t  rv[91];

const T_MAX17262_reg_descr MAX17262_regs[]=
{
  {  MAX17262_STATUS             ,"STATUS         " ,&rv[ 0] },
  {  MAX17262_VALRTTH            ,"VALRTTH        " ,&rv[ 1] },
  {  MAX17262_TALRTTH            ,"TALRTTH        " ,&rv[ 2] },
  {  MAX17262_SALRTTH            ,"SALRTTH        " ,&rv[ 3] },
  {  MAX17262_ATRATE             ,"ATRATE         " ,&rv[ 4] },
  {  MAX17262_REPCAP             ,"REPCAP         " ,&rv[ 5] },
  {  MAX17262_REPSOC             ,"REPSOC         " ,&rv[ 6] },
  {  MAX17262_AGE                ,"AGE            " ,&rv[ 7] },
  {  MAX17262_TEMP               ,"TEMP           " ,&rv[ 8] },
  {  MAX17262_VCELL              ,"VCELL          " ,&rv[ 9] },
  {  MAX17262_CURRENT            ,"CURRENT        " ,&rv[10] },
  {  MAX17262_AVGCURRENT         ,"AVGCURRENT     " ,&rv[11] },
  {  MAX17262_QRESIDUAL          ,"QRESIDUAL      " ,&rv[12] },
  {  MAX17262_MIXSOC             ,"MIXSOC         " ,&rv[13] },
  {  MAX17262_AVSOC              ,"AVSOC          " ,&rv[14] },
  {  MAX17262_MIXCAP             ,"MIXCAP         " ,&rv[15] },
  {  MAX17262_FULLCAPREP         ,"FULLCAPREP     " ,&rv[16] },
  {  MAX17262_TTE                ,"TTE            " ,&rv[17] },
  {  MAX17262_QRTABLE00          ,"QRTABLE00      " ,&rv[18] },
  {  MAX17262_FULLSOCTHR         ,"FULLSOCTHR     " ,&rv[19] },
  {  MAX17262_RCELL              ,"RCELL          " ,&rv[20] },
  {  MAX17262_AVGTA              ,"AVGTA          " ,&rv[21] },
  {  MAX17262_CYCLES             ,"CYCLES         " ,&rv[22] },
  {  MAX17262_DESIGNCAP          ,"DESIGNCAP      " ,&rv[23] },
  {  MAX17262_AVGVCELL           ,"AVGVCELL       " ,&rv[24] },
  {  MAX17262_MAXMINTEMP         ,"MAXMINTEMP     " ,&rv[25] },
  {  MAX17262_MAXMINVOLT         ,"MAXMINVOLT     " ,&rv[26] },
  {  MAX17262_MAXMINCURR         ,"MAXMINCURR     " ,&rv[27] },
  {  MAX17262_CONFIG             ,"CONFIG         " ,&rv[28] },
  {  MAX17262_ICHGTERM           ,"ICHGTERM       " ,&rv[29] },
  {  MAX17262_AVCAP              ,"AVCAP          " ,&rv[30] },
  {  MAX17262_TTF                ,"TTF            " ,&rv[31] },
  {  MAX17262_DEVNAME            ,"DEVNAME        " ,&rv[32] },
  {  MAX17262_QRTABLE10          ,"QRTABLE10      " ,&rv[33] },
  {  MAX17262_FULLCAPNOM         ,"FULLCAPNOM     " ,&rv[34] },
  {  MAX17262_AIN                ,"AIN            " ,&rv[35] },
  {  MAX17262_LEARNCFG           ,"LEARNCFG       " ,&rv[36] },
  {  MAX17262_FILTERCFG          ,"FILTERCFG      " ,&rv[37] },
  {  MAX17262_RELAXCFG           ,"RELAXCFG       " ,&rv[38] },
  {  MAX17262_MISCCFG            ,"MISCCFG        " ,&rv[39] },
  {  MAX17262_TGAIN              ,"TGAIN          " ,&rv[40] },
  {  MAX17262_TOFF               ,"TOFF           " ,&rv[41] },
  {  MAX17262_CGAIN              ,"CGAIN          " ,&rv[42] },
  {  MAX17262_COFF               ,"COFF           " ,&rv[43] },
  {  MAX17262_QRTABLE20          ,"QRTABLE20      " ,&rv[44] },
  {  MAX17262_DIETEMP            ,"DIETEMP        " ,&rv[45] },
  {  MAX17262_FULLCAP            ,"FULLCAP        " ,&rv[46] },
  {  MAX17262_RCOMP0             ,"RCOMP0         " ,&rv[47] },
  {  MAX17262_TEMPCO             ,"TEMPCO         " ,&rv[48] },
  {  MAX17262_VEMPTY             ,"VEMPTY         " ,&rv[49] },
  {  MAX17262_FSTAT              ,"FSTAT          " ,&rv[50] },
  {  MAX17262_TIMER              ,"TIMER          " ,&rv[51] },
  {  MAX17262_SHDNTIMER          ,"SHDNTIMER      " ,&rv[52] },
  {  MAX17262_QRTABLE30          ,"QRTABLE30      " ,&rv[53] },
  {  MAX17262_RGAIN              ,"RGAIN          " ,&rv[54] },
  {  MAX17262_DQACC              ,"DQACC          " ,&rv[55] },
  {  MAX17262_DPACC              ,"DPACC          " ,&rv[56] },
  {  MAX17262_CONVGCFG           ,"CONVGCFG       " ,&rv[57] },
  {  MAX17262_VFREMCAP           ,"VFREMCAP       " ,&rv[58] },
  {  MAX17262_QH                 ,"QH             " ,&rv[59] },
  {  MAX17262_STATUS2            ,"STATUS2        " ,&rv[60] },
  {  MAX17262_POWER              ,"POWER          " ,&rv[61] },
  {  MAX17262_ID_USERMEM2        ,"ID_USERMEM2    " ,&rv[62] },
  {  MAX17262_AVGPOWER           ,"AVGPOWER       " ,&rv[63] },
  {  MAX17262_IALRTTH            ,"IALRTTH        " ,&rv[64] },
  {  MAX17262_TTFCFG             ,"TTFCFG         " ,&rv[65] },
  {  MAX17262_CVMIXCAP           ,"CVMIXCAP       " ,&rv[66] },
  {  MAX17262_CVHALFTIME         ,"CVHALFTIME     " ,&rv[67] },
  {  MAX17262_CGTEMPCO           ,"CGTEMPCO       " ,&rv[68] },
  {  MAX17262_CURVE              ,"CURVE          " ,&rv[69] },
  {  MAX17262_HIBCFG             ,"HIBCFG         " ,&rv[70] },
  {  MAX17262_CONFIG2            ,"CONFIG2        " ,&rv[71] },
  {  MAX17262_VRIPPLE            ,"VRIPPLE        " ,&rv[72] },
  {  MAX17262_RIPPLECFG          ,"RIPPLECFG      " ,&rv[73] },
  {  MAX17262_TIMERH             ,"TIMERH         " ,&rv[74] },
  {  MAX17262_RSENSE_USERMEM3    ,"RSENSE_USERMEM3" ,&rv[75] },
  {  MAX17262_SCOCVLIM           ,"SCOCVLIM       " ,&rv[76] },
  {  MAX17262_VGAIN              ,"VGAIN          " ,&rv[77] },
  {  MAX17262_SOCHOLD            ,"SOCHOLD        " ,&rv[78] },
  {  MAX17262_MAXPEAKPOWER       ,"MAXPEAKPOWER   " ,&rv[79] },
  {  MAX17262_SUSPEAKPOWER       ,"SUSPEAKPOWER   " ,&rv[80] },
  {  MAX17262_PACKRESISTANCE     ,"PACKRESISTANCE " ,&rv[81] },
  {  MAX17262_SYSRESISTANCE      ,"SYSRESISTANCE  " ,&rv[82] },
  {  MAX17262_MINSYSVOLTAGE      ,"MINSYSVOLTAGE  " ,&rv[83] },
  {  MAX17262_MPPCURRENT         ,"MPPCURRENT     " ,&rv[84] },
  {  MAX17262_SPPCURRENT         ,"SPPCURRENT     " ,&rv[85] },
  {  MAX17262_MODELCFG           ,"MODELCFG       " ,&rv[86] },
  {  MAX17262_ATQRESIDUAL        ,"ATQRESIDUAL    " ,&rv[87] },
  {  MAX17262_ATTTE              ,"ATTTE          " ,&rv[88] },
  {  MAX17262_ATAVSOC            ,"ATAVSOC        " ,&rv[89] },
  {  MAX17262_ATAVCAP            ,"ATAVCAP        " ,&rv[90] },
};


static ssp_err_t   _MAX17262_read_buf(uint8_t addr, uint8_t *buf, uint32_t sz);
static ssp_err_t   _MAX17262_write_buf(uint8_t addr, uint8_t *buf, uint32_t sz);


#define MAX1762_VERIFY_RESULT( x )  { ssp_err_t vres; vres = (x); if ( vres != SSP_SUCCESS ) { goto EXIT_ON_ERROR; } }


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Is_MAX17262_opened(void)
{
  return MAX17262_opened;
}

/*-----------------------------------------------------------------------------------------------------
  Открытие коммуникационного канала с чипом MAX17262

  Функция использует глобальный объект  g_sf_i2c_max17262
  Быть внимательным при вызове из разных задач!!!

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t  MAX17262_open(void)
{
  uint32_t      attempt_cnt = 0;
  ssp_err_t     res = SSP_SUCCESS;
  do
  {
    res = g_sf_i2c_max17262.p_api->open(g_sf_i2c_max17262.p_ctrl, g_sf_i2c_max17262.p_cfg);
    if (res == SSP_ERR_ALREADY_OPEN) res = SSP_SUCCESS;
    if (res == SSP_SUCCESS)  break;
    if (attempt_cnt > 5) break;
    Wait_ms(50);
    attempt_cnt++;
  } while (1);

  if (res == SSP_SUCCESS)
  {
    MAX17262_opened = 1;
  }
  else
  {
    MAX17262_opened = 0;
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Закрытие коммуникационного канала с чипом MAX17262
  Поскольку на шине I2C находятся и другие чипы то здесь также производится освобождение шины.


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MAX17262_close(void)
{
  return g_sf_i2c_max17262.p_api->close(g_sf_i2c_max17262.p_ctrl);
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param buf
  \param sz

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
static ssp_err_t _MAX17262_read_buf(uint8_t addr, uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  res = g_sf_i2c_max17262.p_api->write(g_sf_i2c_max17262.p_ctrl,&addr, 1, true , 10); // Записываем адрес
  if (res == SSP_SUCCESS)
  {
    res = g_sf_i2c_max17262.p_api->read(g_sf_i2c_max17262.p_ctrl, buf, sz, false , 10); // Читаем данные
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Чтение регистра.
  Регитсры у MAX17262 двухбайтные

  \param addr
  \param val

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t MAX17262_read_reg(uint8_t addr, uint16_t *val)
{
  return _MAX17262_read_buf(addr,(uint8_t *)val,2);
}


/*-----------------------------------------------------------------------------------------------------
  Первый байт буффера должен быть свободным для размещения в нем адреса
  Размер буфера указывается без учета первого байта

  \param sz

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
static ssp_err_t _MAX17262_write_buf(uint8_t addr, uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  buf[0] = addr;
  res = g_sf_i2c_max17262.p_api->write(g_sf_i2c_max17262.p_ctrl, buf, sz+1, false , 10); // Записываем адрес
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param val

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t MAX17262_write_reg(uint8_t addr, uint16_t val)
{
  uint8_t buf[3];

  buf[1] = val & 0xFF;
  buf[2] = (val >> 8) & 0xFF;
  return _MAX17262_write_buf(addr,buf,2);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MAX17262_reg_count(void)
{
  return sizeof(MAX17262_regs) / sizeof(MAX17262_regs[0]);
}

/*-----------------------------------------------------------------------------------------------------


  \param indx

  \return T_MAX17262_reg_descr*
-----------------------------------------------------------------------------------------------------*/
T_MAX17262_reg_descr const* MAX17262_get_reg_descr(uint8_t indx)
{
  if (indx >= MAX17262_reg_count()) indx = 0;
  return &MAX17262_regs[indx];
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MAX17262_read_reg_transaction(uint8_t addr, uint16_t *val)
{
  if (MAX17262_open() != RES_OK) return RES_ERROR;
  MAX1762_VERIFY_RESULT(MAX17262_read_reg(addr,val));
  MAX17262_close();
  return RES_OK;
EXIT_ON_ERROR:
  MAX17262_close();
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  MAX17262_write_reg_transaction(uint8_t addr, uint16_t val)
{
  if (MAX17262_open() != RES_OK) return RES_ERROR;
  MAX1762_VERIFY_RESULT(MAX17262_write_reg(addr,val));
  MAX17262_close();
  return RES_OK;
EXIT_ON_ERROR:
  MAX17262_close();
  return RES_ERROR;
}
