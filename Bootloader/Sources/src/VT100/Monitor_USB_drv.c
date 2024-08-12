// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-07-02
// 17:45:08
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


static int Mn_usb_drv_init(void **pcbl, void *pdrv);
static int Mn_usb_drv_send_buf(const void *buf, unsigned int len);
static int Mn_usb_drv_wait_ch(unsigned char *b, int timeout);
static int Mn_usb_drv_printf(const char  *fmt_ptr, ...);
static int Mn_usb_drv_deinit(void **pcbl);

const T_serial_io_driver mon_usb_vcom_driver =
{
  MN_DRIVER_MARK,
  MN_USBFS_VCOM0_DRIVER,
  Mn_usb_drv_init,
  Mn_usb_drv_send_buf,
  Mn_usb_drv_wait_ch,
  Mn_usb_drv_printf,
  Mn_usb_drv_deinit,
  0,
};

#define   MB_USBFS_READ_REQUEST BIT(0)
#define   MB_USBFS_READ_DONE    BIT(1)

static    uint8_t module_inited;

T_usb_serial_drv_cbl usbd_cbl[USB_VT100_DRV_INSTANCES_MAX_NUM]  __attribute__((section(".usb_cbls")));

/*-----------------------------------------------------------------------------------------------------


  \param ptr
-----------------------------------------------------------------------------------------------------*/
static void _Receiving_task(ULONG ptr)
{
  UINT              res;
  ULONG             actual_length;
  ULONG             actual_flags;
  T_usb_serial_drv_cbl *dcbl_ptr = (T_usb_serial_drv_cbl *)ptr;
  int32_t           head_buf_number;
  int32_t           tail;

  do
  {
    head_buf_number = dcbl_ptr->head_buf_indx;
    res = ux_device_class_cdc_acm_read(dcbl_ptr->cdc, dcbl_ptr->in_bufs_ring[head_buf_number].buff,USB_VT100_DRV_BUFFER_MAX_LENGTH,&actual_length);
    dcbl_ptr->in_bufs_ring[head_buf_number].actual_len = actual_length;
    if (res == UX_SUCCESS)
    {
      tail = dcbl_ptr->tail_buf_indx;
      head_buf_number++;
      if (head_buf_number >= USB_VT100_DRV_BUF_QUANTITY) head_buf_number = 0;
      dcbl_ptr->head_buf_indx = head_buf_number;

      // Выставляем флаг выполненного чтения
      tx_event_flags_set(dcbl_ptr->event_flags_ptr, MB_USBFS_READ_DONE, TX_OR);

      // Если все буферы на прием заполнены, то значит системе не требуются данные
      if (tail == head_buf_number)
      {
        // Перестаем принимать данные из USB и ждем когда система обработает уже принятые данные и подаст сигнал к началу приема по USB
        dcbl_ptr->all_buffers_full = 1;
        if (tx_event_flags_get(dcbl_ptr->event_flags_ptr, MB_USBFS_READ_REQUEST, TX_OR_CLEAR,&actual_flags, TX_WAIT_FOREVER) != TX_SUCCESS)
        {
          tx_thread_sleep(2); // Задержка после ошибки
        }
      }
    }
    else
    {
      tx_thread_sleep(2); // Задержка после ошибки в функции ux_device_class_cdc_acm_read
    }
  } while (1);

}

/*-----------------------------------------------------------------------------------------------------
  Создаем объект событий и задачу приема

  \param p

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Create_receiving_task(T_usb_serial_drv_cbl *dcbl_ptr, char *task_name, char *evg_name)
{
  uint32_t  res;
  uint32_t  prio;

  if (tx_event_flags_create(dcbl_ptr->event_flags_ptr,evg_name) != TX_SUCCESS)
  {
    return RES_ERROR;
  }

  // Получаем приоритет текущей задачи
  tx_thread_info_get(tx_thread_identify(), TX_NULL, TX_NULL, TX_NULL, &prio, TX_NULL, TX_NULL, TX_NULL, TX_NULL);

  res = tx_thread_create(
                       dcbl_ptr->recv_thread_ptr,
                       (CHAR *)task_name,
                       _Receiving_task,
                       (ULONG)dcbl_ptr,  // Передаем в задачу указатель на служебную стуктуру
                       dcbl_ptr->recv_thread_stack,
                       USB_VT100_DRV_RECV_THREAD_STACK_SZ,
                       prio,
                       prio,
                       1,
                       TX_AUTO_START);
  if (res == TX_SUCCESS)
  {
    APPLOG("USB receiver task created.");
    return RES_OK;
  }
  else
  {
    tx_event_flags_delete(dcbl_ptr->event_flags_ptr);
    APPLOG("USB receiver task creating error %04X.", res);
    return RES_ERROR;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param p
-----------------------------------------------------------------------------------------------------*/
static void _Delete_receiving_task(T_usb_serial_drv_cbl *dcbl_ptr)
{
  if (tx_thread_terminate(dcbl_ptr->recv_thread_ptr) == TX_SUCCESS)
  {
    tx_thread_delete(dcbl_ptr->recv_thread_ptr);
  }
  tx_event_flags_delete(dcbl_ptr->event_flags_ptr);
}

