// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-29
// 16:49:08
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "whd.h"
#include   "whd_events_int.h"
#include   "whd_wifi_api.h"
#include   "whd_resource_api.h"
#include   "whd_network_types.h"
#include   "whd_thread.h"
#include   "cyhal_hw_types.h"
#include   "cyabs_rtos.h"
#include   "cy_result.h"
#include   "cyhal_sdio.h"
#include   "resources.h"
#include   "clm_resources.h"
#include   "wifi_nvram_image.h"
#include   "ssp_common_api.h"
#include   "whd_bus_sdio_protocol.h"
#include   "WiFi_Host_Driver_Port.h"


extern  whd_driver_t         g_whd_driver;

//#define DEBUG_RTT_PRINT


#define    WHD_RTT_LOG_STR_SZ    (256)
static char                       whd_rtt_log_str[WHD_RTT_LOG_STR_SZ];


#ifdef DEBUG_RTT_PRINT
  #define WHD_RTT_PRINT(...)      WHD_RTT_LOGs(##__VA_ARGS__);
#else
  #define WHD_RTT_PRINT(...)
#endif

#define HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_BUS_HEADER_SIZE    (sizeof(whd_buffer_header_t) + BDC_HEADER_WITH_PAD + SDPCM_HEADER)
#define HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE          (HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_BUS_HEADER_SIZE + WHD_ETHERNET_SIZE)

static inline void CY_HALT(void)
{
  do
  {
    __asm("    bkpt    1");
  } while (0);
}
#define BLOCK_SIZE_ALIGNMENT                    (64)
/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param resource
  \param size_out

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_resource_size(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t *size_out)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    *size_out = sizeof(wifi_firmware_image_data);
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    *size_out = sizeof(wifi_nvram_image);
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    *size_out = sizeof(wifi_firmware_clm_blob_data);
    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param type
  \param blockno
  \param data
  \param size_out

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_get_resource_block(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t blockno, const uint8_t **data, uint32_t *size_out)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    *size_out = sizeof(wifi_firmware_image_data);
    *data = wifi_firmware_image_data;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    *size_out = sizeof(wifi_nvram_image);
    *data = (uint8_t *)wifi_nvram_image;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    *size_out = sizeof(wifi_firmware_clm_blob_data);
    *data = wifi_firmware_clm_blob_data;
    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;

  }
}

/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param type
  \param block_count

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_get_resource_no_of_blocks(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t *block_count)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    *block_count = 1;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    *block_count = 1;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    *block_count = 1;
    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;

  }
}

/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param type
  \param size_out

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_get_resource_block_size(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t *size_out)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    *size_out = sizeof(wifi_firmware_image_data);
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    *size_out = sizeof(wifi_nvram_image);
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    *size_out = sizeof(wifi_firmware_clm_blob_data);
    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param whd_drv
  \param type
  \param offset
  \param size
  \param size_out
  \param buffer

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t WHD_resource_read(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t offset, uint32_t size, uint32_t *size_out, void *buffer)
{
  switch (resource)
  {
  case WHD_RESOURCE_WLAN_FIRMWARE:
    memcpy(buffer, wifi_firmware_image_data + offset,  size);
    *size_out = size;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_NVRAM:
    memcpy(buffer, wifi_nvram_image + offset,  size);
    *size_out = size;
    return WHD_SUCCESS;

  case WHD_RESOURCE_WLAN_CLM:
    memcpy(buffer, wifi_firmware_clm_blob_data + offset,  size);
    *size_out = size;
    return WHD_SUCCESS;

  default:
    return WHD_HAL_ERROR;
  }
}


