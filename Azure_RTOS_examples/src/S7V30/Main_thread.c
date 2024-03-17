// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.07.11
// 23:40:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"

extern void  App_function(void);

TX_EVENT_FLAGS_GROUP   app_flags;

char                   g_cpu_id_str[CPU_ID_STR_LEN];
uint8_t                g_cpu_id[CPU_ID_LEN];

extern ssp_err_t       sce_initialize(void);
/*-----------------------------------------------------------------------------------------------------


  \param eventmask
-----------------------------------------------------------------------------------------------------*/
void Set_app_event(uint32_t events_mask)
{
  tx_event_flags_set(&app_flags, events_mask, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t Wait_app_event(uint32_t events_mask, uint32_t opt, uint32_t wait)
{
  ULONG flags;
  return tx_event_flags_get(&app_flags, events_mask, opt,&flags, wait);
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Clear_app_event(void)
{
  ULONG flags;
  return tx_event_flags_get(&app_flags, 0xFFFFFFFF, TX_AND_CLEAR, &flags, TX_NO_WAIT);
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_NV_parameters_instance*
-----------------------------------------------------------------------------------------------------*/
const T_NV_parameters_instance* Get_mod_params_instance(void)
{
  return &ivar_inst;
}

/*-----------------------------------------------------------------------------------------------------


  \param events_mask
  \param p_flags
  \param opt
  \param wait

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_app_events(uint32_t events_mask, ULONG *p_flags,  uint32_t opt, uint32_t wait)
{
  return tx_event_flags_get(&app_flags, events_mask, opt, p_flags, wait);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Main_thread(ULONG arg)
{
  Watchdog_refreshing_start();

  // Разрешаем запись в регистры пинов, проскольку ранее она была запрещена драйвером
  R_PMISC->PWPR_b.BOWI  = 0;
  R_PMISC->PWPR_b.PFSWE = 1;

  RED_LED   = 1;
  GREEN_LED = 0; // Включаем только зеленый светодиод
  BLUE_LED  = 1;

  Get_reference_time();

  g_fmi.p_api->init();
  Get_CPU_UID(g_cpu_id_str, g_cpu_id, CPU_ID_STR_LEN);

  tx_event_flags_create(&app_flags, "APP");

  Init_RTC();
  sce_initialize();                     // Инициализация движка шифрования и генерации случайных чисел. Применяется неявно в DNS
  Flash_driver_init();

  if  (Init_SD_card_file_system() == RES_OK)
  {
    g_file_system_ready = 1;
  }
  else
  {
    g_file_system_ready = 0;
  }

  Restore_settings(&ivar_inst, MODULE_PARAMS);

  Determine_network_type();
  Determine_FreeMaster_interface_type();

  // Фиксируем время завершения старта основного цикла
  Get_hw_timestump(&g_main_thread_start_timestump);

  Init_app_logger();
  System_start_report();

  Create_Backgroung_task();

  Accept_certificates_from_file();

  Set_usb_mode();
  if (ivar.usb_mode != USB_MODE_NONE)
  {
    Init_USB_stack();
    if (Get_usb_1_mode() == USB1_INTF_VIRTUAL_COM_PORT)
    {
      Task_VT100_create((ULONG)Mnsdrv_get_usbfs_vcom0_driver(),0);
    }
  }

  if ((g_file_system_ready != 0) && (ivar.en_log_to_file) && (app_log_cbl.log_inited))
  {
    // Внимание! Задача записи лога может конфликтовать с режимом USB MassStorage
    Create_file_log_task();
    Wait_app_event(EVENT_LOGGER_TASK_READY,TX_OR, TX_WAIT_FOREVER);
  }

  Thread_Net_create();
  if (ivar.en_freemaster)
  {
    Thread_FreeMaster_create();
  }


  RED_LED   = 1;
  GREEN_LED = 1;
  BLUE_LED  = 0; // Включаем только синий светодиод


  App_function();
}

