#ifndef CYBT_PLATFORM_TASK_H
  #define CYBT_PLATFORM_TASK_H

  #include <stdint.h>
  #include <stdbool.h>

  #include "cyabs_rtos.h"
  #include "cybt_platform_hci.h"
  #include "thread_priorities.h"

/*****************************************************************************
 *                                Constants
 *****************************************************************************/
  #define BT_TASK_ID_HCI_RX                   (0)
  #define BT_TASK_ID_HCI_TX                   (1)
  #define BT_TASK_NUM                         (2)

  #define BT_TASK_NAME_HCI_RX                 "CYBT_HCI_RX_Task"
  #define BT_TASK_NAME_HCI_TX                 "CYBT_HCI_TX_Task"


  #define HCI_RX_TASK_STACK_SIZE              (2048)
  #define HCI_TX_TASK_STACK_SIZE              (2048)

  #define CYBT_INVALID_HEAP_UTILIZATION       (0xFF)
  #define CYBT_INVALID_QUEUE_UTILIZATION      (0xFF)

  #define HCI_TX_UNLOCK_THRESHOLD_TX_HEAP_IN_PERCENT    (50)
  #define HCI_TX_UNLOCK_THRESHOLD_HCITX_Q_IN_PERCENT    (50)

/*****************************************************************************
 *                           Type Definitions
 *****************************************************************************/

/**
 * BT event (with message header and payload)
 */
  #define BT_EVT_TO_BTD_EVENT                 (0x0201)
  #define BT_EVT_TO_BTD_ACL                   (0x0202)
  #define BT_EVT_TO_BTD_SCO                   (0x0203)

  #define BT_EVT_TO_HCI_COMMAND               (0x0401)
  #define BT_EVT_TO_HCI_ACL                   (0x0402)
  #define BT_EVT_TO_HCI_SCO                   (0x0403)
  #define BT_EVT_INVALID                      (0xFFFF)
typedef uint16_t bt_task_event_t;


  #define BT_IND_TO_TASK_SHUTDOWN                (1)
  #define BT_IND_TO_HCI_DATA_READY_ACL           (2)
  #define BT_IND_TO_HCI_DATA_READY_SCO           (3)
  #define BT_IND_TO_HCI_DATA_READY_EVT           (4)

  #define FLAG_BLE_DATA_READY                    (1<<1)
  #define FLAG_BLE_TASK_SHUTDOWN                 (1<<2)
  #define FLAG_BLE_TIMER_EXPIRED                 (1<<3)
  #define FLAG_BLE_RX_ACL                        (1<<4)
  #define FLAG_BLE_RX_EVT                        (1<<5)
  #define FLAG_BLE_RX_SCO                        (1<<6)
  #define FLAG_BLE_RX_DIA                        (1<<7)


  #define BT_IND_INVALID                      (0xFFFFFFFF)


/**
 * Message structure is used to communicate between tasks
 */
typedef struct
{
    bt_task_event_t  event;    /**< event id */
    uint16_t         length;   /**< payload length */
} BT_MSG_HDR;



/**
 * Message header size
 */
  #define BT_MSG_HDR_SIZE             (sizeof(BT_MSG_HDR))

  #define HCI_EVT_MSG_HDR_SIZE        (BT_MSG_HDR_SIZE + HCIE_PREAMBLE_SIZE)
  #define HCI_ACL_MSG_HDR_SIZE        (BT_MSG_HDR_SIZE + HCI_DATA_PREAMBLE_SIZE)
  #define HCI_SCO_MSG_HDR_SIZE        (BT_MSG_HDR_SIZE + HCI_SCO_PREAMBLE_SIZE)


  #define HCI_RX_TASK_QUEUE_COUNT      (32)
  #define HCI_TX_TASK_QUEUE_COUNT      (32)
  #define HCI_RX_TASK_QUEUE_ITEM_SIZE  (sizeof(uint32_t))
  #define HCI_TX_TASK_QUEUE_ITEM_SIZE  (sizeof(BT_MSG_HDR *))

  #define CYBT_HCI_TX_NORMAL                    (0)
  #define CYBT_HCI_TX_BLOCKED_HEAP_RAN_OUT      (1 << 0)
  #define CYBT_HCI_TX_BLOCKED_QUEUE_FULL_CMD    (1 << 1)
  #define CYBT_HCI_TX_BLOCKED_QUEUE_FULL_ACL    (1 << 2)
typedef uint8_t cybt_hci_tx_status_t;

  #ifdef __cplusplus
extern "C"
{
  #endif

  /*****************************************************************************
   *                           Function Declarations
   ****************************************************************************/

  /**
   * Initialize Bluetooth related OS tasks.
   *
   * @param[in] p_arg: pointer to an argument if any
   *
   * @returns  CYBT_SUCCESS
   *           CYBT_ERR_INIT_MEMPOOL_FAILED
   *           CYBT_ERR_INIT_QUEUE_FAILED
   *           CYBT_ERR_CREATE_TASK_FAILED
   */
  cybt_result_t cybt_platform_task_init(void *p_arg);


  /**
   * Delete Bluetooth related OS tasks.
   *
   * @returns  CYBT_SUCCESS
   *           CYBT_ERR_OUT_OF_MEMORY
   *           CYBT_ERR_SEND_QUEUE_FAILED
   */
  cybt_result_t cybt_platform_task_deinit(void);



  /**
   * Allocate the memory block from task memory pool.
   *
   * @param[in] req_size: the request size of memory block
   *
   * @returns  the pointer of memory block
   *
   */
  void* cybt_platform_task_tx_mempool_alloc(uint32_t req_size);


  /**
   * Get the start address of pre-allocated memory block for HC transmitting packet.
   *
   * @returns  the pointer of memory block
   *
   */
  void* cybt_platform_task_get_tx_cmd_mem(void);


  /**
   * Free and return the memory block to task tx/rx memory pool.
   *
   * @param[in]   p_mem_block: the pointer of memory block
   *
   * @returns     void
   *
   */
  void cybt_platform_task_mempool_free(void *p_mem_block);


  /**
   * Release task memory pool.
   *
   * @returns     void
   */
  void cybt_platform_task_mempool_deinit(void);


  cybt_result_t cybt_send_flags_to_hci_rx_task(uint32_t bt_ind_msg);


  /**
   * Send message to HCI TX task.
   *
   * @param[in] p_bt_msg    : the pointer of the message
   * @param[in] is_from_isr : true if this function is called from isr context
   *                         otherwise false
   *
   * @returns  CYBT_SUCCESS
   *           CYBT_ERR_BADARG
   *           CYBT_ERR_SEND_QUEUE_FAILED
   */
  cybt_result_t cybt_send_msg_to_hci_tx_task(BT_MSG_HDR *p_bt_msg,bool is_from_isr);

  /**
   * Termiate HCI-TX task.
   *
   * @returns     void
   */
  void cybt_platform_terminate_hci_tx_thread(void);


  /**
   * Termiate HCI-RX task.
   *
   * @returns     void
   */
  void cybt_platform_terminate_hci_rx_thread(void);

  uint32_t  cybt_byte_reciving_handler(uint8_t b);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

