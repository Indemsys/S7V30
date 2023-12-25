#include "S7V30.h"
#include "Modbus.h"

#define RTU_PACKET_END_FLAG  BIT(1)
static  TX_EVENT_FLAGS_GROUP MB_OS_rx_signal_semafores[MODBUS_CFG_MAX_CH]; // Массив семафоров для каналов мастера через которые сигнализируется о приеме данных

static  void                 MB_OS_init_master(void);
static  void                 MB_OS_deinit_master(void);

/*-----------------------------------------------------------------------------------------------------
  Инициализация сервисов RTOS используемых в работе мастера

  \param
-----------------------------------------------------------------------------------------------------*/
static  void  MB_OS_init_master(void)
{
  uint8_t   i;

  for (i = 0; i < MODBUS_CFG_MAX_CH; i++)                             /* Create a semaphore for each channel   */
  {
    tx_event_flags_create(&MB_OS_rx_signal_semafores[i], "MODBUS");
  }
}

/*-----------------------------------------------------------------------------------------------------
  Инициализация сервисов RTOS используемых в работе модуля MODBUS


  \param void
-----------------------------------------------------------------------------------------------------*/
void  MB_OS_Init(void)
{
  MB_OS_init_master();
}

/*-----------------------------------------------------------------------------------------------------
  Завершение работы сервисов RTOS задействованных в работе мастера

  \param
-----------------------------------------------------------------------------------------------------*/
static  void  MB_OS_deinit_master(void)
{
  uint8_t  i;

  for (i = 0; i < MODBUS_CFG_MAX_CH; i++)
  {
    tx_event_flags_delete(&MB_OS_rx_signal_semafores[i]);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Завершение работы сервисов RTOS задействованных в модуле MODBUS

  \param void
-----------------------------------------------------------------------------------------------------*/
void  MB_OS_deinit(void)
{
  MB_OS_deinit_master();
}

/*-----------------------------------------------------------------------------------------------------
  Функция сигнализирующая об окончании приема пакета
  Вызывается из обработчика прерывания таймера в режиме RTU
  или из обработчика прерываний  UART в режие ASCII

  \param pch
-----------------------------------------------------------------------------------------------------*/
void  MB_OS_packet_end_signal(T_MODBUS_ch *pch)
{
  if (pch != (T_MODBUS_ch *)0)
  {
    switch (pch->master_slave_flag)
    {
    case MODBUS_MASTER:
      tx_event_flags_set(&MB_OS_rx_signal_semafores[pch->modbus_channel], RTU_PACKET_END_FLAG, TX_OR);
      break;
    }
  }
}


/*-----------------------------------------------------------------------------------------------------
  Ожидаем получения пакета данных

  \param pch
  \param perr
-----------------------------------------------------------------------------------------------------*/
void  MB_OS_wait_rx_packet_end(T_MODBUS_ch   *pch, uint16_t  *perr)
{

  ULONG     actual_flags = 0;
  uint32_t  res;


  if (pch != (T_MODBUS_ch *)0)
  {
    if (pch->master_slave_flag == MODBUS_MASTER)
    {
      res = tx_event_flags_get(&MB_OS_rx_signal_semafores[pch->modbus_channel], RTU_PACKET_END_FLAG, TX_OR_CLEAR,&actual_flags, MODBUS_ANSWER_TIMEOUT);

      switch (res)
      {
      case TX_WAIT_ERROR:
        if (pch->rx_packet_size < MODBUS_RTU_MIN_MSG_SIZE)
        {
          *perr = MODBUS_ERR_TIMED_OUT;
        }
        else
        {
          *perr = MODBUS_ERR_NONE;
        }
        break;

      case TX_SUCCESS:
        *perr = MODBUS_ERR_NONE;
        break;
      default:
        *perr = MODBUS_ERR_INVALID;
        break;
      }
    }
    else
    {
      *perr = MODBUS_ERR_NOT_MASTER;
    }
  }
  else
  {
    *perr = MODBUS_ERR_NULLPTR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t   Get_MODBUS_min_ans_time(void)
{
  return modbus_cbls[0].ans_min_time;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t   Get_MODBUS_max_ans_time(void)
{
  return modbus_cbls[0].ans_max_time;
}
