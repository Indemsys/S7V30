#include "S7V30.h"
#include "Modbus.h"

uint32_t      rtu_timer_tick_counter;          // Счетчик тиков таймера таймаута RTU

uint8_t       modbus_cbls_counter;             // Счетчик занятых управляющих структур MODBUS каналов
T_MODBUS_ch   modbus_cbls[MODBUS_CFG_MAX_CH];  // Управляющие структуры каналов MODBUS


/*-----------------------------------------------------------------------------------------------------
  Инициализируем модуль MODBUS
  Должнывызываться первой


  \param freq - частота тиков таймера таймаута RTU
-----------------------------------------------------------------------------------------------------*/
void  Modbus_module_init(void)
{
  uint8_t   ch;
  T_MODBUS_ch   *pch;

  pch         =&modbus_cbls[0];
  for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++)
  {
    pch->modbus_channel      = ch;
    pch->slave_node_addr     = 1;
    pch->master_slave_flag   = MODBUS_SLAVE;
    pch->rtu_ascii_mode      = MODBUS_MODE_ASCII;
    pch->rx_packet_size      = 0;
    pch->rx_packet_ptr       =&pch->rx_buf[0];

    pch->ans_max_time        = 0;
    pch->ans_min_time        = 0xFFFFFFFF;

    pch->rtu_timeout_en = MODBUS_FALSE;

    pch++;
  }
  modbus_cbls_counter = 0;
  MB_OS_Init();

  MB_BSP_timer_init();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void  Modbus_module_deinit(void)
{
  MB_BSP_timer_deinit();
  MB_BSP_deinit();
  MB_OS_deinit();
}