//--------------------------------------------------------------------------------------------------
// cy_buffer_allocate_dynamic_packet
//--------------------------------------------------------------------------------------------------
static NX_PACKET* cy_buffer_allocate_dynamic_packet(uint16_t payload_size)
{
  NX_PACKET *packet;
  ULONG header_size;
  UINT total_size;

  //
  // Allocate a dynamic packet to satisfy a request for a payload size that is larger than  the size in the packet pool.
  //
  // NOTE: This API is only used for WHD communications to support IOVARS with payloads larger than WHD_LINK_MTU.
  // The nx_packet_pool_owner pointer in the packet is left as NULL.
  // When the packet is released in WHD_buffer_release, the NULL nx_packet_pool_owner  pointer will trigger a direct free rather than trying to release back to the packet pool.
  //
  // Packets allocated via this API should never be passed to the IP stack as they
  // can not be released properly by the stack since they do not belong to a packet pool.
  //

  total_size =(payload_size + BLOCK_SIZE_ALIGNMENT + sizeof(NX_PACKET)+ 1);

  packet = App_malloc(total_size);
  if (packet)
  {
    // Initialize the packet structure elements that are needed for use.
    header_size                   = (ULONG)(((sizeof(NX_PACKET)+ NX_PACKET_ALIGNMENT - 1) / NX_PACKET_ALIGNMENT) * NX_PACKET_ALIGNMENT);
    packet->nx_packet_data_start  = (UCHAR *)((uint32_t)packet + header_size);
    packet->nx_packet_data_end    = (UCHAR *)((uint32_t)packet + header_size + payload_size);
    packet->nx_packet_prepend_ptr = packet->nx_packet_data_start;
    packet->nx_packet_append_ptr  = (UCHAR *)((uint32_t)packet->nx_packet_prepend_ptr + payload_size);
    packet->nx_packet_length      = payload_size;
  }

  return packet;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer
  \param direction
  \param size
  \param timeout_ms

  \return whd_result_t
-----------------------------------------------------------------------------------------------------*/
whd_result_t WHD_host_buffer_get(whd_buffer_t *buffer, whd_buffer_dir_t direction, uint16_t size, uint32_t timeout_ms)
{
  UINT             status = NX_NO_PACKET;
  NX_PACKET      **nx_buffer = (NX_PACKET **)buffer;

  if (buffer == NULL) return WHD_BADARG;

  if (size > WHD_LINK_MTU)
  {
    // Request for a packet with a payload larger than MTU. Try to create a dynamically allocated packet to satisfy the request.
    *nx_buffer = cy_buffer_allocate_dynamic_packet(size);
    if (*nx_buffer == NULL) return WHD_BUFFER_UNAVAILABLE_PERMANENT;
    return WHD_SUCCESS;
  }

  // WHD_RTT_LOGs("\r\n");
  status = nx_packet_allocate(&net_packet_pool, nx_buffer, 0, ms_to_ticks(timeout_ms));


  //  WHD_RTT_LOGs("- Get packet ");
  if (status != NX_SUCCESS)
  {
    //    WHD_RTT_LOGs("  Error\r\n");
    return WHD_BUFFER_ALLOC_FAIL;
  }
  //  WHD_RTT_LOGs(" %08X sz=%d\r\n", (uint32_t)*nx_buffer, size);

  (*nx_buffer)->nx_packet_length = size;
  (*nx_buffer)->nx_packet_append_ptr =(*nx_buffer)->nx_packet_prepend_ptr + size;

  return WHD_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer
  \param direction
-----------------------------------------------------------------------------------------------------*/
void         WHD_buffer_release(whd_buffer_t buffer, whd_buffer_dir_t direction)
{
  UINT status;

  NX_PACKET *nx_buffer = (NX_PACKET *) buffer;

  if (nx_buffer->nx_packet_pool_owner == NULL)
  {
    App_free(nx_buffer); // This was a dynamically allocated packet since it is not part of a packet pool.  Free it directly.
    return;
  }

  if (direction == WHD_NETWORK_TX)
  {
    //  TCP transmit packet isn't returned immediately to the pool.
    //  The stack holds the packet temporarily until ACK is received. Otherwise, the same packet is used for
    //  re-transmission. Return prepend pointer to the original location which the stack expects (the start of IP header). For other packets, resetting prepend pointer isn't required.
    //
    if (nx_buffer->nx_packet_length > HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE)
    {
      status = WHD_buffer_add_remove_at_front(&buffer, HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE);
      if (status != WHD_SUCCESS) CY_HALT(); // Could not move packet pointer - this shouldn't happen normally
    }
    status = nx_packet_transmit_release(nx_buffer);
    if (status != NX_SUCCESS) CY_HALT(); // Unable to release packet back to the packet pool. Packet will be leaked
  }
  else
  {
    status = nx_packet_release(nx_buffer);
    if (status != NX_SUCCESS) CY_HALT(); // Unable to release packet back to the packet pool. Packet will be leaked
  }
  /*
    WHD_RTT_LOGs("+ Put packet  %08X (%06d)", (uint32_t)buffer, net_packet_pool.nx_packet_pool_available);
    if (status != NX_SUCCESS)
    {
      WHD_RTT_LOGs("  Error %d --------------\r\n", status);
    }
    else
    {
      WHD_RTT_LOGs("\r\n");
    }
    WHD_RTT_LOGs("\r\n");
  */
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer

  \return uint8_t*
-----------------------------------------------------------------------------------------------------*/
uint8_t* WHD_buffer_get_current_piece_data_pointer(whd_buffer_t buffer)
{
  NX_PACKET *nx_buffer = (NX_PACKET *) buffer;
  return nx_buffer->nx_packet_prepend_ptr;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer

  \return uint16_t
-----------------------------------------------------------------------------------------------------*/
uint16_t     WHD_buffer_get_current_piece_size(whd_buffer_t buffer)
{
  NX_PACKET *nx_buffer = (NX_PACKET *) buffer;
  return (unsigned short) nx_buffer->nx_packet_length;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer
  \param size

  \return whd_result_t
-----------------------------------------------------------------------------------------------------*/
whd_result_t WHD_buffer_set_size(whd_buffer_t buffer, uint16_t size)
{
  NX_PACKET *nx_buffer = (NX_PACKET *) buffer;

  if ((nx_buffer->nx_packet_prepend_ptr + size) > nx_buffer->nx_packet_data_end)
  {
    return WHD_BUFFER_SIZE_SET_ERROR;
  }

  nx_buffer->nx_packet_length = size;
  nx_buffer->nx_packet_append_ptr = nx_buffer->nx_packet_prepend_ptr + size;
  return WHD_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer
  \param add_remove_amount

  \return whd_result_t
-----------------------------------------------------------------------------------------------------*/
whd_result_t WHD_buffer_add_remove_at_front(whd_buffer_t *buffer, int32_t add_remove_amount)
{
  NX_PACKET **nx_buffer = (NX_PACKET **) buffer;
  UCHAR *new_start =(*nx_buffer)->nx_packet_prepend_ptr + add_remove_amount;

  if (new_start < (*nx_buffer)->nx_packet_data_start)
  {
    return WHD_BUFFER_POINTER_MOVE_ERROR;  // Trying to move to a location before start - not supported without buffer chaining
  }
  else if (new_start > (*nx_buffer)->nx_packet_data_end)
  {
    return WHD_BUFFER_POINTER_MOVE_ERROR;  // Trying to move to a location after end of buffer - not supported without buffer chaining
  }
  else
  {
    (*nx_buffer)->nx_packet_prepend_ptr = new_start;
    if ((*nx_buffer)->nx_packet_append_ptr < (*nx_buffer)->nx_packet_prepend_ptr)
    {
      (*nx_buffer)->nx_packet_append_ptr =(*nx_buffer)->nx_packet_prepend_ptr;
    }
    (*nx_buffer)->nx_packet_length = (ULONG)((*nx_buffer)->nx_packet_length - (ULONG) add_remove_amount);
  }
  return WHD_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param thread
  \param entry_function
  \param name
  \param stack
  \param stack_size
  \param priority
  \param arg

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_create_thread(cy_thread_t *thread_ptr, cy_thread_entry_fn_t entry_function, const char *name, void *task_stack, uint32_t stack_size, cy_thread_priority_t priority, cy_thread_arg_t arg)
{
  TX_THREAD *p_thread;
  UINT       status;
  UINT       prio;
  ULONG      time_slice = TX_NO_TIME_SLICE;

  switch (priority)
  {
  case CY_RTOS_PRIORITY_MIN         :
    prio = LOGGER_TASK_PRIO - 1;
    break;
  case CY_RTOS_PRIORITY_LOW         :
    prio = LOGGER_TASK_PRIO;
    break;
  case CY_RTOS_PRIORITY_BELOWNORMAL :
    prio = MAIN_TASK_PRIO + 1;
    break;
  case CY_RTOS_PRIORITY_NORMAL      :
    prio = MAIN_TASK_PRIO;
    break;
  case CY_RTOS_PRIORITY_ABOVENORMAL :
    prio = MAIN_TASK_PRIO -1;
    break;
  case CY_RTOS_PRIORITY_HIGH        :
    prio = THREAD_HIGHER_PRIO - 1;
    break;
  case CY_RTOS_PRIORITY_REALTIME    :
    prio = THREAD_HIGHER_PRIO - 2;
    break;
  case CY_RTOS_PRIORITY_MAX         :
    prio = THREAD_HIGHER_PRIO;
    break;
  default:
    prio = MAIN_TASK_PRIO + 1;
    break;
  }

  p_thread = App_malloc(sizeof(TX_THREAD));
  if (p_thread == NULL)  return CY_RTOS_GENERAL_ERROR;
  status = tx_thread_create(p_thread, (char *) name, (void (*)(ULONG)) entry_function, arg, task_stack, (ULONG) stack_size, (UINT) prio, (UINT) prio, time_slice, (UINT) TX_AUTO_START);

  if (status != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  *thread_ptr = p_thread;
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_exit_thread(void)
{
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param thread

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_terminate_thread(cy_thread_t *thread_ptr)
{
  TX_THREAD *p_thread =  (TX_THREAD *)(*thread_ptr);
  UINT  status;
  status = tx_thread_terminate(p_thread);
  if (status != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param thread
  \param state

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_is_thread_running(cy_thread_t *thread_ptr, bool *state)
{
  TX_THREAD *p_thread =*thread_ptr;
  if ((p_thread->tx_thread_state != TX_COMPLETED) && (p_thread->tx_thread_state != TX_TERMINATED))
  {
    *state = WHD_TRUE;
  }
  else
  {
    *state = WHD_FALSE;
  }

  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param thread

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_join_thread(cy_thread_t *thread_ptr)
{
  TX_THREAD *p_thread =  (TX_THREAD *)(*thread_ptr);
  while ((p_thread->tx_thread_state != TX_COMPLETED) && (p_thread->tx_thread_state != TX_TERMINATED))
  {
    Wait_ms(10);
  }
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param semaphore
  \param maxcount
  \param initcount

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_semaphore(cy_semaphore_t *semaphore, char *name,  uint32_t maxcount, uint32_t initcount)
{
  TX_SEMAPHORE *semaphore_ptr;
  semaphore_ptr = App_malloc(sizeof(TX_SEMAPHORE));
  if (semaphore_ptr == NULL)  return CY_RTOS_GENERAL_ERROR;

  if (tx_semaphore_create(semaphore_ptr, name, initcount) != TX_SUCCESS)
  {
    App_free(semaphore_ptr);
    return CY_RTOS_GENERAL_ERROR;
  }
  *semaphore = semaphore_ptr;
  return CY_RSLT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------


  \param semaphore
  \param timeout_ms
  \param in_isr

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_semaphore(cy_semaphore_t *semaphore, cy_time_t timeout_ms, bool in_isr)
{
  TX_SEMAPHORE *semaphore_ptr =*semaphore;
  if ((in_isr == WHD_TRUE) || (timeout_ms == 0))
  {
    if (tx_semaphore_get(semaphore_ptr,0) != TX_SUCCESS)
    {
      return CY_RTOS_GENERAL_ERROR;
    }
  }
  else
  {
    if (tx_semaphore_get(semaphore_ptr,ms_to_ticks(timeout_ms)) != TX_SUCCESS)
    {
      return CY_RTOS_GENERAL_ERROR;
    }
  }
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param semaphore
  \param in_isr

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_set_semaphore(cy_semaphore_t *semaphore, bool in_isr)
{
  TX_SEMAPHORE *semaphore_ptr =*semaphore;
  if (tx_semaphore_put(semaphore_ptr) != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param semaphore

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_semaphore(cy_semaphore_t *semaphore)
{
  TX_SEMAPHORE *semaphore_ptr =*semaphore;
  if (tx_semaphore_delete(semaphore_ptr) != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  App_free(semaphore_ptr);
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
 Create an event.
 This is an event which can be used to signal a set of threads with a 32 bit data element.

 @param[in,out] event Pointer to the event handle to be initialized

 @return The status of the event initialization request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_event(cy_event_t *evnt, char *name)
{
  TX_EVENT_FLAGS_GROUP *event_ptr;
  event_ptr = App_malloc(sizeof(TX_EVENT_FLAGS_GROUP));
  if (event_ptr == NULL)  return CY_RTOS_GENERAL_ERROR;

  if (tx_event_flags_create(event_ptr, name) != TX_SUCCESS)
  {
    App_free(event_ptr);
    return CY_RTOS_GENERAL_ERROR;
  }
  *evnt = event_ptr;
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  Set the event flag bits.

  This is an event which can be used to signal a set of threads
  with a 32 bit data element. Any threads waiting on this event are released

  @param[in] event  Pointer to the event handle
  @param[in] bits   The value of the 32 bit flags
  @param[in] in_isr If true, this is called from an ISR, otherwise from a thread

  @return The status of the set request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_setbits_event(cy_event_t *evnt, uint32_t bits, bool in_isr)
{
  TX_EVENT_FLAGS_GROUP  *evnt_ptr =*evnt;
  if (tx_event_flags_set(evnt_ptr, bits, TX_OR) != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------
  Clear the event flag bits

  This function clears bits in the event.

  @param[in] event   Pointer to the event handle
  @param[in] bits    Any bits set in this value, will be cleared in the event.
  @param[in] in_isr  if true, this is called from an ISR, otherwise from a thread

  @return The status of the clear flags request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_clearbits_event(cy_event_t *evnt, uint32_t bits, bool in_isr)
{
  TX_EVENT_FLAGS_GROUP  *evnt_ptr =*evnt;
  ULONG actual_evnt;
  if (tx_event_flags_get(evnt_ptr, bits, TX_OR_CLEAR,&actual_evnt, 0) != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
   Get the event bits.

  Returns the current bits for the event.

  @param[in]  event Pointer to the event handle
  @param[out] bits  pointer to receive the value of the event flags

  @return The status of the get request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_getbits_event(cy_event_t *evnt, uint32_t *bits)
{
  TX_EVENT_FLAGS_GROUP  *evnt_ptr =*evnt;
  ULONG actual_evnt;
  if (tx_event_flags_get(evnt_ptr, *bits, TX_OR_CLEAR,&actual_evnt, TX_NO_WAIT) != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  *bits= actual_evnt;
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
 Wait for the event and return bits.

 Waits for the event to be set and then returns the bits assocaited
 with the event, or waits for the given timeout period.
 @note This function returns if any bit in the set is set.

 @param[in] event     Pointer to the event handle
 @param[in,out] bits  pointer to receive the value of the event flags
 @param[in] clear     if true, clear any bits set that cause the wait to return
                      if false, do not clear bits
 @param[in] all       if true, all bits in the initial bits value must be set to return
                      if false, any one bit in the initial bits value must be set to return
 @param[in] timeout   The amount of time to wait in milliseconds

 @return The status of the wait for event request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_waitbits_event(cy_event_t *evnt, uint32_t *bits, bool clear, bool all, cy_time_t timeout)
{
  TX_EVENT_FLAGS_GROUP  *evnt_ptr =*evnt;
  ULONG actual_evnt;
  UINT  opt;
  if (all == true )
  {
    if (clear == true) opt = TX_AND_CLEAR;
    else opt = TX_AND;
  }
  else
  {
    if (clear == true) opt = TX_OR_CLEAR;
    else opt = TX_OR;
  }
  if (tx_event_flags_get(evnt_ptr, *bits, opt ,&actual_evnt, MS_TO_TICKS(timeout)) != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  *bits= actual_evnt;
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  Deinitialize a event.
  This function frees the resources associated with an event.

  @param[in] event Pointer to the event handle

  @return The status of the deletion request. [CY_RSLT_SUCCESS, CY_RTOS_GENERAL_ERROR]
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_event(cy_event_t *evnt)
{
  TX_EVENT_FLAGS_GROUP  *evnt_ptr =*evnt;
  if (tx_event_flags_delete(evnt_ptr) != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  App_free(evnt_ptr);
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param tval

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_time(cy_time_t *tval)
{

  *tval =((1000ul * tx_time_get()) / TX_TIMER_TICKS_PER_SECOND);
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param num_ms

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_delay_milliseconds(cy_time_t num_ms)
{
  Wait_ms(num_ms);
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param event
  \param enable
-----------------------------------------------------------------------------------------------------*/
void cyhal_sdio_irq_enable(cyhal_sdio_t *obj, cyhal_sdio_irq_event_t irq_event, bool enable)
{
  if (irq_event == CYHAL_SDIO_CARD_INTERRUPT)
  {
    if (enable == WHD_TRUE)
    {
      SDIO1_irq_enable();
    }
    else
    {
      SDIO1_irq_disable();
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
   Регистрируем функцию вызываемую из прерывания обслуживающего прием данных из SDIO

  \param obj
  \param handler
  \param handler_arg
-----------------------------------------------------------------------------------------------------*/
void cyhal_sdio_register_irq(cyhal_sdio_t *obj, cyhal_sdio_irq_handler_t handler, void *handler_arg)
{

}

/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param direction
  \param argument
  \param data
  \param length
  \param response

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cyhal_sdio_bulk_transfer(cyhal_sdio_t *obj, cyhal_transfer_t direction, uint32_t argument, const uint32_t *data, uint16_t length, uint32_t *response)
{
  static ssp_err_t          status;
  sdmmc_io_transfer_mode_t  tmode;
  const sdmmc_instance_t   *sdio_inst_ptr = (sdmmc_instance_t *)(*obj);

  whd_bus_sdio_cmd53_argument_t *arg = (whd_bus_sdio_cmd53_argument_t *)&argument;

  if (arg->block_mode == 0)
  {
    tmode = SDMMC_IO_MODE_TRANSFER_BYTE;
  }
  else
  {
    tmode = SDMMC_IO_MODE_TRANSFER_BLOCK;
  }

  if (direction == CYHAL_WRITE)
  {
    if (tmode == SDMMC_IO_MODE_TRANSFER_BLOCK)
    {
      WHD_RTT_PRINT("WB> A:%08X F:%d Sz=%02X", arg->register_address, arg->function_number, arg->count * 64);
      status = sdio_inst_ptr->p_api->writeIoExt(sdio_inst_ptr->p_ctrl, (uint8_t *)data, arg->function_number, arg->register_address , arg->count,  SDMMC_IO_MODE_TRANSFER_BLOCK, SDMMC_IO_ADDRESS_MODE_INCREMENT);
      if (status == SSP_SUCCESS)
      {
        if (SDIO1_wait_transfer_complete(10) != RES_OK)
        {
          WHD_RTT_PRINT(" Transfer error\r\n");
          status = SSP_ERR_ABORTED;
        }
        else
        {
          WHD_RTT_PRINT("  Ok\r\n");
        }
      }
    }
    else
    {
      WHD_RTT_PRINT("WB> A:%08X F:%d Sz=%02X", arg->register_address, arg->function_number, arg->count);
      status = sdio_inst_ptr->p_api->writeIoExt(sdio_inst_ptr->p_ctrl,(uint8_t const*)data, arg->function_number, arg->register_address , arg->count,  tmode, SDMMC_IO_ADDRESS_MODE_INCREMENT);
      if (status == SSP_SUCCESS)
      {
        if (SDIO1_wait_transfer_complete(10) != RES_OK)
        {
          WHD_RTT_PRINT(" Transfer error\r\n");
          status = SSP_ERR_ABORTED;
        }
        else
        {
          #ifdef DEBUG_RTT_PRINT
          uint32_t n = arg->count;
          if (n <= 64)
          {
            for (uint32_t i=0; i < n; i++)
            {
              WHD_RTT_PRINT(" %02X", ((uint8_t const *)data)[i]);
            }
          }
          WHD_RTT_PRINT("  Ok\r\n");
          #endif
        }
      }
      else
      {
        WHD_RTT_PRINT("  Cmd Error= %08X\r\n", status);
      }
    }
  }
  else
  {
    uint32_t cnt = arg->count;
    WHD_RTT_PRINT("RB< A:%08X F:%d Sz=%02X", arg->register_address, arg->function_number, cnt);
    status = sdio_inst_ptr->p_api->readIoExt(sdio_inst_ptr->p_ctrl, (uint8_t *)data, arg->function_number, arg->register_address ,&cnt,  tmode, SDMMC_IO_ADDRESS_MODE_INCREMENT);
    if (status == SSP_SUCCESS)
    {
      if (SDIO1_wait_transfer_complete(10) != RES_OK)
      {
        WHD_RTT_PRINT(" Transfer error\r\n");
        status = SSP_ERR_ABORTED;
      }
      else
      {
        #ifdef DEBUG_RTT_PRINT
        uint32_t n = arg->count;
        if (n <= 64)
        {
          for (uint32_t i=0; i < n; i++)
          {
            WHD_RTT_PRINT(" %02X", ((uint8_t const *)data)[i]);
          }
        }
        WHD_RTT_PRINT("  Ok\r\n");
        #endif
      }
    }
    else
    {
      WHD_RTT_PRINT("  Error= %08X\r\n", status);
    }
  }

  if (status != SSP_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param direction
  \param command
  \param argument
  \param response

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cyhal_sdio_send_cmd(const cyhal_sdio_t *obj, cyhal_transfer_t direction, cyhal_sdio_command_t command, uint32_t argument, uint32_t *response)
{
  ssp_err_t                       status;
  const sdmmc_instance_t         *sdio_inst_ptr = (sdmmc_instance_t *)(*obj);
  whd_bus_sdio_cmd52_argument_t  *arg = (whd_bus_sdio_cmd52_argument_t *)&argument;

  if (command != CYHAL_SDIO_CMD_IO_RW_DIRECT)
  {
    return CY_RTOS_GENERAL_ERROR;
  }

  if (direction == CYHAL_WRITE)
  {
    WHD_RTT_PRINT("W > A:%08X F:%d = %02X", arg->register_address, arg->function_number, arg->write_data);
    // CMD52 запись одного байта в регистр
    status = sdio_inst_ptr->p_api->writeIo(sdio_inst_ptr->p_ctrl,&arg->write_data, arg->function_number, arg->register_address , (sdmmc_io_write_mode_t)arg->raw_flag);
    if (status == SSP_SUCCESS)
    {
      WHD_RTT_PRINT("  Ok\r\n");
    }
    else
    {
      WHD_RTT_PRINT("  Error= %08X\r\n", status);
    }
  }
  else
  {
    WHD_RTT_PRINT("R < A:%08X F:%d = ", arg->register_address, arg->function_number);
    // CMD52 чтение одного байта из регистра
    status = sdio_inst_ptr->p_api->readIo(sdio_inst_ptr->p_ctrl,&arg->write_data, arg->function_number, arg->register_address);
    if (status == SSP_SUCCESS)
    {
      WHD_RTT_PRINT("%02X  Ok\r\n",arg->write_data);
    }
    else
    {
      WHD_RTT_PRINT("?   Error= %08X\r\n", status);
    }
  }

  if (response) *response = arg->write_data;


  if (status != SSP_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param pin
  \param event
  \param enable
-----------------------------------------------------------------------------------------------------*/
void cyhal_gpio_irq_enable(cyhal_gpio_t pin, cyhal_gpio_irq_event_t event, bool enable)
{


}


/*-----------------------------------------------------------------------------------------------------


  \param pin
  \param intrPriority
  \param handler
  \param handler_arg
-----------------------------------------------------------------------------------------------------*/
void cyhal_gpio_register_irq(cyhal_gpio_t pin, uint8_t intrPriority, cyhal_gpio_irq_handler_t handler, void *handler_arg)
{


}

/*-----------------------------------------------------------------------------------------------------


  \param pin
  \param direction
  \param drvMode
  \param initVal

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cyhal_gpio_init(cyhal_gpio_t pin, cyhal_gpio_direction_t direction, cyhal_gpio_drive_mode_t drvMode, bool initVal)
{

  return CY_RSLT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая из обработчика прерывания SDIO при приеме данных

  \param void
-----------------------------------------------------------------------------------------------------*/
void WHD_thread_notify_irq(void)
{
  whd_thread_notify_irq(g_whd_driver);
}


/*-----------------------------------------------------------------------------------------------------


  \param name
  \param line_num
  \param severity
  \param fmt_ptr
-----------------------------------------------------------------------------------------------------*/
void WHD_RTT_LOGs(const char *fmt_ptr, ...)
{
  unsigned int     n;
  va_list          ap;

  va_start(ap, fmt_ptr);

  __disable_interrupt();
  n = vsnprintf(whd_rtt_log_str, WHD_RTT_LOG_STR_SZ, (const char *)fmt_ptr, ap);
  SEGGER_RTT_Write(RTT_LOG_CH, whd_rtt_log_str, n);
  __enable_interrupt();

  va_end(ap);
}

#define CASE_RETURN_STRING(val) case val: return # val;

const char* WHD_event_to_string(whd_event_num_t val)
{
  switch (val)
  {
    CASE_RETURN_STRING(WLC_E_NONE)               // = 0x7FFFFFFE,
    CASE_RETURN_STRING(WLC_E_SET_SSID)               // = 0,
    CASE_RETURN_STRING(WLC_E_JOIN)               // = 1,
    CASE_RETURN_STRING(WLC_E_START)               // = 2,
    CASE_RETURN_STRING(WLC_E_AUTH)               // = 3,
    CASE_RETURN_STRING(WLC_E_AUTH_IND)               // = 4,
    CASE_RETURN_STRING(WLC_E_DEAUTH)               // = 5,
    CASE_RETURN_STRING(WLC_E_DEAUTH_IND)               // = 6,
    CASE_RETURN_STRING(WLC_E_ASSOC)               // = 7,
    CASE_RETURN_STRING(WLC_E_ASSOC_IND)               // = 8,
    CASE_RETURN_STRING(WLC_E_REASSOC)               // = 9,
    CASE_RETURN_STRING(WLC_E_REASSOC_IND)               // = 10,
    CASE_RETURN_STRING(WLC_E_DISASSOC)               // = 11,
    CASE_RETURN_STRING(WLC_E_DISASSOC_IND)               // = 12,
    CASE_RETURN_STRING(WLC_E_QUIET_START)               // = 13,
    CASE_RETURN_STRING(WLC_E_QUIET_END)               // = 14,
    CASE_RETURN_STRING(WLC_E_BEACON_RX)               // = 15,
    CASE_RETURN_STRING(WLC_E_LINK)               // = 16,
    CASE_RETURN_STRING(WLC_E_MIC_ERROR)               // = 17,
    CASE_RETURN_STRING(WLC_E_NDIS_LINK)               // = 18,
    CASE_RETURN_STRING(WLC_E_ROAM)               // = 19,
    CASE_RETURN_STRING(WLC_E_TXFAIL)               // = 20,
    CASE_RETURN_STRING(WLC_E_PMKID_CACHE)               // = 21,
    CASE_RETURN_STRING(WLC_E_RETROGRADE_TSF)               // = 22,
    CASE_RETURN_STRING(WLC_E_PRUNE)               // = 23,
    CASE_RETURN_STRING(WLC_E_AUTOAUTH)               // = 24,
    CASE_RETURN_STRING(WLC_E_EAPOL_MSG)               // = 25,
    CASE_RETURN_STRING(WLC_E_SCAN_COMPLETE)               // = 26,
    CASE_RETURN_STRING(WLC_E_ADDTS_IND)               // = 27,
    CASE_RETURN_STRING(WLC_E_DELTS_IND)               // = 28,
    CASE_RETURN_STRING(WLC_E_BCNSENT_IND)               // = 29,
    CASE_RETURN_STRING(WLC_E_BCNRX_MSG)               // = 30,
    CASE_RETURN_STRING(WLC_E_BCNLOST_MSG)               // = 31,
    CASE_RETURN_STRING(WLC_E_ROAM_PREP)               // = 32,
    CASE_RETURN_STRING(WLC_E_PFN_NET_FOUND)               // = 33,
    CASE_RETURN_STRING(WLC_E_PFN_NET_LOST)               // = 34,
    CASE_RETURN_STRING(WLC_E_RESET_COMPLETE)               // = 35,
    CASE_RETURN_STRING(WLC_E_JOIN_START)               // = 36,
    CASE_RETURN_STRING(WLC_E_ROAM_START)               // = 37,
    CASE_RETURN_STRING(WLC_E_ASSOC_START)               // = 38,
    CASE_RETURN_STRING(WLC_E_IBSS_ASSOC)               // = 39,
    CASE_RETURN_STRING(WLC_E_RADIO)               // = 40,
    CASE_RETURN_STRING(WLC_E_PSM_WATCHDOG)               // = 41,
    CASE_RETURN_STRING(WLC_E_CCX_ASSOC_START)               // = 42,
    CASE_RETURN_STRING(WLC_E_CCX_ASSOC_ABORT)               // = 43,
    CASE_RETURN_STRING(WLC_E_PROBREQ_MSG)               // = 44,
    CASE_RETURN_STRING(WLC_E_SCAN_CONFIRM_IND)               // = 45,
    CASE_RETURN_STRING(WLC_E_PSK_SUP)               // = 46,
    CASE_RETURN_STRING(WLC_E_COUNTRY_CODE_CHANGED)               // = 47,
    CASE_RETURN_STRING(WLC_E_EXCEEDED_MEDIUM_TIME)               // = 48,
    CASE_RETURN_STRING(WLC_E_ICV_ERROR)               // = 49,
    CASE_RETURN_STRING(WLC_E_UNICAST_DECODE_ERROR)               // = 50,
    CASE_RETURN_STRING(WLC_E_MULTICAST_DECODE_ERROR)               // = 51,
    CASE_RETURN_STRING(WLC_E_TRACE)               // = 52,
    CASE_RETURN_STRING(WLC_E_BTA_HCI_EVENT)               // = 53,
    CASE_RETURN_STRING(WLC_E_IF)               // = 54,
    CASE_RETURN_STRING(WLC_E_P2P_DISC_LISTEN_COMPLETE)               // = 55,
    CASE_RETURN_STRING(WLC_E_RSSI)               // = 56,
    CASE_RETURN_STRING(WLC_E_PFN_BEST_BATCHING)               // = 57,
    CASE_RETURN_STRING(WLC_E_EXTLOG_MSG)               // = 58,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME)               // = 59,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME_COMPLETE)               // = 60,
    CASE_RETURN_STRING(WLC_E_PRE_ASSOC_IND)               // = 61,
    CASE_RETURN_STRING(WLC_E_PRE_REASSOC_IND)               // = 62,
    CASE_RETURN_STRING(WLC_E_CHANNEL_ADOPTED)               // = 63,
    CASE_RETURN_STRING(WLC_E_AP_STARTED)               // = 64,
    CASE_RETURN_STRING(WLC_E_DFS_AP_STOP)               // = 65,
    CASE_RETURN_STRING(WLC_E_DFS_AP_RESUME)               // = 66,
    CASE_RETURN_STRING(WLC_E_WAI_STA_EVENT)               // = 67,
    CASE_RETURN_STRING(WLC_E_WAI_MSG)               // = 68,
    CASE_RETURN_STRING(WLC_E_ESCAN_RESULT)               // = 69,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE)               // = 70,
    CASE_RETURN_STRING(WLC_E_PROBRESP_MSG)               // = 71,
    CASE_RETURN_STRING(WLC_E_P2P_PROBREQ_MSG)               // = 72,
    CASE_RETURN_STRING(WLC_E_DCS_REQUEST)               // = 73,
    CASE_RETURN_STRING(WLC_E_FIFO_CREDIT_MAP)               // = 74,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME_RX)               // = 75,
    CASE_RETURN_STRING(WLC_E_WAKE_EVENT)               // = 76,
    CASE_RETURN_STRING(WLC_E_RM_COMPLETE)               // = 77,
    CASE_RETURN_STRING(WLC_E_HTSFSYNC)               // = 78,
    CASE_RETURN_STRING(WLC_E_OVERLAY_REQ)               // = 79,
    CASE_RETURN_STRING(WLC_E_CSA_COMPLETE_IND)               // = 80,
    CASE_RETURN_STRING(WLC_E_EXCESS_PM_WAKE_EVENT)               // = 81,
    CASE_RETURN_STRING(WLC_E_PFN_SCAN_NONE)               // = 82,
    CASE_RETURN_STRING(WLC_E_PFN_SCAN_ALLGONE)               // = 83,
    CASE_RETURN_STRING(WLC_E_GTK_PLUMBED)               // = 84,
    CASE_RETURN_STRING(WLC_E_ASSOC_IND_NDIS)               // = 85,
    CASE_RETURN_STRING(WLC_E_REASSOC_IND_NDIS)               // = 86,
    CASE_RETURN_STRING(WLC_E_ASSOC_REQ_IE)               // = 87,
    CASE_RETURN_STRING(WLC_E_ASSOC_RESP_IE)               // = 88,
    CASE_RETURN_STRING(WLC_E_ASSOC_RECREATED)               // = 89,
    CASE_RETURN_STRING(WLC_E_ACTION_FRAME_RX_NDIS)               // = 90,
    CASE_RETURN_STRING(WLC_E_AUTH_REQ)               // = 91,
    CASE_RETURN_STRING(WLC_E_MESH_DHCP_SUCCESS)               // = 92,
    CASE_RETURN_STRING(WLC_E_SPEEDY_RECREATE_FAIL)               // = 93,
    CASE_RETURN_STRING(WLC_E_NATIVE)               // = 94,
    CASE_RETURN_STRING(WLC_E_PKTDELAY_IND)               // = 95,
    CASE_RETURN_STRING(WLC_E_AWDL_AW)               // = 96,
    CASE_RETURN_STRING(WLC_E_AWDL_ROLE)               // = 97,
    CASE_RETURN_STRING(WLC_E_AWDL_EVENT)               // = 98,
    CASE_RETURN_STRING(WLC_E_NIC_AF_TXS)               // = 99,
    CASE_RETURN_STRING(WLC_E_NAN)               // = 100,
    CASE_RETURN_STRING(WLC_E_BEACON_FRAME_RX)               // = 101,
    CASE_RETURN_STRING(WLC_E_SERVICE_FOUND)               // = 102,
    CASE_RETURN_STRING(WLC_E_GAS_FRAGMENT_RX)               // = 103,
    CASE_RETURN_STRING(WLC_E_GAS_COMPLETE)               // = 104,
    CASE_RETURN_STRING(WLC_E_P2PO_ADD_DEVICE)               // = 105,
    CASE_RETURN_STRING(WLC_E_P2PO_DEL_DEVICE)               // = 106,
    CASE_RETURN_STRING(WLC_E_WNM_STA_SLEEP)               // = 107,
    CASE_RETURN_STRING(WLC_E_TXFAIL_THRESH)               // = 108,
    CASE_RETURN_STRING(WLC_E_PROXD)               // = 109,
    CASE_RETURN_STRING(WLC_E_MESH_PAIRED)               // = 110,
    CASE_RETURN_STRING(WLC_E_AWDL_RX_PRB_RESP)               // = 111,
    CASE_RETURN_STRING(WLC_E_AWDL_RX_ACT_FRAME)               // = 112,
    CASE_RETURN_STRING(WLC_E_AWDL_WOWL_NULLPKT)               // = 113,
    CASE_RETURN_STRING(WLC_E_AWDL_PHYCAL_STATUS)               // = 114,
    CASE_RETURN_STRING(WLC_E_AWDL_OOB_AF_STATUS)               // = 115,
    CASE_RETURN_STRING(WLC_E_AWDL_SCAN_STATUS)               // = 116,
    CASE_RETURN_STRING(WLC_E_AWDL_AW_START)               // = 117,
    CASE_RETURN_STRING(WLC_E_AWDL_AW_END)               // = 118,
    CASE_RETURN_STRING(WLC_E_AWDL_AW_EXT)               // = 119,
    CASE_RETURN_STRING(WLC_E_AWDL_PEER_CACHE_CONTROL)               // = 120,
    CASE_RETURN_STRING(WLC_E_CSA_START_IND)               // = 121,
    CASE_RETURN_STRING(WLC_E_CSA_DONE_IND)               // = 122,
    CASE_RETURN_STRING(WLC_E_CSA_FAILURE_IND)               // = 123,
    CASE_RETURN_STRING(WLC_E_CCA_CHAN_QUAL)               // = 124,
    CASE_RETURN_STRING(WLC_E_BSSID)               // = 125,
    CASE_RETURN_STRING(WLC_E_TX_STAT_ERROR)               // = 126,
    CASE_RETURN_STRING(WLC_E_BCMC_CREDIT_SUPPORT)               // = 127,
    CASE_RETURN_STRING(WLC_E_PSTA_PRIMARY_INTF_IND)               // = 128,
    CASE_RETURN_STRING(WLC_E_BT_WIFI_HANDOVER_REQ)               // = 130,
    CASE_RETURN_STRING(WLC_E_SPW_TXINHIBIT)               // = 131,
    CASE_RETURN_STRING(WLC_E_FBT_AUTH_REQ_IND)               // = 132,
    CASE_RETURN_STRING(WLC_E_RSSI_LQM)               // = 133,
    CASE_RETURN_STRING(WLC_E_PFN_GSCAN_FULL_RESULT)               // = 134,
    CASE_RETURN_STRING(WLC_E_PFN_SWC)               // = 135,
    CASE_RETURN_STRING(WLC_E_AUTHORIZED)               // = 136,
    CASE_RETURN_STRING(WLC_E_PROBREQ_MSG_RX)               // = 137,
    CASE_RETURN_STRING(WLC_E_PFN_SCAN_COMPLETE)               // = 138,
    CASE_RETURN_STRING(WLC_E_RMC_EVENT)               // = 139,
    CASE_RETURN_STRING(WLC_E_DPSTA_INTF_IND)               // = 140,
    CASE_RETURN_STRING(WLC_E_RRM)               // = 141,
    CASE_RETURN_STRING(WLC_E_ULP)               // = 146,
    CASE_RETURN_STRING(WLC_E_TKO)               // = 151,
    CASE_RETURN_STRING(WLC_E_EXT_AUTH_REQ)               // = 187,
    CASE_RETURN_STRING(WLC_E_EXT_AUTH_FRAME_RX)               // = 188,
    CASE_RETURN_STRING(WLC_E_MGMT_FRAME_TXSTATUS)               // = 189,
    CASE_RETURN_STRING(WLC_E_LAST)               // = 190,
  default:
    return "Unknown";
  }
}

