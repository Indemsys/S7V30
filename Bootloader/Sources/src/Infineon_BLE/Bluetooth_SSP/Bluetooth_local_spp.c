#include "App.h"
#include "BLE_main.h"


T_local_spp                     local_spp_cbl;
wiced_bool_t                    spp_initialized = WICED_FALSE;
uint32_t                        spp_rx_bytes       = 0; // Счетчик принятых в текущей сессии байт
static uint8_t                 *p_rx_fifo          = NULL;

static T_bt_spp_callback        spp_rx_callback;
static T_bt_spp_callback        spp_tx_cmpl_callback;

/*-----------------------------------------------------------------------------------------------------


  \param func
-----------------------------------------------------------------------------------------------------*/
void BT_ssp_set_rx_callback(T_bt_spp_callback func)
{
  spp_rx_callback = func;
}


/*-----------------------------------------------------------------------------------------------------


  \param func
-----------------------------------------------------------------------------------------------------*/
void BT_ssp_set_tx_cmpl_callback(T_bt_spp_callback func)
{
  spp_tx_cmpl_callback = func;
}


/*-----------------------------------------------------------------------------------------------------
  Process RFCOMM events

  Функция вызывается библиотекой btstack в ответ на события после открытия порта в RFCOMM
  Вызывается в контексте задачи cybt_hci_rx_task

  \param event
  \param handle
-----------------------------------------------------------------------------------------------------*/
static void _spp_port_event_callback(wiced_bt_rfcomm_port_event_t evt_mask, uint16_t handle)
{
  // Обработка события приема данных
  if (evt_mask & PORT_EV_RXCHAR)
  {
    if (spp_rx_callback) spp_rx_callback(handle);
  }

  // Обработка события управления потоком
  if (evt_mask & PORT_EV_FC)
  {
    if (evt_mask & PORT_EV_FCS)
    {
      local_spp_cbl.data_flow_enabled = 1;
    }
    else
    {
      local_spp_cbl.data_flow_enabled = 0;
    }
  }

  // Обработка события освобождения очереди на передачу
  if (evt_mask & PORT_EV_TXEMPTY)
  {
    // Разрешаем потоко передачи
    local_spp_cbl.data_flow_enabled = 1;
  }

  if (evt_mask & PORT_EV_ERR)
  {
    local_spp_cbl.event_error = PORT_EV_ERR;
  }
  else
  {
    local_spp_cbl.event_error = PORT_EV_NONE;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param handle
  \param p_data
-----------------------------------------------------------------------------------------------------*/
static void _spp_rfcomm_port_tx_cmpl_cback(uint16_t handle, void *p_data)
{
  if (spp_tx_cmpl_callback) spp_tx_cmpl_callback(handle);
}

/*-----------------------------------------------------------------------------------------------------
  Handle RFCOMM channel opened.

  Функция вызывается из BT_spp_rfcomm_control_callback в момент открытия порта

  \param p_scb
-----------------------------------------------------------------------------------------------------*/
static void _spp_rfcomm_opened(T_local_spp *p_scb)
{
  p_scb->state = SPP_SESSION_STATE_OPEN;
  spp_rx_bytes = 0;

  Monitor_Bluetooth_Init_callback(p_scb);
  // Здесь устанавливаем функцию принимающую события и функцию вызываемую при окончании отправки данных
  wiced_bt_rfcomm_set_event_callback(p_scb->rfc_conn_handle, _spp_port_event_callback, _spp_rfcomm_port_tx_cmpl_cback);
}

/*-----------------------------------------------------------------------------------------------------
  RFCOMM connection closed.

  \param p_scb
-----------------------------------------------------------------------------------------------------*/
static void _spp_rfcomm_closed(T_local_spp *p_scb)
{
  Monitor_Bluetooth_DeInit_callback(p_scb);
  /* Clear peer bd_addr */
  memset(p_scb->server_addr, 0, BD_ADDR_LEN);

  p_scb->state             = SPP_SESSION_STATE_IDLE;
  p_scb->rfc_conn_handle   = 0;
  p_scb->data_flow_enabled = 0;

  /* Reopen server if needed */
  BT_spp_rfcomm_start_server();
}

/*-----------------------------------------------------------------------------------------------------
  Handle RFCOMM channel opened when accepting connection.

  \param p_scb
-----------------------------------------------------------------------------------------------------*/
static void _spp_rfcomm_acceptor_opened(T_local_spp *p_scb)
{
  uint16_t   lcid;
  uint32_t   status;

  /* set role and connection handle */
  p_scb->b_is_initiator  = WICED_FALSE;
  p_scb->rfc_conn_handle = p_scb->rfc_serv_handle;

  status = wiced_bt_rfcomm_check_connection(p_scb->rfc_conn_handle, p_scb->server_addr,&lcid);
  /* get bd addr of peer */
  if (status != WICED_BT_RFCOMM_SUCCESS)
  {
    _spp_rfcomm_closed(p_scb);
  }
  else
  {
    /* continue with common open processing */
    _spp_rfcomm_opened(p_scb);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Prepare extended inquiry response data.  Current version publishes device name and 16bit SPP service.

-----------------------------------------------------------------------------------------------------*/
static void _spp_write_eir(void)
{
  uint8_t   *pBuf;
  uint8_t   *p;
  uint8_t    length;
  uint16_t   eir_length;

  pBuf = (uint8_t *)wiced_bt_get_buffer(WICED_EIR_BUF_MAX_SIZE);
  p = pBuf;

  length = strlen((char const *)ivar.bt_device_name);

  *p++ = length + 1;
  *p++ = BT_EIR_COMPLETE_LOCAL_NAME_TYPE;         // EIR type full name
  memcpy(p, ivar.bt_device_name, length);
  p += length;

  *p++ = 2 + 1;                                   // Length of 16 bit services
  *p++ = BT_EIR_COMPLETE_16BITS_UUID_TYPE;        // 0x03 EIR type full list of 16 bit service UUIDs
  *p++ = UUID_SERVCLASS_SERIAL_PORT & 0xff;
  *p++ = (UUID_SERVCLASS_SERIAL_PORT >> 8) & 0xff;
  *p++ = 0;                                       // end of EIR Data is 0

  eir_length = (uint16_t)(p - pBuf);

  wiced_bt_dev_write_eir(pBuf, eir_length); // Здесь получаем ошибку. Код 8102 (NO_RESOURCES) - Недостаточно ресурсов
                                            // Однако команта оказывает действие на представление устройства в Windows
  wiced_bt_free_buffer(pBuf);
  return;
}

/*-----------------------------------------------------------------------------------------------------
  RFCOMM management callback

  Фнкция первой в этом стеке вызывается при открытии и закрытии COM порта

  Если порт открывается, то port_status == WICED_BT_RFCOMM_SUCCESS
  Если порт открывается, то port_status == WICED_BT_RFCOMM_CLOSED

  Вызывается в контексте задачи cybt_hci_rx_task


  \param port_status
  \param port_handle
-----------------------------------------------------------------------------------------------------*/
void BT_spp_rfcomm_control_callback(uint32_t port_status, uint16_t port_handle)
{


  /* ignore close event for port handles other than connected handle */
  if ((port_status != WICED_BT_RFCOMM_SUCCESS) && (port_handle != local_spp_cbl.rfc_conn_handle))
  {
    return;
  }


  if ((port_status == WICED_BT_RFCOMM_SUCCESS) && (local_spp_cbl.state != SPP_SESSION_STATE_CLOSING))
  {
    // При открытии порта p_scb->state = SPP_SESSION_STATE_IDLE
    // Здесь настраиваем FIFO приемника
    p_rx_fifo = wiced_bt_get_buffer(SPP_TX_BUFFER_SIZE * 2);
    if (p_rx_fifo != NULL)
    {
      wiced_bt_rfcomm_set_rx_fifo(port_handle, (char *)p_rx_fifo, SPP_TX_BUFFER_SIZE * 2);
    }
    else
    {
      wiced_bt_rfcomm_set_rx_fifo(port_handle, NULL, 0);
    }

    // Устанавливаем маску событий которые будут передаваться в функцию _spp_port_event_cback
    // Функция _spp_port_event_cback обрабатывает все события связанные с приемом и передачей данных
    wiced_bt_rfcomm_set_event_mask(port_handle, (wiced_bt_rfcomm_port_event_t)PORT_MASK_ALL);

    if (local_spp_cbl.state == SPP_SESSION_STATE_IDLE)
    {
      // Порт открывается из закрытого состояния
      _spp_rfcomm_acceptor_opened(&local_spp_cbl);
    }
    else
    {
      _spp_rfcomm_opened(&local_spp_cbl);
    }
  }
  else
  {
    if (port_status == WICED_BT_RFCOMM_CLOSED)
    {
      if (p_rx_fifo != NULL)
      {
        wiced_bt_free_buffer(p_rx_fifo);
        p_rx_fifo = NULL;
      }
    }

    // Во всех случаях, кроме случая успешного открытия порта WICED_BT_RFCOMM_SUCCESS, порт закрываем
    _spp_rfcomm_closed(&local_spp_cbl);
  }
}


/*-----------------------------------------------------------------------------------------------------
  Setup RFCOMM server for use by HS.

  \param p_scb
-----------------------------------------------------------------------------------------------------*/
uint32_t BT_spp_rfcomm_start_server(void)
{
  wiced_bt_rfcomm_result_t res;

  local_spp_cbl.state = SPP_SESSION_STATE_IDLE;

  if (local_spp_cbl.rfc_serv_handle == RFCOMM_INVALID_HANDLE)
  {
    local_spp_cbl.rfc_serv_handle = 0;
    return RES_ERROR;
  }
  else if (!local_spp_cbl.rfc_serv_handle)
  {
    res = wiced_bt_rfcomm_create_connection(UUID_SERVCLASS_SERIAL_PORT,
                                            1,  // Номер канала
                                            WICED_TRUE,
                                            SPP_TX_BUFFER_SIZE,
                                            NULL, //addr_of_peer,
                                            &local_spp_cbl.rfc_serv_handle,
                                            (wiced_bt_port_mgmt_cback_t *)BT_spp_rfcomm_control_callback);
    if (res != WICED_BT_SUCCESS)
    {
      return RES_ERROR;
    }
    return RES_OK;
  }
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в обработчике события BTM_ENABLED_EVT

  \param n      - индекс открываемого канала.

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
uint32_t BT_spp_startup(void)
{
  if (local_spp_cbl.in_use == WICED_TRUE) return WICED_BT_NO_RESOURCES;

  if (!spp_initialized)
  {
    memset(&local_spp_cbl, 0, sizeof(local_spp_cbl));

    _spp_write_eir();

    wiced_bt_sdp_db_init((uint8_t *)sdp_database, sdp_database_len);
    wiced_bt_dev_set_discoverability(BTM_GENERAL_DISCOVERABLE,WICED_BT_CFG_DEFAULT_INQUIRY_SCAN_WINDOW,WICED_BT_CFG_DEFAULT_INQUIRY_SCAN_INTERVAL);
    wiced_bt_dev_set_connectability(BTM_CONNECTABLE,WICED_BT_CFG_DEFAULT_PAGE_SCAN_WINDOW,WICED_BT_CFG_DEFAULT_PAGE_SCAN_INTERVAL);

    spp_initialized = WICED_TRUE;
  }

  local_spp_cbl.in_use    = WICED_TRUE;

  return BT_spp_rfcomm_start_server();
}

