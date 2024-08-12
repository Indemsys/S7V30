#include "App.h"
#include "BLE_main.h"



cy_thread_t cybt_task[BT_TASK_NUM]       = {0};

cybt_task_mem_cb_t  task_mem_cb          = {0};


extern TX_EVENT_FLAGS_GROUP   ble_rx_flags;

extern TX_QUEUE               ble_tx_queue;
extern void*                  ble_tx_queue_buff[];
/*-----------------------------------------------------------------------------------------------------


  \param p_arg

  \return cybt_result_t
-----------------------------------------------------------------------------------------------------*/
cybt_result_t cybt_platform_task_init(void *p_arg)
{
  extern void cybt_hci_tx_task(cy_thread_arg_t arg);
  extern void cybt_hci_rx_task(cy_thread_arg_t arg);

  cy_rslt_t cy_result;

  task_mem_cb.p_tx_cmd_mem   = (uint8_t *) cybt_platform_malloc(CYBT_TX_CMD_MEM_SIZE);
  task_mem_cb.p_rx_mem       = (uint8_t *) cybt_platform_malloc(CYBT_RX_MEM_MIN_SIZE);
  task_mem_cb.p_tx_data_heap = (wiced_bt_heap_t *)cybt_platform_malloc(CYBT_TX_HEAP_MIN_SIZE);

  if ((task_mem_cb.p_tx_cmd_mem == NULL) || (task_mem_cb.p_rx_mem == NULL) || (task_mem_cb.p_tx_data_heap == NULL))
  {
    cybt_platform_task_mempool_deinit();
    return CYBT_ERR_INIT_MEMPOOL_FAILED;
  }
  task_mem_cb.p_tx_data_heap = wiced_bt_create_heap("CYBT_TASK_TX_POOL",task_mem_cb.p_tx_data_heap, CYBT_TX_HEAP_MIN_SIZE, NULL, WICED_FALSE);

  cy_result = tx_queue_create(&ble_tx_queue, "BLE_TX", HCI_TX_TASK_QUEUE_ITEM_SIZE, ble_tx_queue_buff, HCI_TX_TASK_QUEUE_ITEM_SIZE*HCI_TX_TASK_QUEUE_COUNT);
  if (cy_result !=CY_RSLT_SUCCESS )
  {
    MAIN_TRACE_ERROR("task_init(): Init hci_tx task queue failed (0x%x)",cy_result);
    return CYBT_ERR_INIT_QUEUE_FAILED;
  }

  cy_result = tx_event_flags_create(&ble_rx_flags, "ble_rx");
  if (cy_result != CY_RSLT_SUCCESS)
  {
    MAIN_TRACE_ERROR("task_init(): Init hci_rx ble_rx_flags failed (0x%x)",cy_result);
    return CYBT_ERR_GENERIC;
  }

  cy_result = cy_rtos_create_thread(&cybt_task[BT_TASK_ID_HCI_TX],cybt_hci_tx_task,BT_TASK_NAME_HCI_TX,NULL,HCI_TX_TASK_STACK_SIZE,HCI_TX_TASK_PRIORITY,(cy_thread_arg_t) NULL);
  if (cy_result != CY_RSLT_SUCCESS)
  {
    MAIN_TRACE_ERROR("task_init(): Create hci_tx task failed (0x%x)",cy_result);
    return CYBT_ERR_CREATE_TASK_FAILED;
  }

  cy_result = cy_rtos_create_thread(&cybt_task[BT_TASK_ID_HCI_RX],cybt_hci_rx_task,BT_TASK_NAME_HCI_RX,NULL,HCI_RX_TASK_STACK_SIZE,HCI_RX_TASK_PRIORITY,(cy_thread_arg_t) NULL);
  if (cy_result != CY_RSLT_SUCCESS)
  {
    MAIN_TRACE_ERROR("task_init(): Create hci_rx task failed (0x%x)",cy_result);
    return CYBT_ERR_CREATE_TASK_FAILED;
  }

  return CYBT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------



  \return cybt_result_t
-----------------------------------------------------------------------------------------------------*/
cybt_result_t cybt_platform_task_deinit(void)
{

  MAIN_TRACE_DEBUG("cybt_platform_task_deinit()");

  cybt_send_flags_to_hci_rx_task(FLAG_BLE_TASK_SHUTDOWN);
  cybt_send_msg_to_hci_tx_task((BT_MSG_HDR *)BT_IND_TO_TASK_SHUTDOWN, false);

  cybt_platform_task_mempool_deinit();

  return CYBT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------


  \param req_size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* cybt_platform_task_tx_mempool_alloc(uint32_t req_size)
{
  TX_INTERRUPT_SAVE_AREA;
  void *p_mem_block;

  TX_DISABLE;
  p_mem_block = (void *) wiced_bt_get_buffer_from_heap(task_mem_cb.p_tx_data_heap,req_size);
  TX_RESTORE;
  return p_mem_block;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* cybt_platform_task_get_tx_cmd_mem(void)
{
  return (void *) task_mem_cb.p_tx_cmd_mem;
}


/*-----------------------------------------------------------------------------------------------------


  \param p_mem_block
-----------------------------------------------------------------------------------------------------*/
void cybt_platform_task_mempool_free(void *p_mem_block)
{
  TX_INTERRUPT_SAVE_AREA;
  TX_DISABLE;
  wiced_bt_free_buffer((wiced_bt_buffer_t *) p_mem_block);
  TX_RESTORE;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void cybt_platform_task_mempool_deinit(void)
{
  MEM_TRACE_DEBUG("task_mempool_deinit()");

  if (task_mem_cb.p_rx_mem)
  {
    cybt_platform_free(task_mem_cb.p_rx_mem);
  }

  if (task_mem_cb.p_tx_data_heap)
  {
    wiced_bt_delete_heap(task_mem_cb.p_tx_data_heap);
    cybt_platform_free(task_mem_cb.p_tx_data_heap);
  }

  if (task_mem_cb.p_tx_cmd_mem)
  {
    cybt_platform_free(task_mem_cb.p_tx_cmd_mem);
  }

  memset(&task_mem_cb, 0, sizeof(cybt_task_mem_cb_t));
}



/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void cybt_platform_terminate_hci_tx_thread(void)
{
  cy_rslt_t cy_result;
  cy_result = cy_rtos_join_thread(&cybt_task[BT_TASK_ID_HCI_TX]);
  if (CY_RSLT_SUCCESS != cy_result)
  {
    MAIN_TRACE_ERROR("terminate HCI_TX thread failed 0x%x\n", cy_result);
  }
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void cybt_platform_terminate_hci_rx_thread(void)
{
  cy_rslt_t cy_result;
  cy_result = cy_rtos_join_thread(&cybt_task[BT_TASK_ID_HCI_RX]);
  if (CY_RSLT_SUCCESS != cy_result)
  {
    MAIN_TRACE_ERROR("terminate HCI_RX thread failed 0x%x\n", cy_result);
  }
}

