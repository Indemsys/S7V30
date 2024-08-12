#include "App.h"
#include "BLE_main.h"


/******************************************************************************
 *                                Constants
 ******************************************************************************/
#define HCI_VSC_WRITE_SLEEP_MODE             (0xFC27)
#define HCI_VSC_WRITE_SLEEP_MODE_LENGTH      (12)

#define BT_SLEEP_MODE_UART                   (1)
#define BT_SLEEP_THRESHOLD_HOST              (1)
#define BT_SLEEP_THRESHOLD_HOST_CONTROLLER   (1)
#define BT_SLEEP_ALLOW_HOST_SLEEP_DURING_SCO (1)
#define BT_SLEEP_COMBINE_SLEEP_MODE_AND_LPM  (1)
#define BT_SLEEP_ENABLE_UART_TXD_TRISTATE    (0)
#define BT_SLEEP_PULSED_HOST_WAKE            (0)
#define BT_SLEEP_SLEEP_GUARD_TIME            (0)
#define BT_SLEEP_WAKEUP_GUARD_TIME           (0)
#define BT_SLEEP_TXD_CONFIG                  (1)
#define BT_SLEEP_BT_WAKE_IDLE_TIME           (50)


/*****************************************************************************
 *                           Type Definitions
 *****************************************************************************/
typedef struct
{
    wiced_bt_management_cback_t   *p_app_management_callback;
    bool                          is_sleep_mode_enabled;
} cybt_platform_main_cb_t;

cybt_platform_main_cb_t cybt_main_cb = {0};

extern uint32_t host_stack_platform_interface_init(void);

/*-----------------------------------------------------------------------------------------------------


  \param p_command_complete_params
-----------------------------------------------------------------------------------------------------*/
void bt_sleep_status_cback(wiced_bt_dev_vendor_specific_command_complete_params_t *p_command_complete_params)
{
  MAIN_TRACE_DEBUG("bt_sleep_status_cback(): status = 0x%x",p_command_complete_params->p_param_buf[0]);

  if (HCI_SUCCESS == p_command_complete_params->p_param_buf[0])
  {
    cybt_main_cb.is_sleep_mode_enabled = true;
  }
  else
  {
    cybt_main_cb.is_sleep_mode_enabled = false;
  }
}

/*-----------------------------------------------------------------------------------------------------



  \return bool
-----------------------------------------------------------------------------------------------------*/
bool bt_enable_sleep_mode(void)
{
  wiced_result_t result;
  uint8_t        sleep_vsc[HCI_VSC_WRITE_SLEEP_MODE_LENGTH];

  MAIN_TRACE_DEBUG("bt_enable_sleep_mode()");

  sleep_vsc[0] = BT_SLEEP_MODE_UART;
  sleep_vsc[1] = BT_SLEEP_THRESHOLD_HOST;
  sleep_vsc[2] = BT_SLEEP_THRESHOLD_HOST_CONTROLLER;
  sleep_vsc[3] = 0; // device_wake_polarity
  sleep_vsc[4] = 0; // host_wake_polarity
  sleep_vsc[5] = BT_SLEEP_ALLOW_HOST_SLEEP_DURING_SCO;
  sleep_vsc[6] = BT_SLEEP_COMBINE_SLEEP_MODE_AND_LPM;
  sleep_vsc[7] = BT_SLEEP_ENABLE_UART_TXD_TRISTATE;
  sleep_vsc[8] = 0;
  sleep_vsc[9] = 0;
  sleep_vsc[10] = 0;
  sleep_vsc[11] = BT_SLEEP_PULSED_HOST_WAKE;

  result = wiced_bt_dev_vendor_specific_command(HCI_VSC_WRITE_SLEEP_MODE,HCI_VSC_WRITE_SLEEP_MODE_LENGTH,sleep_vsc,bt_sleep_status_cback);
  if (WICED_BT_PENDING != result)
  {
    MAIN_TRACE_DEBUG("bt_enable_sleep_mode(): Fail to send vsc (0x%x)", result);
    return false;
  }

  return true;
}

/*-----------------------------------------------------------------------------------------------------
  On stack initalization complete this call back gets called

-----------------------------------------------------------------------------------------------------*/
void wiced_post_stack_init_cback(void)
{
  wiced_bt_management_evt_data_t event_data;
  //cybt_controller_sleep_config_t *p_sleep_config = &(((cybt_platform_config_t *)cybt_main_cb.p_bt_platform_cfg)->controller_config.sleep_mode);

  MAIN_TRACE_DEBUG("wiced_post_stack_init_cback");

  memset(&event_data, 0, sizeof(wiced_bt_management_evt_t));
  event_data.enabled.status = WICED_BT_SUCCESS;

  if (cybt_main_cb.p_app_management_callback)
  {
    cybt_main_cb.p_app_management_callback(BTM_ENABLED_EVT,&event_data);
  }

//  if ((p_sleep_config->sleep_mode_enabled == CYBT_SLEEP_MODE_ENABLED) && (p_sleep_config->device_wakeup_pin != 0) && (p_sleep_config->host_wakeup_pin != 0))
//  {
//    bool status = bt_enable_sleep_mode();
//
//    if (false == status)
//    {
//      MAIN_TRACE_ERROR("wiced_post_stack_init_cback(): Fail to init sleep mode");
//    }
//  }
//  else
//  {
//    MAIN_TRACE_ERROR("wiced_post_stack_init_cback(): BT sleep mode is NOT enabled");
//  }
}

