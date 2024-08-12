// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.07.11
// 23:40:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BLE_main.h"

TX_EVENT_FLAGS_GROUP   app_flags;

char                   g_cpu_id_str[CPU_ID_STR_LEN];
uint8_t                g_cpu_id[CPU_ID_LEN];

extern ssp_err_t       sce_initialize(void);
extern const sf_crypto_instance_t g_sf_crypto;

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
  return tx_event_flags_get(&app_flags, 0xFFFFFFFF, TX_AND_CLEAR,&flags, TX_NO_WAIT);
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
void Public_names_modifier(void)
{
  char str[17];

  // Имя хоста не изменяем чтобы всегда работал URL по константному имени хоста
  // memset(str, 0, sizeof(str));
  // strncpy(str, (char*)ivar.this_host_name, 5); // От оригинального имени из параметров берем только первые 5 симвлов
  // strncat(str, g_cpu_id_str, 8); // Добавляем 8 первых символов от уникального номера чипа
  // strcpy((char*)ivar.this_host_name, str);

  memset(str, 0, sizeof(str));
  strncpy(str, (char*)ivar.wifi_ap_ssid, 5);
  strncat(str, g_cpu_id_str, 8);
  strcpy((char*)ivar.wifi_ap_ssid, str);


}
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Main_thread(ULONG arg)
{
  IWDT_refreshing_start();
  ELS_soft_event_set_and_enable_interrupts();

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

  SCI8_SPI_init();                      // SPI8 используется для управления часами реального времени и чипами ЦАП
  Init_RTC();
  sce_initialize();                     // Инициализация движка шифрования и генерации случайных чисел. Применяется неявно в DNS
  g_sf_crypto.p_api->open(g_sf_crypto.p_ctrl, g_sf_crypto.p_cfg);
  Flash_driver_bgo_init();


  if  (Init_SD_card_file_system() == RES_OK) g_file_system_ready = 1;

  Restore_settings(&ivar_inst, BOOTL_PARAMS);
  Public_names_modifier();                     // Модификация имени хоста, чтобы все имена были уникальными

  Generate_CRC32_table();                   // Таблица CRC32 нужна для загрузчика образа прошивки
  VT100_task_manager_initialization();      // Инициализируем движок VT100 терминала до того как каналы коммуникации cмогут создать задачи VT100

  // Фиксируем время завершения старта основного цикла
  Get_hw_timestump(&g_main_thread_start_timestump);

  if (Check_boot_MassStorage_mode() == RES_ERROR)
  {
    // Переходим в загрузчик и старт приложения если не нажата кнопока 1
    if (BT2_STATE == 1)
    {
      if (g_file_system_ready == 1)
      {
        Load_and_Flash_Image_File();
      }
      if (ivar.disable_jump_to_applacation == 0)
      {
        if (Integrity_check_App_firmware() == RES_OK)
        {
          Restart_to_App_firmware(); // Вызываем рестарт модуля если основное приложение есть в наличии
        }
      }
    }
  }

  WIFi_control_init();
  Charger_thread_create();


  Init_app_logger();
  System_start_report();
  Auto_protection();

  Create_Backgroung_task();
  Accept_certificates_from_file();

  if (Check_boot_MassStorage_mode() == RES_OK)
  {
    ivar.usb_mode = USB_MODE_MASS_STORAGE_;
  }
  Clear_boot_MassStorage_mode();


  Init_USB_stack();

  Create_File_Logger_task();
  Thread_Net_create();
  BLE_init();

  // Выключаем светодиоды
  RED_LED   = 1;
  GREEN_LED = 1;
  BLUE_LED  = 1;

  do
  {
    GREEN_LED  = 0;
    Wait_ms(2);
    GREEN_LED  = 1;
    Wait_ms(58);

//    if (WIFI_AP_network_active_flag())
//    {
//      BLUE_LED  = 0;
//    }
//    else if (WIFI_STA_network_active_flag())
//    {
//      BLUE_LED  = 0;
//    }
//    else
//    {
//      BLUE_LED  = 1;
//    }

    // Периодически проверяем необходимость удаления отработавшей задачи инициализации стека BLE
    if (BLE_init_task_state() == BLE_INIT_TASK_FINISHED)
    {
      BLE_init_delete();
    }
  }while (1);

}

