// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-11
// 11:50:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define     S7V30_spi1
#include   "S7V30.h"

extern void    Set_LCD_CS(int32_t v);
static void  Transfer_callback(transfer_callback_args_t *p_args);


extern const transfer_instance_t DMA_CH4_transfer_instance;

dmac_instance_ctrl_t DMA_CH4_control_block;

transfer_info_t DMA_CH4_transfer_info =
{
  .dest_addr_mode      = TRANSFER_ADDR_MODE_FIXED,
  .repeat_area         = TRANSFER_REPEAT_AREA_SOURCE,
  .irq                 = TRANSFER_IRQ_EACH,
  .chain_mode          = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode       = TRANSFER_ADDR_MODE_INCREMENTED,
  .size                = TRANSFER_SIZE_2_BYTE,
  .mode                = TRANSFER_MODE_NORMAL,
  .p_dest              = (void *)&R_RSPI1->SPDR_HA,
  .p_src               = (void const *) NULL,
  .num_blocks          = 0,
  .length              = 0,
};

const transfer_on_dmac_cfg_t DMA_CH4_extended_configuration =
{
  .channel             = 4,
  .offset_byte         = 0,
};

const transfer_cfg_t DMA_CH4_configuration =
{
  .p_info              =&DMA_CH4_transfer_info,
  .activation_source   = ELC_EVENT_SPI1_TXI,
  .auto_enable         = false,
  .p_callback          = Transfer_callback,
  .p_context           =&DMA_CH4_transfer_instance,
  .irq_ipl             =(2),
  .p_extend            =&DMA_CH4_extended_configuration,
};

const transfer_instance_t DMA_CH4_transfer_instance =
{
  .p_ctrl        =&DMA_CH4_control_block,
  .p_cfg         =&DMA_CH4_configuration,
  .p_api         =&g_transfer_on_dmac
};


static uint32_t                spi1_initialized;
static IRQn_Type               spi1_rxi_int_num;
//static IRQn_Type               spi1_txi_int_num;  Не объявляем вектор события ELC_EVENT_spi1_TXI чтобы не было конфликтов с DMA
static IRQn_Type               spi1_tei_int_num;
static IRQn_Type               spi1_eri_int_num;
static IRQn_Type               spi1_idle_int_num;

#define FLAG_SPI1_IDLE         BIT(0)
#define FLAG_SPI1_TRANSFER_END BIT(1)

TX_EVENT_FLAGS_GROUP           spi1_flags;