/*-----------------------------------------------------------------------------------------------------
  При подключении USB VCOM порта создаем задачу VT100
  Функция вызывается из задачи VT100_task_manager_thread по сообщению из функции ux_cdc_device_instance_activate

  \param msg_ptr
-----------------------------------------------------------------------------------------------------*/
static void Init_monitor_usb_driver(T_vt100_man_msg *msg_ptr)
{
  VOID *cdc_instance = (VOID *)msg_ptr->arg2;

  uint32_t res           = RES_ERROR;
  int32_t  instance_indx = -1;

  if (module_inited == 0)
  {
    memset(usbd_cbl, 0, sizeof(usbd_cbl));
    module_inited = 1;
  }

  // Ищем свободную запись
  for (uint32_t i = 0; i < USB_VT100_DRV_INSTANCES_MAX_NUM; i++)
  {
    if (usbd_cbl[i].taken == 0)
    {
      instance_indx = i;

      memcpy(&usbd_cbl[instance_indx].serial_io_driver,&mon_usb_vcom_driver, sizeof(mon_usb_vcom_driver));
      usbd_cbl[instance_indx].serial_io_driver.drv_cbl_ptr = &usbd_cbl[instance_indx];

      res = Task_VT100_create(&usbd_cbl[instance_indx].serial_io_driver,&usbd_cbl[instance_indx].vt100_task_instance_index); // В контексте этого вызова будут выполнены функции драйвера init и deinit
      if (res == RES_OK)
      {
        usbd_cbl[instance_indx].cdc =  (UX_SLAVE_CLASS_CDC_ACM *) cdc_instance;
        // Выделяем память для задачи приема из USB
        usbd_cbl[instance_indx].recv_thread_ptr   = App_malloc(sizeof(TX_THREAD));
        usbd_cbl[instance_indx].recv_thread_stack = App_malloc(USB_VT100_DRV_RECV_THREAD_STACK_SZ);
        usbd_cbl[instance_indx].event_flags_ptr           = App_malloc(sizeof(TX_EVENT_FLAGS_GROUP));
        if ((usbd_cbl[instance_indx].recv_thread_ptr == NULL) || (usbd_cbl[instance_indx].recv_thread_stack == NULL) || (usbd_cbl[instance_indx].event_flags_ptr == NULL))
        {
          goto err_exit;
        }
        snprintf(usbd_cbl[instance_indx].recv_task_name, VT100_TASK_NAME_MAX_SZ, "VT100_VCOM_RX_%d", instance_indx);
        snprintf(usbd_cbl[instance_indx].event_flags_name, VT100_TASK_NAME_MAX_SZ, "VT100_VCOM_RX_EV_%d", instance_indx);
        res = _Create_receiving_task(&usbd_cbl[instance_indx], usbd_cbl[instance_indx].recv_task_name, usbd_cbl[instance_indx].event_flags_name);
        if (res != RES_OK)
        {
          goto err_exit;
        }
        Task_VT100_start(usbd_cbl[instance_indx].vt100_task_instance_index);
        usbd_cbl[instance_indx].taken = 1;
        APPLOG("USB VCOM VT100 task %d created", usbd_cbl[instance_indx].vt100_task_instance_index);
      }
      else
      {
        goto err_exit;
      }
      break;
    }
  }
  if (instance_indx == -1)
  {
    APPLOG("USB VCOM VT100 task creating error");
  }
  return;

err_exit:
  App_free(usbd_cbl[instance_indx].recv_thread_ptr);
  App_free(usbd_cbl[instance_indx].recv_thread_stack);
  App_free(usbd_cbl[instance_indx].event_flags_ptr);
  Task_VT100_delete(usbd_cbl[instance_indx].vt100_task_instance_index);
  usbd_cbl[instance_indx].taken = 0;
  usbd_cbl[instance_indx].cdc   = NULL;
  APPLOG("USB VCOM VT100 task creating error");
  return;
}

