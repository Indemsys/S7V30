// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-07-02
// 17:45:08
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BLE_main.h"

//#define ENAVBLE_SPP_RTT_LOG

#ifdef ENAVBLE_SPP_RTT_LOG
  #define SPP_LOGs RTT_LOGs
#else
  #define SPP_LOGs(...)
#endif

static int Mn_bluetooth_drv_init(void **pcbl, void *pdrv);
static int Mn_bluetooth_drv_send_buf(const void *buf, unsigned int len);
static int Mn_bluetooth_drv_wait_ch(unsigned char *b, int timeout);
static int Mn_bluetooth_drv_printf(const char  *fmt_ptr, ...);
static int Mn_bluetooth_drv_deinit(void **pcbl);

const T_serial_io_driver mon_bt_com_driver =
{
  MN_DRIVER_MARK,
  MN_BLUETOOTH_DRIVER,
  Mn_bluetooth_drv_init,
  Mn_bluetooth_drv_send_buf,
  Mn_bluetooth_drv_wait_ch,
  Mn_bluetooth_drv_printf,
  Mn_bluetooth_drv_deinit,
  0,
};

#define   MB_BLUETOOTH_TX_COMPLETE  BIT(0)
#define   MB_BLUETOOTH_READ_DONE    BIT(1)

static    uint8_t module_inited;

T_bluetooth_serial_drv_cbl bt_cbl  __attribute__((section(".bt_cbls")));

