// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-29
// 16:49:08
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "whd.h"
#include   "whd_events_int.h"

#include   "whd_thread.h"
#include   "cyhal_hw_types.h"
#include   "cyabs_rtos.h"
#include   "cy_result.h"
#include   "ssp_common_api.h"
#include   "whd_resource_api.h"
#include   "whd_network_types.h"
#include   "WHD_resource_man.h"
#include   "WHD_buffer_man.h"


extern  whd_driver_t         g_whd_driver;


#define WRAPPER_IDENT           (0xABCDEF01U)
#define MILLISECONDS_PER_SECOND (1000)
#define ALL_EVENT_FLAGS         (0xFFFFFFFFU)
#define MAX_QUEUE_MESSAGE_SIZE  (16)

typedef struct
{
    TX_THREAD  thread;
    uint32_t   magic;      // Магическое число указывающее на то что задача была создана вместе со стеком
    void      *stack_ptr;  // Укзатель на созданный стек
} cy_thread_wrapper_t;


cy_rtos_error_t          cy_last_error;
static uint16_t          _cy_rtos_suspend_count = 0;

/*-----------------------------------------------------------------------------------------------------
  Checks to see if code is currently executing within an interrupt context.

  \return bool Boolean indicating whether this was executed from an interrupt context.
-----------------------------------------------------------------------------------------------------*/
static inline bool is_in_isr(void)
{
  return (__get_IPSR() != 0);
}

//--------------------------------------------------------------------------------------------------
// convert_ms_to_ticks
//--------------------------------------------------------------------------------------------------
static cy_time_t convert_ms_to_ticks(cy_time_t timeout_ms)
{
  if (timeout_ms == CY_RTOS_NEVER_TIMEOUT)
  {
    return TX_WAIT_FOREVER;
  }
  else if (timeout_ms == 0)
  {
    return 0;
  }
  else
  {
    uint64_t ticks = (uint64_t)timeout_ms * (uint64_t)TX_TIMER_TICKS_PER_SECOND / (uint64_t)MILLISECONDS_PER_SECOND;
    if (ticks == 0)
    {
      ticks = 1;
    }
    else if (ticks >= UINT32_MAX)
    {
      // if ticks if more than 32 bits, change ticks to max possible value that isn't TX_WAIT_FOREVER.
      ticks = UINT32_MAX - 1;
    }
    return (cy_time_t)ticks;
  }
}

//--------------------------------------------------------------------------------------------------
// convert_ticks_to_ms
//--------------------------------------------------------------------------------------------------
//static inline cy_time_t convert_ticks_to_ms(cy_time_t timeout_ticks)
//{
//  return (cy_time_t)((uint64_t)timeout_ticks * (uint64_t)MILLISECONDS_PER_SECOND / (uint64_t)TX_TIMER_TICKS_PER_SECOND);
//}

