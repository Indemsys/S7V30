// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.06.01
// 20:23:45
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


// RF Transceiver SX1262IMLTRT соединен с микроконтроолром шиной SPI через интерфейсный порт SCI5


const spi_cfg_t g_sf_spi_SX1262_spi_cfg =
{
  .channel             = 5,
  .operating_mode      = SPI_MODE_MASTER,
  .clk_phase           = SPI_CLK_PHASE_EDGE_ODD,
  .clk_polarity        = SPI_CLK_POLARITY_LOW,
  .mode_fault          = SPI_MODE_FAULT_ERROR_DISABLE,
  .bit_order           = SPI_BIT_ORDER_MSB_FIRST,
  .bitrate             = 1000000,
  .p_transfer_tx       = &DTC_SPI5_TX_transfer_instance,
  .p_transfer_rx       = &DTC_SPI5_RX_transfer_instance,
  .p_callback          = NULL,
  .p_context           = &SPI5_interface_instance,
  .rxi_ipl             = 12,
  .txi_ipl             = 12,
  .tei_ipl             = 12,
  .eri_ipl             = 12,
  .p_extend            = &SPI5_extended_configuration,
};

sf_spi_instance_ctrl_t g_sf_spi_SX1262_ctrl =
{
  .p_lower_lvl_ctrl =&SPI5_control_block,
};

const sf_spi_cfg_t g_sf_spi_SX1262_cfg =
{
  .p_bus                    = (sf_spi_bus_t *)&SPI5_bus,
  .chip_select              = IOPORT_PORT_05_PIN_07,
  .chip_select_level_active = IOPORT_LEVEL_LOW,
  .p_lower_lvl_cfg          =&g_sf_spi_SX1262_spi_cfg,
};

/* Instance structure to use this module. */
const sf_spi_instance_t g_sf_spi_SX1262 =
{
  .p_ctrl        =&g_sf_spi_SX1262_ctrl,
  .p_cfg         =&g_sf_spi_SX1262_cfg,
  .p_api         =&g_sf_spi_on_sf_spi
};



static uint32_t SX1262_opened = 0;


const T_SX1262_reg_descr SX1262_regs[]=
{
  { SX_1262_WHITENING_INITIAL_VALUE_MSB  , "Whitening initial value MSB"},
  { SX_1262_WHITENING_INITIAL_VALUE_LSB  , "Whitening initial value LSB"},
  { SX_1262_CRC_MSB_INITIAL_VALUE_0      , "CRC MSB Initial Value_0    "},
  { SX_1262_CRC_LSB_INITIAL_VALUE_1      , "CRC LSB Initial Value_1    "},
  { SX_1262_CRC_MSB_POLYNOMIAL_VALUE_0   , "CRC MSB polynomial Value_0 "},
  { SX_1262_CRC_LSB_POLYNOMIAL_VALUE_1   , "CRC LSB polynomial Value_1 "},
  { SX_1262_SYNCWORD_0                   , "SyncWord_0                 "},
  { SX_1262_SYNCWORD_1                   , "SyncWord_1                 "},
  { SX_1262_SYNCWORD_2                   , "SyncWord_2                 "},
  { SX_1262_SYNCWORD_3                   , "SyncWord_3                 "},
  { SX_1262_SYNCWORD_4                   , "SyncWord_4                 "},
  { SX_1262_SYNCWORD_5                   , "SyncWord_5                 "},
  { SX_1262_SYNCWORD_6                   , "SyncWord_6                 "},
  { SX_1262_SYNCWORD_7                   , "SyncWord_7                 "},
  { SX_1262_NODE_ADDRESS                 , "Node Address               "},
  { SX_1262_BROADCAST_ADDRESS            , "Broadcast Address          "},
  { SX_1262_LORA_SYNC_WORD_MSB           , "LoRa Sync Word MSB         "},
  { SX_1262_LORA_SYNC_WORD_LSB           , "LoRa Sync Word LSB         "},
  { SX_1262_RANDOMNUMBERGEN_0            , "RandomNumberGen_0          "},
  { SX_1262_RANDOMNUMBERGEN_1            , "RandomNumberGen_1          "},
  { SX_1262_RANDOMNUMBERGEN_2            , "RandomNumberGen_2          "},
  { SX_1262_RANDOMNUMBERGEN_3            , "RandomNumberGen_3          "},
  { SX_1262_RX_GAIN                      , "Rx Gain                    "},
  { SX_1262_OCP_CONFIGURATION            , "OCP Configuration          "},
  { SX_1262_XTA_TRIM                     , "XTA trim                   "},
  { SX_1262_XTB_TRIM                     , "XTB trim                   "},
};


