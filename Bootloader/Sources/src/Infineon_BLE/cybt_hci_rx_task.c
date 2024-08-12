#include "App.h"
#include "BLE_main.h"

#define HCI_MAX_READ_PACKET_NUM_PER_ROUND   (10)

TX_EVENT_FLAGS_GROUP   ble_rx_flags;



extern cybt_task_mem_cb_t  task_mem_cb;

static uint8_t diagnostic_msg_buf[64];

typedef struct
{
  uint32_t   header;        // 4-байтная область содержащая заголовок пакета. Должна быть воровнена по границе 4. Поэтому объявлена как UINT32_t
  uint32_t   receive_state; // Переменная состояния
  uint32_t   packet_type;   // Тип пакета (ACL, EVENT, SCO, DIAG)
  uint8_t   *prt;           // Указатель на текущюю позицию приема данных в памяти
} T_ble_rx_receiver_cbl;

static T_ble_rx_receiver_cbl  rcbl;

extern void cybt_core_stack_init(void);
extern void host_stack_platform_interface_deinit(void);

/*-----------------------------------------------------------------------------------------------------

  \return cybt_result_t
-----------------------------------------------------------------------------------------------------*/
cybt_result_t cybt_send_flags_to_hci_rx_task(uint32_t flags)
{
  tx_event_flags_set(&ble_rx_flags, flags, TX_OR);
  return CYBT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void handle_task_shutdown(void)
{
  wiced_bt_stack_shutdown();
  host_stack_platform_interface_deinit();
}

/*-----------------------------------------------------------------------------------------------------
  Функция принимающая первые байты пакета от модуля BLE

  \param uint8_t

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  cybt_byte_reciving_handler(uint8_t b)
{

  switch (rcbl.receive_state)
  {
  case 0:
    // Принимаем первый байт
    switch (b)
    {
    case HCI_PACKET_TYPE_ACL:
    case HCI_PACKET_TYPE_EVENT:
    case HCI_PACKET_TYPE_SCO:
      rcbl.packet_type = b;
      rcbl.prt = (uint8_t *)&rcbl.header;
      rcbl.receive_state++;
      return 1;
    case HCI_PACKET_TYPE_DIAG:
      cybt_send_flags_to_hci_rx_task(FLAG_BLE_RX_DIA);
      return 0;
    default:
      rcbl.packet_type   = 0;
      rcbl.prt       = 0;
      return 1;
    }
    break;
  default:
    // Здесь принимаем хидер
    *rcbl.prt = b;
    rcbl.prt++;
    rcbl.receive_state++;
    switch (rcbl.packet_type)
    {
    case HCI_PACKET_TYPE_ACL:
      if (rcbl.receive_state > sizeof(hci_acl_packet_header_t))
      {
        cybt_send_flags_to_hci_rx_task(FLAG_BLE_RX_ACL);
        rcbl.receive_state = 0;
        return 0;
      }
      return 1;
    case HCI_PACKET_TYPE_EVENT:
      if (rcbl.receive_state > sizeof(hci_event_packet_header_t))
      {
        cybt_send_flags_to_hci_rx_task(FLAG_BLE_RX_EVT);
        rcbl.receive_state = 0;
        return 0;
      }
      return 1;
    case HCI_PACKET_TYPE_SCO:
      if (rcbl.receive_state > sizeof(hci_sco_packet_header_t))
      {
        cybt_send_flags_to_hci_rx_task(FLAG_BLE_RX_SCO);
        rcbl.receive_state = 0;
        return 0;
      }
      return 1;
    }
    break;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
void cybt_hci_rx_task(cy_thread_arg_t arg)
{
  ULONG            actual_flags;
  uint8_t         *ptr;
  uint32_t         packet_header;
  uint32_t         read_len;
  cybt_result_t    result;

  cybt_core_stack_init();

  while (1)
  {
    if (tx_event_flags_get(&ble_rx_flags, 0xFFFFFFFF, TX_OR_CLEAR,&actual_flags, TX_WAIT_FOREVER) != TX_SUCCESS) return;

    if (actual_flags & FLAG_BLE_TASK_SHUTDOWN)
    {
      handle_task_shutdown();
      break;
    }
    if (actual_flags & FLAG_BLE_RX_ACL)
    {
      hci_acl_packet_header_t   *p_hci_acl_header;
      ptr = task_mem_cb.p_rx_mem;
      packet_header = rcbl.header;
      memcpy(ptr,&packet_header, sizeof(hci_acl_packet_header_t));
      p_hci_acl_header = (hci_acl_packet_header_t *)&packet_header;
      ptr += sizeof(hci_acl_packet_header_t);
      read_len = p_hci_acl_header->content_length;
      result = cybt_platform_hci_read(ptr,&read_len,MS_TO_TICKS(100));
      if (result == CYBT_SUCCESS)
      {
        wiced_bt_process_acl_data(task_mem_cb.p_rx_mem,HCI_DATA_PREAMBLE_SIZE + p_hci_acl_header->content_length);
      }
      HCITXTASK_TRACE_DEBUG("cybt_hci_rx_task(): ACL packet. len = %d, result = %d", p_hci_acl_header->content_length, result);
    }
    if (actual_flags & FLAG_BLE_RX_EVT)
    {
      hci_event_packet_header_t   *p_hci_evt_header;
      ptr = task_mem_cb.p_rx_mem;
      packet_header = rcbl.header;
      memcpy(ptr,&packet_header, sizeof(hci_event_packet_header_t));
      p_hci_evt_header = (hci_event_packet_header_t *)&packet_header;
      ptr += sizeof(hci_event_packet_header_t);
      read_len = p_hci_evt_header->content_length;
      result = cybt_platform_hci_read(ptr,&read_len,MS_TO_TICKS(100));
      if (result == CYBT_SUCCESS)
      {
        wiced_bt_process_hci_events(task_mem_cb.p_rx_mem,HCIE_PREAMBLE_SIZE + p_hci_evt_header->content_length);
      }
      HCITXTASK_TRACE_DEBUG("cybt_hci_rx_task(): EVT packet. len = %d, result = %d", p_hci_evt_header->content_length, result);
    }
    if (actual_flags & FLAG_BLE_RX_SCO)
    {
      hci_sco_packet_header_t   *p_hci_sco_header;
      ptr = task_mem_cb.p_rx_mem;
      packet_header = rcbl.header;
      memcpy(ptr,&packet_header, sizeof(hci_sco_packet_header_t));
      p_hci_sco_header = (hci_sco_packet_header_t *)&packet_header;
      ptr += sizeof(hci_sco_packet_header_t);
      read_len = p_hci_sco_header->content_length;
      result = cybt_platform_hci_read(ptr,&read_len,MS_TO_TICKS(100));
      if (result == CYBT_SUCCESS)
      {
        wiced_bt_process_sco_data(task_mem_cb.p_rx_mem,HCI_SCO_PREAMBLE_SIZE + p_hci_sco_header->content_length);
      }
      HCITXTASK_TRACE_DEBUG("cybt_hci_rx_task(): SCO packet. len = %d, result = %d", p_hci_sco_header->content_length, result);
    }
    if (actual_flags & FLAG_BLE_RX_DIA)
    {
      // Здесь принимаем отладочный пакет.
      // Пока его некуда девать и просто ничего с ним не делаем.
      read_len = 63;
      result = cybt_platform_hci_read((uint8_t *)&diagnostic_msg_buf,&read_len,MS_TO_TICKS(100));
      HCITXTASK_TRACE_DEBUG("cybt_hci_rx_task(): Diag packet. result = %d", result);
    }
    if (actual_flags & FLAG_BLE_TIMER_EXPIRED)
    {
      wiced_bt_process_timer();
    }

  }
  return;
}

