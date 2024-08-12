// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-20
// 16:37:01
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BLE_main.h"

#define CPU_USAGE_FLTR_LEN  128

uint32_t               g_ref_time;             // Калибровочная константа предназначенная для измерения нагрузки микропроцессора
T_run_average_int32_N  filter_cpu_usage;
volatile uint32_t      g_aver_cpu_usage;
volatile uint32_t      g_cpu_usage;            // Процент загрузки процессора
int32_t                cpu_usage_arr[CPU_USAGE_FLTR_LEN];

#define BACKGROUND_TASK_STACK_SIZE (1024*3)
static uint8_t background_stacks[BACKGROUND_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.background")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       background_thread;

static TX_EVENT_FLAGS_GROUP   bkg_flags;
static TX_MUTEX               bkg_mutex;

#define CMD_TO_SAVE_PARAMETERS  BIT(0)
#define CMD_TO_SAVE_BT_NV_DATA  BIT(1)

typedef struct
{
    uint8_t                         request_mask;
    const T_NV_parameters_instance *params_instance;
    uint8_t                         params_type;

} T_params_saving_req;

T_params_saving_req ps_req;

/*-----------------------------------------------------------------------------------------------------
  Получаем  оценку калибровочного интервала времени предназначенного для измерения загрузки процессора

  Проводим несколько измерений и выбираем минимальный интервал
-----------------------------------------------------------------------------------------------------*/
void Get_reference_time(void)
{
  uint32_t i;
  uint32_t t;
  uint32_t tt = 0xFFFFFFFF;

  for (i = 0; i < 10; i++)
  {
    t = (uint32_t)Measure_reference_time_interval(REF_TIME_INTERVAL);
    if (t < tt) tt = t;
  }
  g_ref_time = tt;
}



/*-----------------------------------------------------------------------------------------------------
  Измеряем длительность интервала времени ti заданного в милисекундах
-----------------------------------------------------------------------------------------------------*/
uint64_t Measure_reference_time_interval(uint32_t time_delay_ms)
{
  T_sys_timestump   tickt1;
  T_sys_timestump   tickt2;
  uint64_t diff;


  Get_hw_timestump(&tickt1);
  DELAY_ms(time_delay_ms);
  Get_hw_timestump(&tickt2);

  diff =Hw_timestump_diff64_us(&tickt1,&tickt2);

  return diff;
}

/*-----------------------------------------------------------------------------------------------------
  Зарегистрировать запрос на сохранение параметров

 \param void
-----------------------------------------------------------------------------------------------------*/
void Request_save_nv_parameters(const T_NV_parameters_instance *pinst, uint8_t ptype)
{
  if (tx_mutex_get(&bkg_mutex, MS_TO_TICKS(1000)) == TX_SUCCESS)
  {
    ps_req.request_mask     |= CMD_TO_SAVE_PARAMETERS;
    ps_req.params_instance   = pinst;
    ps_req.params_type       = ptype;
    tx_mutex_put(&bkg_mutex);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param pinst
  \param ptype
-----------------------------------------------------------------------------------------------------*/
void Request_save_bt_nv_data(void)
{
  if (tx_mutex_get(&bkg_mutex, MS_TO_TICKS(1000)) == TX_SUCCESS)
  {
    ps_req.request_mask     |= CMD_TO_SAVE_BT_NV_DATA;
    ps_req.params_instance   = 0;
    ps_req.params_type       = 0;
    tx_mutex_put(&bkg_mutex);
  }
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static void  NV_data_saver(void)
{
  if  (ps_req.request_mask != 0)
  {
    if (tx_mutex_get(&bkg_mutex, MS_TO_TICKS(1000)) == TX_SUCCESS)
    {

      if (ps_req.request_mask & CMD_TO_SAVE_PARAMETERS)
      {
        if (Save_settings(ps_req.params_instance, ps_req.params_type) == RES_OK)
        {
          APPLOG("Type %d settings saved successfully", ps_req.params_type);
        }
        else
        {
          APPLOG("Type %d settings saving error", ps_req.params_type);
        }
        ps_req.request_mask &= ~CMD_TO_SAVE_PARAMETERS;
      }

      if (ps_req.request_mask & CMD_TO_SAVE_BT_NV_DATA)
      {
        if (Save_buf_to_DataFlash(DATAFLASH_BLUETOOTH_DATA_ADDR, (uint8_t*)&bt_nv, sizeof(bt_nv))== RES_OK)
        {
          APPLOG("Bluetooth NV data saved successfully");
        }
        else
        {
          APPLOG("Bluetooth NV data saving error");
        }
        ps_req.request_mask &= ~CMD_TO_SAVE_BT_NV_DATA;
      }

      tx_mutex_put(& bkg_mutex);
    }
    else
    {
      APPLOG("NV data saving error. ");
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param flag

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Send_flag_to_background(uint32_t flag)
{
  return  tx_event_flags_set(&bkg_flags, flag, TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void Background_task(ULONG arg)
{
  uint32_t t, dt;

  g_cpu_usage      = 1000;
  g_aver_cpu_usage = 1000;

  tx_event_flags_create(&bkg_flags, "bkg_flags");
  tx_mutex_create(&bkg_mutex, "bkg_mutex", TX_INHERIT);

  filter_cpu_usage.len = CPU_USAGE_FLTR_LEN;
  filter_cpu_usage.en  = 0;
  filter_cpu_usage.arr = cpu_usage_arr;
  g_aver_cpu_usage = RunAverageFilter_int32_N(g_cpu_usage,&filter_cpu_usage);

  for (;;)
  {
    t = Measure_reference_time_interval(REF_TIME_INTERVAL);

    if (t < g_ref_time)
    {
      dt = 0;
    }
    else
    {
      dt = t - g_ref_time;
    }
    g_cpu_usage =(1000ul * dt) / g_ref_time;
    g_aver_cpu_usage = RunAverageFilter_int32_N(g_cpu_usage,&filter_cpu_usage);

    NV_data_saver();
  }
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Create_Backgroung_task(void)
{
  UINT              res;

  res = tx_thread_create(
       &background_thread,
       (CHAR *)"Background",
       Background_task,
       0,
       background_stacks,
       BACKGROUND_TASK_STACK_SIZE,
       BACKGROUND_TASK_PRIO,
       BACKGROUND_TASK_PRIO,
       1,
       TX_AUTO_START
       );

  if (res == TX_SUCCESS)
  {
    APPLOG("Background task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("Background creating error %04X.", res);
    return RES_ERROR;
  }
}