// Объявляем прерывания таким образом чтобы это не конфликтовало с методом принятым в библиотеке SSP
void  RF_BUSY_IRQ12DS_isr(void);
void  RF_DIO1_IRQ13DS_isr(void);

SSP_VECTOR_DEFINE(RF_BUSY_IRQ12DS_isr, ICU, IRQ12);  // Символы ICU и IRQ12 входя в идентификатор SSP_SIGNAL_ICU_IRQ12 в типе-перечеслении ssp_signal_t объывленном в фале ssp_features.h
SSP_VECTOR_DEFINE(RF_DIO1_IRQ13DS_isr, ICU, IRQ13);  //


static IRQn_Type rf_busy_irq12_int_num;
static IRQn_Type rf_dio1_irq13_int_num;


T_SX1262_cbl           SX1262_cbl;

TX_EVENT_FLAGS_GROUP   SX1262_flags;


/*-----------------------------------------------------------------------------------------------------
  Переход сигнала BUSY из верхнего уровня в нижний говорит о готовности чипа SX1262 к приему следующей команды

-----------------------------------------------------------------------------------------------------*/
void  RF_BUSY_IRQ12DS_isr(void)
{
  IRQn_Type curr_irq_t;
  SF_CONTEXT_SAVE;

  curr_irq_t = R_SSP_CurrentIrqGet();


  R_ICU->IELSRn_b[curr_irq_t].IR = 0;  // Сбрасываем IR флаг в ICU

  //LoRa_Set_chip_ready(); // Устанавливаем флаг готовности чипа к приему следующей команды

  SF_CONTEXT_RESTORE;
}


/*-----------------------------------------------------------------------------------------------------
  Переход сигнала из нижнего уровня в верхний говорит о возникновении прерывания

-----------------------------------------------------------------------------------------------------*/
void  RF_DIO1_IRQ13DS_isr(void)
{
  IRQn_Type curr_irq_t;
  SF_CONTEXT_SAVE;

  curr_irq_t = R_SSP_CurrentIrqGet();

  R_ICU->IELSRn_b[curr_irq_t].IR = 0;  // Сбрасываем IR флаг в ICU

  //LoRa_Set_chip_interrupt_flag(); // Устанавливаем флаг получения прерывания от чипа

  SF_CONTEXT_RESTORE;
}

