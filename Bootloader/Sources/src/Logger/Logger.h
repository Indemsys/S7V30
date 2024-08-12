#ifndef LOGGER_H
  #define LOGGER_H

#define  MAX_LOG_FILE_SIZE            100000000ul

//#define SSP_LOGGER  // Макрос разрешающий логгер ошибок SSP

  #define SEVERITY_RED 1

  #ifdef LOG_TO_ONBOARD_SDRAM
    #define    APP_LOG_SIZE            (1024)  // Должно быть степенью 2
    #define    NET_LOG_SIZE            (1024)  // Должно быть степенью 2
    #define    SSP_LOG_SIZE            (128)
  #else
    #define    APP_LOG_SIZE            (32)    // Должно быть степенью 2
    #define    NET_LOG_SIZE            (32)    // Должно быть степенью 2
  #endif

  #define    LOG_STR_MAX_SZ            (150)
  #define    EVNT_LOG_FNAME_SZ         (64)
  #define    SSP_LOG_MODULE_NAME_SZ    (42)
  #define    RTT_LOG_STR_SZ            (256)

  #define    EVENT_LOG_DISPLAY_ROW     22 // Количество строк лога выводимых на экран при автообновлении

  #define    LOGGER_WR_TIMEOUT_MS      100


  #define    EVT_RESET_APP_FILE_LOG    BIT(0)
  #define    EVT_RESET_NET_FILE_LOG    BIT(1)
  #define    EVT_FILE_LOG_CMD_OK       BIT(2)
  #define    EVT_FILE_LOG_CMD_FAIL     BIT(3)

// Структура хранения записи лога
typedef struct
{
    rtc_time_t       date_time;
    uint64_t         delta_time;
    char             msg[LOG_STR_MAX_SZ+1];
    char             func_name[EVNT_LOG_FNAME_SZ+1];
    unsigned int     line_num;
    unsigned int     severity;
} T_logger_record;


typedef struct
{
    T_sys_timestump  timestump;
    uint32_t         err;
    char             module_name[SSP_LOG_MODULE_NAME_SZ+1];
    unsigned int     line_num;
} T_ssp_log_record;


typedef struct
{
    const char        *name;
    T_sys_timestump   log_start_time;  // Время старта лога
    T_logger_record  *log_records;
    uint32_t          log_size;
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

    TX_MUTEX          log_mutex;
    FX_FILE           log_file;
    uint32_t          t_prev;
    uint32_t          t_now;
    uint8_t           log_file_opened;
} T_log_cbl;

extern T_log_cbl  app_log_cbl;
extern T_log_cbl  ssp_log_cbl;

  #define APPLOG(...)     LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, ##__VA_ARGS__);
  #define NET_LOG(...)    Net_LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, ##__VA_ARGS__);

void      App_log_disable(void);
uint32_t  Init_app_logger(void);
uint32_t  Init_ssp_logger(void);

void      LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...);
void      Net_LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...);
void      RTT_LOGs(const char *fmt_ptr, ...);

void      App_Log_monitor(void);
uint32_t  Create_File_Logger_task(void);
int32_t   AppLog_get_tail_record(T_logger_record *rec);
void      Req_to_reset_log_file(void);
void      Req_to_reset_netlog_file(void);
void      Set_file_logger_event(uint32_t events_mask);
uint32_t  App_Log_get_formated_string(char *str, uint32_t max_str_len);
#endif



