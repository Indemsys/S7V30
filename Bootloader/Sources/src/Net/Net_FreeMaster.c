// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-08-01
// 15:02:54
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define THREAD_BSD_STACK_SIZE (1024*2)
static uint8_t thread_bsd_stack[THREAD_BSD_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.bsd_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);

static uint8_t   g_BSD_initialised;

/*-----------------------------------------------------------------------------------------------------


  \param ip_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_FreeMaster_server_create(NX_IP  *ip_ptr)
{
  uint32_t res;
  if (ivar.en_freemaster)
  {
    if (g_BSD_initialised == 0)
    {
      // Задача BSD нужна для организации связи движка FreeMaster через сеть
      res = bsd_initialize(ip_ptr,&net_packet_pool, (CHAR *)thread_bsd_stack, THREAD_BSD_STACK_SIZE, THREAD_BSD_PRIORITY);
      if (res == TX_SUCCESS)
      {
        NETLOG("BSD initialised");
        g_BSD_initialised = 1;
      }
      else
      {
        NETLOG("BSD initialisation error %d", res);
        return RES_ERROR;
      }
    }
  }
  return RES_OK;
}
