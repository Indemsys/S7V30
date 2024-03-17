// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.05.13
// 16:37:49
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include <stdarg.h>

#ifdef LOG_TO_ONBOARD_SDRAM
  #pragma section = ".sdram"
__no_init T_app_log_record     app_log[EVENT_LOG_SIZE] @ ".sdram";
__no_init T_ssp_log_record     ssp_log[SSP_LOG_SIZE]   @ ".sdram";
#else
__no_init T_app_log_record     app_log[EVENT_LOG_SIZE];
#endif

#define LOGGER_TASK_STACK_SIZE 2048
static uint8_t logger_stacks[LOGGER_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.Logger_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static void Task_Logger(ULONG arg);

T_app_log_cbl                         app_log_cbl;
T_app_log_cbl                         ssp_log_cbl;
extern uint32_t                       g_ssp_log_inited;

#define TIME_DELAY_BEFORE_SAVE        100 // Время в мс перед тем как будут сохранены оставшиеся записи
#define LOG_RECS_BEFORE_SAVE_TO_FILE  10  // Количество записей вызывающее немедленное сохранение
#define LOG_FILE_NAME                 "log.txt"
char                                  file_log_str[LOG_STR_MAX_SZ];
static TX_THREAD                      log_thread;

uint8_t                               request_to_reset_log;

static TX_MUTEX                       logger_mutex;
static char                           rtt_log_str[RTT_LOG_STR_SZ];


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Req_to_reset_log_file(void)
{
  request_to_reset_log  = 1;
}



/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Init_ssp_logger(void)
{
  memset(ssp_log, 0, sizeof(ssp_log));
  ssp_log_cbl.event_log_head = 0;
  ssp_log_cbl.event_log_tail = 0;
  Get_hw_timestump(&ssp_log_cbl.log_start_time);
  ssp_log_cbl.log_inited = 1;
  g_ssp_log_inited = 1;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Init_app_logger(void)
{
  if (tx_mutex_create(&logger_mutex, "Logger", TX_INHERIT) != TX_SUCCESS) return RES_ERROR;

  app_log_cbl.event_log_head = 0;
  app_log_cbl.event_log_tail = 0;
  Get_hw_timestump(&app_log_cbl.log_start_time);
  app_log_cbl.log_inited = 1;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void Task_Logger(ULONG arg)
{
  FX_FILE              f;
  uint32_t             res;
  int32_t              tail;
  int32_t              head;
  int32_t              n;
  uint32_t             t, t2;

  // Открыть файл для записи лога
  res = fx_file_create(&fat_fs_media,LOG_FILE_NAME);
  if ((res == FX_SUCCESS) ||  (res == FX_ALREADY_CREATED))
  {
    res = fx_file_open(&fat_fs_media,&f, LOG_FILE_NAME,  FX_OPEN_FOR_WRITE);
  }

  Set_app_event(EVENT_LOGGER_TASK_READY);

  if (res != FX_SUCCESS)
  {
    tx_thread_terminate(tx_thread_identify());
    return;
  }

  t = tx_time_get();

  // Цикл записи в лог файл
  do
  {
    // Записываем если число записей превысило некторое количество или после истечения контрольного времени остались несохраненные записи


    // Вычисляем колическтво несохраненных записей в логе
    tail = app_log_cbl.file_log_tail;
    head = app_log_cbl.event_log_head;
    if (head >= tail)
    {
      n = head - tail;
    }
    else
    {
      n = EVENT_LOG_SIZE -(tail - head);
    }

    t2 = tx_time_get();
    if ((n > LOG_RECS_BEFORE_SAVE_TO_FILE) || ((n > 0) && ((t2 - t) > ms_to_ticks(TIME_DELAY_BEFORE_SAVE))))
    {

      do
      {
        // Сохраняем записи в файл
        if (app_log_cbl.file_log_overfl_f != 0)
        {
          app_log_cbl.file_log_overfl_f = 0;
          res = snprintf(file_log_str, LOG_STR_MAX_SZ, "... Overflow ...\r\n");
          fx_file_write(&f, file_log_str, res);
        }
        if (app_log_cbl.log_miss_f != 0)
        {
          app_log_cbl.log_miss_f = 0;
          res = snprintf(file_log_str, LOG_STR_MAX_SZ, "... Missed records ....\r\n");
          fx_file_write(&f, file_log_str, res);
        }

        if (rtc_init_res.RTC_valid)
        {
          rtc_time_t *pt =&app_log[tail].date_time;
          res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%04d.%02d.%02d %02d:%02d:%02d |",pt->tm_year,pt->tm_mon,pt->tm_mday,pt->tm_hour, pt->tm_min, pt->tm_sec);
          if (res > 0) fx_file_write(&f, file_log_str, res);
        }

        uint64_t t64 = app_log[tail].delta_time;
        uint32_t t32;
        uint32_t time_msec      = t64 % 1000000ull;
        t32 = (uint32_t)(t64 / 1000000ull);
        uint32_t time_sec       = t32 % 60;
        uint32_t time_min       =(t32 / 60)% 60;
        uint32_t time_hour      =(t32 / (60 * 60))% 24;
        uint32_t time_day       = t32 / (60 * 60 * 24);

        //res = snprintf(tstr, LOG_STR_MAX_SZ, "%04d.%02d.%02d %02d:%02d:%02d.%03d |",app_log[head].time);
        res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%03d d %02d h %02d m %02d s %06d us |",time_day, time_hour, time_min, time_sec, time_msec);
        if (res > 0) fx_file_write(&f, file_log_str, res);
        res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%02d | %-36s | %5d |", app_log[tail].severity, app_log[tail].func_name, app_log[tail].line_num);
        if (res > 0) fx_file_write(&f, file_log_str, res);
        res = snprintf(file_log_str, LOG_STR_MAX_SZ, " %s\r\n", app_log[tail].msg);
        if (res > 0) fx_file_write(&f, file_log_str, res);

        // Проходим по всем не сохраненным записям
        if (tx_mutex_get(&logger_mutex, TX_WAIT_FOREVER) == TX_SUCCESS)
        {
          tail++;
          if (tail >= EVENT_LOG_SIZE) tail = 0;
          app_log_cbl.file_log_tail = tail;
          head = app_log_cbl.event_log_head;
          tx_mutex_put(&logger_mutex);
        }
        if (head == tail) break;

      } while (1);


      fx_media_flush(&fat_fs_media); //  Очищаем кэш записи
      t = t2; // Запоминаем время последней записи
    }
    Wait_ms(10);


  } while (1);

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
  if (head >= EVENT_LOG_SIZE) head = 0;
  ssp_log_cbl.event_log_head = head;

  tail = ssp_log_cbl.event_log_tail;
  // Если голова достигла хвоста, то сдвигает указатель хвоста и теряем непрочитанные данные
  if (head == tail)
  {
    tail++;
    if (tail >= EVENT_LOG_SIZE) tail = 0;
    ssp_log_cbl.event_log_tail = tail;
    ssp_log_cbl.log_overfl_f = 1;
    ssp_log_cbl.log_overfl_err++;
  }
  __enable_interrupt();

  #endif
}



/*------------------------------------------------------------------------------
  Запись сообщения в таблицу лога и в другие места назначения


 \param str         : сообщение
 \param func_name   : имя функции
 \param line_num    : номер строки
 \param severity    : важность сообщения
 ------------------------------------------------------------------------------*/
void Applog_write(char *str, const char *func_name, unsigned int line_num, unsigned int severity)
{
  int         head;
  int         tail;
  T_sys_timestump   ntime;
  rtc_time_t        date_time;

  if (app_log_cbl.log_inited == 1)
  {
    Get_hw_timestump(&ntime);
    RTC_get_system_DateTime(&date_time);
    date_time.tm_mon++;
    date_time.tm_year += 1900;

    // Вызов данной процедуры может производится из процедур обслуживания прерываний,
    // поэтому мьютексы и другие сервисы сихронизации здесь использовать нельзя
    if (tx_mutex_get(&logger_mutex, MS_TO_TICKS(LOGGER_WR_TIMEOUT_MS)) == TX_SUCCESS)
    {

      head = app_log_cbl.event_log_head;
      // Определяем время в микросекундах от старта лога
      app_log[head].date_time = date_time;
      app_log[head].delta_time = Hw_timestump_diff64_us(&app_log_cbl.log_start_time,&ntime);
      strncpy(app_log[head].msg, str, LOG_STR_MAX_SZ - 1);
      strncpy(app_log[head].func_name, func_name, EVNT_LOG_FNAME_SZ - 1);
      app_log[head].line_num = line_num;
      app_log[head].severity = severity;
      // Сдвигаем указатель головы лога
      head++;
      if (head >= EVENT_LOG_SIZE) head = 0;
      app_log_cbl.event_log_head = head;

      tail = app_log_cbl.event_log_tail;
      // Если голова достигла хвоста, то сдвигает указатель хвоста и теряем непрочитанные данные
      if (head == tail)
      {
        tail++;
        if (tail >= EVENT_LOG_SIZE) tail = 0;
        app_log_cbl.event_log_tail = tail;
        app_log_cbl.log_overfl_f = 1;
        app_log_cbl.log_overfl_err++;
      }
      // Если голова достигла хвоста записи в файл, то сдвигает указатель хвоста записи в файл и теряем непрочитанные данные
      tail = app_log_cbl.file_log_tail;
      if (head == tail)
      {
        tail++;
        if (tail >= EVENT_LOG_SIZE) tail = 0;
        app_log_cbl.file_log_tail = tail;
        app_log_cbl.file_log_overfl_f = 1;
        app_log_cbl.file_log_overfl_err++;
      }
      tx_mutex_put(&logger_mutex);
    }
  }
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
  char             log_str[LOG_STR_MAX_SZ+1];

  va_list          ap;

  va_start(ap, fmt_ptr);

  vsnprintf(log_str, LOG_STR_MAX_SZ, (const char *)fmt_ptr, ap);
  Applog_write(log_str , name, line_num, severity);


  va_end(ap);
}

/*------------------------------------------------------------------------------
  Скопировать запись лога от хвоста очереди записей для передачи в канал FreeMaster

 \param rec

 \return int
 ------------------------------------------------------------------------------*/
int32_t AppLog_get_tail_record(T_app_log_record *rec)
{
  int32_t   res = RES_ERROR;
  uint32_t  tail;

  if (tx_mutex_get(&logger_mutex, 10) == TX_SUCCESS)
  {
    tail = app_log_cbl.event_log_tail;
    if (app_log_cbl.event_log_head != tail)
    {
      memcpy(rec,&app_log[tail], sizeof(T_app_log_record));
      tail++;
      if (tail >= EVENT_LOG_SIZE)
      {
        tail = 0;
      }
      app_log_cbl.event_log_tail = tail;
      res = RES_OK;
    }
    tx_mutex_put(&logger_mutex);
    return res;
  }
  else
  {
    return RES_ERROR;
  }
}

/*------------------------------------------------------------------------------



 \param pvt100_cb
 ------------------------------------------------------------------------------*/
void AppLogg_monitor_output(void)
{
  uint32_t   i;
  uint32_t   n;
  uint32_t   reqn;
  uint32_t   outn;
  uint8_t    b;
  uint32_t   head;
  uint32_t   tail;

  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF("Events log. <R> - exit, <D> - print all log\n\r");
  MPRINTF("............................................\n\r");
  MPRINTF("Log overflows=%d, File log overflows=%d, Log miss count=%d\r\n",app_log_cbl.log_overfl_err, app_log_cbl.file_log_overfl_err,app_log_cbl.log_miss_err);
  MPRINTF("********************************************\n\r");

  do
  {
    VT100_set_cursor_pos(3, 0);


    // Вывод последних 22-х строк лога


    // Определяем количестово строк в логе
    head = app_log_cbl.event_log_head;
    tail = app_log_cbl.event_log_tail;
    if (head >= tail)
    {
      reqn = head - tail;
    }
    else
    {
      reqn = EVENT_LOG_SIZE -(tail - head);
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
      n = head - outn;
    }
    else
    {
      if (outn > head)
      {
        n = EVENT_LOG_SIZE -(outn - head);
      }
      else
      {
        n = head - outn;
      }
    }


    for (i = 0; i < EVENT_LOG_DISPLAY_ROW; i++)
    {
      if (i < outn)
      {
        uint64_t t64 = app_log[n].delta_time;
        uint32_t time_usec = t64 % 1000000ull;
        uint32_t time_sec  = (uint32_t)(t64 / 1000000ull);

        if (app_log[n].line_num != 0)
        {
          MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s (%s %d)\n\r",
                  time_sec, time_usec,
                  app_log[n].msg,
                  app_log[n].func_name,
                  app_log[n].line_num);
        }
        else
        {
          MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s\n\r",
                  time_sec, time_usec,
                  app_log[n].msg);
        }
        n++;
        if (n >= EVENT_LOG_SIZE) n = 0;
      }
      else
      {
        MPRINTF(VT100_CLL_FM_CRSR"\n\r");
      }
    }



    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case 'D':
      case 'd':
        MPRINTF(VT100_CLEAR_AND_HOME);
        // Вывод всего лога
        head = app_log_cbl.event_log_head;
        tail = app_log_cbl.event_log_tail;
        //
        if (head >= tail)
        {
          reqn = head - tail;
        }
        else
        {
          reqn = EVENT_LOG_SIZE -(tail - head);
        }

        MPRINTF("\n\r");
        n = tail;
        for (i = 0; i < reqn; i++)
        {
          uint64_t t64 = app_log[n].delta_time;
          uint32_t time_usec = t64 % 1000000ull;
          uint32_t time_sec  = (uint32_t)(t64 / 1000000ull);

          if (app_log[n].line_num != 0)
          {
            MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s (%s %d)\n\r",
                    time_sec, time_usec,
                    app_log[n].msg,
                    app_log[n].func_name,
                    app_log[n].line_num);
          }
          else
          {
            MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s\n\r",
                    time_sec, time_usec,
                    app_log[n].msg);
          }
          n++;
          if (n >= EVENT_LOG_SIZE) n = 0;
        }
        MPRINTF("\n\r");
        WAIT_CHAR(&b, 200000);

        break;
      case 'R':
      case 'r':
      case VT100_ESC:
        return;
      case 'C':
      case 'c':
        app_log_cbl.event_log_head = 0;
        app_log_cbl.event_log_tail = 0;
        break;
      }

    }
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Create_file_log_task(void)
{
  // Создать задачу записи в log файл
  UINT              err;

  err = tx_thread_create(
                         &log_thread,
                         (CHAR *)"Logger",
                         Task_Logger,
                         0,
                         logger_stacks,
                         LOGGER_TASK_STACK_SIZE,
                         LOGGER_TASK_PRIO,
                         LOGGER_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  if (err == TX_SUCCESS)
  {
    APPLOG("Logger task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("Logger task creating error %d.", err);
    return RES_ERROR;
  }
}

