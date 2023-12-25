// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-03-26
// 18:59:34
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"

TX_TIMER wdt_timer;


/*-----------------------------------------------------------------------------------------------------
  IWDT тактируется от независимого осциллятора IWDTCLK 15КГц

  Функция вызывается в ISR SysTick_Handler
  \param void
-----------------------------------------------------------------------------------------------------*/
void IWDT_refresh(ULONG v)
{
    R_IWDT->IWDTRR = 0x00;
    R_IWDT->IWDTRR = 0xFF;
}

/*-----------------------------------------------------------------------------------------------------
  Процедура старта обновления вотчдога

  timer_ptr                         Pointer to timer control block
  name_ptr                          Pointer to timer name
  expiration_function               Application expiration function
  expiration_input                  Application expiration function input - аргумент передаваемый в функцию expiration_function
  initial_ticks                     Initial expiration ticks
  reschedule_ticks                  Reschedule ticks
  auto_activate                     Automatic activation flag

-----------------------------------------------------------------------------------------------------*/
void Watchdog_refreshing_start(void)
{
  tx_timer_create(&wdt_timer,"wdt_refresh",IWDT_refresh, 0, 1 ,ms_to_ticks(2000), TX_AUTO_ACTIVATE);
}





