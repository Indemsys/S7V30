// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-01-16
// 13:15:35
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

T_ELS_soft_event_callback      ELS_soft_event_callback;

static IRQn_Type               elc_soft1_int_num;

SSP_VECTOR_DEFINE(elc_segr1_isr,  ELC,  SOFTWARE_EVENT_1)


/*-----------------------------------------------------------------------------------------------------
  Используем прерывание по событию ELC_EVENT_ELC_SOFTWARE_EVENT_1 для установки флага окончания пересылки по DTC
  Данное прерывание используется при организации циклического обмена данными в  внешними АЦП и расширителями ввода вывода

  Выбран имеено такой способ поскольку вызвать прерывания записью в регитсры NVIC не удалось.
  Пересылка DTC, по всей видимости, не может выполнить запись в область системных регистров ARM с адреса 0xE0000000

  \param void
-----------------------------------------------------------------------------------------------------*/
void  elc_segr1_isr(void)
{
  if (ELS_soft_event_callback != 0) ELS_soft_event_callback();

  R_ICU->IELSRn_b[elc_soft1_int_num].IR = 0;    // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param callback_func
-----------------------------------------------------------------------------------------------------*/
void Set_ELS_soft_event_callback(T_ELS_soft_event_callback func)
{
  ELS_soft_event_callback = func;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void ELS_soft_event_clear_interrupts(void)
{
  elc_soft1_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ELC_SOFTWARE_EVENT_1);
  NVIC_DisableIRQ(elc_soft1_int_num);
  NVIC_ClearPendingIRQ(elc_soft1_int_num);
  R_ICU->IELSRn_b[elc_soft1_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void ELS_soft_event_set_and_enable_interrupts(void)
{
  ELS_soft_event_clear_interrupts();

  NVIC_SetPriority(elc_soft1_int_num,  EXT_ADC_SCAN_PRIO);
  NVIC_EnableIRQ(elc_soft1_int_num);
}