/*-----------------------------------------------------------------------------------------------------


  \param num_ms

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_delay_milliseconds(cy_time_t num_ms)
{
  Wait_ms(num_ms);
  return CY_RSLT_SUCCESS;
}


//--------------------------------------------------------------------------------------------------
// convert_error
//--------------------------------------------------------------------------------------------------
static inline cy_rslt_t convert_error(cy_rtos_error_t error)
{
  if (error != TX_SUCCESS)
  {
    cy_last_error = error;
    return CY_RTOS_GENERAL_ERROR;
  }
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


  \param thread
  \param entry_function
  \param name
  \param stack
  \param stack_size
  \param priority
  \param arg

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_create_thread(cy_thread_t *thread_ptr, cy_thread_entry_fn_t entry_function, const char *name, void *task_stack, uint32_t stack_size, uint32_t prio, cy_thread_arg_t arg)
{
  UINT       status;
  ULONG      time_slice = TX_NO_TIME_SLICE;

  uint32_t malloc_size = sizeof(cy_thread_wrapper_t);
  if (task_stack == NULL)
  {
    malloc_size += stack_size;
  }
  void *buffer = App_malloc(malloc_size);
  if (buffer == NULL) return CY_RTOS_NO_MEMORY;

  cy_thread_wrapper_t *wrapper_ptr;
  if (task_stack == NULL)
  {
    task_stack             = buffer;
    wrapper_ptr            = (cy_thread_wrapper_t *)((uint32_t)buffer + stack_size);
    wrapper_ptr->stack_ptr = task_stack;
  }
  else
  {
    wrapper_ptr            = buffer;
    wrapper_ptr->stack_ptr = NULL;
  }
  wrapper_ptr->magic = WRAPPER_IDENT;

  *thread_ptr = (cy_thread_t)wrapper_ptr;

  status = tx_thread_create(*thread_ptr, (char *) name, (void (*)(ULONG)) entry_function, arg, task_stack, (ULONG) stack_size, (UINT) prio, (UINT) prio, time_slice, (UINT) TX_AUTO_START);

  if (status != TX_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
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
  TX_THREAD *p_thread = *thread_ptr;
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
  if (thread_ptr == NULL) return CY_RTOS_BAD_PARAM;


  UINT thread_state;

  cy_rtos_error_t res = tx_thread_info_get(*thread_ptr, TX_NULL,&thread_state, TX_NULL, TX_NULL,TX_NULL, TX_NULL, TX_NULL, TX_NULL);
  if (TX_SUCCESS != res)
  {
    cy_last_error = res;
    return CY_RTOS_GENERAL_ERROR;
  }

  while (TX_TERMINATED != thread_state && TX_COMPLETED != thread_state)
  {
    res = tx_thread_sleep(1);
    if (TX_SUCCESS != res)
    {
      cy_last_error = res;
      return CY_RTOS_GENERAL_ERROR;
    }

    res = tx_thread_info_get(*thread_ptr, TX_NULL,&thread_state, TX_NULL, TX_NULL, TX_NULL,TX_NULL, TX_NULL, TX_NULL);
    if (TX_SUCCESS != res)
    {
      cy_last_error = res;
      return CY_RTOS_GENERAL_ERROR;
    }
  }
  res = tx_thread_delete(*thread_ptr);
  if (TX_SUCCESS != res)
  {
    cy_last_error = res;
    return CY_RTOS_GENERAL_ERROR;
  }

  cy_thread_wrapper_t *wrapper_ptr = (cy_thread_wrapper_t *)(*thread_ptr);
  if (wrapper_ptr->magic == WRAPPER_IDENT)
  {
    if (wrapper_ptr->stack_ptr != NULL)
    {
      App_free(wrapper_ptr->stack_ptr);
    }
    else
    {
      App_free(wrapper_ptr);
    }
  }
  return CY_RSLT_SUCCESS;

}


//--------------------------------------------------------------------------------------------------
// cy_rtos_thread_get_handle
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_thread_get_handle(cy_thread_t *thread_ptr)
{
  if (thread_ptr == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  *thread_ptr = tx_thread_identify();
  return CY_RSLT_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// cy_rtos_thread_wait_notification
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_thread_wait_notification(cy_time_t timeout_ms)
{
  UINT      ret;
  cy_rslt_t status = CY_RSLT_SUCCESS;

  ret = tx_thread_sleep(convert_ms_to_ticks(timeout_ms));
  /* Update the last known error status */
  cy_last_error = (cy_rtos_error_t)ret;

  if (ret == TX_SUCCESS)
  {
    status = CY_RTOS_TIMEOUT;
  }
  else if (ret != TX_WAIT_ABORTED)
  {
    status = CY_RTOS_GENERAL_ERROR;
  }
  return status;
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_thread_set_notification
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_thread_set_notification(cy_thread_t *thread)
{
  if (thread == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  /* According to ThreadX user guide, this function allowed to
   * be called from ISR
   */
  return convert_error(tx_thread_wait_abort(*thread));
}

//--------------------------------------------------------------------------------------------------
// cy_rtos_thread_get_name
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_thread_get_name(cy_thread_t *thread, const char **thread_name)
{
  char *temp_name;
  cy_rtos_error_t tx_rslt = tx_thread_info_get(*thread,&temp_name, TX_NULL, TX_NULL, TX_NULL,TX_NULL,TX_NULL, TX_NULL, TX_NULL);
  cy_rslt_t result = convert_error(tx_rslt);
  if (result == CY_RSLT_SUCCESS)
  {
    *thread_name = temp_name;
  }
  return result;
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_scheduler_suspend
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_scheduler_suspend(void)
{
  ++_cy_rtos_suspend_count;
  tx_interrupt_control(TX_INT_DISABLE);

  return CY_RSLT_SUCCESS;
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_scheduler_resume
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_scheduler_resume(void)
{
  cy_rslt_t status;
  if (_cy_rtos_suspend_count > 0)
  {
    if (_cy_rtos_suspend_count == 1)
    {
      tx_interrupt_control(TX_INT_ENABLE);
    }
    --_cy_rtos_suspend_count;
    status = CY_RSLT_SUCCESS;
  }
  else
  {
    status = CY_RTOS_BAD_PARAM;
  }

  return status;
}

//--------------------------------------------------------------------------------------------------
// cy_rtos_mutex_init
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_mutex_init(cy_mutex_t *mutex, bool recursive)
{
  if (mutex == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  // Non recursive mutex is not supported by ThreadX. A recursive mutex is returned
  // even if a non-recursive mutex was requested. This is ok because in all the cases
  // where the behavior of the two types differs would have ended in a deadlock. So
  // the difference in behavior should not have a functional impact on application.
  return convert_error(tx_mutex_create(mutex, TX_NULL, TX_INHERIT));
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_mutex_get
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_mutex_get(cy_mutex_t *mutex, cy_time_t timeout_ms)
{
  if (mutex == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  cy_rtos_error_t tx_rslt = tx_mutex_get(mutex, convert_ms_to_ticks(timeout_ms));
  if (TX_NOT_AVAILABLE == tx_rslt)
  {
    return CY_RTOS_TIMEOUT;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_mutex_set
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_mutex_set(cy_mutex_t *mutex)
{
  if (mutex == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_mutex_put(mutex));
}


//--------------------------------------------------------------------------------------------------
//  cy_rtos_deinit_mutex
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_mutex_deinit(cy_mutex_t *mutex)
{
  if (mutex == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_mutex_delete(mutex));
}


/*-----------------------------------------------------------------------------------------------------


  \param semaphore
  \param maxcount
  \param initcount

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_semaphore(cy_semaphore_t *semaphore, char *name,  uint32_t maxcount, uint32_t initcount)
{
  if (semaphore == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  semaphore->maxcount = maxcount;
  return convert_error(tx_semaphore_create(&(semaphore->tx_semaphore), name, initcount));
}

/*-----------------------------------------------------------------------------------------------------


  \param semaphore
  \param timeout_ms
  \param in_isr

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_semaphore(cy_semaphore_t *semaphore, cy_time_t timeout_ms, bool in_isr)
{
  // Based on documentation
  // http://www.ece.ualberta.ca/~cmpe490/documents/ghs/405/threadxug_g40c.pdf
  // pg 168 it specifies that the timeout must be zero when called from ISR.
  if ((semaphore == NULL) || ((in_isr == WHD_TRUE) && (timeout_ms != 0)))
  {
    return CY_RTOS_BAD_PARAM;
  }
  cy_rtos_error_t tx_rslt = tx_semaphore_get(&(semaphore->tx_semaphore), convert_ms_to_ticks(timeout_ms));
  if (TX_NO_INSTANCE == tx_rslt)
  {
    return CY_RTOS_TIMEOUT;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param semaphore
  \param in_isr

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_set_semaphore(cy_semaphore_t *semaphore, bool in_isr)
{
  if (semaphore == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_semaphore_ceiling_put(&(semaphore->tx_semaphore), semaphore->maxcount));
}

//--------------------------------------------------------------------------------------------------
// cy_rtos_semaphore_get_count
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_semaphore_get_count(cy_semaphore_t *semaphore, size_t *count)
{
  if ((semaphore == NULL) || (count == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_semaphore_info_get(&(semaphore->tx_semaphore), TX_NULL, (ULONG *)count,TX_NULL, TX_NULL, TX_NULL));
}
/*-----------------------------------------------------------------------------------------------------


  \param semaphore

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_semaphore(cy_semaphore_t *semaphore)
{
  if (semaphore == NULL)  return CY_RTOS_BAD_PARAM;
  return convert_error(tx_semaphore_delete(&(semaphore->tx_semaphore)));
}

/*-----------------------------------------------------------------------------------------------------
 Create an event.
 This is an event which can be used to signal a set of threads with a 32 bit data element.

 @param[in,out] event Pointer to the event handle to be initialized

 @return The status of the event initialization request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_event(cy_event_t *evnt, char *name)
{
  if (evnt == NULL) return CY_RTOS_BAD_PARAM;
  return convert_error(tx_event_flags_create(evnt, name));
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
  if (evnt == NULL) return CY_RTOS_BAD_PARAM;
  return convert_error(tx_event_flags_set(evnt, bits, TX_OR));
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
  if (evnt == NULL) return CY_RTOS_BAD_PARAM;
  return convert_error(tx_event_flags_set(evnt, ~bits, TX_AND));
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
  if ((evnt == NULL) || (bits == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }
  cy_rtos_error_t tx_rslt = tx_event_flags_get(evnt, ALL_EVENT_FLAGS, TX_OR, (ULONG *)bits, TX_NO_WAIT);
  if (TX_NO_EVENTS == tx_rslt) // If timeout error occur with ALL_EVENT_FLAGS and TX_OR, then no  flag is set
  {
    *bits = 0;
    return CY_RSLT_SUCCESS;
  }
  else
  {
    return convert_error(tx_rslt);
  }
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
cy_rslt_t cy_rtos_waitbits_event(cy_event_t *evnt, uint32_t *bits, bool clear, bool all, cy_time_t timeout_ms)
{
  UINT get_option;

  if ((evnt == NULL) || (bits == NULL)) return CY_RTOS_BAD_PARAM;

  if (all)
  {
    get_option = clear ? TX_AND_CLEAR : TX_AND;
  }
  else
  {
    get_option = clear ? TX_OR_CLEAR : TX_OR;
  }

  cy_rtos_error_t tx_rslt = tx_event_flags_get(evnt,*bits, get_option, (ULONG *)bits, convert_ms_to_ticks(timeout_ms));
  if (TX_NO_EVENTS == tx_rslt)
  {
    return CY_RTOS_TIMEOUT;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Deinitialize a event.
  This function frees the resources associated with an event.

  @param[in] event Pointer to the event handle

  @return The status of the deletion request. [CY_RSLT_SUCCESS, CY_RTOS_GENERAL_ERROR]
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_event(cy_event_t *evnt)
{
  if (evnt == NULL) return CY_RTOS_BAD_PARAM;
  return convert_error(tx_event_flags_delete(evnt));
}

//--------------------------------------------------------------------------------------------------
// cy_rtos_queue_init
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_queue_init(cy_queue_t *queue, const char *name, size_t length, size_t itemsize)
{
  // Valid message lengths are {1-ULONG, 2-ULONG, 4-ULONG, 8-ULONG, 16-ULONG}
  static const uint32_t BYTES_PER_QUEUE_WORD = sizeof(ULONG);

  if ((queue == NULL) || (itemsize == 0) || (itemsize > BYTES_PER_QUEUE_WORD * MAX_QUEUE_MESSAGE_SIZE)) return CY_RTOS_BAD_PARAM;

  // round message words to next power of 2 times word size.
  UINT message_words = 1;
  while (message_words * BYTES_PER_QUEUE_WORD < itemsize) message_words <<= 1;

  queue->itemsize = itemsize;
  ULONG queue_size = length * message_words * BYTES_PER_QUEUE_WORD;
  queue->mem = App_malloc(queue_size);
  if (queue->mem == NULL) return CY_RTOS_NO_MEMORY;

  cy_rtos_error_t tx_rslt = tx_queue_create(&(queue->tx_queue), (char*)name, message_words, queue->mem, queue_size);
  if (TX_SUCCESS != tx_rslt)
  {
    cy_last_error = tx_rslt;
    App_free(queue->mem);
    return CY_RTOS_GENERAL_ERROR;
  }

  return CY_RSLT_SUCCESS;
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_queue_put
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_queue_put(cy_queue_t *queue, const void *item_ptr, cy_time_t timeout_ms)
{
  if ((queue == NULL) || (item_ptr == NULL) || (is_in_isr() && (timeout_ms != 0))) return CY_RTOS_BAD_PARAM;

  cy_rtos_error_t tx_rslt = tx_queue_send(&(queue->tx_queue), (void *)item_ptr, convert_ms_to_ticks(timeout_ms));
  if (TX_QUEUE_FULL == tx_rslt)
  {
    return CY_RTOS_NO_MEMORY;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_queue_get
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_queue_get(cy_queue_t *queue, void *item_ptr, cy_time_t timeout_ms)
{
  ULONG buffer[MAX_QUEUE_MESSAGE_SIZE];
  if ((queue == NULL) || (item_ptr == NULL) || (is_in_isr() && (timeout_ms != 0))) return CY_RTOS_BAD_PARAM;

  cy_rtos_error_t tx_rslt = tx_queue_receive(&(queue->tx_queue), buffer, convert_ms_to_ticks(timeout_ms));
  if (TX_QUEUE_EMPTY == tx_rslt)
  {
    return CY_RTOS_TIMEOUT;
  }
  else if (tx_rslt == TX_SUCCESS)
  {
    memcpy(item_ptr, (void *)buffer, queue->itemsize);
    return CY_RSLT_SUCCESS;
  }
  else
  {
    cy_last_error = tx_rslt;
    return CY_RTOS_GENERAL_ERROR;
  }
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_queue_count
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_queue_count(cy_queue_t *queue, size_t *num_waiting)
{
  if ((queue == NULL) || (num_waiting == NULL)) return CY_RTOS_BAD_PARAM;
  return convert_error(tx_queue_info_get(&(queue->tx_queue), TX_NULL, (ULONG *)num_waiting, TX_NULL, TX_NULL, TX_NULL, TX_NULL));
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_queue_space
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_queue_space(cy_queue_t *queue, size_t *num_spaces)
{
  if ((queue == NULL) || (num_spaces == NULL)) return CY_RTOS_BAD_PARAM;
  return convert_error(tx_queue_info_get(&(queue->tx_queue), TX_NULL, TX_NULL, (ULONG *)num_spaces,TX_NULL, TX_NULL, TX_NULL));
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_queue_reset
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_queue_reset(cy_queue_t *queue)
{
  if (queue == NULL) return CY_RTOS_BAD_PARAM;
  return convert_error(tx_queue_flush(&(queue->tx_queue)));
}


//--------------------------------------------------------------------------------------------------
// cy_rtos_queue_deinit
//--------------------------------------------------------------------------------------------------
cy_rslt_t cy_rtos_queue_deinit(cy_queue_t *queue)
{
  if (queue == NULL) return CY_RTOS_BAD_PARAM;
  cy_rslt_t result = convert_error(tx_queue_delete(&(queue->tx_queue)));
  if (result == CY_RSLT_SUCCESS)
  {
    App_free(queue->mem);
  }
  return result;
}

/*-----------------------------------------------------------------------------------------------------


  \param tval

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_time(cy_time_t *tval)
{

  *tval = ((1000ul * tx_time_get()) / TX_TIMER_TICKS_PER_SECOND);
  return CY_RSLT_SUCCESS;
}