/*-----------------------------------------------------------------------------------------------------


  \param ptr
-----------------------------------------------------------------------------------------------------*/
static void _Receiving_form_rfcomm(uint16_t handle)
{
  uint32_t          res;
  uint16_t          actual_length;
  int32_t           head_buf_number;
  uint8_t           stop_flow = 0;
  TX_INTERRUPT_SAVE_AREA;

  head_buf_number = bt_cbl.head_buf_indx;
  res = (uint32_t)wiced_bt_rfcomm_read_data(handle, (char *)bt_cbl.in_bufs_ring[head_buf_number].buff, BLUETOOTH_VT100_DRV_BUFFER_MAX_LENGTH,&actual_length);

  if ((res == 0) && (actual_length != 0))
  {
    SPP_LOGs("SPP: Recv from rfcomm %d bytes\r\n", actual_length);

    // Перемещаем указатель на следующий буфер в кольце буферов приема
    // Чтобы не было конфликта с задачей приема в терминале, запрещаем прерывания на этом участке
    bt_cbl.in_bufs_ring[head_buf_number].actual_len = actual_length;
    head_buf_number++;
    if (head_buf_number >= BLUETOOTH_VT100_DRV_BUF_QUANTITY) head_buf_number = 0;

    TX_DISABLE;
    if (bt_cbl.tail_buf_indx == head_buf_number)
    {
      bt_cbl.all_buffers_full = 1;
      stop_flow = 1;
    }
    bt_cbl.head_buf_indx = head_buf_number;
    TX_RESTORE;

    // Выставляем флаг выполненного чтения
    tx_event_flags_set(bt_cbl.event_flags_ptr, MB_BLUETOOTH_READ_DONE, TX_OR);

    if (stop_flow)
    {
      // Перестаем принимать данные из RFCOMM, запрещаем поток данных
      wiced_bt_rfcomm_flow_control(handle, FALSE);
    }
  }
  else
  {
    SPP_LOGs("SPP: Recv from rfcomm error. res=%d. len=%d\r\n", res,actual_length);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param handle
-----------------------------------------------------------------------------------------------------*/
static void _Transmit_complete_form_rfcomm(uint16_t handle)
{
  SPP_LOGs("SPP: Transmit complete\r\n");
  tx_event_flags_set(bt_cbl.event_flags_ptr, MB_BLUETOOTH_TX_COMPLETE, TX_OR);

}


/*-----------------------------------------------------------------------------------------------------
  При подключении Bluetooth COM порта создаем задачу VT100
  Функция вызывается из задачи VT100_task_manager_thread по сообщению из функции Monitor_Bluetooth_Init_callback

  \param msg_ptr
-----------------------------------------------------------------------------------------------------*/
static void Init_monitor_bluetooth_driver(T_vt100_man_msg *msg_ptr)
{
  uint32_t res           = RES_ERROR;

  if (module_inited == 0)
  {
    memset(&bt_cbl, 0, sizeof(bt_cbl));
    module_inited = 1;
  }

  if (bt_cbl.taken == 0)
  {

    memcpy(&bt_cbl.serial_io_driver,&mon_bt_com_driver, sizeof(mon_bt_com_driver));
    bt_cbl.serial_io_driver.drv_cbl_ptr = &bt_cbl;
    bt_cbl.spp_ptr = (void *)msg_ptr->arg2;

    res = Task_VT100_create(&bt_cbl.serial_io_driver,&bt_cbl.vt100_task_instance_index); // В контексте этого вызова будут выполнены функции драйвера init и deinit
    if (res == RES_OK)
    {
      // Выделяем память для задачи приема из USB
      bt_cbl.event_flags_ptr     = App_malloc(sizeof(TX_EVENT_FLAGS_GROUP));
      if (bt_cbl.event_flags_ptr == NULL)
      {
        goto err_exit;
      }
      if (tx_event_flags_create(bt_cbl.event_flags_ptr,"RFCOMM_drv") != TX_SUCCESS)
      {
        goto err_exit;
      }
      bt_cbl.taken = 1;
      BT_ssp_set_rx_callback(_Receiving_form_rfcomm);
      BT_ssp_set_tx_cmpl_callback(_Transmit_complete_form_rfcomm);
      Task_VT100_start(bt_cbl.vt100_task_instance_index);
      APPLOG("Bluetooth VT100 task %d created", bt_cbl.vt100_task_instance_index);
    }
    else
    {
      goto err_exit;
    }
  }

  return;

err_exit:
  App_free(bt_cbl.event_flags_ptr);
  Task_VT100_delete(bt_cbl.vt100_task_instance_index);
  bt_cbl.taken = 0;
  APPLOG("Bluetooth VT100 task creating error");
  return;
}

/*-----------------------------------------------------------------------------------------------------
  При отключении Bluetooth COM порта удаляем задачу VT100 и задачу драйвера
  Функция вызывается из задачи VT100_task_manager_thread по сообщению из функции Monitor_Bluetooth_DeInit_callback


  \param msg_ptr
-----------------------------------------------------------------------------------------------------*/
static void DeInit_monitor_bluetooth_driver(T_vt100_man_msg *msg_ptr)
{

  Task_VT100_suspend(bt_cbl.vt100_task_instance_index);
  BT_ssp_set_rx_callback(NULL);
  BT_ssp_set_tx_cmpl_callback(NULL);
  tx_event_flags_delete(bt_cbl.event_flags_ptr);
  App_free(bt_cbl.event_flags_ptr);
  bt_cbl.event_flags_ptr = NULL;
  Task_VT100_delete(bt_cbl.vt100_task_instance_index);
  bt_cbl.taken = 0;

  APPLOG("Bluetooth VT100 task %d deleted", bt_cbl.vt100_task_instance_index);
}


/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте прерывания usbhs_usb_int_resume_isr
  Здесь и во всех вызываемых внутри функциях нельзя вызывать объекты синхронизации с задержками

-----------------------------------------------------------------------------------------------------*/
VOID Monitor_Bluetooth_Init_callback(VOID *arg)
{
  T_vt100_man_msg msg_ptr;
  // Передаем команду менеджеру вызвать функцию Init_monitor_bluetooth_driver с аргументом
  // Не вызываем указанную функцию прямо здесь, поскольку данный код может выполняться из обработчика прерываний
  msg_ptr.arg1 = (uint32_t)Init_monitor_bluetooth_driver;
  msg_ptr.arg2 = (uint32_t)arg;
  msg_ptr.arg3 = 0;
  Send_message_to_VT100_task_manager(&msg_ptr);
}

/*-----------------------------------------------------------------------------------------------------
   При отключении VCOM порта удаляем задачу VT100

-----------------------------------------------------------------------------------------------------*/
VOID Monitor_Bluetooth_DeInit_callback(VOID *arg)
{
  T_vt100_man_msg msg_ptr;
  // Передаем команду менеджеру вызвать функцию Init_monitor_bluetooth_driver с аргументом
  // Не вызываем указанную функцию прямо здесь, поскольку данный код может выполняться из обработчика прерываний
  msg_ptr.arg1 = (uint32_t)DeInit_monitor_bluetooth_driver;
  msg_ptr.arg2 = (uint32_t)arg;
  msg_ptr.arg3 = 0;
  Send_message_to_VT100_task_manager(&msg_ptr);
}

/*-----------------------------------------------------------------------------------------------------


  \param pcbl  - указатель на указатель на структуру T_bluetooth_serial_drv_cbl
  \param pdrv  - указатель на структуру T_serial_io_driver

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_bluetooth_drv_init(void **pcbl, void *pdrv)
{
  if (pcbl != NULL)
  {
    if (*pcbl != NULL)
    {
      T_bluetooth_serial_drv_cbl *drv_cbl_ptr = (T_bluetooth_serial_drv_cbl *)(*pcbl);
      drv_cbl_ptr->head_buf_indx     = 0;
      drv_cbl_ptr->tail_buf_indx     = 0;
      drv_cbl_ptr->all_buffers_full  = 0;
      drv_cbl_ptr->tail_buf_rd_pos            = 0;
    }
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param head_buf_number
  \param tail_buf_number
  \param buf_ptr
  \param len
-----------------------------------------------------------------------------------------------------*/
static void _Read_data_form_bufs_ring(T_bluetooth_serial_drv_cbl  *dcbl_ptr, uint8_t **buf_ptr_ptr, uint32_t *len)
{
  uint32_t      tail_bn   = dcbl_ptr->tail_buf_indx;
  uint32_t      head_bn   = dcbl_ptr->head_buf_indx;
  uint32_t      pos       = dcbl_ptr->tail_buf_rd_pos;
  uint32_t      full      = dcbl_ptr->all_buffers_full;

  uint8_t     *buf        = *buf_ptr_ptr;
  uint32_t     rest_len   = *len;
  uint8_t      en_flow    = 0;
  TX_INTERRUPT_SAVE_AREA;

  do
  {

    *buf = dcbl_ptr->in_bufs_ring[tail_bn].buff[pos]; // Читаем байт данных из хвостового буфера
    buf++;
    pos++;
    if (pos >= dcbl_ptr->in_bufs_ring[tail_bn].actual_len)
    {
      pos = 0;
      tail_bn++;
      if (tail_bn >= BLUETOOTH_VT100_DRV_BUF_QUANTITY) tail_bn = 0;
      full = 0;  // Снимаем флаг заполненности всех буферов если почитали хотя бы один буфер в хвосте

      if (tail_bn == head_bn)
      {
        // Все буфера с данными прочитали, выходим
        break;
      }
    }
    rest_len--;
    if (rest_len == 0)
    {
      // Все требуемое количество данных прочитали. выходим
      break;
    }

  } while (1);

  *len                       = *len - rest_len;
  *buf_ptr_ptr               = buf;

  dcbl_ptr->tail_buf_rd_pos  = pos;
  TX_DISABLE;
  dcbl_ptr->tail_buf_indx    = tail_bn;
  if ((dcbl_ptr->all_buffers_full == 1) && (full == 0))
  {
    dcbl_ptr->all_buffers_full = 0;
    en_flow = 1;
  }
  TX_RESTORE;

  if (en_flow == 1)
  {
    wiced_bt_rfcomm_flow_control(((T_local_spp *)(dcbl_ptr->spp_ptr))->rfc_serv_handle, TRUE);
  }

  return;
}

/*-----------------------------------------------------------------------------------------------------
  Читаем заданное количество данных в заданный бйфер

  \param buf
  \param len
  \param timeout

  \return int - возврат количества реально прочитанных данных
-----------------------------------------------------------------------------------------------------*/
static int Mn_bluetooth_drv_read_buf(uint8_t *buf, uint32_t len, int timeout)
{
  ULONG        actual_flags = 0;
  int32_t      tail_buf_number;
  int32_t      head_buf_number;
  uint8_t      **buf_prt_ptr = &buf;
  uint32_t     rest_len = len;

  UX_THREAD   *thread = tx_thread_identify();
  T_serial_io_driver *mdrv = (T_serial_io_driver *)(thread->driver);
  if (mdrv == NULL)
  {
    Wait_ms(timeout);
    return 0;
  }
  T_bluetooth_serial_drv_cbl  *dcbl_ptr = (T_bluetooth_serial_drv_cbl *)(mdrv->drv_cbl_ptr);
  if (dcbl_ptr == NULL)
  {
    Wait_ms(timeout);
    return 0;
  }

  // Выполняем цикл пока не будут приняты все данные или исчерпано время ожидания
  do
  {
    if (dcbl_ptr->all_buffers_full == 1)
    {
      // Все приемные буфера заняты
      // Читаем накопленные данные
      _Read_data_form_bufs_ring(dcbl_ptr, buf_prt_ptr,&rest_len);
    }
    else
    {
      tail_buf_number = dcbl_ptr->tail_buf_indx;
      head_buf_number = dcbl_ptr->head_buf_indx;
      if (tail_buf_number == head_buf_number)
      {
        if (timeout > 0)
        {
          timeout--;
          // Данных нет, ждем один тик
          if (tx_event_flags_get(dcbl_ptr->event_flags_ptr, MB_BLUETOOTH_READ_DONE, TX_OR_CLEAR,&actual_flags, 1) == TX_SUCCESS)
          {
            // Читаем накопленные данные
            _Read_data_form_bufs_ring(dcbl_ptr, buf_prt_ptr,&rest_len);
          }
        }
        else break;
      }
      else
      {
        // Читаем накопленные данные
        _Read_data_form_bufs_ring(dcbl_ptr, buf_prt_ptr,&rest_len);
      }
    }

  } while (rest_len > 0);

  return (len - rest_len);
}


/*-----------------------------------------------------------------------------------------------------


  \param b
  \param timeout - время ожидания выражается в тиках, если 0 то нет ожидания

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_bluetooth_drv_wait_ch(unsigned char *byte_ptr, int timeout)
{
  if (Mn_bluetooth_drv_read_buf(byte_ptr, 1, timeout) == 1)
  {
    SPP_LOGs("SPP: read char %d\r\n",*byte_ptr);
    return RES_OK;
  }
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param len

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_bluetooth_drv_send_buf(const void *buf, unsigned int len)
{
  ULONG                        actual_flags;
  wiced_bt_rfcomm_result_t     res;
  T_serial_io_driver          *mdrv        = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_bluetooth_serial_drv_cbl  *dcbl_ptr = (T_bluetooth_serial_drv_cbl *)(mdrv->drv_cbl_ptr);

  SPP_LOGs("SPP: write data. Len= %d\r\n", len);

  res = wiced_bt_rfcomm_write_data(((T_local_spp *)(dcbl_ptr->spp_ptr))->rfc_serv_handle, (char *)buf, (uint16_t)len);
  if (res != WICED_BT_RFCOMM_SUCCESS)
  {
    SPP_LOGs("SPP: write data error %d\r\n", (uint32_t)res);
    return RES_ERROR;
  }

  // Ожидать сигнала об окончании передачи
  if (tx_event_flags_get(dcbl_ptr->event_flags_ptr, MB_BLUETOOTH_TX_COMPLETE, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(1000)) != TX_SUCCESS)
  {
    SPP_LOGs("SPP: wait tx cpmplete erorr\r\n");
    return RES_ERROR;
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param fmt_ptr

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_bluetooth_drv_printf(const char  *fmt_ptr, ...)
{
  ULONG                        actual_flags;
  wiced_bt_rfcomm_result_t     res;
  T_serial_io_driver          *mdrv     = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_bluetooth_serial_drv_cbl  *dcbl_ptr = (T_bluetooth_serial_drv_cbl *)(mdrv->drv_cbl_ptr);
  char                        *str      = dcbl_ptr->str;
  va_list                      ap;

  va_start(ap, fmt_ptr);
  uint16_t n = vsnprintf(str, BLUETOOTH_VT100_DRV_STR_SZ, (const char *)fmt_ptr, ap);
  va_end(ap);

  SPP_LOGs("SPP: write data. Len= %d\r\n", n);

  res = wiced_bt_rfcomm_write_data(((T_local_spp *)(dcbl_ptr->spp_ptr))->rfc_serv_handle, (char *)str, n);
  if (res != WICED_BT_RFCOMM_SUCCESS)
  {
    SPP_LOGs("SPP: write data error %d\r\n", (uint32_t)res);
    return RES_ERROR;
  }

  // Ожидать сигнала об окончании передачи
  if (tx_event_flags_get(dcbl_ptr->event_flags_ptr, MB_BLUETOOTH_TX_COMPLETE, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(1000)) != TX_SUCCESS)
  {
    SPP_LOGs("SPP: wait tx cpmplete erorr\r\n");
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param pcbl - указатель на указатель на структуру T_bluetooth_serial_drv_cbl

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_bluetooth_drv_deinit(void **pcbl)
{
  if (pcbl != NULL)
  {
    if (*pcbl != NULL)
    {
      T_bluetooth_serial_drv_cbl *drv_cbl_ptr = (T_bluetooth_serial_drv_cbl *)(*pcbl);
      drv_cbl_ptr->head_buf_indx     = 0;
      drv_cbl_ptr->tail_buf_indx     = 0;
      drv_cbl_ptr->all_buffers_full  = 0;
      drv_cbl_ptr->tail_buf_rd_pos   = 0;
    }
  }
  return RES_OK;
}


