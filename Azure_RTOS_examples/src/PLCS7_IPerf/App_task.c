#include "App.h"


extern uint8_t         usb_1_interface_type;
extern uint8_t         usb_2_interface_type;

uint32_t               g_main_task_max_duration_us;
uint32_t               g_main_task_max_period_us;

extern void            nx_iperf_entry(NX_PACKET_POOL *pool_ptr, NX_IP *ip_ptr, UCHAR *http_stack, ULONG http_stack_size, UCHAR *iperf_stack, ULONG iperf_stack_size);

#define MAIN_TASK_STACK_SIZE 2048
static uint8_t main_stacks[MAIN_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.main_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       main_thread;


static void     Main_task(ULONG arg);
static uint32_t Create_Main_task(void);


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_NV_parameters_instance*
-----------------------------------------------------------------------------------------------------*/
const T_NV_parameters_instance* Get_app_params_instance(void)
{
  return &wvar_inst;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return char*
-----------------------------------------------------------------------------------------------------*/
char* Get_product_name(void)
{
  return (char *)wvar.name;
}
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void App_function(void)
{
  Restore_settings(&wvar_inst, APPLICATION_PARAMS);
  Init_DAC();

  strcpy((char *)ivar.software_version, SOFTWARE_VERSION);
  strcpy((char *)ivar.hardware_version, HARDWARE_VERSION);
  APPLOG("Product  name   : %s", wvar.name);
  APPLOG("Software version: %s", ivar.software_version);
  APPLOG("Hardware version: %s", ivar.hardware_version);

  Create_Main_task();

  while (1)
  {
    IWDT_refresh(0);
    Wait_ms(50);
    BLUE_LED  = 1;
    IWDT_refresh(0);
    Wait_ms(50);
    BLUE_LED  = 0;
  }
}


#define IPERF_TASK_STACK_SIZE (1024*2)
static uint8_t iperf_stack[IPERF_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.iperf_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);

#define HTTP_TASK_STACK_SIZE (1024*2)
static uint8_t http_stack[HTTP_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.http_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);

/*-----------------------------------------------------------------------------------------------------

  Выполнение теста IPerf
  Результаты при включенной опции максимальной оптимизации по размеру
  Пересылка на сервер по TCP выполнялась на скорости около 40 Мбит в сек
  Пересылка на сервер по UDP выполнялась на скорости около 83 Мбит в сек

  \param arg
-----------------------------------------------------------------------------------------------------*/
static void Main_task(ULONG arg)
{
  char ip_addr_str[32];
  char ip_mask_str[32];
  uint32_t flags;

  ENC_LED_RD = 1;

  if (wvar.en_iperf)
  {
    NX_IP *ip;


    APPLOG("Devices '%s' main task started", wvar.name);

    // Ждем инициализации сетевого стека
    flags = Wait_Net_task_event(EVT_NET_IP_READY, 100000);

    if (flags ==  EVT_NET_IP_READY)
    {
      ip = Get_net_ip();
      if (ip != NULL)
      {
        ULONG ip_address;
        ULONG network_mask;
        nx_ip_address_get(ip,&ip_address,&network_mask);
        snprintf(ip_addr_str, 31, "%03d.%03d.%03d.%03d", IPADDR(ip_address));
        snprintf(ip_mask_str, 31, "%03d.%03d.%03d.%03d", IPADDR(network_mask));

        APPLOG("Iperf task . IP address: %s , mask: %s", ip_addr_str, ip_mask_str);
        nx_iperf_entry(&net_packet_pool, ip, (UCHAR *)http_stack, HTTP_TASK_STACK_SIZE, (UCHAR *)iperf_stack, IPERF_TASK_STACK_SIZE);
      }
      else
      {
        APPLOG("Iperf task fail. IP address not received");
      }
    }
    else
    {
      APPLOG("Iperf task fail. No signal from Net stack");
    }
  }
  else
  {
    APPLOG("Devices '%s' main task skipped", wvar.name);
  }
  do
  {
    Wait_ms(10);

  } while (1);

}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Create_Main_task(void)
{
  UINT              err;

  err = tx_thread_create(
                         &main_thread,
                         (CHAR *)"Main",
                         Main_task,
                         0,
                         main_stacks,
                         MAIN_TASK_STACK_SIZE,
                         MAIN_TASK_PRIO,
                         MAIN_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  if (err == TX_SUCCESS)
  {
    APPLOG("Main task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("Main creating error %d.", err);
    return RES_ERROR;
  }
}

