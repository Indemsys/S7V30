#include "App.h"
#include "BLE_main.h"


TX_QUEUE   ble_tx_queue;
void*      ble_tx_queue_buff[HCI_TX_TASK_QUEUE_COUNT];


/*-----------------------------------------------------------------------------------------------------


  \param p_bt_msg
  \param is_from_isr

  \return cybt_result_t
-----------------------------------------------------------------------------------------------------*/
cybt_result_t cybt_send_msg_to_hci_tx_task(BT_MSG_HDR *p_bt_msg,bool is_from_isr)
{
  cy_rslt_t result;

  if (NULL == p_bt_msg)
  {
    return CYBT_ERR_BADARG;
  }

  result = tx_queue_send(&ble_tx_queue, (void *)&p_bt_msg, 0);

  if (CY_RSLT_SUCCESS != result)
  {
    if (false == is_from_isr)
    {
      HCITXTASK_TRACE_ERROR("send_msg_to_hci_tx_task(): send failure (0x%x)",result);
    }
    return CYBT_ERR_SEND_QUEUE_FAILED;
  }
  return CYBT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_bt_msg
-----------------------------------------------------------------------------------------------------*/
void handle_hci_tx_command(BT_MSG_HDR *p_bt_msg)
{
  uint8_t *p_hci_payload;
  cybt_result_t result;

  HCITXTASK_TRACE_DEBUG("handle_hci_tx_command(): msg = 0x%x, len = %d",p_bt_msg,p_bt_msg->length);

  p_hci_payload = (uint8_t *)(p_bt_msg + 1);
  result = cybt_platform_hci_write(HCI_PACKET_TYPE_COMMAND,p_hci_payload,p_bt_msg->length);
  if (CYBT_SUCCESS != result)
  {
    HCITXTASK_TRACE_ERROR("handle_hci_tx_command(): hci write failed (0x%x)",result);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param p_bt_msg
-----------------------------------------------------------------------------------------------------*/
void handle_hci_tx_acl(BT_MSG_HDR *p_bt_msg)
{
  uint8_t *p_hci_payload;
  cybt_result_t result;

  HCITXTASK_TRACE_DEBUG("handle_hci_tx_acl(): msg = 0x%x, len = %d",p_bt_msg,p_bt_msg->length);

  p_hci_payload = (uint8_t *)(p_bt_msg + 1);
  result = cybt_platform_hci_write(HCI_PACKET_TYPE_ACL,p_hci_payload,p_bt_msg->length);
  if (CYBT_SUCCESS != result)
  {
    HCITXTASK_TRACE_ERROR("handle_hci_tx_acl(): hci write failed (0x%x)",result);
  }

  cybt_platform_task_mempool_free((void *) p_bt_msg);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_bt_msg
-----------------------------------------------------------------------------------------------------*/
void handle_hci_tx_sco(BT_MSG_HDR *p_bt_msg)
{
  uint8_t *p_hci_payload;
  cybt_result_t result;

  HCITXTASK_TRACE_DEBUG("handle_hci_tx_sco(): msg = 0x%x, len = %d",p_bt_msg,p_bt_msg->length);

  p_hci_payload = (uint8_t *)(p_bt_msg + 1);
  result = cybt_platform_hci_write(HCI_PACKET_TYPE_SCO,p_hci_payload,p_bt_msg->length);
  if (CYBT_SUCCESS != result)
  {
    HCITXTASK_TRACE_ERROR("handle_hci_tx_sco(): hci write failed (0x%x)",result);
  }

  cybt_platform_task_mempool_free((void *) p_bt_msg);
}

/*-----------------------------------------------------------------------------------------------------
  Задача создается в функции cybt_platform_task_init

  \param arg
-----------------------------------------------------------------------------------------------------*/
void cybt_hci_tx_task(cy_thread_arg_t arg)
{
  HCITXTASK_TRACE_DEBUG("hci_tx_task(): start");

  UART_BLE_set_byte_recieving_handler(cybt_byte_reciving_handler);

  while (1)
  {
    cy_rslt_t  result;
    BT_MSG_HDR *p_bt_msg = NULL;

    result = tx_queue_receive(&ble_tx_queue,(void *)&p_bt_msg, TX_WAIT_FOREVER);

    if ((result != CY_RSLT_SUCCESS) || (p_bt_msg == NULL) )
    {
      HCITXTASK_TRACE_WARNING("hci_tx_task(): queue error (0x%x)", result);
      continue;
    }

    if ((uint32_t)p_bt_msg == BT_IND_TO_TASK_SHUTDOWN)
    {
      while (tx_queue_receive(&ble_tx_queue,(void *)&p_bt_msg,0) == CY_RSLT_SUCCESS)
      {
        if ((uint32_t)p_bt_msg != FLAG_BLE_TASK_SHUTDOWN)
        {
          cybt_platform_task_mempool_free((void *) p_bt_msg);
        }
      }
      tx_queue_delete(&ble_tx_queue);
      break;
    }

    switch (p_bt_msg->event)
    {
    case BT_EVT_TO_HCI_COMMAND:
      handle_hci_tx_command(p_bt_msg);
      break;
    case BT_EVT_TO_HCI_ACL:
      handle_hci_tx_acl(p_bt_msg);
      break;
    case BT_EVT_TO_HCI_SCO:
      handle_hci_tx_sco(p_bt_msg);
      break;
    default:
      HCITXTASK_TRACE_ERROR("hci_tx_task(): Unknown event (0x%x)", p_bt_msg->event);
      cybt_platform_task_mempool_free((void *) p_bt_msg);
      break;
    }

  }

  cybt_platform_hci_close();
  return;
}

