// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020.04.30
// 10:09:05
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"


const i2c_cfg_t g_sf_i2c_max17262_i2c_cfg =
{
  .channel             = 2,
  .rate                = I2C_RATE_STANDARD,
  .slave               = 0x36,
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

sf_i2c_instance_ctrl_t g_sf_i2c_max17262_ctrl =
{
  .p_lower_lvl_ctrl =&I2C2_control_block,
};
const sf_i2c_cfg_t g_sf_i2c_max17262_cfg =
{
  .p_bus                    = (sf_i2c_bus_t *)&I2C2_bus,
  .p_lower_lvl_cfg          =&g_sf_i2c_max17262_i2c_cfg,
};

const sf_i2c_instance_t g_sf_i2c_max17262 =
{
  .p_ctrl        =&g_sf_i2c_max17262_ctrl,
  .p_cfg         =&g_sf_i2c_max17262_cfg,
  .p_api         =&g_sf_i2c_on_sf_i2c
};


const T_MAX17262_reg_descr MAX17262_regs[]=
{
  {  MAX17262_STATUS             ,"STATUS         " },
  {  MAX17262_VALRTTH            ,"VALRTTH        " },
  {  MAX17262_TALRTTH            ,"TALRTTH        " },
  {  MAX17262_SALRTTH            ,"SALRTTH        " },
  {  MAX17262_ATRATE             ,"ATRATE         " },
  {  MAX17262_REPCAP             ,"REPCAP         " },
  {  MAX17262_REPSOC             ,"REPSOC         " },
  {  MAX17262_AGE                ,"AGE            " },
  {  MAX17262_TEMP               ,"TEMP           " },
  {  MAX17262_VCELL              ,"VCELL          " },
  {  MAX17262_CURRENT            ,"CURRENT        " },
  {  MAX17262_AVGCURRENT         ,"AVGCURRENT     " },
  {  MAX17262_QRESIDUAL          ,"QRESIDUAL      " },
  {  MAX17262_MIXSOC             ,"MIXSOC         " },
  {  MAX17262_AVSOC              ,"AVSOC          " },
  {  MAX17262_MIXCAP             ,"MIXCAP         " },
  {  MAX17262_FULLCAPREP         ,"FULLCAPREP     " },
  {  MAX17262_TTE                ,"TTE            " },
  {  MAX17262_QRTABLE00          ,"QRTABLE00      " },
  {  MAX17262_FULLSOCTHR         ,"FULLSOCTHR     " },
  {  MAX17262_RCELL              ,"RCELL          " },
  {  MAX17262_AVGTA              ,"AVGTA          " },
  {  MAX17262_CYCLES             ,"CYCLES         " },
  {  MAX17262_DESIGNCAP          ,"DESIGNCAP      " },
  {  MAX17262_AVGVCELL           ,"AVGVCELL       " },
  {  MAX17262_MAXMINTEMP         ,"MAXMINTEMP     " },
  {  MAX17262_MAXMINVOLT         ,"MAXMINVOLT     " },
  {  MAX17262_MAXMINCURR         ,"MAXMINCURR     " },
  {  MAX17262_CONFIG             ,"CONFIG         " },
  {  MAX17262_ICHGTERM           ,"ICHGTERM       " },
  {  MAX17262_AVCAP              ,"AVCAP          " },
  {  MAX17262_TTF                ,"TTF            " },
  {  MAX17262_DEVNAME            ,"DEVNAME        " },
  {  MAX17262_QRTABLE10          ,"QRTABLE10      " },
  {  MAX17262_FULLCAPNOM         ,"FULLCAPNOM     " },
  {  MAX17262_AIN                ,"AIN            " },
  {  MAX17262_LEARNCFG           ,"LEARNCFG       " },
  {  MAX17262_FILTERCFG          ,"FILTERCFG      " },
  {  MAX17262_RELAXCFG           ,"RELAXCFG       " },
  {  MAX17262_MISCCFG            ,"MISCCFG        " },
  {  MAX17262_TGAIN              ,"TGAIN          " },
  {  MAX17262_TOFF               ,"TOFF           " },
  {  MAX17262_CGAIN              ,"CGAIN          " },
  {  MAX17262_COFF               ,"COFF           " },
  {  MAX17262_QRTABLE20          ,"QRTABLE20      " },
  {  MAX17262_DIETEMP            ,"DIETEMP        " },
  {  MAX17262_FULLCAP            ,"FULLCAP        " },
  {  MAX17262_RCOMP0             ,"RCOMP0         " },
  {  MAX17262_TEMPCO             ,"TEMPCO         " },
  {  MAX17262_VEMPTY             ,"VEMPTY         " },
  {  MAX17262_FSTAT              ,"FSTAT          " },
  {  MAX17262_TIMER              ,"TIMER          " },
  {  MAX17262_SHDNTIMER          ,"SHDNTIMER      " },
  {  MAX17262_QRTABLE30          ,"QRTABLE30      " },
  {  MAX17262_RGAIN              ,"RGAIN          " },
  {  MAX17262_DQACC              ,"DQACC          " },
  {  MAX17262_DPACC              ,"DPACC          " },
  {  MAX17262_CONVGCFG           ,"CONVGCFG       " },
  {  MAX17262_VFREMCAP           ,"VFREMCAP       " },
  {  MAX17262_QH                 ,"QH             " },
  {  MAX17262_STATUS2            ,"STATUS2        " },
  {  MAX17262_POWER              ,"POWER          " },
  {  MAX17262_ID_USERMEM2        ,"ID_USERMEM2    " },
  {  MAX17262_AVGPOWER           ,"AVGPOWER       " },
  {  MAX17262_IALRTTH            ,"IALRTTH        " },
  {  MAX17262_TTFCFG             ,"TTFCFG         " },
  {  MAX17262_CVMIXCAP           ,"CVMIXCAP       " },
  {  MAX17262_CVHALFTIME         ,"CVHALFTIME     " },
  {  MAX17262_CGTEMPCO           ,"CGTEMPCO       " },
  {  MAX17262_CURVE              ,"CURVE          " },
  {  MAX17262_HIBCFG             ,"HIBCFG         " },
  {  MAX17262_CONFIG2            ,"CONFIG2        " },
  {  MAX17262_VRIPPLE            ,"VRIPPLE        " },
  {  MAX17262_RIPPLECFG          ,"RIPPLECFG      " },
  {  MAX17262_TIMERH             ,"TIMERH         " },
  {  MAX17262_RSENSE_USERMEM3    ,"RSENSE_USERMEM3" },
  {  MAX17262_SCOCVLIM           ,"SCOCVLIM       " },
  {  MAX17262_VGAIN              ,"VGAIN          " },
  {  MAX17262_SOCHOLD            ,"SOCHOLD        " },
  {  MAX17262_MAXPEAKPOWER       ,"MAXPEAKPOWER   " },
  {  MAX17262_SUSPEAKPOWER       ,"SUSPEAKPOWER   " },
  {  MAX17262_PACKRESISTANCE     ,"PACKRESISTANCE " },
  {  MAX17262_SYSRESISTANCE      ,"SYSRESISTANCE  " },
  {  MAX17262_MINSYSVOLTAGE      ,"MINSYSVOLTAGE  " },
  {  MAX17262_MPPCURRENT         ,"MPPCURRENT     " },
  {  MAX17262_SPPCURRENT         ,"SPPCURRENT     " },
  {  MAX17262_MODELCFG           ,"MODELCFG       " },
  {  MAX17262_ATQRESIDUAL        ,"ATQRESIDUAL    " },
  {  MAX17262_ATTTE              ,"ATTTE          " },
  {  MAX17262_ATAVSOC            ,"ATAVSOC        " },
  {  MAX17262_ATAVCAP            ,"ATAVCAP        " },
};


static ssp_err_t   _MAX17262_read_buf(uint8_t addr, uint8_t *buf, uint32_t sz);
static ssp_err_t   _MAX17262_write_buf(uint8_t addr, uint8_t *buf, uint32_t sz);

static uint32_t MAX17262_open_err = 0;

#define MAX1762_VERIFY_RESULT( x )  { ssp_err_t vres; vres = (x); if ( vres != SSP_SUCCESS ) { goto EXIT_ON_ERROR; } }

/*-----------------------------------------------------------------------------------------------------
  Открытие коммуникационного канала с чипом MAX17262
  Поскольку на шине I2C находятся и другие чипы то здесь также производится ожидание захвата шины.

  Функция использует глобальный объект  g_sf_i2c_max17262
  Быть внимательным при вызове из разных задач!!!



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  MAX17262_open(uint32_t timeout)
{
  ssp_err_t           res = SSP_SUCCESS;
  res = g_sf_i2c_max17262.p_api->open(g_sf_i2c_max17262.p_ctrl, g_sf_i2c_max17262.p_cfg);
  if (res == SSP_SUCCESS)
  {
    res = g_sf_i2c_max17262.p_api->lockWait(g_sf_i2c_max17262.p_ctrl, timeout);
    if (res != SSP_SUCCESS)
    {
      g_sf_i2c_max17262.p_api->close(g_sf_i2c_max17262.p_ctrl);
      return RES_ERROR;
    }
    return RES_OK;

  }
  else
  {
    MAX17262_open_err++;
    return RES_ERROR;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Закрытие коммуникационного канала с чипом MAX17262
  Поскольку на шине I2C находятся и другие чипы то здесь также производится освобождение шины.


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MAX17262_close(void)
{
  ssp_err_t           res;
  g_sf_i2c_max17262.p_api->reset(g_sf_i2c_max17262.p_ctrl, 2); // Сброс чтобы избежать ошибки при вызове close
  g_sf_i2c_max17262.p_api->unlock(g_sf_i2c_max17262.p_ctrl);
  res = g_sf_i2c_max17262.p_api->close(g_sf_i2c_max17262.p_ctrl);
  if (res != SSP_SUCCESS)
  {
    return RES_ERROR;
  }
  return RES_OK;

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
  if (MAX17262_open(MAX17262_OPEN_TIMEOUT) != RES_OK) return RES_ERROR;
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
  if (MAX17262_open(MAX17262_OPEN_TIMEOUT) != RES_OK) return RES_ERROR;
  MAX1762_VERIFY_RESULT(MAX17262_write_reg(addr,val));
  MAX17262_close();
  return RES_OK;
EXIT_ON_ERROR:
  MAX17262_close();
  return RES_ERROR;
}
