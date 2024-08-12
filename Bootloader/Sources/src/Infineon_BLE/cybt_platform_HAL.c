#include "App.h"
#include "BLE_main.h"

#define HCI_SEMAPHORE_MAX_COUNT  (1)
#define HCI_SEMAPHORE_INIT_COUNT (0)



TX_TIMER        bt_stack_timer;
uint32_t        uart_tx_done_cnt = 0;

/*-----------------------------------------------------------------------------------------------------


  \param callback_arg
  \param event
-----------------------------------------------------------------------------------------------------*/
static void platform_stack_timer_cback(ULONG v)
{
  cybt_send_flags_to_hci_rx_task(FLAG_BLE_TIMER_EXPIRED);
  tx_timer_activate(&bt_stack_timer);
}

/*-----------------------------------------------------------------------------------------------------


  \param req_size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* cybt_platform_malloc(uint32_t req_size)
{
  return App_malloc((size_t) req_size);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_mem_block
-----------------------------------------------------------------------------------------------------*/
void cybt_platform_free(void *p_mem_block)
{
  App_free(p_mem_block);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void cybt_platform_init(void)
{
  tx_timer_create(&bt_stack_timer,                 // timer_ptr                         Pointer to timer control block
                  "bt_stack",                      // name_ptr                          Pointer to timer name
                  platform_stack_timer_cback,      // expiration_function               Application expiration function
                  0,                               // expiration_input                  Параметр передаваемый в функцию callback
                  ms_to_ticks(250),                // initial_ticks                     Initial expiration ticks
                  ms_to_ticks(250),                // reschedule_ticks                  Reschedule ticks
                  TX_NO_ACTIVATE);                 // auto_activate                     Automatic activation flag
  tx_timer_activate(&bt_stack_timer);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void cybt_platform_deinit(void)
{
  MAIN_TRACE_DEBUG("cybt_platform_deinit()");
  tx_timer_deactivate(&bt_stack_timer);

}

/*-----------------------------------------------------------------------------------------------------



  \return uint64_t
-----------------------------------------------------------------------------------------------------*/
uint64_t cybt_platform_get_tick_count_us(void)
{
  uint64_t   us;
  T_sys_timestump ts;
  Get_hw_timestump(&ts);

  us = ts.ticks * (1000000ul / TX_TIMER_TICKS_PER_SECOND);
  us += (ts.cycles * 1000000ul) / ICLK_FREQ;
  return us;
}

/*-----------------------------------------------------------------------------------------------------
  This function is used by BT stack to set next timeout in absolute tick count in micro-second.

  \param abs_tick_us_to_expire
-----------------------------------------------------------------------------------------------------*/
void cybt_platform_set_next_timeout(uint64_t abs_tick_us_to_expire)
{
  uint64_t curr_time_in_us = cybt_platform_get_tick_count_us();
  uint64_t time_to_expire_in_us = abs_tick_us_to_expire - curr_time_in_us;
  if (abs_tick_us_to_expire <= curr_time_in_us)
  {
    // Already expired...
    cybt_send_flags_to_hci_rx_task(FLAG_BLE_TIMER_EXPIRED);

    return;
  }
  tx_timer_change(&bt_stack_timer, ms_to_ticks(time_to_expire_in_us / 1000),  ms_to_ticks(250));
  tx_timer_activate(&bt_stack_timer);
}




/*-----------------------------------------------------------------------------------------------------
  Здесь выставляем сигнал пробуждения BLE модуля
  Вызывается при каждом обращении к модулю.

-----------------------------------------------------------------------------------------------------*/
void cybt_platform_assert_bt_wake(void)
{
  //  if (true == cybt_platform_get_sleep_mode_status())
  //  {
  //    bool wake_polarity;
  //    const cybt_platform_config_t *p_bt_platform_cfg = cybt_platform_get_config();
  //
  //    switch (p_bt_platform_cfg->controller_config.sleep_mode.device_wake_polarity)
  //    {
  //    case CYBT_WAKE_ACTIVE_LOW:
  //      wake_polarity = false;
  //      break;
  //    case CYBT_WAKE_ACTIVE_HIGH:
  //      wake_polarity = true;
  //      break;
  //    default:
  //      HCIDRV_TRACE_ERROR("ASSERT_BT_WAKE: unknown polarity (%d)",p_bt_platform_cfg->controller_config.sleep_mode.device_wake_polarity);
  //      return;
  //    }
  //
  //    cyhal_gpio_write(p_bt_platform_cfg->controller_config.sleep_mode.device_wakeup_pin,wake_polarity);
  //  }
}

/*-----------------------------------------------------------------------------------------------------
  Здесь снимается сигнал пробуждения BLE модуля
  Вызывается после каждого обращения к модулю.

-----------------------------------------------------------------------------------------------------*/
void cybt_platform_deassert_bt_wake(void)
{
}

/*-----------------------------------------------------------------------------------------------------


  \param baudrate

  \return cybt_result_t
-----------------------------------------------------------------------------------------------------*/
cybt_result_t cybt_platform_hci_set_baudrate(uint32_t baudrate)
{
  if (UART_BLE_set_baudrate(baudrate) == RES_OK) return  CYBT_SUCCESS;
  return CYBT_ERR_GENERIC;
}

/*-----------------------------------------------------------------------------------------------------


  \param type
  \param p_data
  \param length

  \return cybt_result_t
-----------------------------------------------------------------------------------------------------*/
cybt_result_t cybt_platform_hci_write(hci_packet_type_t type, uint8_t *p_data, uint32_t length)
{
  cybt_result_t return_status =  CYBT_SUCCESS;

  cybt_platform_assert_bt_wake();

  UART_BLE_send_packet(p_data, length);

  cybt_platform_deassert_bt_wake();

  return return_status;
}

/*-----------------------------------------------------------------------------------------------------
  Чтение заданного количества байт из UART
  Если timeout_ms то читается столько байт сколько есть в FIFO UART и происходит выход без ожидания

  \param type
  \param p_data
  \param p_length
  \param timeout_ms

  \return cybt_result_t
-----------------------------------------------------------------------------------------------------*/
cybt_result_t cybt_platform_hci_read(uint8_t  *p_data, uint32_t *p_length, uint32_t timeout_ms)
{
  if (UART_BLE_receive_packet(p_data, p_length, timeout_ms) == RES_OK)
  {
    return  CYBT_SUCCESS;
  }
  else
  {
    return  CYBT_ERR_HCI_READ_FAILED;
  }
}

/*-----------------------------------------------------------------------------------------------------



  \return cybt_result_t
-----------------------------------------------------------------------------------------------------*/
cybt_result_t cybt_platform_hci_close(void)
{
  // Закрыть драйвер UART, если нужно
  return  CYBT_SUCCESS;
}