/*-----------------------------------------------------------------------------------------------------
 Функция вызывается после MB_Init и конфигурирует канал Modbus на заданном UART-е


 Argument(s) : node_addr     is the Modbus node address that the channel is assigned to.

               master_slave  specifies whether the channel is a MODBUS_MASTER or a MODBUS_SLAVE

               rx_timeout    amount of time Master will wait for a response from the slave.

               modbus_mode   specifies the type of modbus channel.  The choices are:
                             MODBUS_MODE_ASCII
                             MODBUS_MODE_RTU

               port_nbr      is the UART port number associated with the channel

               baud          is the desired baud rate

               parity        is the UART's parity setting:
                             MODBUS_PARITY_NONE
                             MODBUS_PARITY_ODD
                             MODBUS_PARITY_EVEN

               bits          UART's number of bits (7 or 8)

               stops         Number of stops bits (1 or 2)

-----------------------------------------------------------------------------------------------------*/
T_MODBUS_ch* Modbus_channel_init(uint8_t  node_addr,
                      uint8_t  master_slave,
                      uint32_t  rx_timeout,
                      uint8_t  modbus_mode,
                      uint8_t  port_nbr,
                      uint32_t  baud,
                      uint8_t  bits,
                      uint8_t  parity,
                      uint8_t  stops)
{
  T_MODBUS_ch   *pch;
  uint16_t   cnts;

  if (modbus_cbls_counter < MODBUS_CFG_MAX_CH)
  {
    pch =&modbus_cbls[modbus_cbls_counter];
    MB_set_rx_timeout(pch, rx_timeout);
    MB_set_node_addr(pch, node_addr);
    MB_set_mode(pch, master_slave, modbus_mode);
    MB_set_UART_port(pch, port_nbr);
    MB_BSP_config_UART(pch, port_nbr, baud, bits, parity, stops);
    if (pch->master_slave_flag == MODBUS_MASTER)
    {
      pch->rtu_timeout_en = MODBUS_FALSE;
    }
    cnts =((uint32_t)RTU_TIMER_TICK_FREQ * 15L * 10L) / baud;     // Увеличиваем таймаут до 15 символов. 5 символов приводят к ненадежной коммуникации c OMRON MX2
    if (cnts <= 1)
    {
      cnts = 2;
    }
    pch->rtu_timeout = cnts;
    pch->rtu_timeout_counter  = cnts;

    modbus_cbls_counter++;
    return (pch);
  }
  else
  {
    return ((T_MODBUS_ch *)0);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param pch
  \param timeout
-----------------------------------------------------------------------------------------------------*/
void  MB_set_rx_timeout(T_MODBUS_ch  *pch, uint32_t  timeout)
{
  if (pch != (T_MODBUS_ch *)0)
  {
    pch->rx_timeout = timeout;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param pch
  \param master_slave
  \param mode
-----------------------------------------------------------------------------------------------------*/
void  MB_set_mode(T_MODBUS_ch  *pch, uint8_t  master_slave, uint8_t  mode)
{
  if (pch != (T_MODBUS_ch *)0)
  {

    switch (master_slave)
    {
    case MODBUS_MASTER:
      pch->master_slave_flag = MODBUS_MASTER;
      break;

    case MODBUS_SLAVE:
    default:
      pch->master_slave_flag = MODBUS_SLAVE;
      break;
    }

    switch (mode)
    {
    case MODBUS_MODE_ASCII:
      pch->rtu_ascii_mode = MODBUS_MODE_ASCII;
      break;

    case MODBUS_MODE_RTU:
      pch->rtu_ascii_mode = MODBUS_MODE_RTU;
      break;

    default:
      pch->rtu_ascii_mode = MODBUS_MODE_RTU;
      break;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param pch
  \param node_addr
-----------------------------------------------------------------------------------------------------*/
void  MB_set_node_addr(T_MODBUS_ch  *pch, uint8_t  node_addr)
{
  if (pch != (T_MODBUS_ch *)0)
  {
    pch->slave_node_addr = node_addr;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param pch
  \param port_nbr
-----------------------------------------------------------------------------------------------------*/
void  MB_set_UART_port(T_MODBUS_ch  *pch, uint8_t  port_nbr)
{
  if (pch != (T_MODBUS_ch *)0)
  {
    pch->uart_number = port_nbr;
  }
}


/*-----------------------------------------------------------------------------------------------------
  Прием байта.
  Функция вызывается в обработчике прерываний UART

  \param pch
  \param rx_byte
-----------------------------------------------------------------------------------------------------*/
void  MB_rx_byte(T_MODBUS_ch  *pch, uint8_t  rx_byte)
{
  switch (pch->rtu_ascii_mode)
  {

  case MODBUS_MODE_ASCII:
    MB_ASCII_rx_byte(pch, rx_byte & 0x7F);
    break;

  case MODBUS_MODE_RTU:
    MB_RTU_rx_byte(pch, rx_byte);
    break;

  default:
    break;
  }
}


/*-----------------------------------------------------------------------------------------------------
  Прием байта в режиме ASCII
  Функция вызывается в обработчике прерываний UART


  \param pch       Is a pointer to the Modbus channel's data structure.
  \param rx_byte   Is the byte received.
-----------------------------------------------------------------------------------------------------*/
void  MB_ASCII_rx_byte(T_MODBUS_ch  *pch, uint8_t  rx_byte)
{
  uint8_t    node_addr;
  uint8_t   *phex;

  if (rx_byte == ':')
  {
    pch->rx_packet_ptr     =&pch->rx_buf[0];
    pch->rx_packet_size = 0;
  }
  if (pch->rx_packet_size < MODBUS_CFG_BUF_SIZE)
  {
    *pch->rx_packet_ptr++= rx_byte;
    pch->rx_packet_size++;

  }
  if (rx_byte == MODBUS_ASCII_END_FRAME_CHAR2)
  {
    phex      =&pch->rx_buf[1];
    node_addr = MB_ASCII_HexToBin(phex);
    if ((node_addr == pch->slave_node_addr) || (node_addr == 0))
    {
      MB_OS_packet_end_signal(pch);   // Сигналим о завершении если получили символ конца пакета
    }
    else
    {
      pch->rx_packet_ptr     =&pch->rx_buf[0];
      pch->rx_packet_size = 0;
    }
  }
}


/*-----------------------------------------------------------------------------------------------------
  Парсим и формируем промежуточный буфер из поступившего ASCII пакета данных

  \param pch

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  MB_ASCII_Rx(T_MODBUS_ch  *pch)
{
  uint8_t    *pmsg;
  uint8_t    *prx_data;
  uint16_t     rx_size;


  pmsg      =&pch->rx_buf[0];
  rx_size   = pch->rx_packet_size;
  prx_data  =&pch->rx_frame_buf[0];
  if ((rx_size & 0x01) &&
    (rx_size            > MODBUS_ASCII_MIN_MSG_SIZE) &&
    (pmsg[0]           == MODBUS_ASCII_START_FRAME_CHAR) &&
    (pmsg[rx_size - 2] == MODBUS_ASCII_END_FRAME_CHAR1) &&
    (pmsg[rx_size - 1] == MODBUS_ASCII_END_FRAME_CHAR2))
  {
    rx_size -= 3;
    pmsg++;
    pch->rx_data_cnt = 0;
    while (rx_size > 2)
    {
      *prx_data++= MB_ASCII_HexToBin(pmsg);
      pmsg        += 2;
      rx_size     -= 2;
      pch->rx_data_cnt++;
    }
    pch->rx_data_cnt -= 2;
    pch->rx_packet_crc_ref = (uint16_t)MB_ASCII_HexToBin(pmsg);
    return (1);
  }
  else
  {
    return (0);
  }
}


/*-----------------------------------------------------------------------------------------------------
  ормируем окончательный ASCII пакет из промежуточного буфера

  \param pch
-----------------------------------------------------------------------------------------------------*/
void  MB_M_send_ASCII_packet(T_MODBUS_ch  *pch)
{
  uint8_t    *ptx_data;
  uint8_t    *pbuf;
  uint16_t     i;
  uint16_t     tx_bytes;
  uint8_t     lrc;


  ptx_data =&pch->tx_frame_buf[0];
  pbuf     =&pch->tx_buf[0];
  *pbuf++= MODBUS_ASCII_START_FRAME_CHAR;
  pbuf     = MB_ASCII_BinToHex(*ptx_data++, pbuf);
  pbuf     = MB_ASCII_BinToHex(*ptx_data++, pbuf);
  tx_bytes = 5;
  i        = (uint8_t)pch->tx_data_cnt;
  while (i > 0)
  {
    pbuf      = MB_ASCII_BinToHex(*ptx_data++, pbuf);
    tx_bytes += 2;
    i--;
  }
  lrc     = MB_ASCII_TxCalcLRC(pch, tx_bytes);
  pbuf    = MB_ASCII_BinToHex(lrc,  pbuf);
  *pbuf++= MODBUS_ASCII_END_FRAME_CHAR1;
  *pbuf++= MODBUS_ASCII_END_FRAME_CHAR2;
  tx_bytes         += 4;
  pch->tx_packet_crc   = (uint16_t)lrc;
  pch->tx_packet_size = tx_bytes;
  MB_BSP_send_packet(pch);
}


/*-----------------------------------------------------------------------------------------------------
  Прием байта в режиме RTU
  Функция вызывается в обработчике прерываний UART

  \param pch
  \param rx_byte
-----------------------------------------------------------------------------------------------------*/
void  MB_RTU_rx_byte(T_MODBUS_ch  *pch, uint8_t  rx_byte)
{
  MB_RTU_TmrReset(pch); // После приема каждого байта сбрасываем счетчик таймаута ожидания

  if (pch->master_slave_flag == MODBUS_MASTER)
  {
    pch->rtu_timeout_en = MODBUS_TRUE;  // Разрешаем отслеживание таймаута
  }

  if (pch->rx_packet_size < MODBUS_CFG_BUF_SIZE)
  {
    *pch->rx_packet_ptr++= rx_byte;
    pch->rx_packet_size++;
  }

}


/*-----------------------------------------------------------------------------------------------------
  Парсим и формируем промежуточный буфер из поступившего RTU пакета данных

  \param pch

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  MB_RTU_Rx(T_MODBUS_ch  *pch)
{
  uint8_t    *prx_data;
  uint8_t    *pmsg;
  uint16_t     rx_size;
  uint16_t     crc;


  pmsg    =&pch->rx_buf[0];
  rx_size =  pch->rx_packet_size;
  if (rx_size >= MODBUS_RTU_MIN_MSG_SIZE)    // Не слишком ли короткий пакет
  {
    if (rx_size <= MODBUS_CFG_BUF_SIZE)      // Не слишком ли длинный пакет
    {
      prx_data    =&pch->rx_frame_buf[0];
      *prx_data++=*pmsg++;
      rx_size--;

      *prx_data++=*pmsg++;
      rx_size--;

      pch->rx_data_cnt = 0;
      while (rx_size > 2)
      {
        *prx_data++=*pmsg++;
        pch->rx_data_cnt++;
        rx_size--;
      }

      crc              = (uint16_t)*pmsg++;
      crc             += (uint16_t)*pmsg << 8;
      pch->rx_packet_crc_ref  = crc;
      return (1);
    }
    else
    {
      // Пакет слишком длинный
      return (0);
    }
  }
  else
  {
    // Пакет слишком короткий
    return (0);
  }
}


/*-----------------------------------------------------------------------------------------------------
  Формируем окончательный RTU пакет из промежуточного буфера

  \param pch
-----------------------------------------------------------------------------------------------------*/
void  MB_M_send_RTU_packet(T_MODBUS_ch  *pch)
{
  uint8_t  *ptx_data;
  uint8_t  *pbuf;
  uint8_t   i;
  uint16_t   tx_bytes;
  uint16_t   crc;


  tx_bytes  = 0;
  pbuf      =&pch->tx_buf[0];
  ptx_data  =&(pch->tx_frame_buf[0]);
  i         = (uint8_t)pch->tx_data_cnt + 2;
  while (i > 0)
  {
    *pbuf++=*ptx_data++;
    tx_bytes++;
    i--;
  }
  crc       = MB_RTU_TxCalcCRC(pch);
  *pbuf++= (uint8_t)(crc & 0x00FF);
  *pbuf     = (uint8_t)(crc >> 8);
  tx_bytes += 2;
  pch->tx_packet_crc   = crc;
  pch->tx_packet_size = tx_bytes;

  MB_BSP_send_packet(pch);
}


/*-----------------------------------------------------------------------------------------------------


  \param pch
-----------------------------------------------------------------------------------------------------*/
void  MB_RTU_TmrReset(T_MODBUS_ch  *pch)
{
  pch->rtu_timeout_counter = pch->rtu_timeout;
}


/*-----------------------------------------------------------------------------------------------------


  \param
-----------------------------------------------------------------------------------------------------*/
void  MB_RTU_TmrResetAll(void)
{
  uint8_t   ch;
  T_MODBUS_ch   *pch;


  pch =&modbus_cbls[0];
  for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++)
  {
    if (pch->rtu_ascii_mode == MODBUS_MODE_RTU)
    {
      MB_RTU_TmrReset(pch);
    }
    pch++;
  }
}


/*-----------------------------------------------------------------------------------------------------
  Ведем счетчик таймаута ожидания окончания пакета данных в режиме RTU
  Вызывается из обработчика прерывания таймера

  \param
-----------------------------------------------------------------------------------------------------*/
void  MB_RTU_packet_end_detector(void)
{
  uint8_t   ch;
  T_MODBUS_ch   *pch;


  pch =&modbus_cbls[0];

  for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++)
  {
    if (pch->rtu_ascii_mode == MODBUS_MODE_RTU)
    {
      if (pch->rtu_timeout_en == MODBUS_TRUE)
      {
        if (pch->rtu_timeout_counter > 0)
        {
          pch->rtu_timeout_counter--;
          if (pch->rtu_timeout_counter == 0)
          {
            if (pch->master_slave_flag == MODBUS_MASTER)
            {
              pch->rtu_timeout_en = MODBUS_FALSE;
            }
            MB_OS_packet_end_signal(pch); // Сигналим, о том что истек таймаут ожидания данных.  Это также означает конец приема пакета
            MB_BSP_stop_receiving(pch);   // Запрещаем дальнейший прием, чтобы не получать случайные помехи
          }
        }
      }
      else
      {
        pch->rtu_timeout_counter = pch->rtu_timeout;
      }
    }
    pch++;
  }
}

