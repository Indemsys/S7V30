#ifndef S7V30_SCI8_H
  #define S7V30_SCI8_H


uint32_t SCI8_SPI_init(void);
uint32_t SCI8_SPI_mutex_get(uint32_t wait_option);
uint32_t SCI8_SPI_mutex_put(void);
void     SCI8_SPI_set_mode(uint32_t  baud, uint8_t cpol, uint8_t cph);
uint32_t SCI8_SPI_tx_rx_buf(uint8_t *out_buf, uint8_t *in_buf, uint8_t len);

// Процедуры обслуживания прерываний и макросы объявлены здесь
// поскольку при записи их в С файле перестает нормально работать броузинг кода в редакторе

void     sci8_rxi_isr(void);
void     sci8_txi_isr(void);
void     sci8_tei_isr(void);
void     sci8_eri_isr(void);
void     sci8_am_isr(void);
void     elc_segr1_isr(void);

#ifdef S7V30_SCI8

SSP_VECTOR_DEFINE_CHAN(sci8_rxi_isr  ,  SCI,  RXI  , 8)
SSP_VECTOR_DEFINE_CHAN(sci8_txi_isr  ,  SCI , TXI  , 8)
SSP_VECTOR_DEFINE_CHAN(sci8_tei_isr  ,  SCI , TEI  , 8)
SSP_VECTOR_DEFINE_CHAN(sci8_eri_isr  ,  SCI , ERI  , 8)
SSP_VECTOR_DEFINE_CHAN(sci8_am_isr   ,  SCI , AM   , 8)
SSP_VECTOR_DEFINE(elc_segr1_isr ,  ELC,  SOFTWARE_EVENT_1)

#endif

#define SCI8_DTC_CN  33  // Количество циклов выполнения цепочки DTC при считывании информации из двух ADS7028

extern uint8_t    EXT_ADC_RESULTS[SCI8_DTC_CN];

uint32_t          SCI8_ADS7028_DTC_init(void);
uint32_t          SCI8_ADS7028_DTC_rx(void);



#endif



