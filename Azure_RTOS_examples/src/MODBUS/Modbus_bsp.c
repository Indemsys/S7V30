#include "S7V30.h"
#include "Modbus.h"

static TX_TIMER modbus_timer;

extern uint32_t   UART7_RS485_init(uint32_t  baud, uint8_t  parity, uint8_t  stops);
extern void       UART7_RS485_set_hanlers(T_rs485_rx_handler  sci_rx, T_rs485_tc_handler sci_tc);
extern void       UART7_RS485_stop_reciever(void);
extern void       UART7_RS485_start_reciever(void);
extern uint32_t   UART7_RS485_send_packet(uint8_t *data, uint16_t sz);
extern void       UART7_RS485_deinit(void);

/*-----------------------------------------------------------------------------------------------------
  Деинициализация периферии использованной модулем Modbus

  \param
-----------------------------------------------------------------------------------------------------*/
void  MB_BSP_deinit(void)
{
  uint8_t        i;

  for (i = 0; i < MODBUS_CFG_MAX_CH; i++)
  {
    UART7_RS485_deinit();
  }
}


/*-----------------------------------------------------------------------------------------------------
  Callback функци вызываема в прерывании при приеме данных по MODBUS

  \param portn
  \param ch
-----------------------------------------------------------------------------------------------------*/
static void  MB_BSP_rx_handler(uint8_t portn, uint8_t ch)
{
  T_MODBUS_ch   *pch;

  pch =&modbus_cbls[portn];

  // Измерение и ведение статистики длительности ожидания отклика от MODBUS слэйва
  if (pch->rx_packet_size==0)
  {
    T_sys_timestump  now;
    uint32_t          tdif;
    Get_hw_timestump(&now);
    tdif = Timestump_diff_to_usec(&pch->time_stump,&now);
    if (tdif > pch->ans_max_time)  pch->ans_max_time = tdif;
    if (tdif < pch->ans_min_time)  pch->ans_min_time = tdif;
  }

  pch->recv_bytes_count++;
  MB_rx_byte(pch, ch);
}


/*-----------------------------------------------------------------------------------------------------
  Прекращение приема после истечения таймаута
  Вызывается в обработчике прерывани таймера таймаута

  \param pch
-----------------------------------------------------------------------------------------------------*/
void  MB_BSP_stop_receiving(T_MODBUS_ch  *pch)
{
  UART7_RS485_stop_reciever();
}


/*-----------------------------------------------------------------------------------------------------
  Посылка пакета MODBUS

  \param pch
-----------------------------------------------------------------------------------------------------*/
void  MB_BSP_send_packet(T_MODBUS_ch  *pch)
{
   UART7_RS485_send_packet(pch->tx_buf, pch->tx_packet_size);
}

/*-----------------------------------------------------------------------------------------------------
  Callback функци вызываемая в прерывании после окончания передаче данных по MODBUS

  \param portn

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static void MB_BSP_tc_handler(uint8_t portn)
{
  T_MODBUS_ch   *pch;


  pch =&modbus_cbls[portn];
  pch->sent_bytes_count += pch->tx_packet_size;
  pch->tx_packet_ptr =&pch->tx_buf[0];

  if (pch->master_slave_flag == MODBUS_MASTER)
  {
    pch->rtu_timeout_en = MODBUS_FALSE; // После окончания передачи тайм аут ожидания завершения пакета не еще активируем
                                        // Он будет активирован после получения первого байта ответа
    pch->rx_packet_size  = 0;
  }

  Get_hw_timestump(&pch->time_stump);

  UART7_RS485_start_reciever();
}


/*-----------------------------------------------------------------------------------------------------
  Инициализация UART-а для канала MODBUS

  \param pch          указатель на управляющую структуру
  \param uart_nbr     номер UART-а
  \param baud         скрость UART-а
  \param bits         количество бит
  \param parity       режим контроля четности
  \param stops        количество STOP бит
-----------------------------------------------------------------------------------------------------*/
void  MB_BSP_config_UART(T_MODBUS_ch   *pch, uint8_t  portn, uint32_t  baud, uint8_t  bits, uint8_t  parity, uint8_t  stops)
{
  pch->uart_number   = portn;
  pch->baud_rate     = baud;
  pch->parity_chek   = parity;
  pch->bits_num      = bits;
  pch->stop_bits     = stops;

  UART7_RS485_init(baud,parity,stops);
  UART7_RS485_set_hanlers(MB_BSP_rx_handler, MB_BSP_tc_handler);
}




/*-----------------------------------------------------------------------------------------------------
  Callback функция вызываем по прерываниям таймера

  Завершение приема пакета определяется по паузе в передаче данных в течении времени равному передаче 3.5 символа

  \param void
-----------------------------------------------------------------------------------------------------*/
static void  MB_BSP_timer_tick_handler(ULONG arg)
{

  rtu_timer_tick_counter++;
  MB_RTU_packet_end_detector();
}

/*-----------------------------------------------------------------------------------------------------
  Инициализируем таймер

  \param
-----------------------------------------------------------------------------------------------------*/
void  MB_BSP_timer_init(void)
{
  uint32_t ticks = TX_TIMER_TICKS_PER_SECOND / RTU_TIMER_TICK_FREQ;

  if (ticks <= 0) ticks = 1;

  tx_timer_create(&modbus_timer,"modbus",MB_BSP_timer_tick_handler, 0, 1 ,ticks, TX_AUTO_ACTIVATE);

  MB_RTU_TmrResetAll();
}

/*-----------------------------------------------------------------------------------------------------
  Прекращаем работу таймера

  \param void
-----------------------------------------------------------------------------------------------------*/
void  MB_BSP_timer_deinit(void)
{
  tx_timer_delete(&modbus_timer);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_MODBUS_ch*
-----------------------------------------------------------------------------------------------------*/
T_MODBUS_ch* Get_MODBUS_cbl(void)
{
  return &modbus_cbls[0];
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void  Clear_MODBUS_errors(void)
{
  modbus_cbls[0].err_cnt = 0;
  modbus_cbls[0].last_err = 0;
  modbus_cbls[0].last_except_code = 0;
}

