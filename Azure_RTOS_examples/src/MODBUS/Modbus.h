#ifndef  __MODBUS_H
#define  __MODBUS_H

#include "stdint.h"
#include <Modbus_config.h>
#include <Modbus_defs.h>

typedef void  (*T_rs485_rx_handler)(uint8_t portn, uint8_t ch); // Тип обработчика после получения байта
typedef void  (*T_rs485_tc_handler)(uint8_t portn);             // Тип обработчика после отправки всего пакета данных


typedef  struct  modbus_ch
{
    uint8_t      modbus_channel;                    // Канал MODBUS. Каждый канал работает на отдельном UART и может быть мастером или слэйвом

    uint8_t      slave_node_addr;                   //  Адрес этого узла если работаем в режиме слэйва
    uint8_t      uart_number;
    uint32_t     baud_rate;
    uint8_t      parity_chek;
    uint8_t      bits_num;
    uint8_t      stop_bits;
    uint8_t      rtu_ascii_mode;                    // режим ASCII когда = MODBUS_MODE_ASCII, режим RTU когда = MODBUS_MODE_RTU
    uint8_t      master_slave_flag;                 // Слэйв когда = MODBUS_SLAVE, мастер когда = MODBUS_MASTER

    uint8_t      rtu_timeout_en;                    // Флаг разрешения контроля таймаута ожидания пакета RTU
    uint16_t     rtu_timeout;                       // Таймаут ожидания пакета RTU в тиках таймера
    uint16_t     rtu_timeout_counter;               // Счетчик тиков тамера для отслеживания таймаута RTU

    uint32_t     rx_timeout;                        // Тайм аут ожидания приема данных в мс

    uint32_t     recv_bytes_count;                  // Счетчик принятых байт
    uint16_t     rx_packet_size;                    // Размер принятого пакета
    uint8_t      *rx_packet_ptr;                    // Указатель на пакет принимаемых данных
    uint8_t      rx_buf[MODBUS_CFG_BUF_SIZE];       // Буфер принимаемого пакета данных

    uint32_t     sent_bytes_count;                  // Счетчик отправленных байт
    uint16_t     tx_packet_size;                    // Размер посылаемого пакета
    uint8_t      *tx_packet_ptr;                    // Указатель на пакет посылаемых данных
    uint8_t      tx_buf[MODBUS_CFG_BUF_SIZE];       // Буфер посылаемого пакета данных

    uint8_t      rx_frame_buf[MODBUS_CFG_BUF_SIZE]; // Промежуточный буфер хранения поступивших данных. В него копируются данные из rx_buf кроме адреса узла и CRC
    uint16_t     rx_data_cnt;                       // Счетчик обработанных данных
    uint16_t     rx_packet_crc_ref;                 // Значение crc принятое с пакетом
    uint16_t     rx_packet_crc;                     // Расчитнанное CRC принятого пакета

    uint8_t      tx_frame_buf[MODBUS_CFG_BUF_SIZE]; // Промежуточный буфер хранения отправляемых данных. Из него копируются данные в tx_buf кроме CRC
    uint16_t     tx_data_cnt;                       // Количество данных в пакете для отправки
    uint16_t     tx_packet_crc;                     // crc пакета высылаемых данных

    uint32_t     last_err;
    uint32_t     err_cnt;
    uint8_t      last_except_code;

    T_sys_timestump  time_stump;                    // Переменные для измерения времени задержки ответа от слэйва в ответ на команду мастера
    uint32_t     ans_min_time;                      //
    uint32_t     ans_max_time;                      //
}
T_MODBUS_ch;


extern   uint32_t        rtu_timer_tick_counter;

extern   uint8_t         modbus_cbls_counter;
extern   T_MODBUS_ch     modbus_cbls[MODBUS_CFG_MAX_CH];


void          Modbus_module_init(void);
T_MODBUS_ch*  Modbus_channel_init(uint8_t  node_addr, uint8_t  master_slave, uint32_t  rx_timeout, uint8_t  modbus_mode, uint8_t  port_nbr, uint32_t  baud, uint8_t  bits, uint8_t  parity, uint8_t  stops);
void          MB_set_rx_timeout(T_MODBUS_ch  *pch, uint32_t  timeout);
void          MB_set_mode(T_MODBUS_ch  *pch, uint8_t  master_slave, uint8_t  mode);
void          MB_set_node_addr(T_MODBUS_ch  *pch, uint8_t  addr);
void          MB_set_UART_port(T_MODBUS_ch  *pch, uint8_t  port_nbr);
void          Modbus_module_deinit(void);

