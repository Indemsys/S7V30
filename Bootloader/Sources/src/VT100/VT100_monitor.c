#include "App.h"


TX_MUTEX           vt100_mutex;
T_VT100_task_cbl   vt100_task_cbls[VT100_TASKS_MAX_NUM];

#define VT100_MANAGER_TASK_STACK_SIZE (1024*2)
static uint8_t background_stacks[VT100_MANAGER_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.vt100_manager")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       vt100_manager_thread;


T_vt100_man_msg        vt100_man_queue_buf[VT100_MAG_QUEUE_BUF_LEN];
TX_QUEUE               vt100_man_queue;


/*-----------------------------------------------------------------------------------------------------


  \param msg_ptr
-----------------------------------------------------------------------------------------------------*/
void Send_message_to_VT100_task_manager(T_vt100_man_msg *msg_ptr)
{
  tx_queue_send(&vt100_man_queue,msg_ptr,TX_NO_WAIT);
}


/*-----------------------------------------------------------------------------------------------------
  Задача менеджера получает сообщения о создании и удалении задач VT100 вмемте с задачами драйверов

  \param arg
-----------------------------------------------------------------------------------------------------*/
static void VT100_task_manager_thread(ULONG arg)
{
  T_vt100_man_msg        msg;
  T_vt100_man_callback   func;

  while (tx_queue_receive(&vt100_man_queue,&msg, TX_WAIT_FOREVER) == TX_SUCCESS)
  {
    func = (T_vt100_man_callback)msg.arg1;
    func(&msg);
  }
}


/*-----------------------------------------------------------------------------------------------------
  Иницализация работы монитора VT100

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t VT100_task_manager_initialization(void)
{
  uint32_t res;
  res = tx_mutex_create(&vt100_mutex, "vt100_mutex", TX_INHERIT);
  if (res != TX_SUCCESS) goto err_exit;


  res = tx_queue_create(&vt100_man_queue, (CHAR *) "VT100_manager", sizeof(T_vt100_man_msg) / sizeof(uint32_t), vt100_man_queue_buf, sizeof(T_vt100_man_msg) * VT100_MAG_QUEUE_BUF_LEN);
  if (res != TX_SUCCESS) goto err_exit;


  res = tx_thread_create(
       &vt100_manager_thread,
       (CHAR *)"VT100 manager",
       VT100_task_manager_thread,
       0,
       background_stacks,
       VT100_MANAGER_TASK_STACK_SIZE,
       VT100_MANAGER_TASK_PRIO,
       VT100_MANAGER_TASK_PRIO,
       1,
       TX_AUTO_START
       );
  if (res == TX_SUCCESS)
  {
    APPLOG("VT100 manager task created");
  }
  else
  {
    APPLOG("VT100 manager task creation error: %04X", res);
    goto err_exit;
  }

  return RES_OK;

err_exit:
  tx_mutex_delete(&vt100_mutex);
  tx_queue_delete(&vt100_man_queue);
  tx_thread_delete(&vt100_manager_thread);
  APPLOG("VT100 manager creating error %d.", res);
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param ip_ptr
-----------------------------------------------------------------------------------------------------*/
static void _Print_IP_MAC(NX_INTERFACE *ip_interface, whd_mac_t *rem_mac)
{
  GET_MCBL;
  char        str[49];

  if (Get_IP_addr_str(ip_interface, str, 48) == RES_OK)
  {
    MPRINTF("IP: %s ",str);
    if (Get_IP_addr_MAC_str(ip_interface, str, 48) == RES_OK)
    {
      MPRINTF("MAC: %s ",str);
      if (rem_mac != NULL)
      {
        MPRINTF(" Remote MAC: %02X:%02X:%02X:%02X:%02X:%02X",rem_mac->octet[0],rem_mac->octet[1],rem_mac->octet[2],rem_mac->octet[3],rem_mac->octet[4],rem_mac->octet[5]);
      }
    }
  }
}
/*-------------------------------------------------------------------------------------------------------------
   Задача монитора

   В initial_data содержится индекс задаче в массиве управляющих структур задач VT100
-------------------------------------------------------------------------------------------------------------*/
static void Task_VT100(ULONG arg)
{
  uint8_t    b;

  // Передаем в структуру задачи данные указатели на структуры монитора и драйвера
  T_monitor_cbl   *monitor_cbl = (T_monitor_cbl *)(arg);

  tx_thread_identify()->environment =  (ULONG)(monitor_cbl);
  tx_thread_identify()->driver      =  (ULONG)(monitor_cbl->pdrv);

  GET_MCBL;

  fx_directory_default_set(&fat_fs_media, "/");

  do
  {
    MPRINTF(VT100_CLEAR_AND_HOME);
  } while (Access_control() != RES_OK);

  // Очищаем экран
  MPRINTF(VT100_CLEAR_AND_HOME);
  Goto_main_menu();

  do
  {
    if (WAIT_CHAR(&b, ms_to_ticks(1000)) == RES_OK)
    {
      if (b != 0)
      {
        if ((b == 0x1B) && (mcbl->Monitor_func != Edit_func))
        {
          MPRINTF(VT100_CLEAR_AND_HOME);
          //Entry_check();
          Goto_main_menu();
        }
        else
        {
          if (mcbl->Monitor_func) mcbl->Monitor_func(b);  // Обработчик нажатий главного цикла
        }
      }
    }


    if (mcbl->menu_trace[mcbl->menu_nesting] == &MENU_MAIN)
    {

      rtc_time_t  curr_time;

      VT100_set_cursor_pos(19, 0);
      MPRINTF(VT100_CLR_LINE"Software     : %s\r\n", ivar.software_version);
      MPRINTF(VT100_CLR_LINE"Hardware     : %s\r\n", ivar.hardware_version);
      MPRINTF(VT100_CLR_LINE"Chip UID     : %s\r\n", g_cpu_id_str);
      MPRINTF(VT100_CLR_LINE"Compile time : %s %s\r\n",Get_build_date(),Get_build_time());
      MPRINTF(VT100_CLR_LINE"Up time      : %d s. CPU usage %d%% \r\n",_tx_time_get() / TX_TIMER_TICKS_PER_SECOND, g_aver_cpu_usage / 10);

      rtc_cbl.p_api->calendarTimeGet(rtc_cbl.p_ctrl,&curr_time);
      curr_time.tm_mon++;
      MPRINTF(VT100_CLR_LINE"Date Time    : %04d.%02d.%02d  %02d:%02d:%02d\r\n", curr_time.tm_year + 1900, curr_time.tm_mon, curr_time.tm_mday, curr_time.tm_hour, curr_time.tm_min, curr_time.tm_sec);

      {
        uint32_t avail_bytes;
        uint32_t fragments;
        App_get_RAM_pool_statistic(&avail_bytes,&fragments);
        MPRINTF(VT100_CLR_LINE"Chip RAM free: %d bytes\r\n", avail_bytes);
        App_get_SDRAM_pool_statistic(&avail_bytes,&fragments);
        MPRINTF(VT100_CLR_LINE"SDRAM free   : %d bytes\r\n", avail_bytes);
      }

      {
        uint8_t       net_active = 0;
        NX_INTERFACE *ip_interface;
        ULONG         ip_addr;

        MPRINTF("WiFi AP  Link: ");
        ip_interface = WIFI_AP_link_state();
        if (ip_interface != NULL)
        {
          net_active = 1;
          _Print_IP_MAC(ip_interface, WIFI_AP_get_remote_sta_addr());
        }
        else
        {
          MPRINTF("Down");
        }
        MPRINTF(VT100_CLL_FM_CRSR"\r\n");

        MPRINTF("WiFi STA Link: ");
        ip_interface = WIFI_STA_link_state();
        if (ip_interface != NULL)
        {
          net_active = 1;
          _Print_IP_MAC(ip_interface, WIFI_STA_get_remote_ap_addr());
          MPRINTF(" SSID= %s",WIFI_STA_get_current_SSID());

        }
        else
        {
          MPRINTF("Down");
        }
        MPRINTF(VT100_CLL_FM_CRSR"\r\n");

        if (ivar.usb_mode == USB_MODE_RNDIS)
        {
          MPRINTF("RNDIS    Link: ");
          ip_interface = RNDIS_link_state();
          if (ip_interface != NULL)
          {
            net_active = 1;
            _Print_IP_MAC(ip_interface, NULL);
          }
          else
          {
            MPRINTF("Down");
          }
          MPRINTF(VT100_CLL_FM_CRSR"\r\n");
        }

        if (ivar.usb_mode == USB_MODE_HOST_ECM)
        {
          MPRINTF("ECM host Link: ");
          ip_interface = ECM_host_link_state();
          if (ip_interface != NULL)
          {
            net_active = 1;
            _Print_IP_MAC(ip_interface, NULL);
          }
          else
          {
            MPRINTF("Down");
          }
          MPRINTF(VT100_CLL_FM_CRSR"\r\n");
        }

        if (net_active)
        {
          nx_ip_gateway_address_get(Net_get_ip_ptr(),&ip_addr);
          MPRINTF("Gateway addr.: %03d.%03d.%03d.%03d", IPADDR(ip_addr));
          MPRINTF(VT100_CLL_FM_CRSR"\r\n");
        }
      }

    }


  }while (1);

}

/*-----------------------------------------------------------------------------------------------------
  Создание задачи VT100.
  Можем создать не более VT100_TASKS_MAX_NUM задач
  Задача не стартует автоматически. Ее нужно запустить функцией Task_VT100_start

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Task_VT100_create(T_serial_io_driver *serial_drv_ptr, int32_t *task_instance_index_ptr)
{
  UINT                 res;
  int32_t              instance_indx = -1;

  if ((task_instance_index_ptr == 0) || (serial_drv_ptr == 0)) return RES_ERROR;

  if (tx_mutex_get(&vt100_mutex, MS_TO_TICKS(1000)) != TX_SUCCESS) return RES_ERROR;

  // Ищем свободную запись для задачи
  for (uint32_t i = 0; i < VT100_TASKS_MAX_NUM; i++)
  {
    if (vt100_task_cbls[i].taken == 0)
    {
      instance_indx = i;
      vt100_task_cbls[instance_indx].taken = 1;


      // Выделяем память для структуры монитора
      vt100_task_cbls[instance_indx].monitor_cbl_ptr = App_malloc(sizeof(T_monitor_cbl));
      if (vt100_task_cbls[instance_indx].monitor_cbl_ptr == NULL) break;

      // Выделяем память для структуры задачи
      vt100_task_cbls[instance_indx].VT100_thread_prt = App_malloc(sizeof(TX_THREAD));
      if (vt100_task_cbls[instance_indx].VT100_thread_prt == NULL) break;

      // Выделяем память для стека задачи
      vt100_task_cbls[instance_indx].VT100_thread_stack = App_malloc(VT100_TASK_STACK_SIZE);
      if (vt100_task_cbls[instance_indx].VT100_thread_stack == NULL) break;


      vt100_task_cbls[instance_indx].monitor_cbl_ptr->pdrv       = serial_drv_ptr;
      snprintf(vt100_task_cbls[instance_indx].task_name,VT100_TASK_NAME_MAX_SZ,"VT100_%d",i);

      break;
    }
  }
  if (instance_indx == -1) goto err_exit;

  // Инициализируем драйвер
  if (serial_drv_ptr->_init(&serial_drv_ptr->drv_cbl_ptr, serial_drv_ptr) != RES_OK)
  {
    vt100_task_cbls[instance_indx].taken = 0;
    goto err_exit;
  }

  res = tx_thread_create(
       vt100_task_cbls[instance_indx].VT100_thread_prt,
       (CHAR *)vt100_task_cbls[instance_indx].task_name,
       Task_VT100,
       (ULONG)(vt100_task_cbls[instance_indx].monitor_cbl_ptr),
       vt100_task_cbls[instance_indx].VT100_thread_stack,
       VT100_TASK_STACK_SIZE,
       VT100_TASK_PRIO,
       VT100_TASK_PRIO,
       1,
       0
       );
  if (res == TX_SUCCESS)
  {
    APPLOG("VT100 task %d created", instance_indx);
  }
  else
  {
    APPLOG("VT100 task %d creation error: %04X",instance_indx, res);
    serial_drv_ptr->_deinit(&serial_drv_ptr->drv_cbl_ptr);
    goto err_exit;
  }

  *task_instance_index_ptr = instance_indx;
  tx_mutex_put(&vt100_mutex);
  return RES_OK;

err_exit:

  if (instance_indx != -1)
  {
    vt100_task_cbls[instance_indx].taken = 0;
    memset(vt100_task_cbls[instance_indx].task_name, 0, VT100_TASK_NAME_MAX_SZ);
    App_free(vt100_task_cbls[instance_indx].monitor_cbl_ptr);
    App_free(vt100_task_cbls[instance_indx].VT100_thread_prt);
    App_free(vt100_task_cbls[instance_indx].VT100_thread_stack);
    APPLOG("VT100 task %d creating error", instance_indx);
  }

  tx_mutex_put(&vt100_mutex);
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param instance_indx_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Task_VT100_start(int32_t instance_indx)
{
  UINT    res;

  if (instance_indx >= VT100_TASKS_MAX_NUM) return RES_ERROR;

  res = tx_thread_resume(vt100_task_cbls[instance_indx].VT100_thread_prt);
  if (res != TX_SUCCESS) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param instance_indx

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Task_VT100_suspend(int32_t instance_indx)
{
  UINT    res;

  if (instance_indx >= VT100_TASKS_MAX_NUM) return RES_ERROR;

  res = tx_thread_suspend(vt100_task_cbls[instance_indx].VT100_thread_prt);
  if (res != TX_SUCCESS) return RES_ERROR;
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------


  \param alloc_indx

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Task_VT100_delete(int32_t instance_indx)
{
  UINT    res;

  if (instance_indx >= VT100_TASKS_MAX_NUM) return RES_ERROR;

  if (tx_mutex_get(&vt100_mutex, MS_TO_TICKS(1000)) != TX_SUCCESS) return RES_ERROR;

  res = tx_thread_terminate(vt100_task_cbls[instance_indx].VT100_thread_prt);
  if (res == TX_SUCCESS)
  {
    res = tx_thread_delete(vt100_task_cbls[instance_indx].VT100_thread_prt);
    if (res == TX_SUCCESS)
    {
      APPLOG("VT100 task %d deleted", instance_indx);
    }
    else
    {
      APPLOG("VT100 task %d deleting error %d", instance_indx, res);
    }
  }
  else
  {
    APPLOG("VT100 task %d terminating error %d", instance_indx, res);
  }

  vt100_task_cbls[instance_indx].monitor_cbl_ptr->pdrv->_deinit(&(vt100_task_cbls[instance_indx].monitor_cbl_ptr->pdrv->drv_cbl_ptr));

  App_free(vt100_task_cbls[instance_indx].monitor_cbl_ptr);
  App_free(vt100_task_cbls[instance_indx].VT100_thread_prt);
  App_free(vt100_task_cbls[instance_indx].VT100_thread_stack);
  memset(vt100_task_cbls[instance_indx].task_name, 0, VT100_TASK_NAME_MAX_SZ);
  vt100_task_cbls[instance_indx].taken = 0;

  tx_mutex_put(&vt100_mutex);
  return RES_OK;
}


