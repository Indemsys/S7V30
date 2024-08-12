// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.22
// 11:49:41
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"


#define              TELNET_SRV_NAME  "Telnet_srv"
#define              TELNET_SERVER_STACK_SIZE  (1024*2)
#pragma data_alignment=8
uint8_t             telnet_server_stack_memory[TELNET_SERVER_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.telnet_server")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
;
NX_TELNET_SERVER    telnet_server;


static int Mn_telnet_drv_init(void **pcbl, void *pdrv);
static int Mn_telnet_drv_send_buf(const void *buf, unsigned int len);
static int Mn_telnet_drv_wait_ch(unsigned char *b, int timeout);
static int Mn_telnet_drv_printf(const char  *fmt_ptr, ...);
static int Mn_telnet_drv_deinit(void **pcbl);


T_serial_io_driver mon_telnet_driver =
{
  MN_DRIVER_MARK,
  MN_NET_TELNET_DRIVER,
  Mn_telnet_drv_init,
  Mn_telnet_drv_send_buf,
  Mn_telnet_drv_wait_ch,
  Mn_telnet_drv_printf,
  Mn_telnet_drv_deinit,
  0,
};


static int32_t telnet_vt100_task_instance_index = -1; // Индекс

#define   TELNET_LOGICAL_CONNECTION  0


#define   MB_TELNET_READ_REQUEST   BIT(0)
#define   MB_TELNET_READ_DONE      BIT(1)

#define   TELNET_BUFFER_MAX_LENGTH 512
#define   TELNET_DRV_STR_SZ        512
#define   IN_BUF_QUANTITY          2           // Количество приемных буферов

// Ведем прием циклически в N приемных буферов
typedef struct
{
    uint32_t  actual_len;  // Длина пакета
    uint8_t   buff[NX_TELNET_SERVER_PACKET_PAYLOAD]; // Буфер с пакетов

} T_telnet_in_buf_cbl;


typedef struct
{
    char                   str[TELNET_DRV_STR_SZ];
    TX_EVENT_FLAGS_GROUP   evt;         // Группа флагов для взаимодействия с задачей приема
    char                   evt_grp_name[32];
    void                  *dbuf;              // Указатель на буфер с принимаемыми данными
    uint32_t               dsz;               // Количество принимаемых байт
    volatile uint8_t       head_buf_indx;     //  Индекс головы циклической очереди буферов приема
    volatile uint8_t       tail_buf_indx;     //  Индекс хвоста циклической очереди буферов приема
    volatile uint32_t      all_buffers_full;  //  Сигнал о том что все приемные буферы заняты принятыми данными
    uint32_t               tail_buf_rd_pos;   //  Позиция чтения в хвостовом буфере
    T_telnet_in_buf_cbl    in_bufs_ring[IN_BUF_QUANTITY]; // Кольцо управляющих структур приема-обработки входящих пакетов

} T_telnet_drv_cbl;

T_telnet_drv_cbl telnet_drv_cbl;

/*-----------------------------------------------------------------------------------------------------


  pcbl - указатель на указатель на структуру со специальными данными необходимыми драйверу
  pdrv - указатель на структуру T_monitor_driver

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_telnet_drv_init(void **pcbl, void *pdrv)
{
  // Если драйвер еще не был инициализирован, то выделить память для управлющей структуры и ждать сигнала из интерфеса
  if (*pcbl == 0)
  {
    telnet_drv_cbl.head_buf_indx    = 0;
    telnet_drv_cbl.tail_buf_indx    = 0;
    telnet_drv_cbl.all_buffers_full = 0;
    telnet_drv_cbl.tail_buf_rd_pos  = 0;
    sprintf(telnet_drv_cbl.evt_grp_name,   "Telnet Drv");
    if (tx_event_flags_create(&(telnet_drv_cbl.evt),telnet_drv_cbl.evt_grp_name) != TX_SUCCESS)
    {
      return RES_ERROR;
    }
    *pcbl = &telnet_drv_cbl;
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param pcbl

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_telnet_drv_deinit(void **pcbl)
{
  T_telnet_drv_cbl *p;
  if (*pcbl != 0)
  {
    p = *pcbl;
    tx_event_flags_delete(&(p->evt));
    *pcbl = 0;
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param len

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_telnet_drv_send_buf(const void *buf, unsigned int len)
{
  UINT                     res;
  NX_PACKET                *packet;

  res = nx_packet_allocate(&net_packet_pool,&packet, NX_TCP_PACKET, MS_TO_TICKS(10));
  if (res != NX_SUCCESS) return RES_ERROR;

  res = nx_packet_data_append(packet, (void *)buf, len,&net_packet_pool, MS_TO_TICKS(10));
  if (res != NX_SUCCESS)
  {
    nx_packet_release(packet);
    return RES_ERROR;
  }

  res = nx_telnet_server_packet_send(&telnet_server,TELNET_LOGICAL_CONNECTION,packet, MS_TO_TICKS(10));
  if (res != NX_SUCCESS)
  {
    nx_packet_release(packet);
    return RES_ERROR;
  }

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param fmt_ptr

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_telnet_drv_printf(const char  *fmt_ptr, ...)
{
  UINT              res;
  NX_PACKET        *packet;
  T_serial_io_driver *mdrv = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_telnet_drv_cbl *p      = (T_telnet_drv_cbl *)(mdrv->drv_cbl_ptr);
  char             *s = p->str;
  uint32_t         len;
  va_list           ap;


  va_start(ap, fmt_ptr);
  len = vsnprintf(s, TELNET_DRV_STR_SZ, (const char *)fmt_ptr, ap);
  va_end(ap);

  if (len > 0)
  {
    res = nx_packet_allocate(&net_packet_pool,&packet, NX_TCP_PACKET, MS_TO_TICKS(10));
    if (res != NX_SUCCESS) return RES_ERROR;

    res = nx_packet_data_append(packet, s, len,&net_packet_pool, MS_TO_TICKS(10));
    if (res != NX_SUCCESS)
    {
      nx_packet_release(packet);
      return RES_ERROR;
    }

    res = nx_telnet_server_packet_send(&telnet_server, TELNET_LOGICAL_CONNECTION, packet, MS_TO_TICKS(10));
    if (res != NX_SUCCESS)
    {
      nx_packet_release(packet);
      return RES_ERROR;
    }
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param b
  \param timeout - время ожидания выражается в тиках, если 0 то нет ожидания

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_telnet_drv_wait_ch(unsigned char *b, int timeout)
{
  ULONG        actual_flags;
  uint32_t     tail_buf_number;
  uint8_t      head_buf_number;

  T_serial_io_driver *mdrv = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_telnet_drv_cbl *p      = (T_telnet_drv_cbl *)(mdrv->drv_cbl_ptr);


  // Если индексы буферов равны то это значит отсутствие принятых пакетов
  head_buf_number = p->head_buf_indx;
  if (p->tail_buf_indx == head_buf_number)
  {
    if (tx_event_flags_get(&(p->evt), MB_TELNET_READ_DONE, TX_AND_CLEAR,&actual_flags, MS_TO_TICKS(timeout)) != TX_SUCCESS)
    {
      return RES_ERROR;
    }
    // Еще раз проверяем наличие данных поскольку флаг мог остаться от предыдущего чтения, когда данные били приняты без проверки флага и соответственно без его сброса
    head_buf_number = p->head_buf_indx;
    if (p->tail_buf_indx == head_buf_number)
    {
      return RES_ERROR;
    }
  }

  tail_buf_number = p->tail_buf_indx;               // Получаем индекс хвостового буфера
  *b = p->in_bufs_ring[tail_buf_number].buff[p->tail_buf_rd_pos]; // Читаем байт данных из хвостового буфера
  p->tail_buf_rd_pos++;                        // Смещаем указатель на следующий байт данных

  // Если позиция достигла конца данных в текущем буфере, то буфер освобождается для приема
  if (p->tail_buf_rd_pos >= p->in_bufs_ring[tail_buf_number].actual_len)
  {
    p->tail_buf_rd_pos = 0;
    // Смещаем указатель хвоста очереди приемных буфферов
    // Появляется место для движения головы очереди приемных буфферов

    tail_buf_number++;
    if (tail_buf_number >= IN_BUF_QUANTITY) tail_buf_number = 0;
    p->tail_buf_indx = tail_buf_number;

    // Если очередь пакетов была заполнена, то сообщить задаче о продолжении приема
    if (p->all_buffers_full == 1)
    {
      p->all_buffers_full = 0;
      if (tx_event_flags_set(&(p->evt), MB_TELNET_READ_REQUEST, TX_OR) != TX_SUCCESS)
      {
        Wait_ms(timeout); // Задержка после ошибки нужна для того чтобы задача не захватила все ресурсы в случает постоянного появления ошибки
        return RES_ERROR;
      }
    }
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается из функции Telnet_receive_data_callback

  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Read_data_from_net(NX_PACKET *packet_ptr)
{
  UINT              res;
  ULONG             actual_length;
  ULONG             actual_flags;
  uint32_t          head_buf_number;

  head_buf_number = telnet_drv_cbl.head_buf_indx;

  res = nx_packet_data_retrieve(packet_ptr, telnet_drv_cbl.in_bufs_ring[head_buf_number].buff,&actual_length);
  if (res == NX_SUCCESS)
  {
    telnet_drv_cbl.in_bufs_ring[head_buf_number].actual_len = actual_length;

    head_buf_number++;
    if (head_buf_number >= IN_BUF_QUANTITY) head_buf_number = 0;
    telnet_drv_cbl.head_buf_indx = head_buf_number;

    // Выставляем флаг выполненного чтения
    if (tx_event_flags_set(&(telnet_drv_cbl.evt), MB_TELNET_READ_DONE, TX_OR) == TX_SUCCESS)
    {
      // Если все буферы на прием заполнены, то значит системе не требуются данные
      if (telnet_drv_cbl.tail_buf_indx == head_buf_number)
      {
        // Перестаем принимать данные и ждем 10 мс когда система обработает уже принятые данные и подаст сигнал к началу приема
        // Если система в течении 10 мс не взяла данные, то данные перезаписываются новыми
        telnet_drv_cbl.all_buffers_full = 1;
        tx_event_flags_get(&(telnet_drv_cbl.evt), MB_TELNET_READ_REQUEST, TX_AND_CLEAR,&actual_flags, MS_TO_TICKS(10));
      }
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  При подключении Telnet порта создаем задачу VT100
  Функция вызывается из задачи VT100_task_manager_thread по сообщению из функции _Telnet_client_connect_callback


  \param msg_ptr
-----------------------------------------------------------------------------------------------------*/
static void Init_monitor_Telnet_driver(T_vt100_man_msg *msg_ptr)
{
  NX_TELNET_SERVER *telnet_server_ptr   = (NX_TELNET_SERVER*)msg_ptr->arg2;
  UINT              logical_connection  = (UINT)msg_ptr->arg3;
  uint32_t res;

  res = Task_VT100_create(&mon_telnet_driver,&telnet_vt100_task_instance_index); // В контексте этого вызова будут выполнены функции драйвера init и deinit
  if (res == RES_OK)
  {
    NET_LOG("Telnet VT100 task %d created", telnet_vt100_task_instance_index);
    Task_VT100_start(telnet_vt100_task_instance_index);
  }
  else
  {
    nx_telnet_server_disconnect(telnet_server_ptr, logical_connection);
    NET_LOG("Telnet VT100 task %d creation error");
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param msg_ptr
-----------------------------------------------------------------------------------------------------*/
static void DeInit_monitor_Telnet_driver(T_vt100_man_msg *msg_ptr)
{
  Task_VT100_delete(telnet_vt100_task_instance_index);
  telnet_vt100_task_instance_index = -1;
  NET_LOG("Telnet VT100 port disconnected");
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте потока сервера Telnet

  Здесь допускается создание ТОЛЬКО ОДНОГО! Telnet соединения с задачей монитора VT100

  \param telnet_server_ptr
  \param logical_connection
-----------------------------------------------------------------------------------------------------*/
static void _Telnet_client_connect_callback(NX_TELNET_SERVER *telnet_server_ptr, UINT logical_connection)
{
  // Создать задачу VT100 для данного соединения и передать ей драйвер
  if ((logical_connection == TELNET_LOGICAL_CONNECTION) && (telnet_server_ptr == &telnet_server))
  {
    // Если задача VT100 еще не создана для Telnet соединения, то создать ее
    if (telnet_vt100_task_instance_index < 0)
    {
      T_vt100_man_msg msg_ptr;
      msg_ptr.arg1 = (uint32_t)Init_monitor_Telnet_driver;
      msg_ptr.arg2 = (uint32_t)telnet_server_ptr;
      msg_ptr.arg3 = logical_connection;
      Send_message_to_VT100_task_manager(&msg_ptr);
    }
    else
    {
      nx_telnet_server_disconnect(telnet_server_ptr, logical_connection);
      NET_LOG("Telnet logical connection %d rejected", logical_connection);
    }
  }
  else
  {
    nx_telnet_server_disconnect(telnet_server_ptr, logical_connection);
    NET_LOG("Telnet logical connection %d rejected", logical_connection);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте потока сервера Telnet

  \param telnet_server_ptr
  \param logical_connection
-----------------------------------------------------------------------------------------------------*/
static void _Telnet_client_disconnect_callback(NX_TELNET_SERVER *telnet_server_ptr, UINT logical_connection)
{
  if ((telnet_server_ptr == &telnet_server) && (logical_connection == TELNET_LOGICAL_CONNECTION))
  {
    T_vt100_man_msg msg_ptr;
    msg_ptr.arg1 = (uint32_t)DeInit_monitor_Telnet_driver;
    msg_ptr.arg2 = 0;
    msg_ptr.arg3 = 0;
    Send_message_to_VT100_task_manager(&msg_ptr);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте потока сервера Telnet

  \param telnet_server_ptr
  \param logical_connection
  \param packet_ptr
-----------------------------------------------------------------------------------------------------*/
static void _Telnet_receive_data_callback(NX_TELNET_SERVER *telnet_server_ptr, UINT logical_connection, NX_PACKET *packet)
{
  if ((telnet_server_ptr == &telnet_server) && (logical_connection == TELNET_LOGICAL_CONNECTION))
  {
    _Read_data_from_net(packet);
  }
  // В конце всегда нужно пакет освободить или переиспользовать
  nx_packet_release(packet);
}


/*-----------------------------------------------------------------------------------------------------
  Вызывается из задач создания сетевого соединения

  \param telnet_server_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_TELNET_server_create(NX_IP *ip)
{
  UINT    err;

  if (ivar.en_telnet == 0) return RES_ERROR;
  if (telnet_server.nx_telnet_server_id !=  0)  return RES_OK;


  err = nx_telnet_server_create(&telnet_server,
                                TELNET_SRV_NAME,
                                ip,
                                &telnet_server_stack_memory[0],
                                TELNET_SERVER_STACK_SIZE,
                                _Telnet_client_connect_callback,
                                _Telnet_receive_data_callback,
                                _Telnet_client_disconnect_callback);
  if (NX_SUCCESS != err)
  {
    NET_LOG("Telent server: Error %d", err);
    return RES_ERROR;
  }

  err = nx_telnet_server_packet_pool_set(&telnet_server,&net_packet_pool);
  if (NX_SUCCESS != err)
  {
    nx_telnet_server_delete(&telnet_server);
    NET_LOG("Telent server: Error %d", err);
    return RES_ERROR;
  }

  err = nx_telnet_server_start(&telnet_server);
  if (err != NX_SUCCESS)
  {
    nx_telnet_server_delete(&telnet_server);
    NET_LOG("Telent server: Error %d", err);
    return RES_ERROR;
  }
  NET_LOG("Telent server: Started");
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается из задач удаления сетевого соединения

  \param telnet_server_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t TELNET_server_delete(void)
{
  UINT res;

  if (telnet_server.nx_telnet_server_id !=  0)
  {
    res = nx_telnet_server_delete(&telnet_server);
    NET_LOG("Telent server: Deleted. Resuly=%04X", res);
    return res;
  }
  else
  {
    return RES_OK;
  }
}