/*-----------------------------------------------------------------------------------------------------
  This call back gets called for each HCI event.

  \param p_event

  \return wiced_bool_t
-----------------------------------------------------------------------------------------------------*/
wiced_bool_t wiced_stack_event_handler_cback(uint8_t *p_event)
{
  return WICED_FALSE;
}

/*-----------------------------------------------------------------------------------------------------


  \param event
  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
wiced_result_t cybt_core_management_cback(wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data)
{
  wiced_result_t result = WICED_BT_SUCCESS;

  if (cybt_main_cb.p_app_management_callback)
  {
    result = cybt_main_cb.p_app_management_callback(event, p_event_data);
  }
  return result;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается на старте задачи cybt_hci_rx_task
  Устанавливает callback функции библиотеки Infineon

  \param void
-----------------------------------------------------------------------------------------------------*/
void cybt_core_stack_init(void)
{
  wiced_bt_stack_init_internal(cybt_core_management_cback,  wiced_post_stack_init_cback,  wiced_stack_event_handler_cback);
}

/*-----------------------------------------------------------------------------------------------------



  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
wiced_result_t wiced_bt_stack_deinit(void)
{
  extern cy_thread_t cybt_task[BT_TASK_NUM];
  cy_thread_t cur_thread_handle;

  if (CY_RSLT_SUCCESS != cy_rtos_get_thread_handle(&cur_thread_handle))
  {
    MAIN_TRACE_ERROR("wiced_bt_stack_deinit(): Fail to deinit stack.");
    return WICED_BT_ERROR;
  }

  if ((cur_thread_handle == cybt_task[BT_TASK_ID_HCI_RX]) || (cur_thread_handle == cybt_task[BT_TASK_ID_HCI_TX]))
  {
    /** This API is used to terminate all BT-own tasks and release all related resources.
     *  According to the current design, the OS task cannot be deleted by itself.
        In other words, the application must call wiced_stack_deinit( ) in the task context other than BT-own ones.
     */
    MAIN_TRACE_ERROR("Please call this API- wiced_bt_stack_deinit() in application thread.");
    return WICED_BT_ERROR;
  }
  else
  {
    cybt_platform_task_deinit();

    cybt_platform_deinit();

    cybt_platform_terminate_hci_rx_thread();

    cybt_platform_terminate_hci_tx_thread();

  }

  return WICED_BT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return bool
-----------------------------------------------------------------------------------------------------*/
bool cybt_platform_get_sleep_mode_status(void)
{
  return cybt_main_cb.is_sleep_mode_enabled;
}

/*-----------------------------------------------------------------------------------------------------
  Эту функцию вызываем при старте стека

  \param p_bt_management_cback  - Эту функцию надо реализовать
  \param p_bt_cfg_settings      - Эти настройки надо создать

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
wiced_result_t wiced_bt_stack_init(wiced_bt_management_cback_t *p_bt_management_cback, const wiced_bt_cfg_settings_t *p_bt_cfg_settings)
{
  uint32_t       res;

#ifdef ENABLE_BLE_LOG  
  static cybt_result_t  cres;
#endif  
  

  MAIN_TRACE_DEBUG("wiced_bt_stack_init()");

  cybt_main_cb.is_sleep_mode_enabled       = false;
  cybt_main_cb.p_app_management_callback   = p_bt_management_cback;


  cybt_platform_init();                                     // Создаем таймер посылающий события в задачу

  res = host_stack_platform_interface_init();               // Устанавливаем callback функции для библиотеки Infineon
  BLELOG("BLE host stack platform interface init result = %d.", res);


  res = wiced_bt_set_stack_config(p_bt_cfg_settings);       // Внутренняя функция библиотеки Infineon. Передаем в нее все параметры приложения Bluetooth
  if (res == 0)
  {
    BLELOG("BLE host stack config error");
    return WICED_BT_ERROR;
  }
  else
  {
    BLELOG("BLE host stack requared memory size = %d bytes", res);
  }


#ifdef ENABLE_BLE_LOG  
  cres = cybt_platform_task_init((void *)p_bt_cfg_settings); // Создаем очереди и задачи. Аргумент p_bt_cfg_settings не используется
  BLELOG("BLE host stack task init result = %s (%d).",get_bt_stack_err_name(cres), cres);
#else
  cybt_platform_task_init((void *)p_bt_cfg_settings);
#endif  
  
  return WICED_BT_SUCCESS;
}


