﻿// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-20
// 16:37:01
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"


#define CPU_USAGE_FLTR_LEN  128

uint32_t               g_ref_time;             // Калибровочная константа предназначенная для измерения нагрузки микропроцессора
T_run_average_int32_N  filter_cpu_usage;
volatile uint32_t      g_aver_cpu_usage;
volatile uint32_t      g_cpu_usage;            // Процент загрузки процессора
int32_t                cpu_usage_arr[CPU_USAGE_FLTR_LEN];

#define BACKGROUND_TASK_STACK_SIZE (1024*2)
static uint8_t background_stacks[BACKGROUND_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.background")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       background_thread;

static TX_EVENT_FLAGS_GROUP   bkg_flags;
static TX_MUTEX               bkg_mutex;


typedef struct
{
    uint8_t                   need_to_save_settings;
    const T_NV_parameters_instance *pinst;
    uint8_t                   ptype;

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
  if (tx_mutex_get(&bkg_mutex, MS_TO_TICKS(1000))== TX_SUCCESS)
  {
    ps_req.pinst = pinst;
    ps_req.ptype = ptype;
    ps_req.need_to_save_settings = 1;
    tx_mutex_put(&bkg_mutex);
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static void  Parameters_saver(void)
{
  if  (ps_req.need_to_save_settings != 0)
  {
    if (tx_mutex_get(&bkg_mutex, MS_TO_TICKS(1000))== TX_SUCCESS)
    {
      ps_req.need_to_save_settings = 0;
      if (Save_settings(ps_req.pinst, ps_req.ptype) == RES_OK)
      {
        APPLOG("Type %d settings saved successfully.", ps_req.ptype);
      }
      else
      {
        APPLOG("Type %d settings saving error.", ps_req.ptype);
      }
      tx_mutex_put(&bkg_mutex);
    }
    else
    {
      APPLOG("Type %d settings saving error. ", ps_req.ptype);
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
  tx_mutex_create(&bkg_mutex, "bkg_mitex", TX_INHERIT);

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

    Parameters_saver();

  }
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Create_Backgroung_task(void)
{
  UINT              err;

  err = tx_thread_create(
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

  if (err == TX_SUCCESS)
  {
    APPLOG("Background task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("Background creating error %d.", err);
    return RES_ERROR;
  }
}

