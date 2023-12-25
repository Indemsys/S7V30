// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.06.01
// 20:23:00
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"

const i2c_cfg_t g_sf_i2c_ISM330_i2c_cfg =
{
  .channel             = 2,
  .rate                = I2C_RATE_STANDARD,
  .slave               = 0x6B,
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

sf_i2c_instance_ctrl_t g_sf_i2c_ISM330_ctrl =
{
  .p_lower_lvl_ctrl =&I2C2_control_block,
};

const sf_i2c_cfg_t g_sf_i2c_ISM330_cfg =
{
  .p_bus                    = (sf_i2c_bus_t *)&I2C2_bus,
  .p_lower_lvl_cfg          =&g_sf_i2c_ISM330_i2c_cfg,
};


/* Instance structure to use this module. */
const sf_i2c_instance_t g_sf_i2c_ISM330 =
{
  .p_ctrl        =&g_sf_i2c_ISM330_ctrl,
  .p_cfg         =&g_sf_i2c_ISM330_cfg,
  .p_api         =&g_sf_i2c_on_sf_i2c
};


static uint32_t ISM330_opened = 0;


const T_ISM330_reg_descr ISM330_regs[]=
{
  { ISM330_FUNC_CFG_ACCESS           ,"ISM330_FUNC_CFG_ACCESS          " },
  { ISM330_SENSOR_SYNC_TIME_FRAME    ,"ISM330_SENSOR_SYNC_TIME_FRAME   " },
  { ISM330_SENSOR_SYNC_RES_RATIO     ,"ISM330_SENSOR_SYNC_RES_RATIO    " },
  { ISM330_FIFO_CTRL1                ,"ISM330_FIFO_CTRL1               " },
  { ISM330_FIFO_CTRL2                ,"ISM330_FIFO_CTRL2               " },
  { ISM330_FIFO_CTRL3                ,"ISM330_FIFO_CTRL3               " },
  { ISM330_FIFO_CTRL4                ,"ISM330_FIFO_CTRL4               " },
  { ISM330_FIFO_CTRL5                ,"ISM330_FIFO_CTRL5               " },
  { ISM330_DRDY_PULSE_CFG            ,"ISM330_DRDY_PULSE_CFG           " },
  { ISM330_INT1_CTRL                 ,"ISM330_INT1_CTRL                " },
  { ISM330_INT2_CTRL                 ,"ISM330_INT2_CTRL                " },
  { ISM330_WHO_AM_I                  ,"ISM330_WHO_AM_I                 " },
  { ISM330_CTRL1_XL                  ,"ISM330_CTRL1_XL                 " },
  { ISM330_CTRL2_G                   ,"ISM330_CTRL2_G                  " },
  { ISM330_CTRL3_C                   ,"ISM330_CTRL3_C                  " },
  { ISM330_CTRL4_C                   ,"ISM330_CTRL4_C                  " },
  { ISM330_CTRL5_C                   ,"ISM330_CTRL5_C                  " },
  { ISM330_CTRL6_C                   ,"ISM330_CTRL6_C                  " },
  { ISM330_CTRL7_G                   ,"ISM330_CTRL7_G                  " },
  { ISM330_CTRL8_XL                  ,"ISM330_CTRL8_XL                 " },
  { ISM330_CTRL9_XL                  ,"ISM330_CTRL9_XL                 " },
  { ISM330_CTRL10_C                  ,"ISM330_CTRL10_C                 " },
  { ISM330_MASTER_CONFIG             ,"ISM330_MASTER_CONFIG            " },
  { ISM330_WAKE_UP_SRC               ,"ISM330_WAKE_UP_SRC              " },
  { ISM330_TAP_SRC                   ,"ISM330_TAP_SRC                  " },
  { ISM330_D6D_SRC                   ,"ISM330_D6D_SRC                  " },
  { ISM330_STATUS_REG                ,"ISM330_STATUS_REG               " },
  { ISM330_OUT_TEMP_L                ,"ISM330_OUT_TEMP_L               " },
  { ISM330_OUT_TEMP_H                ,"ISM330_OUT_TEMP_H               " },
  { ISM330_OUTX_L_G                  ,"ISM330_OUTX_L_G                 " },
  { ISM330_OUTX_H_G                  ,"ISM330_OUTX_H_G                 " },
  { ISM330_OUTY_L_G                  ,"ISM330_OUTY_L_G                 " },
  { ISM330_OUTY_H_G                  ,"ISM330_OUTY_H_G                 " },
  { ISM330_OUTZ_L_G                  ,"ISM330_OUTZ_L_G                 " },
  { ISM330_OUTZ_H_G                  ,"ISM330_OUTZ_H_G                 " },
  { ISM330_OUTX_L_XL                 ,"ISM330_OUTX_L_XL                " },
  { ISM330_OUTX_H_XL                 ,"ISM330_OUTX_H_XL                " },
  { ISM330_OUTY_L_XL                 ,"ISM330_OUTY_L_XL                " },
  { ISM330_OUTY_H_XL                 ,"ISM330_OUTY_H_XL                " },
  { ISM330_OUTZ_L_XL                 ,"ISM330_OUTZ_L_XL                " },
  { ISM330_OUTZ_H_XL                 ,"ISM330_OUTZ_H_XL                " },
  { ISM330_SENSORHUB1_REG            ,"ISM330_SENSORHUB1_REG           " },
  { ISM330_SENSORHUB2_REG            ,"ISM330_SENSORHUB2_REG           " },
  { ISM330_SENSORHUB3_REG            ,"ISM330_SENSORHUB3_REG           " },
  { ISM330_SENSORHUB4_REG            ,"ISM330_SENSORHUB4_REG           " },
  { ISM330_SENSORHUB5_REG            ,"ISM330_SENSORHUB5_REG           " },
  { ISM330_SENSORHUB6_REG            ,"ISM330_SENSORHUB6_REG           " },
  { ISM330_SENSORHUB7_REG            ,"ISM330_SENSORHUB7_REG           " },
  { ISM330_SENSORHUB8_REG            ,"ISM330_SENSORHUB8_REG           " },
  { ISM330_SENSORHUB9_REG            ,"ISM330_SENSORHUB9_REG           " },
  { ISM330_SENSORHUB10_REG           ,"ISM330_SENSORHUB10_REG          " },
  { ISM330_SENSORHUB11_REG           ,"ISM330_SENSORHUB11_REG          " },
  { ISM330_SENSORHUB12_REG           ,"ISM330_SENSORHUB12_REG          " },
  { ISM330_FIFO_STATUS1              ,"ISM330_FIFO_STATUS1             " },
  { ISM330_FIFO_STATUS2              ,"ISM330_FIFO_STATUS2             " },
  { ISM330_FIFO_STATUS3              ,"ISM330_FIFO_STATUS3             " },
  { ISM330_FIFO_STATUS4              ,"ISM330_FIFO_STATUS4             " },
  { ISM330_FIFO_DATA_OUT_L           ,"ISM330_FIFO_DATA_OUT_L          " },
  { ISM330_FIFO_DATA_OUT_H           ,"ISM330_FIFO_DATA_OUT_H          " },
  { ISM330_TIMESTAMP0_REG            ,"ISM330_TIMESTAMP0_REG           " },
  { ISM330_TIMESTAMP1_REG            ,"ISM330_TIMESTAMP1_REG           " },
  { ISM330_TIMESTAMP2_REG            ,"ISM330_TIMESTAMP2_REG           " },
  { ISM330_SENSORHUB13_REG           ,"ISM330_SENSORHUB13_REG          " },
  { ISM330_SENSORHUB14_REG           ,"ISM330_SENSORHUB14_REG          " },
  { ISM330_SENSORHUB15_REG           ,"ISM330_SENSORHUB15_REG          " },
  { ISM330_SENSORHUB16_REG           ,"ISM330_SENSORHUB16_REG          " },
  { ISM330_SENSORHUB17_REG           ,"ISM330_SENSORHUB17_REG          " },
  { ISM330_SENSORHUB18_REG           ,"ISM330_SENSORHUB18_REG          " },
  { ISM330_FUNC_SRC1                 ,"ISM330_FUNC_SRC1                " },
  { ISM330_FUNC_SRC2                 ,"ISM330_FUNC_SRC2                " },
  { ISM330_TAP_CFG                   ,"ISM330_TAP_CFG                  " },
  { ISM330_TAP_THS_6D                ,"ISM330_TAP_THS_6D               " },
  { ISM330_INT_DUR2                  ,"ISM330_INT_DUR2                 " },
  { ISM330_WAKE_UP_THS               ,"ISM330_WAKE_UP_THS              " },
  { ISM330_WAKE_UP_DUR               ,"ISM330_WAKE_UP_DUR              " },
  { ISM330_FREE_FALL                 ,"ISM330_FREE_FALL                " },
  { ISM330_MD1_CFG                   ,"ISM330_MD1_CFG                  " },
  { ISM330_MD2_CFG                   ,"ISM330_MD2_CFG                  " },
  { ISM330_MASTER_CMD_CODE           ,"ISM330_MASTER_CMD_CODE          " },
  { ISM330_SENS_SYNC_SPI_ERROR_CODE  ,"ISM330_SENS_SYNC_SPI_ERROR_CODE " },
  { ISM330_OUT_MAG_RAW_X_L           ,"ISM330_OUT_MAG_RAW_X_L          " },
  { ISM330_OUT_MAG_RAW_X_H           ,"ISM330_OUT_MAG_RAW_X_H          " },
  { ISM330_OUT_MAG_RAW_Y_L           ,"ISM330_OUT_MAG_RAW_Y_L          " },
  { ISM330_OUT_MAG_RAW_Y_H           ,"ISM330_OUT_MAG_RAW_Y_H          " },
  { ISM330_OUT_MAG_RAW_Z_L           ,"ISM330_OUT_MAG_RAW_Z_L          " },
  { ISM330_OUT_MAG_RAW_Z_H           ,"ISM330_OUT_MAG_RAW_Z_H          " },
  { ISM330_INT_OIS                   ,"ISM330_INT_OIS                  " },
  { ISM330_CTRL1_OIS                 ,"ISM330_CTRL1_OIS                " },
  { ISM330_CTRL2_OIS                 ,"ISM330_CTRL2_OIS                " },
  { ISM330_CTRL3_OIS                 ,"ISM330_CTRL3_OIS                " },
  { ISM330_X_OFS_USR                 ,"ISM330_X_OFS_USR                " },
  { ISM330_Y_OFS_USR                 ,"ISM330_Y_OFS_USR                " },
  { ISM330_Z_OFS_USR                 ,"ISM330_Z_OFS_USR                " },
};


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ISM330_reg_count(void)
{
  return sizeof(ISM330_regs) / sizeof(ISM330_regs[0]);
}

/*-----------------------------------------------------------------------------------------------------


  \param indx

  \return T_da7217_reg_descr
-----------------------------------------------------------------------------------------------------*/
T_ISM330_reg_descr const* ISM330_get_reg_descr(uint8_t indx)
{
  if (indx >= ISM330_reg_count()) indx = 0;
  return &ISM330_regs[indx];
}

/*-----------------------------------------------------------------------------------------------------



  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t  ISM330_open(void)
{
  ssp_err_t           res = SSP_SUCCESS;
  if (ISM330_opened == 0)
  {
    res = g_sf_i2c_ISM330.p_api->open(g_sf_i2c_ISM330.p_ctrl, g_sf_i2c_ISM330.p_cfg);
    if (res == SSP_SUCCESS)
    {
      ISM330_opened = 1;
    }
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t  ISM330_close(void)
{
  ISM330_opened = 0;
  return g_sf_i2c_ISM330.p_api->close(g_sf_i2c_ISM330.p_ctrl);
}


/*-----------------------------------------------------------------------------------------------------
  Запись буфера с данными

-----------------------------------------------------------------------------------------------------*/
ssp_err_t ISM330_write_buf(uint8_t addr, uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  if (ISM330_opened == 1)
  {
    buf[0] = addr;
    res = g_sf_i2c_ISM330.p_api->write(g_sf_i2c_ISM330.p_ctrl, buf, sz+1, false , 10); // Записываем адрес
    return res;
  }
  return SSP_ERR_NOT_OPEN;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param buf
  \param sz

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t ISM330_read_buf(uint8_t addr, uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  if (ISM330_opened == 1)
  {
    res = g_sf_i2c_ISM330.p_api->write(g_sf_i2c_ISM330.p_ctrl,&addr, 1, true , 10); // Записываем адрес
    if (res == SSP_SUCCESS)
    {
      res = g_sf_i2c_ISM330.p_api->read(g_sf_i2c_ISM330.p_ctrl, buf, sz, false , 10); // Записываем адрес
    }
    return res;
  }
  return SSP_ERR_NOT_OPEN;
}


/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param val

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t ISM330_read_register(uint8_t addr, uint8_t *val)
{
  return ISM330_read_buf(addr,val,1);
}