/*-----------------------------------------------------------------------------------------------------
  При отключении USB VCOM порта удаляем задачу VT100 и задачу драйвера
  Функция вызывается из задачи VT100_task_manager_thread по сообщению из функции ux_cdc_device_instance_deactivate


  \param msg_ptr
-----------------------------------------------------------------------------------------------------*/
static void DeInit_monitor_usb_driver(T_vt100_man_msg *msg_ptr)
{
  VOID *cdc_instance = (VOID *)msg_ptr->arg2;

  int32_t  instance_indx = -1;

  // Ищем запись с совпадающим указателем на cdc_instance
  for (uint32_t i = 0; i < USB_VT100_DRV_INSTANCES_MAX_NUM; i++)
  {
    if (usbd_cbl[i].cdc == (UX_SLAVE_CLASS_CDC_ACM *) cdc_instance)
    {
      instance_indx = i;
      Task_VT100_suspend(usbd_cbl[instance_indx].vt100_task_instance_index);
      _Delete_receiving_task(&usbd_cbl[instance_indx]);
      App_free(usbd_cbl[instance_indx].recv_thread_ptr);
      App_free(usbd_cbl[instance_indx].recv_thread_stack);
      App_free(usbd_cbl[instance_indx].event_flags_ptr);
      usbd_cbl[instance_indx].event_flags_ptr = NULL;
      Task_VT100_delete(usbd_cbl[instance_indx].vt100_task_instance_index);
      usbd_cbl[instance_indx].taken = 0;
      usbd_cbl[instance_indx].cdc = NULL;
      APPLOG("USB VCOM VT100 task %d deleted", usbd_cbl[instance_indx].vt100_task_instance_index);
    }
  }
  if (instance_indx == -1)
  {
    APPLOG("USB VCOM VT100 task deleting error");
  }
}


/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте прерывания usbhs_usb_int_resume_isr
  Здесь и во всех вызываемых внутри функциях нельзя вызывать объекты синхронизации с задержками

  \param cdc_instance - ссылка на экземпляр типа UX_SLAVE_CLASS_CDC_ACM
-----------------------------------------------------------------------------------------------------*/
VOID Monitor_USB_Init_callback(VOID *cdc_instance)
{
  T_vt100_man_msg msg_ptr;

  // Передаем команду менеджеру вызвать функцию Init_monitor_usb_driver с аргументом cdc_instance
  // Не вызываем указанную функцию прямо здесь, поскольку данный код может выполняться из обработчика прерываний
  msg_ptr.arg1 = (uint32_t)Init_monitor_usb_driver;
  msg_ptr.arg2 = (uint32_t)cdc_instance;
  msg_ptr.arg3 = 0;
  Send_message_to_VT100_task_manager(&msg_ptr);
}

/*-----------------------------------------------------------------------------------------------------
   При отключении VCOM порта удаляем задачу VT100


  \param cdc_instance - ссылка на экземпляр типа UX_SLAVE_CLASS_CDC_ACM
-----------------------------------------------------------------------------------------------------*/
VOID Monitor_USB_DeInit_callback(VOID *cdc_instance)
{
  T_vt100_man_msg msg_ptr;

  // Передаем команду менеджеру вызвать функцию DeInit_monitor_usb_driver с аргументом cdc_instance
  // Не вызываем указанную функцию прямо здесь, поскольку данный код может выполняться из обработчика прерываний
  msg_ptr.arg1 = (uint32_t)DeInit_monitor_usb_driver;
  msg_ptr.arg2 = (uint32_t)cdc_instance;
  msg_ptr.arg3 = 0;
  Send_message_to_VT100_task_manager(&msg_ptr);
}

