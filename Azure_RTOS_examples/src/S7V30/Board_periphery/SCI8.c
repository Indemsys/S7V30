// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-14
// 15:58:11
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define    S7V30_SCI8
#include   "S7V30.h"

static IRQn_Type               sci8_rxi_int_num;
static IRQn_Type               sci8_txi_int_num;
static IRQn_Type               sci8_tei_int_num;
static IRQn_Type               sci8_eri_int_num;
static IRQn_Type               sci8_am_int_num;
static IRQn_Type               elc_soft1_int_num;

static uint8_t                 sci8_inited;
static TX_EVENT_FLAGS_GROUP    sci8_flags;
static TX_MUTEX                sci8_mutex;
static uint8_t                *sci8_out_packet_ptr;
static uint8_t                *sci8_in_packet_ptr;
static uint16_t                sci8_rx_packet_data_left;
static uint16_t                sci8_tx_packet_data_left;

uint32_t sci8_err_cnt;

#define  SCI8_RX_COMPLETE  BIT(0)

// Объявление для драйвера канала DTC принимающего поток из I2S интерфейса
dtc_instance_ctrl_t               DTC_SCI8_ctrl;
extern const transfer_instance_t  DTC_SCI8_RX_inst;

// Для того чтобы выдать сигнал ADC1_CS = 0, ADC2_CS = 1 нужно в порт записать значение R_IOPORT11->PCNTR3 = (BIT(2) << 16) || BIT(6)
// Для того чтобы выдать сигнал ADC1_CS = 1, ADC2_CS = 0 нужно в порт записать значение R_IOPORT11->PCNTR3 = (BIT(6) << 16) || BIT(2)
// Для того чтобы выдать сигнал ADC1_CS = 1, ADC2_CS = 1 нужно в порт записать значение R_IOPORT11->PCNTR3 = BIT(6) || BIT(2)

#define  CS_0 ( (BIT(2) << 16) | BIT(6))
#define  CS_1 ( (BIT(6) << 16) | BIT(2))
#define  N_CS ( BIT(6) | BIT(2)        )


uint32_t   S_CS_ARRAY[SCI8_DTC_CN] =
{
  CS_0, CS_0, CS_1, CS_1, CS_0, CS_0, CS_1, CS_1, CS_0, CS_0, CS_1, CS_1, CS_0, CS_0, CS_1, CS_1,
  CS_0, CS_0, CS_1, CS_1, CS_0, CS_0, CS_1, CS_1, CS_0, CS_0, CS_1, CS_1, CS_0, CS_0, CS_1, CS_1,
  N_CS
};

// Массив для приема данных из SPI при пересылке с использованием DTC
uint8_t   EXT_ADC_RESULTS[SCI8_DTC_CN] = {0 };

// Байты флагов записываемые в регистр ELSEGR1 используемый для генерации прерываний по окончании пересылки с использованием DTC
uint8_t   INT_PEND_ARRAY[SCI8_DTC_CN] =
{
   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,
   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,   0x40,
   0x41   // Последний отправленный байт вызывает прерывание
};

uint8_t   zero_byte = 0;

// Массив адресов регистра назначения отправления байт данных при пересылке с использованием DTC
volatile uint8_t   *DEST_REG_ARRAY[SCI8_DTC_CN] =
{
  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,
  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,  &R_SCI8->TDR,
  &zero_byte     // Последний адрес назначения указывает на пустую область памяти чтобы не вызвать лишнюю передачу байта
};