/*-----------------------------------------------------------------------------------------------------
  Устанавливаем флаг готовности чипа к приему следующей команды

  \param void

  \return uint32_t - TX_SUCCESS   в случае успеха
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_Set_chip_ready(void)
{
  return tx_event_flags_set(&SX1262_flags, FLAG_LORA_READY, TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param flags
  \param wait_option

  \return uint32_t - TX_SUCCESS   в случае успеха
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_wait_flags(uint32_t flags, ULONG *actual_flags, ULONG wait_option)
{
  return tx_event_flags_get(&SX1262_flags, flags, TX_OR_CLEAR, actual_flags,  wait_option);
}

/*-----------------------------------------------------------------------------------------------------
  Разрешение прерываний от чипа радиочипа LoRA

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_Init_interrupts(void)
{


  R_ICU->IRQCRn_b[12].IRQMD = 0;  // Прерывание по спаду
  R_ICU->IRQCRn_b[13].IRQMD = 1;  // Прерывание по нарастанию

  rf_busy_irq12_int_num = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ICU_IRQ12);
  NVIC_SetPriority(rf_busy_irq12_int_num, INT_SSI0_TXI_PRIO);

  rf_dio1_irq13_int_num = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ICU_IRQ13);
  NVIC_SetPriority(rf_dio1_irq13_int_num, INT_SSI0_RXI_PRIO);


  R_ICU->IELSRn_b[rf_busy_irq12_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  NVIC_ClearPendingIRQ(rf_busy_irq12_int_num);
  NVIC_EnableIRQ(rf_busy_irq12_int_num);


  R_ICU->IELSRn_b[rf_dio1_irq13_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  NVIC_ClearPendingIRQ(rf_dio1_irq13_int_num);
  NVIC_EnableIRQ(rf_dio1_irq13_int_num);

  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void SX1262_convert_status(uint8_t val)
{
  SX1262_cbl.cmd_status =(val >> 4) & 0x07;
  SX1262_cbl.chip_mode =(val >> 1) & 0x07;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_get_status(void)
{
  ssp_err_t res;
  uint8_t buf[2];
  buf[0] = sx_GetStatus;
  buf[1] = 0;
  res = SX1262_write_read(buf,1);
  if (res != SSP_SUCCESS) return RES_ERROR;

  SX1262_convert_status(buf[1]);
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Ожидаем готовность чипа SX1262 к обработке команд

  \param void

  \return uint32_t - TX_SUCCESS   в случае успеха
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_Wait_ready_flag(ULONG wait_option)
{
  ULONG actual_flags = 0;
  return tx_event_flags_get(&SX1262_flags, FLAG_LORA_READY, TX_OR_CLEAR,&actual_flags,  wait_option);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void SX1262_Clear_ready_flag(void)
{
  tx_event_flags_set(&SX1262_flags,~FLAG_LORA_READY,TX_AND);
}


/*-----------------------------------------------------------------------------------------------------
  Ожидать готовности на линии BUSY


  \param timeout_ms

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_wait_ready(uint32_t timeout_ms)
{
  SX1262_Clear_ready_flag();
  // Если BUSY не в низком уровне то ожидать флага готовности
  if (RF_BUSY_STATE != 0)
  {
    if (SX1262_Wait_ready_flag(ms_to_ticks(timeout_ms)) != TX_SUCCESS) return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Запись буфера с данными

-----------------------------------------------------------------------------------------------------*/
ssp_err_t SX1262_write(uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  if (SX1262_opened == 1)
  {
    res = g_sf_spi_SX1262.p_api->write(g_sf_spi_SX1262.p_ctrl, buf, sz, SPI_BIT_WIDTH_8_BITS , 10); // Записываем адрес
    if (res != SSP_SUCCESS)
    {
      SX1262_cbl.wr_err_cnt++;
    }
    return res;
  }
  return SSP_ERR_NOT_OPEN;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
ssp_err_t SX1262_read(uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  if (SX1262_opened == 1)
  {
    res = g_sf_spi_SX1262.p_api->read(g_sf_spi_SX1262.p_ctrl, buf, sz, SPI_BIT_WIDTH_8_BITS , 10); // Записываем адрес
    if (res != SSP_SUCCESS)
    {
      SX1262_cbl.rd_err_cnt++;
    }
    return res;
  }
  return SSP_ERR_NOT_OPEN;
}

/*-----------------------------------------------------------------------------------------------------
  Запись буфера с данными

-----------------------------------------------------------------------------------------------------*/
ssp_err_t SX1262_write_read(uint8_t *buf, uint32_t sz)
{
  ssp_err_t           res;

  if (SX1262_opened == 1)
  {
    res = g_sf_spi_SX1262.p_api->writeRead(g_sf_spi_SX1262.p_ctrl, buf, buf, sz, SPI_BIT_WIDTH_8_BITS , 10); // Записываем адрес
    if (res != SSP_SUCCESS)
    {
      SX1262_cbl.wr_rd_err_cnt++;
    }
    return res;
  }
  return SSP_ERR_NOT_OPEN;
}

/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_io_transfer(uint8_t *buf, uint32_t sz)
{
  ssp_err_t    res;

  // Готовность перед передачей проверяем потому что BUSY может возникнуть асинхроноо по отношению к передаче команд
  if (SX1262_wait_ready(100) != RES_OK)
  {
    SX1262_cbl.ready_wait_err_cnt++;
    return RES_ERROR;
  }

  res = SX1262_write_read(buf,sz);
  if (res != SSP_SUCCESS)
  {
    return RES_ERROR;
  }

  // Готовность после передачи проверяем чтобы следующие команды не беспокоили до момента выполнения текущей команды
  if (SX1262_wait_ready(100) != RES_OK)
  {
    SX1262_cbl.ready_wait_err_cnt++;
    return RES_ERROR;
  }

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Чтение 8-и битного регитсра

  \param addr
  \param val

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_ReadRegister(uint16_t addr, uint8_t *val)
{
  uint8_t      buf[5];

  buf[0] = sx_ReadRegister;
  buf[1] =(addr >> 8) & 0xFF;
  buf[2] =(addr >> 0) & 0xFF;
  buf[3] = 0; // В этот элемент массива бедет записано значение статуса
  buf[4] = 0; // В этот элемент массива бедет записано значение регистра

  if (SX1262_io_transfer(buf,5) != RES_OK) return RES_ERROR;
  *val = buf[4];
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param reg_addr
  \param reg_val

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_WriteRegister(uint16_t reg_addr, uint8_t reg_val)
{
  uint8_t buf[4];
  buf[0] = sx_WriteRegister;
  buf[1] =(reg_addr >> 8) & 0xFF;
  buf[2] =(reg_addr >> 0) & 0xFF;
  buf[3] = reg_val;
  if (SX1262_io_transfer(buf,4) != RES_OK) return RES_ERROR;
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param offs
  \param buf
  \param sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_WriteBuffer(uint8_t offs, uint8_t *data, uint8_t sz)
{
  uint8_t buf[257];
  buf[0] = sx_WriteBuffer;
  buf[1] = offs;
  memcpy(&buf[2],data, sz);
  if (SX1262_io_transfer(buf,sz+2) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param offs
  \param data
  \param sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_ReadBuffer(uint8_t offs, uint8_t *data, uint8_t sz)
{
  uint8_t buf[258];
  memset(buf, 0 , 258);
  buf[0] = sx_ReadBuffer;
  buf[1] = offs;
  if (SX1262_io_transfer(buf,sz+3) != RES_OK) return RES_ERROR;
  SX1262_convert_status(buf[1]);
  memcpy(data, &buf[3], sz);
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Configure radio to control an RF switch from DIO2

  \param val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetDIO2AsRfSwitchCtrl(uint8_t val)
{
  uint8_t buf[2];
  buf[0] = sx_SetDIO2AsRfSwitchCtrl;
  buf[1] = 1;                                         // DIO2 is selected to be used to control an RF switch. In this case: DIO2 = 0 in SLEEP, STDBY_RX, STDBY_XOSC, FS and RX modes, DIO2 = 1 in TX mode
  if (SX1262_io_transfer(buf,2) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param tcxoVoltage
  \param delay

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  SX1262_SetDIO3AsTcxoCtrl(uint8_t tcxoVoltage, uint32_t delay)
{
  uint8_t buf[5];

  buf[0] = sx_SetDIO3AsTcxoCtrl;
  buf[1] = tcxoVoltage;               // DIO3 outputs 3.0 V to supply the TCXO
  buf[2] =((delay >> 16) & 0xFF);     // Delay duration = Delay(23:0) *15.625 µs
  buf[3] =((delay >>  8) & 0xFF);
  buf[4] =((delay >>  0) & 0xFF);
  if (SX1262_io_transfer(buf,5) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param mode

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetStandby(uint8_t mode)
{
  uint8_t buf[2];
  buf[0] = sx_SetStandby;
  buf[1] = mode;
  if (SX1262_io_transfer(buf,2) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  The calibration must be launched in STDBY_RC mode

  \param calibParam

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_Calibrate(uint8_t calibParam)
{
  uint8_t buf[2];
  buf[0] = sx_Calibrate;
  buf[1] = calibParam;
  if (SX1262_io_transfer(buf,2) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param mode

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetRegulatorMode(uint8_t mode)
{
  uint8_t buf[2];
  buf[0] = sx_SetRegulatorMode;
  buf[1] = mode;
  if (SX1262_io_transfer(buf,2) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param tx_base_address
  \param rx_base_address

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetBufferBaseAddress(uint8_t tx_base_address, uint8_t rx_base_address)
{
  uint8_t buf[3];
  buf[0] = sx_SetBufferBaseAddress;
  buf[1] = tx_base_address;
  buf[2] = rx_base_address;
  if (SX1262_io_transfer(buf,3) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param paDutyCycle  paDutyCycle controls the duty cycle (conduction angle) of PA
  \param hpMax        The valid range is between 0x00 and 0x07 and 0x07 is the maximum supported value for the SX1262 to achieve +22 dBm output power.
  \param deviceSel    0: SX1262
  \param paLut        reserved and always 0x01

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetPaConfig(uint8_t paDutyCycle, uint8_t hpMax, uint8_t deviceSel, uint8_t paLut)
{
  uint8_t buf[5];
  buf[0] = sx_SetPaConfig;
  buf[1] = paDutyCycle;
  buf[2] = hpMax;
  buf[3] = deviceSel;
  buf[4] = paLut;
  if (SX1262_io_transfer(buf,5) != RES_OK) return RES_ERROR;
  return RES_OK;
}



/*-----------------------------------------------------------------------------------------------------


  \param power     The output power is defined as power in dBm in a range of - 9 (0xF7) to +22 (0x16) dBm by step of 1 dB
  \param RampTime  The power ramp time is defined by the parameter RampTime as defined in the table

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetTxParams(uint8_t power, uint8_t RampTime)
{
  uint8_t buf[3];
  buf[0] = sx_SetTxParams;
  buf[1] = power;
  buf[2] = RampTime;
  if (SX1262_io_transfer(buf,3) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param Mask
  \param DIO1Mask
  \param DIO2Mask
  \param DIO3Mask

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetDioIrqParams(uint16_t Irq_Mask, uint16_t DIO1Mask, uint16_t DIO2Mask, uint16_t DIO3Mask)
{
  uint8_t buf[9];
  buf[0] = sx_SetDioIrqParams;
  buf[1] =((Irq_Mask >> 8) & 0xFF);
  buf[2] =((Irq_Mask >> 0) & 0xFF);
  buf[3] =((DIO1Mask >> 8) & 0xFF);
  buf[4] =((DIO1Mask >> 0) & 0xFF);
  buf[5] =((DIO2Mask >> 8) & 0xFF);
  buf[6] =((DIO2Mask >> 0) & 0xFF);
  buf[7] =((DIO3Mask >> 8) & 0xFF);
  buf[8] =((DIO3Mask >> 0) & 0xFF);
  if (SX1262_io_transfer(buf,9) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param freq

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_CalibrateImage(uint32_t freq)
{
  uint8_t buf[3];

  buf[0] = sx_CalibrateImage;
  if (freq > 900000000)
  {
    buf[1] = 0xE1;
    buf[2] = 0xE9;
  }
  else if (freq > 850000000)
  {
    buf[1] = 0xD7;
    buf[2] = 0xD8;
  }
  else if (freq > 770000000)
  {
    buf[1] = 0xC1;
    buf[2] = 0xC5;
  }
  else if (freq > 460000000)
  {
    buf[1] = 0x75;
    buf[2] = 0x81;
  }
  else if (freq > 425000000)
  {
    buf[1] = 0x6B;
    buf[2] = 0x6F;
  }
  if (SX1262_io_transfer(buf,3) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param freq

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetRfFrequency(uint32_t freq)
{
  uint8_t buf[5];

  if (SX1262_cbl.ImageCalibrated == 0)
  {
    if (SX1262_CalibrateImage(freq) != RES_OK) return RES_ERROR;
    SX1262_cbl.ImageCalibrated = 1;
  }

  uint32_t fr = ( uint32_t )(( double )freq / ( double )SX_FREQ_STEP);

  buf[0] = sx_SetRfFrequency;
  buf[1] =((fr >> 24) & 0xFF);
  buf[2] =((fr >> 16) & 0xFF);
  buf[3] =((fr >>  8) & 0xFF);
  buf[4] =((fr >>  0) & 0xFF);
  if (SX1262_io_transfer(buf,5) != RES_OK) return RES_ERROR;
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param pack_type

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetPacketType(uint8_t pack_type)
{
  uint8_t buf[2];
  buf[0] = sx_SetPacketType;
  buf[1] = pack_type;
  if (SX1262_io_transfer(buf,2) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
   Установка параметров модуляции в режиме LoRa

  \param sf     Spreading Factor
  \param bw     Signal Bandwidth
  \param cr     FEC Coding Rate
  \param ldro   Low Data Rate Optimization

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetModulationParams(uint8_t sf, uint8_t bw, uint8_t cr, uint8_t ldro)
{
  uint8_t buf[5];
  buf[0] = sx_SetModulationParams;
  buf[1] = sf;   // SF
  buf[2] = bw;   // BW
  buf[3] = cr;   // CR
  buf[4] = ldro; // LowDataRateOptimize
  if (SX1262_io_transfer(buf,5) != RES_OK) return RES_ERROR;
  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------
  Установка параметров пакета в режиме LoRa

  \param PreambleLength
  \param HeaderType
  \param Payloadlength
  \param CRCType
  \param InvertIQ

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetPacketParams(uint16_t PreambleLength, uint8_t HeaderType, uint8_t Payloadlength, uint8_t CRCType, uint8_t InvertIQ)
{
  uint8_t buf[7];
  buf[0] = sx_SetPacketParams;
  buf[1] =((PreambleLength >> 8) & 0xFF);
  buf[2] =((PreambleLength >> 0) & 0xFF);
  buf[3] = HeaderType;
  buf[4] = Payloadlength;
  buf[5] = CRCType;
  buf[6] = InvertIQ;
  if (SX1262_io_transfer(buf,7) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  Disable 0x00. Timer is stopped upon Sync Word or Header detection. Отработка таймаута прекращается только после примеа хидера
  Enable  0x01. Timer is stopped upon preamble detection             Отработка таймаута прекращается сразу после приема преамбулы

  \param StopOnPreambleParam

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_StopTimerOnPreamble(uint8_t StopOnPreambleParam)
{
  uint8_t buf[2];
  buf[0] = sx_StopTimerOnPreamble;
  buf[1] = StopOnPreambleParam;
  if (SX1262_io_transfer(buf,2) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  This command sets the number of symbols used by the modem to validate a successful reception.

  \param SymbNum

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetLoRaSymbNumTimeout(uint8_t SymbNum)
{
  uint8_t buf[2];
  buf[0] = sx_SetLoRaSymbNumTimeout;
  buf[1] = SymbNum;
  if (SX1262_io_transfer(buf,2) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param cadSymbolNum
  \param cadDetPeak
  \param cadDetMin
  \param cadExitMode
  \param cadTimeout

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetCadParams(uint8_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, uint8_t cadExitMode, uint32_t cadTimeout)
{
  uint8_t buf[8];
  buf[0] = sx_SetCadParams;
  buf[1] = cadSymbolNum;
  buf[2] = cadDetPeak;
  buf[3] = cadDetMin;
  buf[4] = cadExitMode;
  buf[5] =((cadTimeout >> 16) & 0xFF);
  buf[6] =((cadTimeout >>  8) & 0xFF);
  buf[7] =((cadTimeout >>  0) & 0xFF);
  if (SX1262_io_transfer(buf,8) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Set Channel Activity Detection


  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetCad(void)
{
  uint8_t buf[1];
  buf[0] = sx_SetCad;
  if (SX1262_io_transfer(buf,1) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param timeout

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetRx(uint32_t timeout)
{
  uint8_t buf[4];
  buf[0] = sx_SetRx;
  buf[1] = (timeout >> 16) & 0xFF;
  buf[2] = (timeout >>  8) & 0xFF;
  buf[3] = (timeout >>  0) & 0xFF;
  if (SX1262_io_transfer(buf,4) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param timeout

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetTx(uint32_t timeout)
{
  uint8_t buf[4];
  buf[0] = sx_SetTx;
  buf[1] = (timeout >> 16) & 0xFF;
  buf[2] = (timeout >>  8) & 0xFF;
  buf[3] = (timeout >>  0) & 0xFF;
  if (SX1262_io_transfer(buf,4) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  This command returns the value of the IRQ register.

  \param status
  \param irq_status

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_GetIrqStatus(uint8_t *p_status, uint16_t *p_irq_status)
{
  uint8_t buf[4];
  buf[0] = sx_GetIrqStatus;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  if (SX1262_io_transfer(buf,4) != RES_OK) return RES_ERROR;

  *p_status = buf[1];
  SX1262_convert_status(buf[1]);
  *p_irq_status =(buf[2]<<8)+ buf[3];
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param irq_status

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_ClearIrqStatus(uint16_t irq_status)
{
  uint8_t buf[4];
  buf[0] = sx_ClearIrqStatus;
  buf[1] =(irq_status >> 8) & 0xFF;
  buf[2] =(irq_status >> 0) & 0xFF;
  if (SX1262_io_transfer(buf,3) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_GetStats(void)
{
  uint8_t buf[8];
  buf[0] = sx_GetStats;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  buf[4] = 0;
  buf[5] = 0;
  buf[6] = 0;
  buf[7] = 0;
  if (SX1262_io_transfer(buf,8) != RES_OK) return RES_ERROR;
  SX1262_cbl.NbPktReceived  =(buf[2] << 8)+ buf[3];
  SX1262_cbl.NbPktCrcError  =(buf[4] << 8)+ buf[5];
  SX1262_cbl.NbPktHeaderErr =(buf[6] << 8)+ buf[7];
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_ResetStats(void)
{
  uint8_t buf[7];
  buf[0] = sx_ResetStats;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  buf[4] = 0;
  buf[5] = 0;
  buf[6] = 0;
  if (SX1262_io_transfer(buf,7) != RES_OK) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param PayloadLengthRx
  \param RxStartBufferPointer

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_GetRxBufferStatus(uint8_t *PayloadLengthRx, uint8_t *RxStartBufferPointer)
{
  uint8_t buf[4];
  buf[0] = sx_GetRxBufferStatus;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  if (SX1262_io_transfer(buf,4) != RES_OK) return RES_ERROR;
  SX1262_convert_status(buf[1]);
  *PayloadLengthRx      = buf[2];
  *RxStartBufferPointer = buf[3];
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param RssiPkt
  \param SnrPkt
  \param SignalRssiPkt

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_GetPacketStatus(uint8_t *RssiPkt, uint8_t *SnrPkt, uint8_t *SignalRssiPkt)
{
  uint8_t buf[5];
  buf[0] = sx_GetPacketStatus;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  buf[4] = 0;
  if (SX1262_io_transfer(buf,5) != RES_OK) return RES_ERROR;
  SX1262_convert_status(buf[1]);
  *RssiPkt       = buf[2];
  *SnrPkt        = buf[3];
  *SignalRssiPkt = buf[4];
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param RssiInst

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_GetRssiInst(uint8_t *RssiInst)
{
  uint8_t buf[5];
  buf[0] = sx_GetRssiInst;
  buf[1] = 0;
  buf[2] = 0;
  if (SX1262_io_transfer(buf,3) != RES_OK) return RES_ERROR;
  SX1262_convert_status(buf[1]);
  *RssiInst       = buf[2];
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param mode

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_SetRxTxFallbackMode(uint8_t mode)
{
  uint8_t buf[2];
  buf[0] = sx_SetRxTxFallbackMode;
  buf[1] = mode;
  if (SX1262_io_transfer(buf,2) != RES_OK) return RES_ERROR;
  return RES_OK;
}



/*-----------------------------------------------------------------------------------------------------
  Инициализация чипа LoRa

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_Init(void)
{
  SX1262_cbl.cad_symbols    = LORA_CAD_02_SYMBOL;
  SX1262_cbl.cad_det_peak   = LORA_CAD_DET_PEAK;
  SX1262_cbl.cad_det_min    = LORA_CAD_DET_MIN;
  SX1262_cbl.cad_exit       = LORA_CAD_RX;
  SX1262_cbl.cad_timeout    = LORA_CAD_TIMEOUT_MS;

  S7G2_LoRa_pins_init();

  if (SX1262_Init_interrupts() != RES_OK) return RES_ERROR;

  tx_event_flags_create(&SX1262_flags, "SX1262");

  //..........................................
  // Процедура сброса
  //..........................................
  RF_ANT_SW = 1; // Установаливаем режим управления антенным ключом только через вход CTRL (pin 4)
  RF_RST = 0;    // Подаем сигнал сброса
  Wait_ms(20);
  RF_RST = 1;    // Снимаем сигнал сброса
  Wait_ms(10);



  //..........................................
  // Выполнение Wakeup и переход в режим Standby
  //..........................................
  if (SX1262_get_status() != RES_OK) return RES_ERROR;
  SX1262_SetStandby(SX_STDBY_RC);

  //..........................................
  // Назначить функции пинам DIO1, DIO2, DIO3
  //..........................................
  SX1262_SetDIO2AsRfSwitchCtrl(1); // Включаем управление антеной

  SX1262_SetDIO3AsTcxoCtrl(SX_TCXO_CTRL_3_0V, RADIO_TCXO_SETUP_TIME);
  SX1262_Calibrate(0x7F);          // Калибровка всех узлов
  SX1262_SetStandby(SX_STDBY_XOSC);

  SX1262_SetRegulatorMode(SX_REG_DC_DC_LDO);

  SX1262_SetBufferBaseAddress(0,0);

  SX1262_SetPaConfig(0x04, 0x07, 0x00, 0x01);            // Capable +22 dBm output power
  SX1262_WriteRegister(SX_1262_OCP_CONFIGURATION, 0x38); // SX1262: 0x38 (140 mA)
  SX1262_SetTxParams(22, SX_RADIO_RAMP_200_US);          // Set +22 dBm output power

  SX1262_SetDioIrqParams(SX_IRQ_RADIO_ALL, SX_IRQ_RADIO_ALL, SX_IRQ_RADIO_NONE, SX_IRQ_RADIO_NONE);

  SX1262_SetRfFrequency(SX_RF_FREQUENCY);

  SX1262_SetPacketType(SX_PACKET_TYPE_LORA);
  SX1262_SetRxTxFallbackMode(LORA_FALLBACK_TO_STDBY_XOSC);

  SX1262_WriteRegister(SX_1262_LORA_SYNC_WORD_MSB,(LORA_MAC_PUBLIC_SYNCWORD >> 8) & 0xFF);
  SX1262_WriteRegister(SX_1262_LORA_SYNC_WORD_LSB, LORA_MAC_PUBLIC_SYNCWORD & 0xFF);

  SX1262_SetModulationParams(SX_LORA_SF7, SX_LORA_BW_125, SX_LORA_CR_4_5, 0);

  SX1262_SetPacketParams(LORA_PREAMBLE_LENGTH, LORA_EXPLICIT_HEADER, LORA_PAYLOAD_LEN, LORA_CRC_ON, LORA_NOT_INVERT_IQ);
  SX1262_StopTimerOnPreamble(1);
  SX1262_SetLoRaSymbNumTimeout(LORA_SYMBOL_TIMEOUT);

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SX1262_reg_count(void)
{
  return sizeof(SX1262_regs) / sizeof(SX1262_regs[0]);
}

/*-----------------------------------------------------------------------------------------------------


  \param indx

  \return T_da7217_reg_descr
-----------------------------------------------------------------------------------------------------*/
T_SX1262_reg_descr const* SX1262_get_reg_descr(uint8_t indx)
{
  if (indx >= SX1262_reg_count()) indx = 0;
  return &SX1262_regs[indx];
}

/*-----------------------------------------------------------------------------------------------------



  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t  SX1262_open(void)
{
  ssp_err_t           res = SSP_SUCCESS;
  if (SX1262_opened == 0)
  {
    res = g_sf_spi_SX1262.p_api->open(g_sf_spi_SX1262.p_ctrl, g_sf_spi_SX1262.p_cfg);
    if (res == SSP_SUCCESS)
    {
      SX1262_opened = 1;
    }
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t  SX1262_close(void)
{
  SX1262_opened = 0;
  return g_sf_spi_SX1262.p_api->close(g_sf_spi_SX1262.p_ctrl);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_SX1262_cbl*
-----------------------------------------------------------------------------------------------------*/
T_SX1262_cbl* SX1262_Get_cbl(void)
{
  return &SX1262_cbl;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void SX1262_Set_TX_done(void)
{
  tx_event_flags_set(&SX1262_flags,FLAG_LORA_TX_DONE,TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void SX1262_Set_RX_done(void)
{
  tx_event_flags_set(&SX1262_flags,FLAG_LORA_RX_DONE,TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* SX1262_Get_chip_mode_string(void)
{
  switch (SX1262_cbl.chip_mode)
  {
  case SX_CHPM_UNUSED      :
    return "UNUSED   ";
  case SX_CHPM_STBY_RC     :
    return "STBY_RC  ";
  case SX_CHPM_STBY_XOSC   :
    return "STBY_XOSC";
  case SX_CHPM_FS          :
    return "FS       ";
  case SX_CHPM_RX          :
    return "RX       ";
  case SX_CHPM_TX          :
    return "TX       ";
  default:
    return "undefined";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* SX1262_Get_last_command_status_string(void)
{
  switch (SX1262_cbl.cmd_status)
  {
  case SX_CMD_STAT_RESERVED                    :
    return "RESERVED                   ";
  case SX_CMD_STAT_DATA_IS_AVAILABLE_TO_HOST1  :
    return "DATA_IS_AVAILABLE_TO_HOST1 ";
  case SX_CMD_STAT_COMMAND_TIMEOUT2            :
    return "COMMAND_TIMEOUT2           ";
  case SX_CMD_STAT_COMMAND_PROCESSING_ERROR3   :
    return "COMMAND_PROCESSING_ERROR3  ";
  case SX_CMD_STAT_FAILURE_TO_EXECUTE_COMMAND4 :
    return "FAILURE_TO_EXECUTE_COMMAND4";
  case SX_CMD_STAT_COMMAND_TX_DONE5            :
    return "COMMAND_TX_DONE5           ";
  default:
    return "undefined";
  }
}

