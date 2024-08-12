// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-04-24
// 14:31:55
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


TX_THREAD charger_thread;
#define CHARGER_THREAD_STACK_SIZE (1024*4)
static uint8_t charger_thread_stack[CHARGER_THREAD_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.charger_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);

uint8_t  bq25619_status0;
uint8_t  bq25619_status1;
uint8_t  bq25619_status2;
uint8_t  bq25619_status0_prev;
uint8_t  charge_state;
uint8_t  charge_state_prev;
uint32_t charge_state_change_cnt;

static uint8_t  charger_task_suspended;

static TX_EVENT_FLAGS_GROUP   charger_flags;

T_charger_cbl  ch_cbl;

/*-----------------------------------------------------------------------------------------------------


  \param eventmask
-----------------------------------------------------------------------------------------------------*/
void Set_charger_event(uint32_t events_mask)
{
  tx_event_flags_set(&charger_flags, events_mask, TX_OR);
}



/*-----------------------------------------------------------------------------------------------------


  \param src

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_BQ25619_power_status_str(uint8_t b)
{
  uint8_t v = (b >> 2) & 0x01;
  switch (v)
  {
  case 0:
    return "Power NOT good";
  case 1:
    return "Power good";
  }
  return "UNDEF   ";
}


/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void Charger_thread(ULONG arg)
{
  ssp_err_t       ssp_res;
  uint8_t         b;
  T_sys_timestump tst;
  ULONG           actual_flags;

  ssp_res = BQ25619_open();
  if (ssp_res != SSP_SUCCESS)
  {
    APPLOG("Charger IC BQ25619 communication error %d", (uint32_t)ssp_res);
    return;
  }
  ssp_res = BQ25619_read_reg(BQ25619_PART_INFORMATION,&b);
  if (ssp_res != SSP_SUCCESS)
  {
    APPLOG("Charger IC BQ25619 communication error %d", (uint32_t)ssp_res);
    return;
  }
  if ((b & 0x7F) != 0x2C)
  {
    APPLOG("Charger chip BQ25619 is not identified %02X", b);
    return;
  }


  ssp_res = BQ25619_read_reg(BQ25619_SYSTEM_STATUS0,&bq25619_status0);
  if (ssp_res != SSP_SUCCESS)
  {
    APPLOG("Charger chip BQ25619_SYSTEM_STATUS0 error");
    return;
  }
  ssp_res = BQ25619_read_reg(BQ25619_SYSTEM_STATUS1,&bq25619_status1);
  if (ssp_res != SSP_SUCCESS)
  {
    APPLOG("Charger chip BQ25619_SYSTEM_STATUS1 error");
    return;
  }
  ssp_res = BQ25619_read_reg(BQ25619_SYSTEM_STATUS2,&bq25619_status2);
  if (ssp_res != SSP_SUCCESS)
  {
    APPLOG("Charger chip BQ25619_SYSTEM_STATUS2 error");
    return;
  }
  APPLOG("Charger chip BQ25619 Status 0 = 0x%02X: %s, %s, %s ", bq25619_status0, Get_BQ25619_VBUS_status_str(bq25619_status0), Get_BQ25619_power_status_str(bq25619_status0), Get_BQ25619_charging_status_str(bq25619_status0));
  APPLOG("Charger chip BQ25619 Status 1 = 0x%02X, Status 2 = 0x%02X", bq25619_status1, bq25619_status2);
  bq25619_status0_prev = bq25619_status0;
  charge_state = BQ25619_get_charge_state(bq25619_status0);
  charge_state_prev = charge_state;

  ssp_res = MAX17262_open();
  if (ssp_res != SSP_SUCCESS)
  {
    APPLOG("Fuel Gauge IC MAX17262 communication error %d", (uint32_t)ssp_res);
  }

  Get_hw_timestump(&tst);

  tx_event_flags_create(&charger_flags, "Charger");

  BQ25619_set_max_sys_voltage();

  do
  {
    // Ожидаем события 10 мс
    if (tx_event_flags_get(&charger_flags,0xFFFFFFFF, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(10)) == TX_SUCCESS)
    {
      if (actual_flags & CHARGER_RESET_POWER)
      {
        BQ25619_switch_off_vbus();
      }
      if (actual_flags & CHARGER_SW_OFF_ACCUM)
      {
        BQ25619_switch_off_accum();
      }
      if (actual_flags & CHARGER_SUSPEND)
      {
        charger_task_suspended = 1;
      }
      if (actual_flags & CHARGER_RESUME)
      {
        charger_task_suspended = 0;
      }
    }

    if (charger_task_suspended == 0)
    {
      ssp_res = BQ25619_read_reg(BQ25619_SYSTEM_STATUS0,&bq25619_status0);
      if (ssp_res != SSP_SUCCESS)
      {
        APPLOG("Read chip BQ25619_SYSTEM_STATUS0 error");
      }
      else
      {
        ch_cbl.charger_state_str = Get_BQ25619_charging_status_str(bq25619_status0);

        if (bq25619_status0_prev != bq25619_status0)
        {
          APPLOG("Charger chip BQ25619 Status = 0x%02X: %s, %s, %s ", bq25619_status0, Get_BQ25619_VBUS_status_str(bq25619_status0), Get_BQ25619_power_status_str(bq25619_status0), ch_cbl.charger_state_str);
          bq25619_status0_prev = bq25619_status0;
          charge_state = BQ25619_get_charge_state(bq25619_status0);
          if (charge_state_prev != charge_state)
          {
            // Если состояние зарядки поменялось более 5 раз за 1 сек, то это признак отсутствия аккумулятора и отключаем аккумулятор в таком случае
            charge_state_change_cnt++;
            if  (charge_state_change_cnt > 5)
            {
              if (Time_elapsed_msec(&tst) < 1000)
              {
                ch_cbl.no_accum  = 1;
                // Определяем состояние с отключенным аккумулятором и запрещаем работу зарядника
                BQ25619_disable_charger();
                APPLOG("Charger disabled");
              }
              Get_hw_timestump(&tst);
              charge_state_change_cnt = 0;
            }
            charge_state_prev = charge_state;
          }
        }
      }
    }

  } while (1);

}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void Charger_task_suspend(void)
{
  Set_charger_event(CHARGER_SUSPEND);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void Charger_task_resume(void)
{
  Set_charger_event(CHARGER_RESUME);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t Charger_thread_create(void)
{
  uint32_t              res;
  ch_cbl.charger_state_str = "";
  res = tx_thread_create(
                   &charger_thread,
                   (CHAR *)"Charger Thread",
                   Charger_thread,
                   (ULONG) NULL,
                   &charger_thread_stack,
                   CHARGER_THREAD_STACK_SIZE,
                   CHARGER_TASK_PRIO,
                   CHARGER_TASK_PRIO,
                   1,
                   TX_AUTO_START
                  );
  if (res == TX_SUCCESS)
  {
    APPLOG("Charger task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("Charger task creating error %04X.", res);
    return RES_ERROR;
  }

}