void          MB_ASCII_rx_byte(T_MODBUS_ch   *pch, uint8_t   rx_byte);

void          MB_RTU_rx_byte(T_MODBUS_ch   *pch, uint8_t   rx_byte);
void          MB_RTU_TmrReset(T_MODBUS_ch   *pch);
void          MB_RTU_TmrResetAll(void);
void          MB_RTU_packet_end_detector(void);

void          MB_rx_byte(T_MODBUS_ch   *pch, uint8_t   rx_byte);

uint8_t       MB_ASCII_Rx(T_MODBUS_ch   *pch);
void          MB_M_send_ASCII_packet(T_MODBUS_ch   *pch);


uint8_t       MB_RTU_Rx(T_MODBUS_ch   *pch);
void          MB_M_send_RTU_packet(T_MODBUS_ch   *pch);


void          MB_OS_Init(void);
void          MB_OS_deinit(void);
void          MB_OS_packet_end_signal(T_MODBUS_ch  *pch);
void          MB_OS_wait_rx_packet_end(T_MODBUS_ch *pch, uint16_t *perr);


uint8_t*      MB_ASCII_BinToHex(uint8_t  value, uint8_t *pbuf);
uint8_t       MB_ASCII_HexToBin(uint8_t *phex);
uint8_t       MB_ASCII_RxCalcLRC(T_MODBUS_ch  *pch);
uint8_t       MB_ASCII_TxCalcLRC(T_MODBUS_ch  *pch, uint16_t tx_bytes);


uint16_t      MB_RTU_CalcCRC(T_MODBUS_ch  *pch);
uint16_t      MB_RTU_TxCalcCRC(T_MODBUS_ch  *pch);
uint16_t      MB_RTU_RxCalcCRC(T_MODBUS_ch  *pch);


void          MB_BSP_deinit(void);
void          MB_BSP_config_UART(T_MODBUS_ch   *pch,  uint8_t  uart_nbr, uint32_t  baud, uint8_t  bits, uint8_t  parity, uint8_t  stops);
void          MB_BSP_send_packet(T_MODBUS_ch  *pch);
void          MB_BSP_stop_receiving(T_MODBUS_ch  *pch);


void          MB_BSP_timer_init(void);
void          MB_BSP_timer_deinit(void);
void          MB_RTU_TmrISR_Handler(void);



uint16_t      MB_M_FC01_coils_read(T_MODBUS_ch   *pch, uint8_t   slave_addr, uint16_t   start_addr, uint8_t  *p_coil_tbl, uint16_t   nbr_coils);
uint16_t      MB_M_FC02_input_read(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, uint8_t  *p_di_tbl, uint16_t   nbr_di);
uint16_t      MB_M_FC03_hold_regs_read(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, uint16_t  *p_reg_tbl, uint16_t   nbr_regs);
uint16_t      MB_M_FC03_hold_reg_fp_read(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, float    *p_reg_tbl, uint16_t   nbr_regs);
uint16_t      MB_M_FC04_input_reg_read(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, uint16_t  *p_reg_tbl, uint16_t   nbr_regs);
uint16_t      MB_M_FC05_single_coil_write(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, uint8_t  coil_val);
uint16_t      MB_M_FC06_single_reg_write(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, uint16_t   reg_val);
uint16_t      MB_M_FC06_single_reg_fp_write(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, float     reg_val_fp);
uint16_t      MB_M_FC08_diagnostic(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   fnct, uint16_t   fnct_data, uint16_t  *pval);
uint16_t      MB_M_FC15_coils_write(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, uint8_t  *p_coil_tbl, uint16_t   nbr_coils);
uint16_t      MB_M_FC16_registers_write(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, uint16_t  *p_reg_tbl, uint16_t   nbr_regs);
uint16_t      MB_M_FC16_registers_fp_write(T_MODBUS_ch   *pch, uint8_t   slave_node, uint16_t   slave_addr, float    *p_reg_tbl, uint16_t   nbr_regs);
uint16_t      MB_M_FC17_regs_write_read(T_MODBUS_ch *pch, uint8_t slave_node, uint16_t wr_addr, uint16_t *wr_regs, uint16_t wr_num, uint16_t rd_addr, uint16_t *rd_regs, uint16_t rd_num);

int32_t       Get_MODBUS_max_ans_time(void);
int32_t       Get_MODBUS_min_ans_time(void);


T_MODBUS_ch*  Get_MODBUS_cbl(void);
void          Clear_MODBUS_errors(void);


#endif

