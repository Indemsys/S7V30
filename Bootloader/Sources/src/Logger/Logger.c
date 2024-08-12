// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.05.13
// 16:37:49
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include <stdarg.h>

#define  NUM_OF_LOGS   2

TX_EVENT_FLAGS_GROUP           file_logger_flags;
T_log_cbl                      app_log_cbl;
T_log_cbl                      net_log_cbl;

T_log_cbl  *log_cbls[NUM_OF_LOGS]              = {&app_log_cbl,&net_log_cbl};
uint32_t    log_file_reset_events[NUM_OF_LOGS] = {EVT_RESET_APP_FILE_LOG, EVT_RESET_NET_FILE_LOG};
const char *log_file_names[NUM_OF_LOGS]        = {APP_LOG_FILE_PATH, NET_LOG_FILE_PATH};
const char *log_file_prev_names[NUM_OF_LOGS]   = {APP_LOG_PREV_FILE_PATH, NET_LOG_PREV_FILE_PATH};

#ifdef LOG_TO_ONBOARD_SDRAM
T_logger_record     app_log[APP_LOG_SIZE] @ ".sdram";
T_logger_record     net_log[NET_LOG_SIZE] @ ".sdram";
#else
T_logger_record     app_log[APP_LOG_SIZE];
T_logger_record     net_log[NET_LOG_SIZE];
#endif

#define LOGGER_TASK_STACK_SIZE 2048
static uint8_t logger_stacks[LOGGER_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.Logger_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static void LogFile_Task(ULONG arg);


#ifdef ENABLE_SDRAM
  #ifdef LOG_TO_ONBOARD_SDRAM
    #ifdef SSP_LOGGER
__no_init T_ssp_log_record            ssp_log[SSP_LOG_SIZE]   @ ".sdram";
T_log_cbl                             ssp_log_cbl;
extern uint32_t                       g_ssp_log_inited;
    #endif
  #endif
#endif

#define TIME_DELAY_BEFORE_SAVE        100  // Время в мс перед тем как будут сохранены оставшиеся записи
#define LOG_RECS_BEFORE_SAVE_TO_FILE  100  // Количество записей вызывающее немедленное сохранение

char                                  file_log_str[LOG_STR_MAX_SZ];
static TX_THREAD                      log_thread;

static char                           rtt_log_str[RTT_LOG_STR_SZ];

static void Log_write(T_log_cbl *log_cbl_ptr, char *str, const char *func_name, unsigned int line_num, unsigned int severity);

/*-----------------------------------------------------------------------------------------------------


  \param log_cbl_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t  Log_init(T_log_cbl *log_cbl_ptr, uint32_t log_size, T_logger_record *records_arr,  const char *name)
{
  if (tx_mutex_create(&log_cbl_ptr->log_mutex, (char *)name, TX_INHERIT) != TX_SUCCESS) return RES_ERROR;
  Get_hw_timestump(&log_cbl_ptr->log_start_time);
  log_cbl_ptr->name           = name;
  log_cbl_ptr->log_records    = records_arr;
  log_cbl_ptr->log_size       = log_size;
  log_cbl_ptr->event_log_head = 0;
  log_cbl_ptr->event_log_tail = 0;
  log_cbl_ptr->logger_ready   = 1;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Init_app_logger(void)
{
  Log_init(&app_log_cbl, APP_LOG_SIZE, app_log, "Application log");
  Log_init(&net_log_cbl, NET_LOG_SIZE, net_log, "Net log");
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void App_log_disable(void)
{
  app_log_cbl.logger_ready = 0;
  net_log_cbl.logger_ready = 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param name
  \param line_num
  \param severity
  \param fmt_ptr
-----------------------------------------------------------------------------------------------------*/
void RTT_LOGs(const char *fmt_ptr, ...)
{
  unsigned int     n;
  va_list          ap;
  va_start(ap, fmt_ptr);
  __disable_interrupt();
  n = vsnprintf(rtt_log_str, RTT_LOG_STR_SZ, (const char *)fmt_ptr, ap);
  SEGGER_RTT_Write(RTT_LOG_CH, rtt_log_str, n);
  __enable_interrupt();
  va_end(ap);
}

/*-----------------------------------------------------------------------------------------------------


  \param name
  \param line_num
  \param severity
  \param fmt_ptr
-----------------------------------------------------------------------------------------------------*/
void LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...)
{
  char             log_str[LOG_STR_MAX_SZ + 1];
  va_list          ap;
  va_start(ap, fmt_ptr);
  vsnprintf(log_str, LOG_STR_MAX_SZ, (const char *)fmt_ptr, ap);
  Log_write(&app_log_cbl, log_str , name, line_num, severity);
  va_end(ap);
}

