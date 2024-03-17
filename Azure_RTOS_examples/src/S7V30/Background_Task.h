#ifndef BACKGROUND_TASK_H
  #define BACKGROUND_TASK_H

#define     APP_DO_SYSTEM_RESTART      BIT(0)

#define     REF_TIME_INTERVAL          5       // Интервал времени в милисекундах на котором производлится калибровка и измерение загруженности процессора

extern volatile uint32_t     g_aver_cpu_usage;
extern volatile uint32_t     g_cpu_usage;            // Процент загрузки процессора

uint32_t    Send_flag_to_background(uint32_t flag);
uint32_t    Create_Backgroung_task(void);
void        Get_reference_time(void);
uint64_t    Measure_reference_time_interval(uint32_t time_delay_ms);
void        Request_save_nv_parameters(const T_NV_parameters_instance* pinst, uint8_t ptype);

#endif // BACKGROUND_TASK_H



