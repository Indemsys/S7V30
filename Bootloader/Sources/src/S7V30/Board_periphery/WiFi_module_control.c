// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-06-16
// 19:17:38
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

static TX_MUTEX               wifi_ctl_mutex;
static volatile uint8_t       wifi_switched_on;
static volatile uint8_t       bluetooth_switched_on;
static TX_EVENT_FLAGS_GROUP   wifi_flag;

/*-----------------------------------------------------------------------------------------------------

  \param msg

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Send_wifi_event(uint32_t event_flag)
{
  return  tx_event_flags_set(&wifi_flag, event_flag, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param event_flags
  \param timeout_ms

  \return uint32_t Возвращает битовоую маску обнаруженнызсобытий
-----------------------------------------------------------------------------------------------------*/
uint32_t Wait_wifi_event(uint32_t event_flags, uint32_t *actual_flags, uint32_t timeout_ms)
{
  if (timeout_ms == TX_NO_WAIT)
  {
    return tx_event_flags_get(&wifi_flag, event_flags, TX_OR_CLEAR, (ULONG*)actual_flags, TX_NO_WAIT);
  }
  else
  {
    return tx_event_flags_get(&wifi_flag, event_flags, TX_OR_CLEAR, (ULONG*)actual_flags,  ms_to_ticks(timeout_ms));
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void WIFi_control_init(void)
{
  tx_event_flags_create(&wifi_flag, "wifi_ctl");
  tx_mutex_create(&wifi_ctl_mutex, "wifi_clt", TX_INHERIT);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void WiFi_module_switch_ON(void)
{
  if (tx_mutex_get(&wifi_ctl_mutex, MS_TO_TICKS(10000)) == TX_SUCCESS)
  {
    if (wifi_switched_on != 0)
    {
      tx_mutex_put(&wifi_ctl_mutex);
      return;
    }
    WIFI_REG_ON = 1; // Подача питания на VBAT и VDDIO
    Wait_ms(300);    // Задержка для WiFi части модуля
    wifi_switched_on = 1;
    tx_mutex_put(&wifi_ctl_mutex);
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void WiFi_module_switch_OFF(void)
{
  if (tx_mutex_get(&wifi_ctl_mutex, MS_TO_TICKS(10000)) == TX_SUCCESS)
  {
    WIFI_REG_ON = 0; // Снятие питания с VBAT и VDDIO
    Wait_ms(300);    // Задержка для WiFi части модуля
    wifi_switched_on = 0;
    tx_mutex_put(&wifi_ctl_mutex);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void    WiFi_module_switch_on_Bluetooth(void)
{
  if (bluetooth_switched_on != 0) return;
  if (wifi_switched_on == 0)
  {
    WiFi_module_switch_ON();
  }
  if (tx_mutex_get(&wifi_ctl_mutex, MS_TO_TICKS(10000)) == TX_SUCCESS)
  {
    if (bluetooth_switched_on != 0)
    {
      tx_mutex_put(&wifi_ctl_mutex);
      return;
    }
    BT_REG_ON = 1;    // Подача питания на BT_REG_ON
    Wait_ms(10);     // Задержка для Bluetooth части модуля
    bluetooth_switched_on = 1;
    tx_mutex_put(&wifi_ctl_mutex);
  }
}

