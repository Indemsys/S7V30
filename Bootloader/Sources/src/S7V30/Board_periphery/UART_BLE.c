#define    UART_BLE_VECT_DEFINES
#include   "App.h"

static IRQn_Type        bsci_rxi_int_num;
static IRQn_Type        bsci_txi_int_num;
static IRQn_Type        bsci_tei_int_num;
static IRQn_Type        bsci_eri_int_num;
static IRQn_Type        bsci_am_int_num;
#ifdef UART_BLE_USE_DMA
static IRQn_Type        bsci_dma_rx_int_num;
static IRQn_Type        bsci_dma_tx_int_num;
#endif

static T_uart_byte_recieve_handler bsci_byte_receive_handler;

TX_EVENT_FLAGS_GROUP    bsci_flags;

static T_uart_cbl ucbl;


#define  BSCI_TX_COMPLETE  BIT(0)
#define  BSCI_RX_COMPLETE  BIT(1)

T_SCI_error_stat BLE_UART_err_stat;
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void UART_BLE_Clear_Interrupts(void)
{
  bsci_rxi_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_BSCI_RXI);
  bsci_txi_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_BSCI_TXI);
  bsci_tei_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_BSCI_TEI);
  bsci_eri_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_BSCI_ERI);
  bsci_am_int_num     = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_BSCI_AM );
  #ifdef UART_BLE_USE_DMA
  bsci_dma_rx_int_num = (IRQn_Type)Find_IRQ_number_by_evt(UART_BLE_RX_DMA_ELC_EVENT);
  bsci_dma_tx_int_num = (IRQn_Type)Find_IRQ_number_by_evt(UART_BLE_TX_DMA_ELC_EVENT);
  #endif


  NVIC_DisableIRQ(bsci_rxi_int_num);
  NVIC_DisableIRQ(bsci_txi_int_num);
  NVIC_DisableIRQ(bsci_tei_int_num);
  NVIC_DisableIRQ(bsci_eri_int_num);
  NVIC_DisableIRQ(bsci_am_int_num);
  #ifdef UART_BLE_USE_DMA
  NVIC_DisableIRQ(bsci_dma_rx_int_num);
  NVIC_DisableIRQ(bsci_dma_tx_int_num);
  #endif

  NVIC_ClearPendingIRQ(bsci_rxi_int_num);
  NVIC_ClearPendingIRQ(bsci_txi_int_num);
  NVIC_ClearPendingIRQ(bsci_tei_int_num);
  NVIC_ClearPendingIRQ(bsci_eri_int_num);
  NVIC_ClearPendingIRQ(bsci_am_int_num);
  #ifdef UART_BLE_USE_DMA
  NVIC_ClearPendingIRQ(bsci_dma_rx_int_num);
  NVIC_ClearPendingIRQ(bsci_dma_tx_int_num);
  #endif


  R_ICU->IELSRn_b[bsci_rxi_int_num   ].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[bsci_txi_int_num   ].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[bsci_tei_int_num   ].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[bsci_eri_int_num   ].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[bsci_am_int_num    ].IR = 0;  // Сбрасываем IR флаг в ICU
  #ifdef UART_BLE_USE_DMA
  R_ICU->IELSRn_b[bsci_dma_rx_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[bsci_dma_tx_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  #endif
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void BSCI_set_and_enable_interrupts(void)
{
  UART_BLE_Clear_Interrupts();

  NVIC_SetPriority(bsci_rxi_int_num    ,  UART_BLE_PRIO);
  NVIC_SetPriority(bsci_txi_int_num    ,  UART_BLE_PRIO);
  NVIC_SetPriority(bsci_tei_int_num    ,  UART_BLE_PRIO);
  NVIC_SetPriority(bsci_eri_int_num    ,  UART_BLE_PRIO);
  NVIC_SetPriority(bsci_am_int_num     ,  UART_BLE_PRIO);
  #ifdef UART_BLE_USE_DMA
  NVIC_SetPriority(bsci_dma_rx_int_num ,  UART_BLE_PRIO);
  NVIC_SetPriority(bsci_dma_tx_int_num ,  UART_BLE_PRIO);
  #endif

  NVIC_EnableIRQ(bsci_rxi_int_num);
  NVIC_EnableIRQ(bsci_txi_int_num);
  NVIC_EnableIRQ(bsci_tei_int_num);
  NVIC_EnableIRQ(bsci_eri_int_num);
  NVIC_EnableIRQ(bsci_am_int_num);
  #ifdef UART_BLE_USE_DMA
  NVIC_EnableIRQ(bsci_dma_rx_int_num);
  NVIC_EnableIRQ(bsci_dma_tx_int_num);
  #endif


}

#define SCI_SSR_DR    BIT(0)  // Receive Data Ready Flag
#define SCI_SSR_PER   BIT(3)  // Parity Error Flag
#define SCI_SSR_FER   BIT(4)  // Framing Error Flag
#define SCI_SSR_ORER  BIT(5)  // Overrun Error Flag
#define SCI_SSR_RDF   BIT(6)  // Receive FIFO Data Full Flag
#define SCI_SSR_TDFE  BIT(7)  // Transmit FIFO Data Empty Flag

#ifdef UART_BLE_USE_DMA

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void bsci_rxi_isr(void)
{
  volatile uint8_t ssr = R_BSCI->SSR; // Этот регистр надо прочитать, чтобы возникало прерывание по приему одиночного байта
  uint8_t          n   = R_BSCI->FDR_b.R;
  volatile uint8_t b;

  //ITM_EVENT16(3, ssr | 0x1000);
  if (ssr & (SCI_SSR_PER | SCI_SSR_FER | SCI_SSR_ORER))
  {
      b = R_BSCI->FRDRL_b.RDAT;
      R_BSCI->SSR = (ssr & (SCI_SSR_DR | SCI_SSR_PER | SCI_SSR_FER | SCI_SSR_ORER | SCI_SSR_DR | SCI_SSR_RDF)) ^ 0xFF;
      //ITM_EVENT16(3, ssr | 0x8000);
  }
  else
  {
    while (n > 0)
    {
      //ITM_EVENT16(3, n);
      if (bsci_byte_receive_handler(R_BSCI->FRDRL_b.RDAT) == 0)
      {
        //ITM_EVENT16(3, n | 0x2000);
        R_BSCI->SCR_b.RIE   = 0;
        break;
      }
      n--;
    }
    // Здесь этот сброс обязателен, иначе не возникнет следующее прерывание
    R_BSCI->SSR = (ssr & (SCI_SSR_DR | SCI_SSR_RDF)) ^ 0xFF; // Сбрасываем флаг превышения уровня триггера наполненности FIFO - RDF Receive FIFO Data Full Flag
  }
  //ITM_EVENT16(3, 0);
  R_ICU->IELSRn_b[bsci_rxi_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

#else
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void bsci_rxi_isr(void)
{
  volatile uint8_t ssr = R_BSCI->SSR; // Этот регистр надо прочитать, чтобы возникало прерывание по приему одиночного байта
  uint8_t          n   = R_BSCI->FDR_b.R;
  uint8_t          b   = 0;

  if (ssr & (SCI_SSR_PER | SCI_SSR_FER | SCI_SSR_ORER))
  {
    R_BSCI->SSR = ssr &  ~(SCI_SSR_DR | SCI_SSR_PER | SCI_SSR_FER | SCI_SSR_ORER);
  }
  else if (ucbl.rx_data_left_sz == 0)
  {
    if (bsci_byte_receive_handler)
    {
      while (n > 0)
      {
        b = R_BSCI->FRDRL_b.RDAT;
        if (bsci_byte_receive_handler(b) == 0)
        {
          R_BSCI->SCR_b.RIE   = 0;
          break;
        }
        n--;
      }
    }
  }
  else
  {
    // Читать данные накопленные в FIFO
    while (n > 0)
    {
      b = R_BSCI->FRDRL_b.RDAT;
      *ucbl.rx_data_buf = b;
      ucbl.rx_data_left_sz--;
      if (ucbl.rx_data_left_sz == 0)
      {
        // Передать событие об окончании приема
        R_BSCI->SCR_b.RIE   = 0;
        tx_event_flags_set(&bsci_flags, BSCI_RX_COMPLETE, TX_OR);
        break;
      }
      ucbl.rx_data_buf++;
      n--;
    }

    R_BSCI->SSR = ssr & ~(SCI_SSR_DR | SCI_SSR_RDF); // Сбрасываем флаг превышения уровня триггера наполненности FIFO - RDF Receive FIFO Data Full Flag
  }

  R_ICU->IELSRn_b[bsci_rxi_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}
#endif



/*-----------------------------------------------------------------------------------------------------
  Transmit data empty
  Прерывание происходит при старте передатчика с одновременной установкой битов SCR.TE and SCR.TIE bit to 1
  и при перемещении данных из регистра передатчика TDR в сдвиговый регистр TSR
  либо в момент гогда в FIFO передатчика становиться меньше данных чем установленная граница

  \param void
-----------------------------------------------------------------------------------------------------*/
void bsci_txi_isr(void)
{
  volatile uint8_t ssr = R_BSCI->SSR;
  // Читаем количество ячеек на передачу свободных в FIFO
  uint32_t  fifo_sz = 16 - R_BSCI->FDR_b.T;

  if (ucbl.tx_data_sz > 0)
  {
    while (fifo_sz > 0)
    {
      R_BSCI->TDRHL_b.TDRL = *ucbl.tx_data_buf;
      fifo_sz--;
      ucbl.tx_data_buf++;
      ucbl.tx_data_sz--;
      if (ucbl.tx_data_sz == 0)
      {
        R_BSCI->SCR_b.TIE  = 0;
        break;
      }
    }
  }
  else
  {
    R_BSCI->SCR_b.TIE  = 0;
  }
  R_BSCI->SSR = (uint8_t)(~SCI_SSR_TDFE); // Сбрасываем флаг TDFE Transmit FIFO Data Empty Flag


  R_ICU->IELSRn_b[bsci_txi_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Transmit end
  Прерывание происходит когда все данные были отосланы

  \param void
-----------------------------------------------------------------------------------------------------*/
void bsci_tei_isr(void)
{
  uint8_t ssr = R_BSCI->SSR;

  R_BSCI->SSR = ssr ^ 0xFF; // Сбрасываем все флаги

  //ITM_EVENT8(2, ssr);
  tx_event_flags_set(&bsci_flags, BSCI_TX_COMPLETE, TX_OR);

  R_ICU->IELSRn_b[bsci_tei_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Receive error

  \param void
-----------------------------------------------------------------------------------------------------*/
void bsci_eri_isr(void)
{
  uint8_t ssr = R_BSCI->SSR;

  R_BSCI->SSR = ssr ^ 0x3F; // Сбрасываем флаги ошибок

  R_ICU->IELSRn_b[bsci_eri_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Address match

  \param void
-----------------------------------------------------------------------------------------------------*/
void bsci_am_isr(void)
{
  R_ICU->IELSRn_b[bsci_am_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}


#ifdef UART_BLE_USE_DMA
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void uart_ble_rx_dmac_int_isr(void)
{
  tx_event_flags_set(&bsci_flags, BSCI_RX_COMPLETE, TX_OR);
  R_ICU->IELSRn_b[bsci_dma_rx_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void uart_ble_tx_dmac_int_isr(void)
{
  //ITM_EVENT8(2, 0);
  tx_event_flags_set(&bsci_flags, BSCI_TX_COMPLETE, TX_OR);
  R_ICU->IELSRn_b[bsci_dma_tx_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}
#endif

/*-----------------------------------------------------------------------------------------------------

   Тактирование от PCLKA = 120 МГц

   Сигнал CTS слушает сигнал от внешнего дивайса.
   Если сигнал в высоком уровне то передача и прием прекращаются.
   Сигнал RTS не обслуживается

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t UART_BLE_init(uint32_t  baud, uint8_t  parity, uint8_t  stops)
{
  uint8_t   reg8;
  uint16_t  reg16;
  uint8_t   en_parity       = 0;
  uint8_t   v_parity        = 0;
  uint8_t   v_char_len_bit0 = 0;
  uint8_t   v_char_len_bit1 = 0;
  uint8_t   cks    = 0;
  uint8_t   absce  = 0;
  uint8_t   brme   = 0;  // Enable bit rate modulation function.
  uint8_t   N;
  uint8_t   M;

  BSCI_EN_FLAG = 0; // Разрешаем работу модуля SCI
  DELAY_1us;

  if (SCI_Find_best_divider(baud,&brme,&absce,&cks,&N,&M) != RES_OK) return RES_ERROR;

  // Держим все в выключенном состоянии
  reg8 = 0
         + LSHIFT(0, 7) // TIE      | Transmit Interrupt Enable
         + LSHIFT(0, 6) // RIE      | Receive Interrupt Enable
         + LSHIFT(0, 5) // TE       | Transmit Enable
         + LSHIFT(0, 4) // RE       | Receive Enable
         + LSHIFT(0, 3) // MPIE     | Multi-Processor Interrupt Enable
         + LSHIFT(0, 2) // TEIE     | Transmit End Interrupt Enable
         + LSHIFT(0, 0) // CKE[1:0] | Clock Enable. 0 0: On-chip baud rate generator
  ;
  R_BSCI->SCR = reg8;


  if (parity == SCI_NO_PRITY)
  {
    en_parity  = 0;
    v_char_len_bit0 = 0; // 1 0: Transmit/receive in 8-bit data
    v_char_len_bit1 = 1;
  }
  else
  {
    en_parity  = 1;
    v_char_len_bit0 = 0; // 0 0: Transmit/receive in 9-bit data
    v_char_len_bit1 = 0;

    if (parity == SCI_EVEN_PRITY)
    {
      v_parity = 0;
    }
    else
    {
      v_parity = 1;
    }
  }


  R_BSCI->SIMR1_b.IICM = 0; //  Asynchronous mode (including multi-processor mode), clock synchronous mode, or simple SPI mode
  R_BSCI->SPMR = BIT(1);    //  Устанавливаем работу сигнала CTS для управления потоком


  //
  reg8 = 0
         + LSHIFT(1,               7) // BCP2     | Base Clock Pulse 2. Selects the number of base clock cycles in combination with the SMR_SMCI.BCP[1:0] bits in smart card interface mode
         + LSHIFT(1,               6) // reserved |
         + LSHIFT(1,               5) // reserved |
         + LSHIFT(v_char_len_bit1, 4) // CHR1     | Character Length 1. 1 0: Transmit/receive in 8-bit data (initial value)
         + LSHIFT(0,               3) // SDIR     | Transmitted/Received Data Transfer Direction. 0: Transfer LSB-first, 1: Transfer MSB-first
         + LSHIFT(0,               2) // SINV     | Transmitted/Received Data Invert
         + LSHIFT(1,               1) // reserved |
         + LSHIFT(0,               0) // SMIF     | 0: Non-smart card interface mode (asynchronous mode, clock synchronous mode, simple SPI mode, or simple IIC mode)
  ;
  R_BSCI->SCMR = reg8;



  // Выбираем частоту тактирования так чтобы делитель тактирования помещался в диапазод до 255
  // Для 9600 делитель будет равен 96.65625
  reg8 = 0
         + LSHIFT(0,               7) // CM        | Communication Mode   | 0: Asynchronous or simple IIC mode, 1: Clock synchronous or simple SPI mode.
         + LSHIFT(v_char_len_bit0, 6) // CHR       | Character Length     | Only valid in asynchronous mode.
                                      //                                    0 0: Transmit/receive in 9-bit data,
         //                                    0 1: Transmit/receive in 9-bit data,
         //                                    1 0: Transmit/receive in 8-bit data,
         //                                    1 1: Transmit/receive in 7-bit data
         + LSHIFT(en_parity,       5) // PE        | Parity Enable        | Only valid in asynchronous mode. 0: Do not add parity bit, 1: Add parity bit.
         + LSHIFT(v_parity,        4) // PM        | Parity Mode          | 0: Even parity, 1: Odd parity.
         + LSHIFT(stops,           3) // STOP      | Stop Bit Length      | 0: 1 stop bit,  1: 2 stop bits.
         + LSHIFT(0,               2) // MP        | Multi-Processor Mode | 0: Disable multi-processor communications function
         + LSHIFT(cks,             0) // CKS[1:0]  | Clock Select         | 0 0: PCLKA clock (n = 0), 0 1: PCLKA/4 clock (n = 1)
  ;
  R_BSCI->SMR = reg8;


  // В этом регистре устанавливаются биты влияющие на формулу вычисления битовой скорсти
  reg8 = 0
         + LSHIFT(0, 7) // RXDESEL | Asynchronous Start Bit Edge Detection Select.
                        //            0: Detect low level on RXDn pin as start bit,
         //            1: Detect falling edge of RXDn pin as start bit.
         + LSHIFT(0, 6) // BGDM    | Baud Rate Generator Double-Speed Mode.
                        //            0: Output clock from baud rate generator with single frequency.
         //            1: Output clock from baud rate generator with double frequency
         + LSHIFT(0, 5) // NFEN    | Digital Noise Filter Function Enable
         + LSHIFT(0, 4) // ABCS    | Asynchronous Mode Base Clock Select.
                        //            0: Select 16 base clock cycles for 1-bit period.
         //            1: Select 8 base clock cycles for 1-bit period.
         + LSHIFT(0, 3) // ABCSE   | Asynchronous Mode Extended Base Clock Select 1.
                        //            0: Clock cycles for 1-bit period determined by combination of BGDM and ABCS in SEMR
         //            1: Baud rate is 6 base clock cycles for 1-bit period.
         + LSHIFT(1, 2) // BRME    | Bit Rate Modulation Enable
         + LSHIFT(0, 1) // Reserved
         + LSHIFT(0, 0) // Reserved
  ;

  R_BSCI->SEMR = reg8;

  R_BSCI->BRR  = N;
  R_BSCI->MDDR = M;
  R_BSCI->SEMR_b.ABCSE = absce;
  R_BSCI->SEMR_b.BRME  = brme;


  // Сбрасываем FIFO и одновременно установливаем уровни
  // Если RTRG = 0 то не будет происходить прерывание по приему при включенном FIFO (FM = 1)
  reg16 = 0
          + LSHIFT(14,12)  // RSTRG[3:0]| RTS Output Active Trigger Number Select
                           //              0000: Trigger number 0
                           //              1111: Trigger number 15
          + LSHIFT(15, 8)  // RTRG[3:0] | Receive FIFO Data Trigger Number
                           //              0000: Trigger number 0
                           //              1111: Trigger number 15
          + LSHIFT(15, 4)  // TTRG[3:0] | Transmit FIFO Data Trigger Number
                           //              0000: Trigger number 0
                           //              1111: Trigger number 15
          + LSHIFT(0, 3)   // DRES      | Receive Data Ready Error Select Bit
                           //              0: Receive data full interrupt (SCIn_RXI)
                           //              1: Receive error interrupt (SCIn_ERI)
          + LSHIFT(1, 2)   // TFRST     | Transmit FIFO Data Register Reset
                           //              0: Do not reset FTDRHL
                           //              1: Reset FTDRHL
          + LSHIFT(1, 1)   // RFRST     | Receive FIFO Data Register Reset
                           //              0: Do not reset FRDRHL
                           //              1: Reset FRDRHL
          + LSHIFT(1, 0)   // FM        | FIFO Mode Select
                           //              0: Non-FIFO mode. Selects TDR/RDR or TDRHL/RDRHL for communication.
                           //              1: FIFO mode. Selects FTDRHL/FRDRHL for communication.
  ;
  R_BSCI->FCR = reg16;


  memset(&BLE_UART_err_stat, 0, sizeof(BLE_UART_err_stat));

  tx_event_flags_create(&bsci_flags, "BSCI");

  BSCI_set_and_enable_interrupts();

  #ifdef UART_BLE_USE_DMA

  R_ICU->IELSRn[bsci_txi_int_num] = 0; // Если применяем DMA для передатчика, то нужно обнулить регистр IELSRn с номером прерывания передатчика
  R_ICU->DELSRn[UART_BLE_TX_DMA_CH].DELSRn_b.DELS = ELC_EVENT_BSCI_TXI; // Программируем канал передачи сигнала в модуль DMA через ICU
  R_DMA_BLE_TX->DMSAR        = 0;                     // DMA Source Address Register
  R_DMA_BLE_TX->DMDAR        = ((uint32_t)&R_BSCI->TDRHL) + 1; // DMA Destination Address Register
  R_DMA_BLE_TX->DMTMD_b.DCTG = 1;      // Transfer Request Source Select. 1: Interrupts from peripheral modules or external interrupt input pins
  R_DMA_BLE_TX->DMTMD_b.SZ   = 0;      // Transfer Data Size Select.      0: 8 bits
  R_DMA_BLE_TX->DMTMD_b.DTS  = 1;      // Repeat Area Select.   1: Specify source as the repeat area or block area
  R_DMA_BLE_TX->DMTMD_b.MD   = 0;      // Transfer Mode Select. 0: Normal transfer
  //R_DMA_BLE_TX->DMINT_b.DTIE = 0;      // Transfer End Interrupt disable. Вместо этого используем прерывние SCI по факту окончания пересылки из сдвигового регистра
  R_DMA_BLE_TX->DMINT_b.DTIE = 1;      // Transfer End Interrupt enable. Окончание пересылки получим сразу как только данные будут загружены в FIFO
  R_DMA_BLE_TX->DMAMD_b.DARA = 0;      // Destination Address Extended Repeat Area
  R_DMA_BLE_TX->DMAMD_b.SARA = 0;      // Source Address Extended Repeat Area
  R_DMA_BLE_TX->DMAMD_b.DM   = 0;      // Destination Address Update Mode. 0: Fixed address
  R_DMA_BLE_TX->DMAMD_b.SM   = 2;      // Source Address Update Mode.      2: Incremented address

  R_DMA_BLE_RX->DMSAR        = (uint32_t)&R_BSCI->FRDRL;       // DMA Source Address Register
  R_DMA_BLE_RX->DMDAR        = 0;      // DMA Destination Address Register
  R_DMA_BLE_RX->DMTMD_b.DCTG = 1;      // Transfer Request Source Select. 1: Interrupts from peripheral modules or external interrupt input pins
  R_DMA_BLE_RX->DMTMD_b.SZ   = 0;      // Transfer Data Size Select.      0: 8 bits
  R_DMA_BLE_RX->DMTMD_b.DTS  = 0;      // Repeat Area Select.   0: Specify destination as the repeat area or block area
  R_DMA_BLE_RX->DMTMD_b.MD   = 0;      // Transfer Mode Select. 0: Normal transfer
  R_DMA_BLE_RX->DMINT_b.DTIE = 1;      // Transfer End Interrupt enable.
  R_DMA_BLE_RX->DMAMD_b.DARA = 0;      // Destination Address Extended Repeat Area
  R_DMA_BLE_RX->DMAMD_b.SARA = 0;      // Source Address Extended Repeat Area
  R_DMA_BLE_RX->DMAMD_b.DM   = 2;      // Destination Address Update Mode. 2: Incremented address
  R_DMA_BLE_RX->DMAMD_b.SM   = 0;      // Source Address Update Mode.      0: Fixed address

  R_BSCI->FCR_b.RTRG = 1; // Тригер для возниконовения прерывания приемника устанавливаем в 1, чтобы DMA срабатывало по приему каждого байта
  #endif

  // Запускаем приемник
  reg8 = 0
         + LSHIFT(0, 7) // TIE      | Transmit Interrupt Enable
         + LSHIFT(1, 6) // RIE      | Receive Interrupt Enable
         + LSHIFT(0, 5) // TE       | Transmit Enable
         + LSHIFT(1, 4) // RE       | Receive Enable
         + LSHIFT(0, 3) // MPIE     | Multi-Processor Interrupt Enable
         + LSHIFT(0, 2) // TEIE     | Transmit End Interrupt Enable
         + LSHIFT(0, 0) // CKE[1:0] | Clock Enable. 0 0: On-chip baud rate generator
  ;
  R_BSCI->SCR = reg8;

  return RES_OK;

}
/*-----------------------------------------------------------------------------------------------------


  \param baud
  \param parity
  \param stops

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t UART_BLE_set_baudrate(uint32_t baudrate)
{
  uint8_t   reg8;
  uint8_t   cmr;
  uint8_t   scr;

  uint8_t   cks    = 0;
  uint8_t   absce  = 0;
  uint8_t   brme   = 0;  // Enable bit rate modulation function.
  uint8_t   N;
  uint8_t   M;

  if (SCI_Find_best_divider(baudrate,&brme,&absce,&cks,&N,&M) != RES_OK) return RES_ERROR;

  scr = R_BSCI->SCR;
  // Выключаем UART
  reg8 = 0
         + LSHIFT(0, 7) // TIE      | Transmit Interrupt Enable
         + LSHIFT(0, 6) // RIE      | Receive Interrupt Enable
         + LSHIFT(0, 5) // TE       | Transmit Enable
         + LSHIFT(0, 4) // RE       | Receive Enable
         + LSHIFT(0, 3) // MPIE     | Multi-Processor Interrupt Enable
         + LSHIFT(0, 2) // TEIE     | Transmit End Interrupt Enable
         + LSHIFT(0, 0) // CKE[1:0] | Clock Enable. 0 0: On-chip baud rate generator
  ;
  R_BSCI->SCR = reg8;

  R_BSCI->SEMR_b.ABCSE = absce;
  R_BSCI->SEMR_b.BRME  = brme;
  R_BSCI->BRR  = N;
  R_BSCI->MDDR = M;

  cmr = R_BSCI->SMR;
  cmr &= ~LSHIFT(3, 0);
  cmr |= LSHIFT(cks, 0);
  R_BSCI->SMR = cmr;

  R_BSCI->FCR_b.RFRST = 1; // Reset FRDRHL  Сброс FIFO приемника
  R_BSCI->FCR_b.TFRST = 1; // Reset FTDRHL  Сброс FIFO передатчика

  R_BSCI->SSR = (R_BSCI->SSR & (SCI_SSR_DR | SCI_SSR_PER | SCI_SSR_FER | SCI_SSR_ORER | SCI_SSR_DR | SCI_SSR_RDF)) ^ 0xFF; // Сбрасываем флаги ошибок

  R_BSCI->SCR = scr; // Переводим UART в предыдущее состояние

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param sci_rx
  \param sci_tc
-----------------------------------------------------------------------------------------------------*/
void UART_BLE_set_byte_recieving_handler(T_uart_byte_recieve_handler  handler)
{
  bsci_byte_receive_handler = handler;
}




#ifdef UART_BLE_USE_DMA
/*-----------------------------------------------------------------------------------------------------
  Прием пакета  данных с использованием DMA


  \param data
  \param sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t UART_BLE_receive_packet(uint8_t *data_buf, uint32_t *p_sz, uint32_t timeout)
{
  UINT     res;
  ULONG    actual_flags;

  //ITM_EVENT8(4, *p_sz);
  R_ICU->IELSRn[bsci_rxi_int_num] = 0;                                  // Если применяем DMA, то нужно обнулить регистр IELSRn с номером прерывания
  R_ICU->DELSRn[UART_BLE_RX_DMA_CH].DELSRn_b.DELS = ELC_EVENT_BSCI_RXI; // Программируем канал передачи сигнала в модуль DMA через ICU

  R_DMA_BLE_RX->DMSTS = 0;
  R_DMA_BLE_RX->DMDAR = (uint32_t)data_buf;  // DMA Destination Address Register
  R_DMA_BLE_RX->DMCRA = *p_sz; //
  R_DMA_BLE_RX->DMCNT = 1; // DMA Transfer Enable

  R_BSCI->SCR_b.RIE   = 1; // Разрешаем прерывания приемника

  res = tx_event_flags_get(&bsci_flags,BSCI_RX_COMPLETE, TX_OR_CLEAR,&actual_flags, ms_to_ticks(timeout));
  if (res != TX_SUCCESS)
  {
    BLE_UART_err_stat.rx_err_cnt++;
    *p_sz = 0;
    res = RES_ERROR;
  }
  else
  {
    res = RES_OK;
  }


  R_BSCI->SCR_b.RIE   = 0; // Запрещаем прерывания приемника

  R_ICU->IELSRn[bsci_rxi_int_num] = ELC_EVENT_BSCI_RXI;      // Восстанавливаем канал передачи сигнала прерываний к процессору
  R_ICU->DELSRn[UART_BLE_RX_DMA_CH].DELSRn_b.DELS = 0; // Очищаем канал передачи сигнала в модуль DMA

  R_BSCI->SCR_b.RIE   = 1; // Разрешаем прерывания приемника
  //ITM_EVENT8(4, 0);
  return res;
}
/*-----------------------------------------------------------------------------------------------------
  Отправка данных с использованием DMA

  \param data
  \param sz
-----------------------------------------------------------------------------------------------------*/
uint32_t UART_BLE_send_packet(uint8_t *data_buf, uint16_t sz)
{
  ULONG        actual_flags;
  uint32_t     res;

  //ITM_EVENT8(1, sz);

  R_DMA_BLE_TX->DMSTS = 0;
  R_DMA_BLE_TX->DMSAR = (uint32_t)data_buf;  // DMA Source Address Register
  R_DMA_BLE_TX->DMCRA = sz; //

  R_BSCI->SCR |=   BIT(5);  // TE       | Transmit Enable
  R_DMA_BLE_TX->DMCNT = 1;  // DMA Transfer Enable

//  Этот вариант может понадобиться если решим получать флаг по факту окончания отправки данных
//  R_BSCI->SCR |=   BIT(7)   // TIE      | Transmit Interrupt Enable
//                 + BIT(2);  // TEIE     | Transmit End Interrupt Enable

  R_BSCI->SCR |=   BIT(7);   // TIE      | Transmit Interrupt Enable

  // Здесь флаг окончания будет получен сразу как только данные будут помещены в FIFO
  // Для того чтобы флаг выставлялся только после окончания отправки данных из порта надо использовать прерывание Transmit End Interrupt и
  // отключить обработку прерывания DMA т.е. установить R_DMA_BLE_TX->DMINT_b.DTIE = 0;
  res = tx_event_flags_get(&bsci_flags,BSCI_TX_COMPLETE, TX_OR_CLEAR,&actual_flags, 1000);
  if (res != TX_SUCCESS)
  {
    BLE_UART_err_stat.tx_err_cnt++;
  }
  R_DMA_BLE_TX->DMCNT = 0;  // DMA Transfer disable

//  R_BSCI->SCR &= ~(BIT(7)     // TIE      | Transmit Interrupt disable
//                   + BIT(2)); // TEIE     | Transmit End Interrupt disable
  R_BSCI->SCR &= ~(BIT(7));     // TIE      | Transmit Interrupt disable

  //ITM_EVENT8(1, 0);
  return res;
}

#else // #ifdef UART_BLE_USE_DMA
/*-----------------------------------------------------------------------------------------------------


  \param data
  \param sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t UART_BLE_receive_packet(uint8_t *data_buf, uint32_t *p_sz, uint32_t timeout)
{
  UINT     res;
  ULONG    actual_flags;

  ucbl.rx_data_buf     = data_buf;
  ucbl.rx_data_sz      = *p_sz;
  ucbl.rx_data_left_sz = ucbl.rx_data_sz;

  R_BSCI->SCR_b.RIE   = 1; // Разрешаем прерывания приемника

  res = tx_event_flags_get(&bsci_flags,BSCI_RX_COMPLETE, TX_OR_CLEAR,&actual_flags, ms_to_ticks(timeout));

  R_BSCI->SCR_b.RIE   = 0; // Запрещаем прерывания приемника

  *p_sz = ucbl.rx_data_sz - ucbl.rx_data_left_sz;
  if (res != TX_SUCCESS)
  {
    res = RES_ERROR;
  }
  else
  {
    res = RES_OK;
  }
  ucbl.rx_data_buf     = 0;
  ucbl.rx_data_sz      = 0;
  ucbl.rx_data_left_sz = 0;

  R_BSCI->SCR_b.RIE   = 1; // Разрешаем прерывания приемника
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param data
  \param sz
-----------------------------------------------------------------------------------------------------*/
uint32_t UART_BLE_send_packet(uint8_t *data_buf, uint16_t sz)
{
  ULONG        actual_flags;
  uint32_t     res;
  uint8_t      reg8;

  ucbl.tx_data_buf  = data_buf;
  ucbl.tx_data_sz   = sz;

  reg8 = 0
         + LSHIFT(1, 7) // TIE      | Transmit Interrupt Enable
         + LSHIFT(1, 6) // RIE      | Receive Interrupt Enable
         + LSHIFT(1, 5) // TE       | Transmit Enable
         + LSHIFT(1, 4) // RE       | Receive Enable
         + LSHIFT(0, 3) // MPIE     | Multi-Processor Interrupt Enable
         + LSHIFT(1, 2) // TEIE     | Transmit End Interrupt Enable
         + LSHIFT(0, 0) // CKE[1:0] | Clock Enable. 0 0: On-chip baud rate generator
  ;
  R_BSCI->SCR = reg8;

  res = tx_event_flags_get(&bsci_flags,BSCI_TX_COMPLETE, TX_OR_CLEAR,&actual_flags, 1000);

  R_BSCI->SCR_b.TIE  = 0;
  R_BSCI->SCR_b.TEIE = 0;
  return res;
}



#endif

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void UART_BLE_deinit(void)
{
  R_BSCI->SCR = 0;
  bsci_byte_receive_handler = 0;
  UART_BLE_Clear_Interrupts();
}