/*-----------------------------------------------------------------------------------------------------


  \param pcbl  - указатель на указатель на структуру T_usb_serial_drv_cbl
  \param pdrv  - указатель на структуру T_serial_io_driver

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_usb_drv_init(void **pcbl, void *pdrv)
{
  if (pcbl != NULL)
  {
    if (*pcbl != NULL)
    {
      T_usb_serial_drv_cbl *dcbl_ptr = (T_usb_serial_drv_cbl *)(*pcbl);
      dcbl_ptr->head_buf_indx     = 0;
      dcbl_ptr->tail_buf_indx     = 0;
      dcbl_ptr->all_buffers_full  = 0;
      dcbl_ptr->tail_buf_rd_pos   = 0;
    }
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param len

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_usb_drv_send_buf(const void *buf, unsigned int len)
{

  UINT                   res;
  ULONG                  actual_length;
  T_serial_io_driver    *mdrv           = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_usb_serial_drv_cbl  *dcbl_ptr    = (T_usb_serial_drv_cbl *)(mdrv->drv_cbl_ptr);

  res = ux_device_class_cdc_acm_write(dcbl_ptr->cdc, (UCHAR *)buf, len ,&actual_length);
  if ((res != UX_SUCCESS) || (actual_length != len))
  {
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param b
  \param timeout - время ожидания выражается в тиках, если 0 то нет ожидания

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_usb_drv_wait_ch(unsigned char *b, int timeout)
{
  ULONG        actual_flags = 0;
  int32_t      tail_buf_number;
  int32_t      head_buf_number;

  UX_THREAD   *thread = tx_thread_identify();

  T_serial_io_driver *mdrv = (T_serial_io_driver *)(thread->driver);
  if (mdrv == NULL)
  {
    Wait_ms(timeout);
    return RES_ERROR;
  }

  T_usb_serial_drv_cbl  *dcbl_ptr = (T_usb_serial_drv_cbl *)(mdrv->drv_cbl_ptr);
  if (dcbl_ptr == NULL)
  {
    Wait_ms(timeout);
    return RES_ERROR;
  }

  // Если индексы буферов равны то это значит отсутствие принятых пакетов
  head_buf_number = dcbl_ptr->head_buf_indx;
  if (dcbl_ptr->tail_buf_indx == head_buf_number)
  {
    if (tx_event_flags_get(dcbl_ptr->event_flags_ptr, MB_USBFS_READ_DONE, TX_OR_CLEAR,&actual_flags, ms_to_ticks(timeout)) != TX_SUCCESS)
    {
      return RES_ERROR;
    }
    // Еще раз проверяем наличие данных поскольку флаг мог остаться от предыдущего чтения, когда данные били приняты без проверки флага и соответственно без его сброса
    head_buf_number = dcbl_ptr->head_buf_indx;
    if (dcbl_ptr->tail_buf_indx == head_buf_number)
    {
      return RES_ERROR;
    }
  }

  tail_buf_number = dcbl_ptr->tail_buf_indx;                      // Получаем индекс хвостового буфера
  *b = dcbl_ptr->in_bufs_ring[tail_buf_number].buff[dcbl_ptr->tail_buf_rd_pos]; // Читаем байт данных из хвостового буфера
  dcbl_ptr->tail_buf_rd_pos++;                        // Смещаем указатель на следующий байт данных

  // Если позиция достигла конца данных в текущем буфере, то буфер освобождается для приема
  if (dcbl_ptr->tail_buf_rd_pos >= dcbl_ptr->in_bufs_ring[tail_buf_number].actual_len)
  {
    dcbl_ptr->tail_buf_rd_pos = 0;
    // Смещаем указатель хвоста очереди приемных буфферов
    // Появляется место для движения головы очереди приемных буфферов

    tail_buf_number++;
    if (tail_buf_number >= USB_VT100_DRV_BUF_QUANTITY) tail_buf_number = 0;
    dcbl_ptr->tail_buf_indx = tail_buf_number;

    // Если очередь пакетов была заполнена, то сообщить задаче о продолжении приема
    if (dcbl_ptr->all_buffers_full == 1)
    {
      dcbl_ptr->all_buffers_full = 0;
      if (tx_event_flags_set(dcbl_ptr->event_flags_ptr, MB_USBFS_READ_REQUEST, TX_OR) != TX_SUCCESS)
      {
        Wait_ms(timeout); // Задержка после ошибки нужна для того чтобы задача не захватила все ресурсы в случает постоянного появления ошибки
        return RES_ERROR;
      }
    }
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param fmt_ptr

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_usb_drv_printf(const char  *fmt_ptr, ...)
{
  UINT                   res;
  ULONG                  actual_length;
  uint32_t               n;
  T_serial_io_driver    *mdrv           = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_usb_serial_drv_cbl  *dcbl_ptr    = (T_usb_serial_drv_cbl *)(mdrv->drv_cbl_ptr);
  char                  *str            = dcbl_ptr->str;
  va_list                ap;

  va_start(ap, fmt_ptr);
  n = vsnprintf(str, USB_VT100_DRV_STR_SZ, (const char *)fmt_ptr, ap);
  va_end(ap);
  res = ux_device_class_cdc_acm_write(dcbl_ptr->cdc, (UCHAR *)str, n ,&actual_length);
  if ((res != UX_SUCCESS) || (actual_length != n))
  {
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param pcbl - указатель на указатель на структуру T_usb_serial_drv_cbl

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_usb_drv_deinit(void **pcbl)
{
  if (pcbl != NULL)
  {
    if (*pcbl != NULL)
    {
      T_usb_serial_drv_cbl *dcbl_ptr = (T_usb_serial_drv_cbl *)(*pcbl);
      dcbl_ptr->head_buf_indx     = 0;
      dcbl_ptr->tail_buf_indx     = 0;
      dcbl_ptr->all_buffers_full  = 0;
      dcbl_ptr->tail_buf_rd_pos   = 0;
    }
  }
  return RES_OK;
}
