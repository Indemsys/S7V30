#ifndef UART_BLE_H
  #define UART_BLE_H


  #define  UART_BLE_USE_DMA


typedef struct
{
    uint8_t         *tx_data_buf;
    uint32_t         tx_data_sz;
    uint8_t         *rx_data_buf;
    uint32_t         rx_data_sz;
    uint32_t         rx_data_left_sz;
}
T_uart_cbl;


typedef struct
{
    uint32_t parity_err_cnt;
    uint32_t framing_err_cnt;
    uint32_t overrun_err_cnt;
    uint32_t tx_err_cnt;
    uint32_t rx_err_cnt;

} T_SCI_error_stat;


typedef uint32_t  (*T_uart_byte_recieve_handler)(uint8_t b); // Тип обработчика для извещения о поступленн данных


uint32_t UART_BLE_init(uint32_t  baud, uint8_t  parity, uint8_t  stops);
void     UART_BLE_set_byte_recieving_handler(T_uart_byte_recieve_handler  handler);
uint32_t UART_BLE_send_packet(uint8_t *data_buf, uint16_t sz);
uint32_t UART_BLE_send_packet(uint8_t *data_buf, uint16_t sz);
uint32_t UART_BLE_receive_packet(uint8_t *data_buf, uint32_t *p_sz, uint32_t timeout);
void     UART_BLE_deinit(void);
uint32_t UART_BLE_set_baudrate(uint32_t baudrate);

// Процедуры обслуживания прерываний и макросы объявлены здесь
// поскольку при записи их в С файле перестает нормально работать броузинг кода в редакторе

void     bsci_rxi_isr(void);
void     bsci_txi_isr(void);
void     bsci_tei_isr(void);
void     bsci_eri_isr(void);
void     bsci_am_isr(void);

// Макросы ниже должны быть отредактированы в случае смены номера SCI

  #define  BSCI_EN_FLAG                  R_MSTP->MSTPCRB_b.MSTPB30   // Разрешаем работу модуля SCI1
  //#define  BSCI_EN_FLAG                  R_MSTP->MSTPCRB_b.MSTPB23   // Разрешаем работу модуля SCI8

  #define  R_BSCI                       ((R_SCI0_Type*) R_SCI1_BASE)

  #define  UART_BLE_RX_DMA_CH            4
  #define  UART_BLE_TX_DMA_CH            5


  #define  ELC_EVENT_BSCI_RXI            ELC_EVENT_SCI1_RXI
  #define  ELC_EVENT_BSCI_TXI            ELC_EVENT_SCI1_TXI
  #define  ELC_EVENT_BSCI_TEI            ELC_EVENT_SCI1_TEI
  #define  ELC_EVENT_BSCI_ERI            ELC_EVENT_SCI1_ERI
  #define  ELC_EVENT_BSCI_AM             ELC_EVENT_SCI1_AM

  #define  UART_BLE_RX_DMA_ELC_EVENT     ELC_EVENT_DMAC4_INT
  #define  UART_BLE_TX_DMA_ELC_EVENT     ELC_EVENT_DMAC5_INT

  #define  R_DMA_BLE_RX                  R_DMAC4
  #define  R_DMA_BLE_TX                  R_DMAC5

  #ifdef UART_BLE_VECT_DEFINES

    #ifdef  UART_BLE_USE_DMA
SSP_VECTOR_DEFINE_CHAN(uart_ble_rx_dmac_int_isr,  DMAC, INT, 4);
SSP_VECTOR_DEFINE_CHAN(uart_ble_tx_dmac_int_isr,  DMAC, INT, 5);
    #endif
SSP_VECTOR_DEFINE_CHAN(bsci_rxi_isr,  SCI,  RXI, 1)
SSP_VECTOR_DEFINE_CHAN(bsci_txi_isr,  SCI , TXI, 1)
SSP_VECTOR_DEFINE_CHAN(bsci_tei_isr,  SCI , TEI, 1)
SSP_VECTOR_DEFINE_CHAN(bsci_eri_isr,  SCI , ERI, 1)
SSP_VECTOR_DEFINE_CHAN(bsci_am_isr ,  SCI , AM , 1)

  #endif

#endif