/*-----------------------------------------------------------------------------------------------------
  Не объявляем вектор события ELC_EVENT_spi1_TXI чтобы не было конфликтов с DMA

  \param void
-----------------------------------------------------------------------------------------------------*/
static void spi1_init_interrupts(void)
{
  spi1_rxi_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SPI1_RXI);
  //  spi1_txi_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_spi1_TXI);
  spi1_tei_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SPI1_TEI);
  spi1_eri_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SPI1_ERI);
  spi1_idle_int_num   = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SPI1_IDLE);

  NVIC_DisableIRQ(spi1_rxi_int_num);
  //  NVIC_DisableIRQ(spi1_txi_int_num);
  NVIC_DisableIRQ(spi1_tei_int_num);
  NVIC_DisableIRQ(spi1_eri_int_num);
  NVIC_DisableIRQ(spi1_idle_int_num);

  NVIC_ClearPendingIRQ(spi1_rxi_int_num);
  //  NVIC_ClearPendingIRQ(spi1_txi_int_num);
  NVIC_ClearPendingIRQ(spi1_tei_int_num);
  NVIC_ClearPendingIRQ(spi1_eri_int_num);
  NVIC_ClearPendingIRQ(spi1_idle_int_num);

  NVIC_SetPriority(spi1_rxi_int_num ,  SPI1_PRIO);
  //  NVIC_SetPriority(spi1_txi_int_num ,  spi1_PRIO);
  NVIC_SetPriority(spi1_tei_int_num ,  SPI1_PRIO);
  NVIC_SetPriority(spi1_eri_int_num ,  SPI1_PRIO);
  NVIC_SetPriority(spi1_idle_int_num,  SPI1_PRIO);

  R_ICU->IELSRn_b[spi1_rxi_int_num].IR  = 0;  // Сбрасываем IR флаг в ICU
  //  R_ICU->IELSRn_b[spi1_txi_int_num].IR  = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[spi1_tei_int_num].IR  = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[spi1_eri_int_num].IR  = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[spi1_idle_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void     spi1_rxi_isr(void)
{

  R_ICU->IELSRn_b[spi1_rxi_int_num].IR  = 0;
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Прерывание срабатывает сразу же как только будет разрешено, если еще не началась передача по SPI
  Поэтому его не используем для индикации окончания работы шины при использовании DMA

  \param void
-----------------------------------------------------------------------------------------------------*/
void     spi1_tei_isr(void)
{
  NVIC_DisableIRQ(spi1_tei_int_num);
  R_ICU->IELSRn_b[spi1_tei_int_num].IR  = 0;
  __DSB();
}
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void     spi1_eri_isr(void)
{

  R_ICU->IELSRn_b[spi1_eri_int_num].IR  = 0;
  __DSB();
}
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void     spi1_idle_isr(void)
{
  Set_LCD_CS(1);
  R_RSPI1->SPCR2  = 0; // Прерывание по IDLE отключаем, чтобы не зациклится в этой процедуре обслуживания прерывания
  NVIC_DisableIRQ(spi1_idle_int_num);

  tx_event_flags_set(&spi1_flags, FLAG_SPI1_IDLE, TX_OR);
  R_ICU->IELSRn_b[spi1_idle_int_num].IR = 0;
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param p_args
-----------------------------------------------------------------------------------------------------*/
static void  Transfer_callback(transfer_callback_args_t *p_args)
{
  Set_LCD_CS(1);
  // Запрещаем прерывания Transmit Buffer Empty Interrupt
  uint8_t reg8 = 0
        + LSHIFT(0, 7) // SPRIE  | SPI Receive Buffer Full Interrupt Enable |
        + LSHIFT(1, 6) // SPE    | SPI Function Enable                      |
        + LSHIFT(0, 5) // SPTIE  | Transmit Buffer Empty Interrupt Enable   | Этот бит надо установить чтобы работал запрос на DMA
        + LSHIFT(0, 4) // SPEIE  | SPI Error Interrupt Enable               |
        + LSHIFT(1, 3) // MSTR   | SPI Master/Slave Mode Select             | 0: Select slave mode. 1: Select master mode.
        + LSHIFT(0, 2) // MODFEN | Mode Fault Error Detection Enable        |
        + LSHIFT(1, 1) // TXMD   | Communications Operating Mode Select     | 0: Select full-duplex synchronous serial communications. 1: Select serial communications with transmit-only
        + LSHIFT(1, 0) // SPMS   | SPI Mode Select                          | 0: Select SPI operation (4-wire method). 1: Select clock synchronous operation (3-wire method).
  ;
  R_RSPI1->SPCR = reg8;
  tx_event_flags_set(&spi1_flags, FLAG_SPI1_TRANSFER_END, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI1_init(void)
{
  uint8_t   reg8;
  uint16_t  reg16;

  if (spi1_initialized) return RES_OK;

  R_MSTP->MSTPCRB_b.MSTPB18 = 0; // Разрешаем работу модуля Serial Peripheral Interface 1
  DELAY_1us;


  R_RSPI1->SPCR  = 0;      // Выключаем работу SPI

  R_RSPI1->SSLP  = 0;      // Set SSL0..SSL3 signal to active low
  R_RSPI1->SPPCR = 0;      // 0: Set level output on MOSIn pin during MOSI idling to correspond to low. 0: Set MOSI output value to equal final data from previous transfer
  R_RSPI1->SPSCR = 0;      // SPI Sequence Length Specification. Используем только командный регистр 0

  R_RSPI1->SPBR  = 3;      // Тактовая частота = PCLKA_FREQ / 2*(n+1) = 120/6 = 20

  //
  reg8 = 0
        + LSHIFT(0, 7) // -         |
        + LSHIFT(0, 6) // -         |
        + LSHIFT(0, 5) // SPLW      | SPI Word Access/Halfword Access Specification. 0: Set SPDR_HA to valid for halfword access
        + LSHIFT(0, 4) // SPRDTD    | SPI Receive/Transmit Data Select               0: Read SPDR/SPDR_HA values from receive buffer
        + LSHIFT(0, 3) // -         |
        + LSHIFT(0, 2) // -         |
        + LSHIFT(0, 0) // SPFC[1:0] | Number of Frames Specification. 0 0: 1 frame
  ;
  R_RSPI1->SPDCR = reg8;

  R_RSPI1->SPCKD = 2; // RSPCK Delay Setting. 0 1 0: 3 RSPCK. SPCKD specifies the RSPCK delay, the period from the beginning of SSLni signal assertion to RSPCK oscillation, when the SPCMDm.SCKDEN bit is 1

  R_RSPI1->SSLND = 0; // SSLND specifies the SSL negation delay, the period from the transmission of a final RSPCK edge to the negation of the SSLni signal during a serial transfer by the SPI in master mode.

  R_RSPI1->SPND  = 2; // SPI Next-Access Delay Setting. 0 1 0: 3 RSPCK + 2 PCLKA. SPND specifies the next-access delay, the non-active period of the SSLni signal after termination of a serial transfer, when the SPCMDm.SPNDEN bit is 1

  R_RSPI1->SPCR2 = 0; // Проверку четности не выполняем и прерывние по IDLE не вызываем

  //
  reg16 = 0
         + LSHIFT(1  , 15) // SCKDEN    | RSPCK Delay Setting Enable. 1: Select RSPCK delay equal to the setting in the SPI Clock Delay Register (SPCKD).
         + LSHIFT(1  , 14) // SLNDEN    | SSL Negation Delay Setting Enable. 1: Select SSL negation delay equal to the setting in the SPI Slave Select Negation Delay Register (SSLND).
         + LSHIFT(1  , 13) // SPNDEN    | SPI Next-Access Delay Enable. 1: Select next-access delay equal to the setting in the SPI NextAccess Delay Register (SPND).
         + LSHIFT(0  , 12) // LSBF      | SPI LSB First. 0: MSB first, 1: LSB first
         + LSHIFT(0xF,  8) // SPB[3:0]  | SPI Data Length Setting.  0111: 8 bits, 1111: 16 bits
         + LSHIFT(0  ,  7) // SSLKP     | SSL Signal Level Keeping.  0: Negate all SSL signals on completion of transfer
         + LSHIFT(0  ,  4) // SSLA[2:0] | SSL Signal Assertion Setting. 000: SSL0 ... 011: SSL3
         + LSHIFT(1  ,  2) // BRDV[1:0] | Bit Rate Division Setting. Снижаем частоту в 2 раза до 15 Мгц
         + LSHIFT(0  ,  1) // CPOL      | RSPCK Polarity Setting
         + LSHIFT(0  ,  0) // CPHA      | RSPCK Phase Setting
  ;
  R_RSPI1->SPCMDn[0] = reg16;

  reg8 = 0
        + LSHIFT(0, 7) // SPRIE  | SPI Receive Buffer Full Interrupt Enable |
        + LSHIFT(1, 6) // SPE    | SPI Function Enable                      |
        + LSHIFT(0, 5) // SPTIE  | Transmit Buffer Empty Interrupt Enable   |
        + LSHIFT(0, 4) // SPEIE  | SPI Error Interrupt Enable               |
        + LSHIFT(1, 3) // MSTR   | SPI Master/Slave Mode Select             | 0: Select slave mode. 1: Select master mode.
        + LSHIFT(0, 2) // MODFEN | Mode Fault Error Detection Enable        |
        + LSHIFT(1, 1) // TXMD   | Communications Operating Mode Select     | 0: Select full-duplex synchronous serial communications. 1: Select serial communications with transmit-only
        + LSHIFT(1, 0) // SPMS   | SPI Mode Select                          | 0: Select SPI operation (4-wire method). 1: Select clock synchronous operation (3-wire method).
  ;
  R_RSPI1->SPCR = reg8;


  tx_event_flags_create(&spi1_flags, "spi1_flags");
  spi1_init_interrupts();

  spi1_initialized = 1;
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Дисплей может работать до частоты 20 МГц
  Послылаем однобайтные команды
  CPOL = 0
  CPHA = 0

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI1_set_for_byte_transfer_to_display(void)
{
  uint8_t   reg8;
  uint16_t  reg16;

  if (R_RSPI1->SPSR & BIT(1)) return RES_ERROR;
  R_RSPI1->SPCR  = 0;

  reg16 = 0
         + LSHIFT(1  , 15) // SCKDEN    | RSPCK Delay Setting Enable. 1: Select RSPCK delay equal to the setting in the SPI Clock Delay Register (SPCKD).
         + LSHIFT(1  , 14) // SLNDEN    | SSL Negation Delay Setting Enable. 1: Select SSL negation delay equal to the setting in the SPI Slave Select Negation Delay Register (SSLND).
         + LSHIFT(1  , 13) // SPNDEN    | SPI Next-Access Delay Enable. 1: Select next-access delay equal to the setting in the SPI NextAccess Delay Register (SPND).
         + LSHIFT(0  , 12) // LSBF      | SPI LSB First. 0: MSB first, 1: LSB first
         + LSHIFT(0x7,  8) // SPB[3:0]  | SPI Data Length Setting.  0111: 8 bits, 1111: 16 bits
         + LSHIFT(0  ,  7) // SSLKP     | SSL Signal Level Keeping.  0: Negate all SSL signals on completion of transfer
         + LSHIFT(0  ,  4) // SSLA[2:0] | SSL Signal Assertion Setting. 000: SSL0 ... 011: SSL3
         + LSHIFT(0  ,  2) // BRDV[1:0] | Bit Rate Division Setting. Не снижаем чатоту
         + LSHIFT(0  ,  1) // CPOL      | RSPCK Polarity Setting     0: Set RSPCK low during idle
         + LSHIFT(0  ,  0) // CPHA      | RSPCK Phase Setting.       0: Select data sampling on leading edge, data change on trailing edge
  ;
  R_RSPI1->SPCMDn[0] = reg16;

  reg8 = 0
        + LSHIFT(0, 7) // SPRIE  | SPI Receive Buffer Full Interrupt Enable |
        + LSHIFT(1, 6) // SPE    | SPI Function Enable                      |
        + LSHIFT(0, 5) // SPTIE  | Transmit Buffer Empty Interrupt Enable   |
        + LSHIFT(0, 4) // SPEIE  | SPI Error Interrupt Enable               |
        + LSHIFT(1, 3) // MSTR   | SPI Master/Slave Mode Select             | 0: Select slave mode. 1: Select master mode.
        + LSHIFT(0, 2) // MODFEN | Mode Fault Error Detection Enable        |
        + LSHIFT(1, 1) // TXMD   | Communications Operating Mode Select     | 0: Select full-duplex synchronous serial communications. 1: Select serial communications with transmit-only
        + LSHIFT(1, 0) // SPMS   | SPI Mode Select                          | 0: Select SPI operation (4-wire method). 1: Select clock synchronous operation (3-wire method).
  ;
  R_RSPI1->SPCR = reg8;

  NVIC_EnableIRQ(spi1_idle_int_num);

  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------
  Дисплей может работать до частоты 20 МГц
  Послылаем однобайтные команды
  CPOL = 0
  CPHA = 0

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI1_set_for_hword_transfer_to_display(void)
{
  uint8_t   reg8;
  uint16_t  reg16;


  if (R_RSPI1->SPSR & BIT(1)) return RES_ERROR;
  R_RSPI1->SPCR  = 0;

  reg16 = 0
         + LSHIFT(1  , 15) // SCKDEN    | RSPCK Delay Setting Enable. 1: Select RSPCK delay equal to the setting in the SPI Clock Delay Register (SPCKD).
         + LSHIFT(1  , 14) // SLNDEN    | SSL Negation Delay Setting Enable. 1: Select SSL negation delay equal to the setting in the SPI Slave Select Negation Delay Register (SSLND).
         + LSHIFT(1  , 13) // SPNDEN    | SPI Next-Access Delay Enable. 1: Select next-access delay equal to the setting in the SPI NextAccess Delay Register (SPND).
         + LSHIFT(0  , 12) // LSBF      | SPI LSB First. 0: MSB first, 1: LSB first
         + LSHIFT(0xF,  8) // SPB[3:0]  | SPI Data Length Setting.  0111: 8 bits, 1111: 16 bits
         + LSHIFT(0  ,  7) // SSLKP     | SSL Signal Level Keeping.  0: Negate all SSL signals on completion of transfer
         + LSHIFT(0  ,  4) // SSLA[2:0] | SSL Signal Assertion Setting. 000: SSL0 ... 011: SSL3
         + LSHIFT(0  ,  2) // BRDV[1:0] | Bit Rate Division Setting. Не снижаем чатоту
         + LSHIFT(0  ,  1) // CPOL      | RSPCK Polarity Setting
         + LSHIFT(0  ,  0) // CPHA      | RSPCK Phase Setting
  ;
  R_RSPI1->SPCMDn[0] = reg16;

  reg8 = 0
        + LSHIFT(0, 7) // SPRIE  | SPI Receive Buffer Full Interrupt Enable |
        + LSHIFT(1, 6) // SPE    | SPI Function Enable                      |
        + LSHIFT(0, 5) // SPTIE  | Transmit Buffer Empty Interrupt Enable   |
        + LSHIFT(0, 4) // SPEIE  | SPI Error Interrupt Enable               |
        + LSHIFT(1, 3) // MSTR   | SPI Master/Slave Mode Select             | 0: Select slave mode. 1: Select master mode.
        + LSHIFT(0, 2) // MODFEN | Mode Fault Error Detection Enable        |
        + LSHIFT(1, 1) // TXMD   | Communications Operating Mode Select     | 0: Select full-duplex synchronous serial communications. 1: Select serial communications with transmit-only
        + LSHIFT(1, 0) // SPMS   | SPI Mode Select                          | 0: Select SPI operation (4-wire method). 1: Select clock synchronous operation (3-wire method).
  ;
  R_RSPI1->SPCR = reg8;

  NVIC_EnableIRQ(spi1_idle_int_num);
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Дисплей может работать до частоты 20 МГц
  Послылаем буффер данных с помощью DMA
  CPOL = 0
  CPHA = 0

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI1_set_for_buffer_transfer_to_display(void)
{
  uint8_t   reg8;
  uint16_t  reg16;

  while (R_RSPI1->SPSR & BIT(1))
  {
    __no_operation(); // Здесь можем задержаться не более чем на 14 циклов, по результатам измерений.
  }
  R_RSPI1->SPCR   = 0;
  R_RSPI1->SPCR2  = 0;

  reg16 = 0
         + LSHIFT(0  , 15) // SCKDEN    | RSPCK Delay Setting Enable. 1: Select RSPCK delay equal to the setting in the SPI Clock Delay Register (SPCKD).
         + LSHIFT(0  , 14) // SLNDEN    | SSL Negation Delay Setting Enable. 1: Select SSL negation delay equal to the setting in the SPI Slave Select Negation Delay Register (SSLND).
         + LSHIFT(0  , 13) // SPNDEN    | SPI Next-Access Delay Enable. 1: Select next-access delay equal to the setting in the SPI NextAccess Delay Register (SPND).
         + LSHIFT(0  , 12) // LSBF      | SPI LSB First. 0: MSB first, 1: LSB first
         + LSHIFT(0xF,  8) // SPB[3:0]  | SPI Data Length Setting.  0111: 8 bits, 1111: 16 bits
         + LSHIFT(1  ,  7) // SSLKP     | SSL Signal Level Keeping.  0: Negate all SSL signals on completion of transfer. 1: Keep SSL signal level from the end of transfer until the beginning of the next access.
         + LSHIFT(0  ,  4) // SSLA[2:0] | SSL Signal Assertion Setting. 000: SSL0 ... 011: SSL3
         + LSHIFT(0  ,  2) // BRDV[1:0] | Bit Rate Division Setting. Не снижаем чатоту
         + LSHIFT(0  ,  1) // CPOL      | RSPCK Polarity Setting
         + LSHIFT(0  ,  0) // CPHA      | RSPCK Phase Setting
  ;
  R_RSPI1->SPCMDn[0] = reg16;


  reg8 = 0
        + LSHIFT(0, 7) // SPRIE  | SPI Receive Buffer Full Interrupt Enable |
        + LSHIFT(1, 6) // SPE    | SPI Function Enable                      |
        + LSHIFT(1, 5) // SPTIE  | Transmit Buffer Empty Interrupt Enable   | Этот бит надо установить чтобы работал запрос на DMA
        + LSHIFT(0, 4) // SPEIE  | SPI Error Interrupt Enable               |
        + LSHIFT(1, 3) // MSTR   | SPI Master/Slave Mode Select             | 0: Select slave mode. 1: Select master mode.
        + LSHIFT(0, 2) // MODFEN | Mode Fault Error Detection Enable        |
        + LSHIFT(1, 1) // TXMD   | Communications Operating Mode Select     | 0: Select full-duplex synchronous serial communications. 1: Select serial communications with transmit-only
        + LSHIFT(1, 0) // SPMS   | SPI Mode Select                          | 0: Select SPI operation (4-wire method). 1: Select clock synchronous operation (3-wire method).
  ;
  R_RSPI1->SPCR = reg8;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI1_send_byte_to_display(uint8_t b)
{
  uint32_t res;
  ULONG actual_flags;

  if (SPI1_set_for_byte_transfer_to_display() != RES_OK) return RES_ERROR;

  Set_LCD_CS(0);
  uint16_t *p_reg = (uint16_t *)&R_RSPI1->SPDR;
  *p_reg  = b;
  R_RSPI1->SPCR2 = BIT(2); // Разрешаем прерывания по IDLE

  res = tx_event_flags_get(&spi1_flags, FLAG_SPI1_IDLE, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(10));
  Set_LCD_CS(1);

  if (res != TX_SUCCESS)
  {
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param b

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI1_send_hword_to_display(uint16_t w)
{
  uint32_t res;
  ULONG actual_flags;

  if (SPI1_set_for_hword_transfer_to_display() != RES_OK) return RES_ERROR;

  Set_LCD_CS(0);
  R_RSPI1->SPDR_HA  = w;
  R_RSPI1->SPCR2 = BIT(2); // Разрешаем прерывания по IDLE

  res = tx_event_flags_get(&spi1_flags, FLAG_SPI1_IDLE, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(10));
  Set_LCD_CS(1);

  if (res != TX_SUCCESS)
  {
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  На частоте SPI 20 МГц пересылка экрана 240x240 16 бит длиться 54 мс
  Эффективная скорость 17 мегабит в сек

  \param buf
  \param sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI1_send_buff_to_display(uint16_t *buf, uint32_t sz)
{
  uint32_t  res;
  ULONG     actual_flags;

  DMA_CH4_transfer_info.p_src  = buf;
  DMA_CH4_transfer_info.length = sz/2;
  if (DMA_CH4_transfer_instance.p_api->open(DMA_CH4_transfer_instance.p_ctrl,DMA_CH4_transfer_instance.p_cfg) != SSP_SUCCESS)
  {
    return RES_ERROR;
  }


  if (DMA_CH4_transfer_instance.p_api->enable(DMA_CH4_transfer_instance.p_ctrl) != SSP_SUCCESS)
  {
    DMA_CH4_transfer_instance.p_api->close(DMA_CH4_transfer_instance.p_ctrl);
    return RES_ERROR;
  }

  Set_LCD_CS(0);
  res = SPI1_set_for_buffer_transfer_to_display();
  if (res == RES_OK)
  {
    res = tx_event_flags_get(&spi1_flags, FLAG_SPI1_TRANSFER_END, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(200));
    if (res != TX_SUCCESS)
    {
      res = RES_ERROR;
    }
  }
  Set_LCD_CS(1);

  DMA_CH4_transfer_instance.p_api->close(DMA_CH4_transfer_instance.p_ctrl);
  if (res != RES_OK)
  {
    return RES_ERROR;
  }
  return RES_OK;
}