/*-----------------------------------------------------------------------------------------------------


  \param name
  \param line_num
  \param severity
  \param fmt_ptr
-----------------------------------------------------------------------------------------------------*/
void Net_LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...)
{
  char             log_str[LOG_STR_MAX_SZ + 1];
  va_list          ap;
  if (ivar.en_net_log == 0) return;
  va_start(ap, fmt_ptr);
  vsnprintf(log_str, LOG_STR_MAX_SZ, (const char *)fmt_ptr, ap);
  Log_write(&net_log_cbl, log_str , name, line_num, severity);
  va_end(ap);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Req_to_reset_log_file(void)
{
  Set_file_logger_event(EVT_RESET_APP_FILE_LOG);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Req_to_reset_netlog_file(void)
{
  Set_file_logger_event(EVT_RESET_NET_FILE_LOG);
}


/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Init_ssp_logger(void)
{
  #ifdef SSP_LOGGER
    #ifdef LOG_TO_ONBOARD_SDRAM
  memset(ssp_log, 0, sizeof(ssp_log));
  ssp_log_cbl.event_log_head = 0;
  ssp_log_cbl.event_log_tail = 0;
  Get_hw_timestump(&ssp_log_cbl.log_start_time);
  ssp_log_cbl.logger_ready = 1;
  g_ssp_log_inited = 1;
    #endif
  #endif
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------

  Вызов данной процедуры может производится из процедур обслуживания прерываний,
  поэтому мьютексы и другие сервисы сихронизации здесь использовать нельзя

  \param err
  \param module
  \param line
-----------------------------------------------------------------------------------------------------*/
void ssp_error_log(ssp_err_t err, const char *module_name, int32_t line_num)
{
  #ifdef SSP_LOGGER
    #ifdef LOG_TO_ONBOARD_SDRAM
  int         head;
  int         tail;

  if (g_ssp_log_inited == 0) return;
  __disable_interrupt();
  head = ssp_log_cbl.event_log_head;
  Get_hw_timestump(&ssp_log[head].timestump);
  strncpy(ssp_log[head].module_name, module_name, SSP_LOG_MODULE_NAME_SZ);
  ssp_log[head].line_num = line_num;
  ssp_log[head].err = (uint32_t)err;

  // Сдвигаем указатель головы лога
  head++;
  if (head >= SSP_LOG_SIZE) head = 0;
  ssp_log_cbl.event_log_head = head;

  tail = ssp_log_cbl.event_log_tail;
  // Если голова достигла хвоста, то сдвигает указатель хвоста и теряем непрочитанные данные
  if (head == tail)
  {
    tail++;
    if (tail >= SSP_LOG_SIZE) tail = 0;
    ssp_log_cbl.event_log_tail = tail;
    ssp_log_cbl.log_overfl_f = 1;
    ssp_log_cbl.log_overfl_err++;
  }
  __enable_interrupt();

    #endif
  #endif
}


/*------------------------------------------------------------------------------
  Запись сообщения в таблицу лога и в другие места назначения


 \param str         : сообщение
 \param func_name   : имя функции
 \param line_num    : номер строки
 \param severity    : важность сообщения
 ------------------------------------------------------------------------------*/
static void Log_write(T_log_cbl *log_cbl_ptr, char *str, const char *func_name, unsigned int line_num, unsigned int severity)
{
  int         head;
  int         tail;
  T_sys_timestump   ntime;
  rtc_time_t        date_time;

  if (log_cbl_ptr->logger_ready == 1)
  {
    Get_hw_timestump(&ntime);
    RTC_get_system_DateTime(&date_time);
    date_time.tm_mon++;
    date_time.tm_year += 1900;

    // Вызов данной процедуры может производится из процедур обслуживания прерываний,
    // поэтому мьютексы и другие сервисы сихронизации здесь использовать нельзя
    if (tx_mutex_get(&log_cbl_ptr->log_mutex, MS_TO_TICKS(LOGGER_WR_TIMEOUT_MS)) == TX_SUCCESS)
    {

      head = log_cbl_ptr->event_log_head;
      // Определяем время в микросекундах от старта лога
      log_cbl_ptr->log_records[head].date_time = date_time;
      log_cbl_ptr->log_records[head].delta_time = Hw_timestump_diff64_us(&log_cbl_ptr->log_start_time,&ntime);
      strncpy(log_cbl_ptr->log_records[head].msg, str, LOG_STR_MAX_SZ - 1);
      strncpy(log_cbl_ptr->log_records[head].func_name, func_name, EVNT_LOG_FNAME_SZ - 1);
      log_cbl_ptr->log_records[head].line_num = line_num;
      log_cbl_ptr->log_records[head].severity = severity;
      // Сдвигаем указатель головы лога
      head++;
      if (head >= log_cbl_ptr->log_size) head = 0;
      log_cbl_ptr->event_log_head = head;

      tail = log_cbl_ptr->event_log_tail;
      // Если голова достигла хвоста, то сдвигает указатель хвоста и теряем непрочитанные данные
      if (head == tail)
      {
        tail++;
        if (tail >= log_cbl_ptr->log_size) tail = 0;
        log_cbl_ptr->event_log_tail = tail;
        log_cbl_ptr->log_overfl_f = 1;
        log_cbl_ptr->log_overfl_err++;
      }
      // Если голова достигла хвоста записи в файл, то сдвигает указатель хвоста записи в файл и теряем непрочитанные данные
      tail = log_cbl_ptr->file_log_tail;
      if (head == tail)
      {
        tail++;
        if (tail >= log_cbl_ptr->log_size) tail = 0;
        log_cbl_ptr->file_log_tail = tail;
        log_cbl_ptr->file_log_overfl_f = 1;
        log_cbl_ptr->file_log_overfl_err++;
      }
      tx_mutex_put(&log_cbl_ptr->log_mutex);
    }
  }
}




/*-----------------------------------------------------------------------------------------------------


  \param str
  \param max_str_len
  \param p_log_rec
-----------------------------------------------------------------------------------------------------*/
static uint32_t Log_get_formated_string(T_log_cbl *log_cbl_ptr, char *str, uint32_t max_str_len)
{
  uint32_t           tail;
  uint64_t           t64;
  uint32_t           t32;
  uint32_t           time_msec;
  uint32_t           time_sec;
  uint32_t           time_min;
  uint32_t           time_hour;
  uint32_t           time_day;
  T_logger_record  *log_rec_ptr;

  if (tx_mutex_get(&log_cbl_ptr->log_mutex, 10) == TX_SUCCESS)
  {
    tail = log_cbl_ptr->event_log_tail;
    if (log_cbl_ptr->event_log_head != tail)
    {
      tail++;
      if (tail >= log_cbl_ptr->log_size)
      {
        tail = 0;
      }
      log_cbl_ptr->event_log_tail = tail;

      log_rec_ptr = &log_cbl_ptr->log_records[tail];

      t64 = log_rec_ptr->delta_time;
      time_msec      = t64 % 1000000ull;
      t32             = (uint32_t)(t64 / 1000000ull);
      time_sec       = t32 % 60;
      time_min       = (t32 / 60) % 60;
      time_hour      = (t32 / (60 * 60)) % 24;
      time_day       = t32 / (60 * 60 * 24);

      snprintf(str, max_str_len, "%03d d %02d h %02d m %02d s %06d us |",time_day, time_hour, time_min, time_sec, time_msec);
      uint32_t  len = strlen(str);

      if (log_rec_ptr->line_num != 0)
      {
        snprintf(&str[len], max_str_len - len, " %s (%s %d)\n\r",log_rec_ptr->msg,log_rec_ptr->func_name,log_rec_ptr->line_num);
      }
      else
      {
        snprintf(&str[len], max_str_len - len, " %s\n\r",log_rec_ptr->msg);
      }
      tx_mutex_put(&log_cbl_ptr->log_mutex);
      return RES_OK;
    }
    else
    {
      tx_mutex_put(&log_cbl_ptr->log_mutex);
      return RES_OK;
    }
  }
  return RES_OK;

}
/*-----------------------------------------------------------------------------------------------------


  \param n
-----------------------------------------------------------------------------------------------------*/
static void Log_show_record(T_log_cbl *log_cbl_ptr, uint32_t n)
{
  GET_MCBL;

  uint64_t t64 = log_cbl_ptr->log_records[n].delta_time;
  uint32_t time_usec = t64 % 1000000ull;
  uint32_t time_sec  = (uint32_t)(t64 / 1000000ull);

  if (log_cbl_ptr->log_records[n].line_num != 0)
  {
    MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s (%s %d)\n\r",
            time_sec, time_usec,
            log_cbl_ptr->log_records[n].msg,
            log_cbl_ptr->log_records[n].func_name,
            log_cbl_ptr->log_records[n].line_num);
  }
  else
  {
    MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s\n\r",
            time_sec, time_usec,
            log_cbl_ptr->log_records[n].msg);
  }

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void Log_show_all_records(T_log_cbl *log_cbl_ptr)
{
  uint32_t   i;
  uint32_t   indx;
  uint32_t   head;
  uint32_t   tail;
  uint32_t   reqn;
  uint8_t    b;

  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  // Вывод всего лога

  head = log_cbl_ptr->event_log_head;
  tail = log_cbl_ptr->event_log_tail;
  //
  if (head >= tail)
  {
    reqn = head - tail;
  }
  else
  {
    reqn = log_cbl_ptr->log_size - (tail - head);
  }

  indx = tail;
  for (i = 0; i < reqn; i++)
  {
    Log_show_record(log_cbl_ptr, indx);
    indx++;
    if (indx >= log_cbl_ptr->log_size) indx = 0;
  }
  tail = head;

  do
  {
    if (WAIT_CHAR(&b, 10) == RES_OK)
    {
      switch (b)
      {
      case VT100_ESC:
        return;
      }
    }

    head = log_cbl_ptr->event_log_head;
    if (head >= tail)
    {
      reqn = head - tail;
    }
    else
    {
      reqn = log_cbl_ptr->log_size - (tail - head);
    }

    indx = tail;
    for (i = 0; i < reqn; i++)
    {
      Log_show_record(log_cbl_ptr, indx);
      indx++;
      if (indx >= log_cbl_ptr->log_size) indx = 0;
    }
    tail = head;

  }while (1);
}

/*-----------------------------------------------------------------------------------------------------
  Вывод последних EVENT_LOG_DISPLAY_ROW строк лога


  \param void
-----------------------------------------------------------------------------------------------------*/
void Log_show_last_records(T_log_cbl *log_cbl_ptr)
{
  uint32_t   i;
  uint32_t   indx;
  uint32_t   head;
  uint32_t   tail;
  uint32_t   reqn;
  uint32_t   outn;
  uint8_t    b;

  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);

  tail = log_cbl_ptr->event_log_tail;

  do
  {
    // Определяем количестово строк в логе

    head = log_cbl_ptr->event_log_head;

    if (tail != head)
    {
      if (head >= tail)
      {
        reqn = head - tail;
      }
      else
      {
        reqn = log_cbl_ptr->log_size - (tail - head);
      }

      // Определяем количество выводимых на экран строк лога
      if (reqn < EVENT_LOG_DISPLAY_ROW)
      {
        outn = reqn;
      }
      else
      {
        outn = EVENT_LOG_DISPLAY_ROW;
      }

      // Вычисляем индекс строки в логе с которой начинается вывод
      if (head >= tail)
      {
        indx = head - outn;
      }
      else
      {
        if (outn > head)
        {
          indx = log_cbl_ptr->log_size - (outn - head);
        }
        else
        {
          indx = head - outn;
        }
      }


      for (i = 0; i < outn; i++)
      {
        Log_show_record(log_cbl_ptr, indx);
        indx++;
        if (indx >= log_cbl_ptr->log_size) indx = 0;
      }

      tail = head;
    }


    if (WAIT_CHAR(&b, 10) == RES_OK)
    {
      switch (b)
      {
      case VT100_ESC:
        return;
      }
    }


  } while (1);

}

/*-----------------------------------------------------------------------------------------------------


  \param log_cbl_ptr
-----------------------------------------------------------------------------------------------------*/
static void Log_clear(T_log_cbl *log_cbl_ptr)
{
  log_cbl_ptr->event_log_head      = 0;
  log_cbl_ptr->event_log_tail      = 0;
  log_cbl_ptr->log_miss_err        = 0;
  log_cbl_ptr->log_overfl_err      = 0;
  log_cbl_ptr->file_log_overfl_err = 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Log_monitor_show_header(T_log_cbl *log_cbl_ptr)
{
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF("%s monitor\n\r", log_cbl_ptr->name);
  MPRINTF("<D>- print all records, <L>- print last records, <C>- clear, <E> - Erase log file, <N> - Toggle log, <R>- exit\n\r");
  MPRINTF("............................................\n\r");
  MPRINTF("Log overflows=%d, File log overflows=%d, Log miss count=%d\r\n",log_cbl_ptr->log_overfl_err, log_cbl_ptr->file_log_overfl_err,log_cbl_ptr->log_miss_err);
  MPRINTF("********************************************\n\r");
}
/*------------------------------------------------------------------------------



 \param pvt100_cb
 ------------------------------------------------------------------------------*/
void App_Log_monitor(void)
{
  static uint32_t  log_indx = 0;
  uint8_t    b;

  GET_MCBL;

  Log_monitor_show_header(log_cbls[log_indx]);

  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case 'D':
      case 'd':
        Log_show_all_records(log_cbls[log_indx]);
        Log_monitor_show_header(log_cbls[log_indx]);
        break;
      case 'L':
      case 'l':
        Log_show_last_records(log_cbls[log_indx]);
        Log_monitor_show_header(log_cbls[log_indx]);
        break;
      case 'R':
      case 'r':
      case VT100_ESC:
        return;
      case 'C':
      case 'c':
        Log_clear(&app_log_cbl);
        break;
      case 'E':
      case 'e':
        Set_file_logger_event(log_file_reset_events[log_indx]);
        Log_clear(log_cbls[log_indx]);
        break;
      case 'N':
      case 'n':
        log_indx++;
        if (log_indx >= NUM_OF_LOGS) log_indx = 0;
        Log_monitor_show_header(log_cbls[log_indx]);
        break;
      }
    }
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------


  \param str
  \param max_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t App_Log_get_formated_string(char *str, uint32_t max_str_len)
{
  return Log_get_formated_string(&app_log_cbl, str, max_str_len);
}

/*-----------------------------------------------------------------------------------------------------


  \param eventmask
-----------------------------------------------------------------------------------------------------*/
void Set_file_logger_event(uint32_t events_mask)
{
  tx_event_flags_set(&file_logger_flags, events_mask, TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param indx
-----------------------------------------------------------------------------------------------------*/
static void LogFile_Open(uint32_t indx)
{
  uint32_t    res;
  T_log_cbl  *log_cbl_ptr = log_cbls[indx];

  log_cbl_ptr->t_prev = tx_time_get();
  // Открыть файл для записи лога
  res = fx_file_create(&fat_fs_media,(char *)log_file_names[indx]);
  if ((res == FX_SUCCESS) ||  (res == FX_ALREADY_CREATED))
  {
    res = fx_file_open(&fat_fs_media,&log_cbl_ptr->log_file, (char *)log_file_names[indx],  FX_OPEN_FOR_WRITE);
    if (res == FX_SUCCESS)  log_cbl_ptr->log_file_opened = 1;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param indx
-----------------------------------------------------------------------------------------------------*/
static void LogFile_Reset(uint32_t indx)
{
  uint8_t     flag = 0;
  T_log_cbl  *log_cbl_ptr = log_cbls[indx];

  if (log_cbl_ptr->log_file_opened == 0) return;

  if (fx_file_close(&log_cbl_ptr->log_file) == FX_SUCCESS)
  {
    if (fx_file_delete(&fat_fs_media, (char *)log_file_names[indx]) == FX_SUCCESS)
    {
      if (fx_file_create(&fat_fs_media,(char *)log_file_names[indx]) == FX_SUCCESS)
      {
        if (fx_file_open(&fat_fs_media,&log_cbl_ptr->log_file, (char *)log_file_names[indx],  FX_OPEN_FOR_WRITE) == FX_SUCCESS)
        {
          flag = 1;
          APPLOG("Log file %s successfully reset.", log_file_names[indx]);
        }
      }
    }
  }
  if (flag == 0)
  {
    log_cbl_ptr->log_file_opened = 0;
    APPLOG("Error resetting log file %s.", log_file_names[indx]);
    return;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param indx
-----------------------------------------------------------------------------------------------------*/
static void LogFile_SaveRecords(uint32_t indx)
{
  uint32_t    res;
  int32_t     tail;
  int32_t     head;
  int32_t     n;

  T_log_cbl  *log_cbl_ptr = log_cbls[indx];


  if (log_cbl_ptr->log_file_opened == 0) return;


  // Вычисляем количество несохраненных записей в логе
  tail = log_cbl_ptr->file_log_tail;
  head = log_cbl_ptr->event_log_head;
  if (head >= tail)
  {
    n = head - tail;
  }
  else
  {
    n = log_cbl_ptr->log_size - (tail - head);
  }

  // Записываем если число записей превысило некоторое количество или после истечения контрольного времени остались несохраненные записи
  log_cbl_ptr->t_now = tx_time_get();
  if ((n > LOG_RECS_BEFORE_SAVE_TO_FILE) || ((n > 0) && ((log_cbl_ptr->t_now - log_cbl_ptr->t_prev) > ms_to_ticks(TIME_DELAY_BEFORE_SAVE))))
  {
    do
    {
      // Сохраняем записи в файл
      if (log_cbl_ptr->file_log_overfl_f != 0)
      {
        log_cbl_ptr->file_log_overfl_f = 0;
        res = snprintf(file_log_str, LOG_STR_MAX_SZ, "... Overflow ...\r\n");
        fx_file_write(&log_cbl_ptr->log_file, file_log_str, res);
      }
      if (log_cbl_ptr->log_miss_f != 0)
      {
        log_cbl_ptr->log_miss_f = 0;
        res = snprintf(file_log_str, LOG_STR_MAX_SZ, "... Missed records ....\r\n");
        fx_file_write(&log_cbl_ptr->log_file, file_log_str, res);
      }

      if (rtc_init_res.RTC_valid)
      {
        rtc_time_t *pt = &log_cbl_ptr->log_records[tail].date_time;
        res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%04d.%02d.%02d %02d:%02d:%02d |",pt->tm_year,pt->tm_mon,pt->tm_mday,pt->tm_hour, pt->tm_min, pt->tm_sec);
        if (res > 0) fx_file_write(&log_cbl_ptr->log_file, file_log_str, res);
      }

      uint64_t t64 = log_cbl_ptr->log_records[tail].delta_time;
      uint32_t t32;
      uint32_t time_msec      = t64 % 1000000ull;
      t32                     = (uint32_t)(t64 / 1000000ull);
      uint32_t time_sec       = t32 % 60;
      uint32_t time_min       = (t32 / 60) % 60;
      uint32_t time_hour      = (t32 / (60 * 60)) % 24;
      uint32_t time_day       = t32 / (60 * 60 * 24);

      //res = snprintf(tstr, LOG_STR_MAX_SZ, "%04d.%02d.%02d %02d:%02d:%02d.%03d |",app_log[head].time);
      res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%03d d %02d h %02d m %02d s %06d us |",time_day, time_hour, time_min, time_sec, time_msec);
      if (res > 0) fx_file_write(&log_cbl_ptr->log_file, file_log_str, res);
      res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%02d | %-36s | %5d |", log_cbl_ptr->log_records[tail].severity, log_cbl_ptr->log_records[tail].func_name, log_cbl_ptr->log_records[tail].line_num);
      if (res > 0) fx_file_write(&log_cbl_ptr->log_file, file_log_str, res);
      res = snprintf(file_log_str, LOG_STR_MAX_SZ, " %s\r\n", log_cbl_ptr->log_records[tail].msg);
      if (res > 0) fx_file_write(&log_cbl_ptr->log_file, file_log_str, res);

      // Проходим по всем не сохраненным записям
      if (tx_mutex_get(&log_cbl_ptr->log_mutex, TX_WAIT_FOREVER) == TX_SUCCESS)
      {
        tail++;
        if (tail >= log_cbl_ptr->log_size) tail = 0;
        log_cbl_ptr->file_log_tail = tail;
        head = log_cbl_ptr->event_log_head;
        tx_mutex_put(&log_cbl_ptr->log_mutex);
      }
      if (head == tail) break;

      if (log_cbl_ptr->log_file.fx_file_current_file_size >  MAX_LOG_FILE_SIZE)
      {
        log_cbl_ptr->log_file_opened = 0;
        if (fx_file_close(&log_cbl_ptr->log_file)==FX_SUCCESS)
        {
           fx_file_delete(&fat_fs_media, (char*)log_file_prev_names[indx]);
           if (fx_file_rename(&fat_fs_media, (char*)log_file_names[indx], (char*)log_file_prev_names[indx])==FX_SUCCESS)
           {
             if (fx_file_create(&fat_fs_media,(char *)log_file_names[indx])==FX_SUCCESS)
             {
               if (fx_file_open(&fat_fs_media,&log_cbl_ptr->log_file, (char *)log_file_names[indx],  FX_OPEN_FOR_WRITE)==FX_SUCCESS)
               {
                 log_cbl_ptr->log_file_opened = 1;
               }
             }
           }
        }
      }

      fx_media_flush(&fat_fs_media); //  Очищаем кэш записи
      log_cbl_ptr->t_prev = log_cbl_ptr->t_now;

    } while (1);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void LogFile_Task(ULONG arg)
{
  ULONG        flags;

  // Открываем все лог файлы
  for (uint32_t i = 0; i < NUM_OF_LOGS; i++) LogFile_Open(i);


  // Цикл записи в лог файл
  do
  {
    if (tx_event_flags_get(&file_logger_flags, 0xFFFFFFFF, TX_OR_CLEAR,&flags, MS_TO_TICKS(10)) == TX_SUCCESS)
    {
      for (uint32_t i = 0; i < NUM_OF_LOGS; i++) if (log_file_reset_events[i] & flags) LogFile_Reset(i);
    }

    for (uint32_t i = 0; i < NUM_OF_LOGS; i++)  LogFile_SaveRecords(i);

  } while (1);
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Create_File_Logger_task(void)
{
  // Создать задачу записи в log файл
  uint32_t              res;

  if ((g_file_system_ready == 0)
      || (ivar.en_log_to_file == 0)
      || (ivar.usb_mode == USB1_INTF_MASS_STORAGE_DEVICE)  // Логированние в файл конфликтует с режимом Mass Storage. Поэтому его запрещаем
      || (ivar.usb_mode == USB_MODE_VCOM_AND_MASS_STORAGE))
  {
    APPLOG("Writing to the log file is impossible or not allowed");
    return RES_ERROR;
  }

  tx_event_flags_create(&file_logger_flags, "FileLogger");

  res = tx_thread_create(
                         &log_thread,
                         (CHAR *)"Logger",
                         LogFile_Task,
                         0,
                         logger_stacks,
                         LOGGER_TASK_STACK_SIZE,
                         LOGGER_TASK_PRIO,
                         LOGGER_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  if (res == TX_SUCCESS)
  {
    APPLOG("Log to file task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("Log to a file task creating error %04X.", res);
    return RES_ERROR;
  }
}