// =========================================================================================================================================================================================================================================
// Структура повторяющая управляющую структуру DTC
transfer_info_t SCI8_RX_dtc[] =
{
  // DM[1:0]         | DTS             | DISEL            | CHNS & CHNE     | SM[1:0]         | SZ[1:0]       |  MD[1:0]      | SAR                   | DAR                          | CRB              | CRA
  // Destination     | DTC Transfer    | DTC Interrupt    | DTC Chain       | Source          | DTC Data      |  DTC Transfer | DTC Transfer          | DTC Transfer                 | DTC Transfer     | DTC Transfer
  // Addressing Mode | Mode Select     | Select           | Transfer Select | Addressing Mode | Transfer Size |  Mode Select  | Source Register       | Destination Register         | Count Register B | Count Register A

  // Устанавливаем сигнал CS в начале
  {ADDR_FIXED,        SRC_REPEATED,    DISEL_1_SETIRQ,    CHAIN_NO_LOOPING,   ADDR__INCR,      UNIT_4_BYTE,      MODE_REPEAT,   &S_CS_ARRAY[0],        (void*)&R_IOPORT11->PCNTR3,     0 ,        (SCI8_DTC_CN << 8)| SCI8_DTC_CN },

  // Записываем в 4-ую строку адрес назначения - регистр данных SPI.
  // На последнем цикле адрес назначения будет указавать в пустую облать RAM
  // Таким образом бедет исключено инициирование приема лишнего байта в последнем цикле DTC
  {ADDR_FIXED,        SRC_REPEATED,    DISEL_1_SETIRQ,    CHAIN_NO_LOOPING,   ADDR__INCR,      UNIT_4_BYTE,      MODE_REPEAT,   &DEST_REG_ARRAY[0],    (void*)&SCI8_RX_dtc[3].p_dest,  0 ,        (SCI8_DTC_CN << 8)| SCI8_DTC_CN },

  // Читаем байт из SPI
  {ADDR__INCR,        DST_REPEATED,    DISEL_1_SETIRQ,    CHAIN_NO_LOOPING,   ADDR_FIXED,      UNIT_1_BYTE,      MODE_REPEAT,   (void*)&R_SCI8->RDR ,  &EXT_ADC_RESULTS[0],            0 ,        (SCI8_DTC_CN << 8)| SCI8_DTC_CN },

  // Записываем нулевой байт в SPI чтобы продолжить чтение
  {ADDR_FIXED,        DST_REPEATED,    DISEL_1_SETIRQ,    CHAIN_NO_LOOPING,   ADDR_FIXED,      UNIT_1_BYTE,      MODE_REPEAT,   &zero_byte,            (void*)&R_SCI8->TDR,            0 ,        (SCI8_DTC_CN << 8)| SCI8_DTC_CN },

  // При обработке этой строки движок DTC посылает маску в регистр ELSEGR1 и на последней пересылке маска содержит бит вызова прерывания по событию ELC_EVENT_ELC_SOFTWARE_EVENT_1
  // В обработчике прерывания взводится флаг окончания циклов переслыки DTC
  // Наличие DISEL=0 в этой записи критично. В ином случае не происходит циклического повторения пересылок
  {ADDR_FIXED,        SRC_REPEATED,    DISEL_0_IR_RES,    CHAIN___DISABLED,   ADDR__INCR,      UNIT_1_BYTE,      MODE_NORMAL,   &INT_PEND_ARRAY[0],    (void*)&R_ELC->ELSEGRnRC0[1],   0 ,        SCI8_DTC_CN                     },

};
// =========================================================================================================================================================================================================================================


transfer_cfg_t DTC_SCI8_RX_cfg =
{
  .p_info              = SCI8_RX_dtc,
  .activation_source   = ELC_EVENT_SCI8_RXI,
  .auto_enable         = false,              // Отключен автозапуск DTC после открытия драйвера командой open
  .p_callback          = NULL,               // Используется только в случае использования в качестве источника прерываний ELC_EVENT_ELC_SOFTWARE_EVENT_0 или ELC_EVENT_ELC_SOFTWARE_EVENT_1
                                             // И если включена опция компиляции DTC_CFG_SOFTWARE_START_ENABLE
  .p_context           =&DTC_SCI8_RX_inst,
  .irq_ipl             =(BSP_IRQ_DISABLED)
};

