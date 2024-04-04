#ifndef LOGGER_H
  #define LOGGER_H

//#define SSP_LOGGER  // Макрос разрешающий логгер ошибок SSP

  #define SEVERITY_RED 1

  #ifdef LOG_TO_ONBOARD_SDRAM
    #define    EVENT_LOG_SIZE          (1024)  // Должно быть степенью 2
    #define    SSP_LOG_SIZE            (128)
  #else
    #define    EVENT_LOG_SIZE          (32)    // Должно быть степенью 2
  #endif

  #define    LOG_STR_MAX_SZ            (150)
  #define    EVNT_LOG_FNAME_SZ         (64)
  #define    SSP_LOG_MODULE_NAME_SZ    (42)
  #define    RTT_LOG_STR_SZ            (256)

  #define    EVENT_LOG_DISPLAY_ROW     22 // Количество строк лога выводимых на экран при автообновлении

  #define    LOGGER_WR_TIMEOUT_MS      100


  #define    EVT_FILE_LOG_RESET_LOG    BIT(0)
  #define    EVT_FILE_LOG_CMD_OK       BIT(1)
  #define    EVT_FILE_LOG_CMD_FAIL     BIT(2)

// Структура хранения записи лога
typedef struct
{
    rtc_time_t       date_time;
    uint64_t         delta_time;
    char             msg[LOG_STR_MAX_SZ+1];
    char             func_name[EVNT_LOG_FNAME_SZ+1];
    unsigned int     line_num;
    unsigned int     severity;
} T_app_log_record;


typedef struct
{
    T_sys_timestump  timestump;
    uint32_t         err;
    char             module_name[SSP_LOG_MODULE_NAME_SZ+1];
    unsigned int     line_num;
} T_ssp_log_record;


typedef struct
{
    T_sys_timestump   log_start_time;  // Время старта лога

    volatile uint32_t event_log_head;
    volatile uint32_t event_log_tail;
    volatile uint32_t file_log_tail;

    unsigned int      log_miss_err;         // Счетчик ошибок ожидания доступа к логу
    unsigned int      log_overfl_err;       // Счетчик ошибок переполнения лога
    unsigned int      file_log_overfl_err;  // Счетчик ошибок переполнения файлового лога

    unsigned int      log_miss_f;           // Флаг ошибоки ожидания доступа к логу
    unsigned int      log_overfl_f;         // Флаг ошибоки переполнения лога
    unsigned int      file_log_overfl_f;    // Флаг ошибоки переполнения файлового лога


    uint32_t          logger_ready;         // Флаг готовности к записи в лог

} T_app_log_cbl;

extern T_app_log_cbl  app_log_cbl;
extern T_app_log_cbl  ssp_log_cbl;

  #define APPLOG(...)    LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, ##__VA_ARGS__);

uint32_t  Init_app_logger(void);
uint32_t  Init_ssp_logger(void);
void      Applog_write(char *str, const char *func_name, unsigned int line_num, unsigned int severity);

void      LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...);
void      RTT_LOGs(const char *fmt_ptr, ...);

void      AppLogg_monitor_output(void);
uint32_t  Create_File_Logger_task(void);
int32_t   AppLog_get_tail_record(T_app_log_record *rec);
void      Req_to_reset_log_file(void);
void      Set_file_logger_event(uint32_t events_mask);
uint32_t  Wait_file_logger_event(uint32_t events_mask, ULONG *actual_flags, uint32_t opt, uint32_t wait);

#endif



