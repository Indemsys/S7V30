#ifndef S7V30_ISM330_H
  #define S7V30_ISM330_H

  #define ISM330_CHIP_ID                              0x6A


  #define ISM330_FUNC_CFG_ACCESS                      0x01
  #define ISM330_SENSOR_SYNC_TIME_FRAME               0x04
  #define ISM330_SENSOR_SYNC_RES_RATIO                0x05
  #define ISM330_FIFO_CTRL1                           0x06
  #define ISM330_FIFO_CTRL2                           0x07
  #define ISM330_FIFO_CTRL3                           0x08
  #define ISM330_FIFO_CTRL4                           0x09
  #define ISM330_FIFO_CTRL5                           0x0A
  #define ISM330_DRDY_PULSE_CFG                       0x0B
  #define ISM330_INT1_CTRL                            0x0D
  #define ISM330_INT2_CTRL                            0x0E
  #define ISM330_WHO_AM_I                             0x0F
  #define ISM330_CTRL1_XL                             0x10
  #define ISM330_CTRL2_G                              0x11
  #define ISM330_CTRL3_C                              0x12
  #define ISM330_CTRL4_C                              0x13
  #define ISM330_CTRL5_C                              0x14
  #define ISM330_CTRL6_C                              0x15
  #define ISM330_CTRL7_G                              0x16
  #define ISM330_CTRL8_XL                             0x17
  #define ISM330_CTRL9_XL                             0x18
  #define ISM330_CTRL10_C                             0x19
  #define ISM330_MASTER_CONFIG                        0x1A
  #define ISM330_WAKE_UP_SRC                          0x1B
  #define ISM330_TAP_SRC                              0x1C
  #define ISM330_D6D_SRC                              0x1D
  #define ISM330_STATUS_REG                           0x1E
  #define ISM330_OUT_TEMP_L                           0x20
  #define ISM330_OUT_TEMP_H                           0x21
  #define ISM330_OUTX_L_G                             0x22
  #define ISM330_OUTX_H_G                             0x23
  #define ISM330_OUTY_L_G                             0x24
  #define ISM330_OUTY_H_G                             0x25
  #define ISM330_OUTZ_L_G                             0x26
  #define ISM330_OUTZ_H_G                             0x27
  #define ISM330_OUTX_L_XL                            0x28
  #define ISM330_OUTX_H_XL                            0x29
  #define ISM330_OUTY_L_XL                            0x2A
  #define ISM330_OUTY_H_XL                            0x2B
  #define ISM330_OUTZ_L_XL                            0x2C
  #define ISM330_OUTZ_H_XL                            0x2D
  #define ISM330_SENSORHUB1_REG                       0x2E
  #define ISM330_SENSORHUB2_REG                       0x2F
  #define ISM330_SENSORHUB3_REG                       0x30
  #define ISM330_SENSORHUB4_REG                       0x31
  #define ISM330_SENSORHUB5_REG                       0x32
  #define ISM330_SENSORHUB6_REG                       0x33
  #define ISM330_SENSORHUB7_REG                       0x34
  #define ISM330_SENSORHUB8_REG                       0x35
  #define ISM330_SENSORHUB9_REG                       0x36
  #define ISM330_SENSORHUB10_REG                      0x37
  #define ISM330_SENSORHUB11_REG                      0x38
  #define ISM330_SENSORHUB12_REG                      0x39
  #define ISM330_FIFO_STATUS1                         0x3A
  #define ISM330_FIFO_STATUS2                         0x3B
  #define ISM330_FIFO_STATUS3                         0x3C
  #define ISM330_FIFO_STATUS4                         0x3D
  #define ISM330_FIFO_DATA_OUT_L                      0x3E
  #define ISM330_FIFO_DATA_OUT_H                      0x3F
  #define ISM330_TIMESTAMP0_REG                       0x40
  #define ISM330_TIMESTAMP1_REG                       0x41
  #define ISM330_TIMESTAMP2_REG                       0x42
  #define ISM330_SENSORHUB13_REG                      0x4D
  #define ISM330_SENSORHUB14_REG                      0x4E
  #define ISM330_SENSORHUB15_REG                      0x4F
  #define ISM330_SENSORHUB16_REG                      0x50
  #define ISM330_SENSORHUB17_REG                      0x51
  #define ISM330_SENSORHUB18_REG                      0x52
  #define ISM330_FUNC_SRC1                            0x53
  #define ISM330_FUNC_SRC2                            0x54
  #define ISM330_TAP_CFG                              0x58
  #define ISM330_TAP_THS_6D                           0x59
  #define ISM330_INT_DUR2                             0x5A
  #define ISM330_WAKE_UP_THS                          0x5B
  #define ISM330_WAKE_UP_DUR                          0x5C
  #define ISM330_FREE_FALL                            0x5D
  #define ISM330_MD1_CFG                              0x5E
  #define ISM330_MD2_CFG                              0x5F
  #define ISM330_MASTER_CMD_CODE                      0x60
  #define ISM330_SENS_SYNC_SPI_ERROR_CODE             0x61
  #define ISM330_OUT_MAG_RAW_X_L                      0x66
  #define ISM330_OUT_MAG_RAW_X_H                      0x67
  #define ISM330_OUT_MAG_RAW_Y_L                      0x68
  #define ISM330_OUT_MAG_RAW_Y_H                      0x69
  #define ISM330_OUT_MAG_RAW_Z_L                      0x6A
  #define ISM330_OUT_MAG_RAW_Z_H                      0x6B
  #define ISM330_INT_OIS                              0x6F
  #define ISM330_CTRL1_OIS                            0x70
  #define ISM330_CTRL2_OIS                            0x71
  #define ISM330_CTRL3_OIS                            0x72
  #define ISM330_X_OFS_USR                            0x73
  #define ISM330_Y_OFS_USR                            0x74
  #define ISM330_Z_OFS_USR                            0x75


extern const sf_spi_instance_t g_sf_spi_ISM330;


typedef struct
{
    uint16_t      reg_addr;
    char  const  *reg_name;

} T_ISM330_reg_descr;

uint32_t   ISM330_reg_count(void);
T_ISM330_reg_descr const* ISM330_get_reg_descr(uint8_t indx);
ssp_err_t  ISM330_open(void);
ssp_err_t  ISM330_close(void);
ssp_err_t  ISM330_write_buf(uint8_t addr, uint8_t *buf, uint32_t sz);
ssp_err_t  ISM330_read_buf(uint8_t addr, uint8_t *buf, uint32_t sz);
ssp_err_t  ISM330_read_register(uint8_t addr, uint8_t *val);

#endif