/* Instance structure to use this module. */
const transfer_instance_t DTC_SCI8_RX_inst =
{
  .p_ctrl             =&DTC_SCI8_ctrl,
  .p_cfg              =&DTC_SCI8_RX_cfg,
  .p_api              =&g_transfer_on_dtc
};

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void sci8_clear_interrupts(void)
{
  sci8_rxi_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SCI8_RXI);
  sci8_txi_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SCI8_TXI);
  sci8_tei_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SCI8_TEI);
  sci8_eri_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SCI8_ERI);
  sci8_am_int_num     = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_SCI8_AM);
  elc_soft1_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ELC_SOFTWARE_EVENT_1);

  NVIC_DisableIRQ(sci8_rxi_int_num);
  NVIC_DisableIRQ(sci8_txi_int_num);
  NVIC_DisableIRQ(sci8_tei_int_num);
  NVIC_DisableIRQ(sci8_eri_int_num);
  NVIC_DisableIRQ(sci8_am_int_num);
  NVIC_DisableIRQ(elc_soft1_int_num);

  NVIC_ClearPendingIRQ(sci8_rxi_int_num);
  NVIC_ClearPendingIRQ(sci8_txi_int_num);
  NVIC_ClearPendingIRQ(sci8_tei_int_num);
  NVIC_ClearPendingIRQ(sci8_eri_int_num);
  NVIC_ClearPendingIRQ(sci8_am_int_num);
  NVIC_ClearPendingIRQ(elc_soft1_int_num);


  R_ICU->IELSRn_b[sci8_rxi_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[sci8_txi_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[sci8_tei_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[sci8_eri_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[sci8_am_int_num ].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[elc_soft1_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void sci8_set_and_enable_interrupts(void)
{
  sci8_clear_interrupts();

  NVIC_SetPriority(sci8_rxi_int_num,  EXT_ADC_SCAN_PRIO);
  NVIC_SetPriority(sci8_txi_int_num,  EXT_ADC_SCAN_PRIO);
  NVIC_SetPriority(sci8_tei_int_num,  EXT_ADC_SCAN_PRIO);
  NVIC_SetPriority(sci8_eri_int_num,  EXT_ADC_SCAN_PRIO);
  NVIC_SetPriority(sci8_am_int_num ,  EXT_ADC_SCAN_PRIO);
  NVIC_SetPriority(elc_soft1_int_num,  EXT_ADC_SCAN_PRIO);

  NVIC_EnableIRQ(sci8_rxi_int_num);
  NVIC_EnableIRQ(sci8_txi_int_num);
  NVIC_EnableIRQ(sci8_tei_int_num);
  NVIC_EnableIRQ(sci8_eri_int_num);
  NVIC_EnableIRQ(sci8_am_int_num );
  NVIC_EnableIRQ(elc_soft1_int_num);
}

/*-----------------------------------------------------------------------------------------------------
  Обслуживание прерывания по приему
  Здесь также ведется и передача для того чтобы избежать переполение данных по приему
  Не отправляем новые данные пока не примем данные из предыдущей отправки

  \param void
-----------------------------------------------------------------------------------------------------*/
void     sci8_rxi_isr(void)
{
  static volatile uint8_t b;

  b = R_SCI8->RDR;

  // Здесь прием данных
  if (sci8_in_packet_ptr != 0)
  {
    *sci8_in_packet_ptr = b; // Читаем данные если они есть
    sci8_in_packet_ptr++;
  }

  // Здесь передача данных.
  // Флаг готовности к передаче не проверяем, поскольку после примема байтв модуль SCI к передаче должен быть уже готов
  if (sci8_tx_packet_data_left > 0)
  {
    if (sci8_out_packet_ptr != 0)
    {
      b = *sci8_out_packet_ptr;
      R_SCI8->TDR = *sci8_out_packet_ptr; // Записываем данные
      sci8_out_packet_ptr++;
    }
    else
    {
      R_SCI8->TDR = 0;
    }
    sci8_tx_packet_data_left--;
  }

  sci8_rx_packet_data_left--;
  if (sci8_rx_packet_data_left == 0)
  {
    R_SCI8->SCR = 0;         // Выключаем приемник и передатчик
    tx_event_flags_set(&sci8_flags, SCI8_RX_COMPLETE, TX_OR);
  }

  R_ICU->IELSRn_b[sci8_rxi_int_num].IR = 0;  // Сбрасываем IR флаг в ICU

  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void     sci8_txi_isr(void)
{
  do
  {
    R_SCI8->SSR = (uint8_t)(~BIT(7)); // Сбрасываем флаг TDRE (Transmit Data Empty Flag)
  }
  while ((R_SCI8->SSR & BIT(7))!=0);  // Убеждаемся что флаг сброшен


  R_ICU->IELSRn_b[sci8_txi_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void     sci8_tei_isr(void)
{
  do
  {
    R_SCI8->SSR = (uint8_t)(~BIT(2)); // Сбрасываем флаг TEND (Transmit End Flag)
  }
  while ((R_SCI8->SSR & BIT(2))!=0);  // Убеждаемся что флаг сброшен

  R_ICU->IELSRn_b[sci8_tei_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void     sci8_eri_isr(void)
{
  do
  {
    R_SCI8->SSR = (uint8_t)(~BIT(5)); // Сбрасываем флаг ORER (Overrun Error Flag)
  }
  while ((R_SCI8->SSR & BIT(5))!=0);  // Убеждаемся что флаг сброшен

  R_ICU->IELSRn_b[sci8_eri_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void     sci8_am_isr(void)
{
  R_ICU->IELSRn_b[sci8_am_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Используем прерывание по событию ELC_EVENT_ELC_SOFTWARE_EVENT_1 для установки флага окончания пересылки по DTC
  Данное прерывание используется при организации циклического обмена данными в двумя внешними АЦП ADS7028

  Выбран имеено такой способ поскольку вызвать прерывания записью в регитсры NVIC не удалось.
  Пересылка DTC, по всей видимости, не может выполнить запись в область системных регистров ARM с адреса 0xE0000000

  \param void
-----------------------------------------------------------------------------------------------------*/
void  elc_segr1_isr(void)
{
  tx_event_flags_set(&sci8_flags, SCI8_RX_COMPLETE, TX_OR);


  R_ICU->IELSRn_b[sci8_rxi_int_num].IR = 0; // Сбрасываем оставшийся активным флаг прерывания от приемника
  NVIC_ClearPendingIRQ(sci8_rxi_int_num);

  R_ICU->IELSRn_b[elc_soft1_int_num].IR = 0;    // Сбрасываем IR флаг в ICU
  __DSB();

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t SCI8_SPI_mutex_get(uint32_t wait_option)
{
  return tx_mutex_get(&sci8_mutex, wait_option);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SCI8_SPI_mutex_put(void)
{
  return tx_mutex_put(&sci8_mutex);
}


/*-----------------------------------------------------------------------------------------------------
   SPI8 приняется для управления чипами часов реального времени AB1815 и ЦАП ADS7028


   ЦАП ADS7028  имеет время выборки = 400 нс и ковертирования = 600 нс
   По умолчанию требует режим тактирования CPOL=0 CPHA=0  - тактирование положительными импульсами (при переходе CS в 0 начальный уровень CLK = 0 ) и выборка нарастающему фронту импульса



  \param baud

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SCI8_SPI_init(void)
{
  uint8_t   reg8;
  uint16_t  reg16;
  //volatile  uint8_t dummy;

  if (sci8_inited) return RES_OK;


  tx_mutex_create(&sci8_mutex,"SCI8", TX_TRUE);

  sci8_inited  =1;

  R_MSTP->MSTPCRB_b.MSTPB23 = 0; // Разрешаем работу модуля Serial Communication Interface 8
  DELAY_1us;

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
  R_SCI8->SCR = reg8;
  //dummy = R_SCI8->SCR;

  R_SCI8->SIMR1_b.IICM = 0; //  Asynchronous mode (including multi-processor mode), clock synchronous mode, or simple SPI mode
  R_SCI8->SPMR = 0;         // Disable SSn pin function. Disable CTS function (enable RTS output function).0: Transmit through TXDn pin and receive through RXDn pin (master mode)

  //
  reg8 = 0
        + LSHIFT(1, 7) // BCP2     | Base Clock Pulse 2. Selects the number of base clock cycles in combination with the SMR_SMCI.BCP[1:0] bits in smart card interface mode
        + LSHIFT(1, 6) // reserved |
        + LSHIFT(1, 5) // reserved |
        + LSHIFT(1, 4) // CHR1     | Character Length 1. 1 0: Transmit/receive in 8-bit data (initial value)
        + LSHIFT(1, 3) // SDIR     | Transmitted/Received Data Transfer Direction. 0: Transfer LSB-first, 1: Transfer MSB-first
        + LSHIFT(0, 2) // SINV     | Transmitted/Received Data Invert
        + LSHIFT(1, 1) // reserved |
        + LSHIFT(0, 0) // SMIF     | 0: Non-smart card interface mode (asynchronous mode, clock synchronous mode, simple SPI mode, or simple IIC mode)
  ;
  R_SCI8->SCMR = reg8;


  // Выбираем частоту тактирования так чтобы делитель тактирования помещался в диапазод до 255

  reg8 = 0
        + LSHIFT(1,   7) // CM        | Communication Mode   | 0: Asynchronous or simple IIC mode, 1: Clock synchronous or simple SPI mode.
        + LSHIFT(0,   6) // CHR       | Character Length     | Only valid in asynchronous mode. 0 0: Transmit/receive in 9-bit data, 1 0: Transmit/receive in 8-bit data, 1 1: Transmit/receive in 7-bit data
        + LSHIFT(0,   5) // PE        | Parity Enable        | Only valid in asynchronous mode. 0: Do not add parity bit, 1: Add parity bit.
        + LSHIFT(0,   4) // PM        | Parity Mode          | 0: Even parity, 1: Odd parity.
        + LSHIFT(0,   3) // STOP      | Stop Bit Length      | 0: 1 stop bit,  1: 2 stop bits.
        + LSHIFT(0,   2) // MP        | Multi-Processor Mode | 0: Disable multi-processor communications function
        + LSHIFT(0,   0) // CKS[1:0]  | Clock Select         | 0 0: PCLKA clock (n = 0), 0 1: PCLKA/4 clock (n = 1)
  ;
  R_SCI8->SMR = reg8;


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
        + LSHIFT(0, 2) // BRME    | Bit Rate Modulation Enable
        + LSHIFT(0, 1) // Reserved
        + LSHIFT(0, 0) // Reserved
  ;
  R_SCI8->SEMR = reg8;

  R_SCI8->BRR  = 0;
  R_SCI8->MDDR = 0;


  // FIFO  в режиме SPI не работает
  reg16 = 0
         + LSHIFT(0, 12) // RSTRG[3:0]| RTS Output Active Trigger Number Select
                         //              0000: Trigger number 0
                         //              1111: Trigger number 15
         + LSHIFT(0, 8)  // RTRG[3:0] | Receive FIFO Data Trigger Number
                         //              0000: Trigger number 0
                         //              1111: Trigger number 15
         + LSHIFT(0, 4)  // TTRG[3:0] | Transmit FIFO Data Trigger Number
                         //              0000: Trigger number 0
                         //              1111: Trigger number 15
         + LSHIFT(0, 3)  // DRES      | Receive Data Ready Error Select Bit
                         //              0: Receive data full interrupt (SCIn_RXI)
                         //              1: Receive error interrupt (SCIn_ERI)
         + LSHIFT(0, 2)  // TFRST     | Transmit FIFO Data Register Reset
                         //              0: Do not reset FTDRHL
                         //              1: Reset FTDRHL
         + LSHIFT(0, 1)  // RFRST     | Receive FIFO Data Register Reset
                         //              0: Do not reset FRDRHL
                         //              1: Reset FRDRHL
         + LSHIFT(0, 0)  // FM        | FIFO Mode Select
                         //              0: Non-FIFO mode. Selects TDR/RDR or TDRHL/RDRHL for communication.
                         //              1: FIFO mode. Selects FTDRHL/FRDRHL for communication.
  ;
  R_SCI8->FCR = reg16;

  R_SCI8->SPMR = 0; // В регитсре SPI все обнуляем

  tx_event_flags_create(&sci8_flags, "SCI8");

  sci8_set_and_enable_interrupts();


  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Установка скорости и режимов сэмплирования
  Уровни сигналов меняются сразу после применения этой функции

  cpol   cph

   0      0    - тактирование отрицательными импульсами. На старте CS сигнал клока в 1. Выборка на подъеме импульса
   0      1    - тактирование положительными импульсами. На старте CS сигнал клока в 0. Выборка на подъеме импульса
   1      0    - тактирование положительными импульсами. На старте CS сигнал клока в 0. Выборка на спаде   импульса
   1      1    - тактирование отрицательными импульсами. На старте CS сигнал клока в 1. Выборка на спаде   импульса


  \param baud

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void SCI8_SPI_set_mode(uint32_t  baud, uint8_t cpol, uint8_t cph)
{
  uint8_t N;

  R_SCI8->SCR = 0;

  if (baud > (PCLKA_FREQ / 4))
  {
    N = 0;
  }
  else
  {
    N =(PCLKA_FREQ / (4 * baud))- 1;
  }
  R_SCI8->BRR  = N;
  R_SCI8->SPMR_b.CKPOL = cpol;
  R_SCI8->SPMR_b.CKPH  = cph;

}



/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SCI8_SPI_tx_rx_buf(uint8_t *out_buf, uint8_t *in_buf, uint8_t len)
{
  volatile  uint8_t b;
  ULONG        actual_flags = 0;
  uint32_t     res;

  sci8_out_packet_ptr      = out_buf;
  sci8_in_packet_ptr       = in_buf;
  sci8_rx_packet_data_left = len;
  sci8_tx_packet_data_left = len;

  R_SCI8->SSR = 0; // Сбрасываем флаги

  R_SCI8->SCR = BIT(6) | BIT(5) | BIT(4); // Разрешаем приемники и передатчики и прерывания только от приема

  if (sci8_out_packet_ptr!=0)
  {
    b = *sci8_out_packet_ptr;
    R_SCI8->TDR = b; // Записываем данные
    sci8_out_packet_ptr++;
  }
  else
  {
    R_SCI8->TDR = 0;
  }
  sci8_tx_packet_data_left--;



  res = tx_event_flags_get(&sci8_flags, SCI8_RX_COMPLETE, TX_OR_CLEAR,&actual_flags, 100);


  R_SCI8->SCR  = 0;

  if (res != TX_SUCCESS)
  {
    sci8_err_cnt++;
    return RES_ERROR;
  }

  if (sci8_rx_packet_data_left != 0)
  {
    sci8_err_cnt++;
    return RES_ERROR;
  }

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SCI8_ADS7028_DTC_init(void)
{
  // Используем регистр ELSEGRnRC0[1] для вызова прерываний по окончании циклов пересылки из ЦАП-ов в RAM с помощью механизма DTC
  R_ELC->ELSEGRnRC0[1].ELSEGRn_b.WI = 0; // Сбросов в 0 разрешаем запись в регистр
  R_ELC->ELSEGRnRC0[1].ELSEGRn_b.WE = 1; // Установкой в 1 разрешаем запись в бит прерываний

  DTC_SCI8_RX_inst.p_api->open(DTC_SCI8_RX_inst.p_ctrl,DTC_SCI8_RX_inst.p_cfg);

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SCI8_ADS7028_DTC_rx(void)
{
  volatile  uint8_t b;
  ULONG        actual_flags;
  uint32_t     res;

  SCI8_RX_dtc[0].p_src  =&S_CS_ARRAY[0];
  SCI8_RX_dtc[0].length =(SCI8_DTC_CN << 8) | SCI8_DTC_CN;
  SCI8_RX_dtc[1].p_src  =&DEST_REG_ARRAY[0];
  SCI8_RX_dtc[1].length =(SCI8_DTC_CN << 8) | SCI8_DTC_CN;
  SCI8_RX_dtc[2].p_dest =&EXT_ADC_RESULTS[0];
  SCI8_RX_dtc[2].length =(SCI8_DTC_CN << 8) | SCI8_DTC_CN;
  SCI8_RX_dtc[3].p_src  =&zero_byte;
  SCI8_RX_dtc[3].length =(SCI8_DTC_CN << 8) | SCI8_DTC_CN;
  SCI8_RX_dtc[4].p_src  =&INT_PEND_ARRAY[0];
  SCI8_RX_dtc[4].length = SCI8_DTC_CN;

  DTC_SCI8_RX_inst.p_api->enable(DTC_SCI8_RX_inst.p_ctrl);

  R_SCI8->SSR = 0;                          // Сбрасываем флаги
  R_SCI8->SCR =   BIT(6) | BIT(5) | BIT(4); // Разрешаем приемник и передатчик. Происходит обслуживание по DTC
  R_SCI8->TDR = 0;                          // Записью нулевого байта запускаем работу SPI и активизацию DTC по приему

  res = tx_event_flags_get(&sci8_flags, SCI8_RX_COMPLETE, TX_OR_CLEAR,&actual_flags, 100);

  DTC_SCI8_RX_inst.p_api->disable(DTC_SCI8_RX_inst.p_ctrl);

  if (res != TX_SUCCESS)
  {
    sci8_err_cnt++;
    return RES_ERROR;
  }
  return RES_OK;
}

