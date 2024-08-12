#ifndef CYBT_PLATFORM_HCI_H
  #define CYBT_PLATFORM_HCI_H

typedef enum
{
  CYBT_SUCCESS = 0,
  CYBT_ERR_BADARG = 0xB1,
  CYBT_ERR_OUT_OF_MEMORY,
  CYBT_ERR_TIMEOUT,
  CYBT_ERR_HCI_INIT_FAILED,
  CYBT_ERR_HCI_UNSUPPORTED_IF,
  CYBT_ERR_HCI_UNSUPPORTED_BAUDRATE,
  CYBT_ERR_HCI_NOT_INITIALIZE,
  CYBT_ERR_HCI_WRITE_FAILED,
  CYBT_ERR_HCI_READ_FAILED,
  CYBT_ERR_HCI_GET_TX_MUTEX_FAILED,
  CYBT_ERR_HCI_GET_RX_MUTEX_FAILED,
  CYBT_ERR_HCI_SET_BAUDRATE_FAILED,
  CYBT_ERR_HCI_SET_FLOW_CTRL_FAILED,
  CYBT_ERR_INIT_MEMPOOL_FAILED,
  CYBT_ERR_INIT_QUEUE_FAILED,
  CYBT_ERR_CREATE_TASK_FAILED,
  CYBT_ERR_SEND_QUEUE_FAILED,
  CYBT_ERR_MEMPOOL_NOT_INITIALIZE,
  CYBT_ERR_QUEUE_ALMOST_FULL,
  CYBT_ERR_QUEUE_FULL,
  CYBT_ERR_GPIO_POWER_INIT_FAILED,
  CYBT_ERR_GPIO_DEV_WAKE_INIT_FAILED,
  CYBT_ERR_GPIO_HOST_WAKE_INIT_FAILED,
  CYBT_ERR_GENERIC
} cybt_result_t;

/******************************************************************************
 *                                Constants
 ******************************************************************************/
  #define HCI_UART_TYPE_HEADER_SIZE  (1)         /**< HCI UART header size */



/*****************************************************************************
 *                           Type Definitions
 *****************************************************************************/
/**
 * HCI packet type
 */
typedef enum
{
  HCI_PACKET_TYPE_IGNORE   = 0x00,
  HCI_PACKET_TYPE_COMMAND  = 0x01,
  HCI_PACKET_TYPE_ACL      = 0x02,
  HCI_PACKET_TYPE_SCO      = 0x03,
  HCI_PACKET_TYPE_EVENT    = 0x04,
  HCI_PACKET_TYPE_DIAG     = 0x07,
  HCI_PACKET_TYPE_LOOPBACK = 0xFF
} hci_packet_type_t;

/**
 * HCI Event packet header
 */
typedef struct
{
    uint8_t           event_code;
    uint8_t           content_length;
} hci_event_packet_header_t;

/**
 * HCI ACL packet header
 */
typedef struct
{
    uint16_t          hci_handle;
    uint16_t          content_length;
} hci_acl_packet_header_t;

/**
 * HCI SCO packet header
 */
typedef struct
{
    uint16_t          hci_handle;
    uint8_t           content_length;
} hci_sco_packet_header_t;

/**
 * HCI Loopback packet header
 */
typedef struct
{
    uint8_t           content_length;
} hci_loopback_packet_header_t;


  #ifdef __cplusplus
extern "C"
{
  #endif

  /*****************************************************************************
   *                           Function Declarations
   ****************************************************************************/



  /**
   * Set the new baudrate of HCI UART trasnport.
   *
   * @param[in] req_size: the requested size of memory
   *
   * @returns the status of set baudrate operation
   */
  cybt_result_t cybt_platform_hci_set_baudrate(uint32_t baudrate);


  /**
   * Write data to HCI transport.
   *
   * Note: This fuction shall be returned only when all data was written done.
   *
   * @param[in] type  : HCI packet type
   * @param[in] p_data: the pointer of the data to be written
   * @param[in] lenght: the length of the data to be written
   *
   * @returns the status of write operation
   */
  cybt_result_t cybt_platform_hci_write(hci_packet_type_t type,uint8_t *p_data,uint32_t length);


  /**
   * Read data from HCI transpot.
   *

   * @param[in] p_data: the pointer of received buffer
   * @param[in/out] p_length: The pointer of requested/actual length.
   *                          The request read length shall be specified
   *                          through this parameter. The actual read length
   *                          shall be provided in the same parameter,
   *                          along with function return.
   * @param[in] timeout_ms: the timeout (millisecond) to wait to
   *                        receive data from HCI transport
   *
   * @returns the status of read operation
   */
  cybt_result_t cybt_platform_hci_read(uint8_t *p_data,uint32_t *p_length,uint32_t timeout_ms);


  /**
   * Close HCI transport.
   *
   * @returns the status of close operation
   */
  cybt_result_t cybt_platform_hci_close(void);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

