#ifndef CHARGER_TASK_H
  #define CHARGER_TASK_H


#define     CHARGER_RESET_POWER  BIT(0)
#define     CHARGER_SUSPEND      BIT(1)
#define     CHARGER_RESUME       BIT(2)
#define     CHARGER_SW_OFF_ACCUM BIT(3)

typedef struct
{
  uint8_t           no_accum;
  const char*       charger_state_str;
} T_charger_cbl;

extern T_charger_cbl  ch_cbl;

uint32_t    Charger_thread_create(void);

char const* Get_BQ25619_power_status_str(uint8_t src);
void        Set_charger_event(uint32_t events_mask);
void        Charger_task_suspend(void);
void        Charger_task_resume(void);

#endif



