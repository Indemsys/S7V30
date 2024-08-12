// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-06-04
// 17:27:06
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BLE_main.h"


T_gatt_connection_cbl  gatt_conn_list[MAX_BLE_CONNECTION_NUMBER];
uint32_t               gatt_conn_cnt;


T_gatt_callbacks *gatt_cb[GATT_CALLBACKS_NUM];


/*-----------------------------------------------------------------------------------------------------



  \return int32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t  _Get_free_gat_conn_indx(void)
{
  for (uint32_t i = 0; i < MAX_BLE_CONNECTION_NUMBER; i++)
  {
    if (gatt_conn_list[i].active == 0)
    {
      return i;
    }
  }
  return -1;
}




/*-----------------------------------------------------------------------------------------------------
  This function searches through the GATT DB to point to the attribute corresponding to the given handle

  \param handle - Handle to search for in the GATT DB

  \return gatt_db_lookup_table_t* - Pointer to the correct attribute in the GATT DB
-----------------------------------------------------------------------------------------------------*/
static gatt_db_lookup_table_t* _Get_attribute_by_handle(uint16_t handle)
{
  // Search for the given handle in the GATT DB and return the pointer to the correct attribute
  uint8_t array_index = 0;

  for (array_index = 0; array_index < app_gatt_db_ext_attr_tbl_size; array_index++)
  {
    if (app_gatt_db_ext_attr_tbl[array_index].handle == handle)
    {
      return (&app_gatt_db_ext_attr_tbl[array_index]);
    }
  }
  return NULL;
}

/*-----------------------------------------------------------------------------------------------------
  This function handles the GATT read request events from the stack

  \param conn_id       Connection ID
  \param opcode        GATT opcode
  \param p_read_data   Read data structure
  \param len_requested Length requested

  \return wiced_bt_gatt_status_t GATT result
-----------------------------------------------------------------------------------------------------*/
static wiced_bt_gatt_status_t _Request_attr_read_handler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode, wiced_bt_gatt_read_t *p_read_data, uint16_t len_requested)
{

  gatt_db_lookup_table_t *attr_ptr;
  int32_t                 attr_len;
  int32_t                 chank_len;
  uint8_t                *start_ptr;

  attr_ptr = _Get_attribute_by_handle(p_read_data->handle);
  if (attr_ptr == NULL)
  {
    BLELOG("BLE: Read handle attribute not found. Handle:0x%X",p_read_data->handle);
    wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_read_data->handle,WICED_BT_GATT_INVALID_HANDLE);
    return WICED_BT_GATT_INVALID_HANDLE;
  }

  attr_len = attr_ptr->cur_len;

  BLELOG("BLE: GATT Read handler: handle:0x%X, len:%d",p_read_data->handle, attr_len);


  if (p_read_data->offset >= attr_ptr->cur_len)
  {
    // Здесь, если смещение имеет некорректное значение превышающее длину данных
    wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_read_data->handle,WICED_BT_GATT_INVALID_OFFSET);
    return (WICED_BT_GATT_INVALID_OFFSET);
  }

  chank_len = attr_len - p_read_data->offset;
  if (len_requested < chank_len) chank_len = len_requested;

  start_ptr = ((uint8_t *)attr_ptr->p_data) + p_read_data->offset;

  wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, chank_len, start_ptr, NULL);

  return WICED_BT_GATT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  This function handles the GATT read by type request events from the stack

  \param conn_id       Connection ID
  \param opcode        GATT opcode
  \param p_read_data   Read data structure
  \param len_requested Length requested

  \return wiced_bt_gatt_status_t GATT result
-----------------------------------------------------------------------------------------------------*/
static wiced_bt_gatt_status_t _Request_attr_read_by_type_handler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode, wiced_bt_gatt_read_by_type_t *p_read_data, uint16_t len_requested)
{
  gatt_db_lookup_table_t *attr_ptr;
  uint16_t                attr_handle;
  uint8_t                *buff;
  uint8_t                 pair_len = 0;
  int32_t                 accumulated_data = 0;
  int32_t                 amount;

  attr_handle = p_read_data->s_handle;

  buff = wiced_bt_get_buffer(len_requested);
  if (buff == NULL)
  {
    wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, attr_handle,WICED_BT_GATT_INSUF_RESOURCE);
    return WICED_BT_GATT_INSUF_RESOURCE;
  }

  // Read by type returns all attributes of the specified type, between the start and end handles
  while (WICED_TRUE)
  {
    attr_handle = wiced_bt_gatt_find_handle_by_type(attr_handle, p_read_data->e_handle,&p_read_data->uuid);

    if (attr_handle == 0) break;

    attr_ptr = _Get_attribute_by_handle(attr_handle);
    if (attr_ptr == NULL)
    {
      wiced_bt_gatt_server_send_error_rsp(conn_id, opcode,p_read_data->s_handle, WICED_BT_GATT_ERR_UNLIKELY);
      wiced_bt_free_buffer(buff);
      return WICED_BT_GATT_ERR_UNLIKELY;
    }

    amount = wiced_bt_gatt_put_read_by_type_rsp_in_stream(buff + accumulated_data, len_requested - accumulated_data,&pair_len, attr_handle,attr_ptr->cur_len, attr_ptr->p_data);
    if (amount == 0) break;
    accumulated_data += amount;

    /* Increment starting handle for next search to one past current */
    attr_handle++;
  }

  if (accumulated_data == 0)
  {
    wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_read_data->s_handle,WICED_BT_GATT_INVALID_HANDLE);
    wiced_bt_free_buffer(buff);
    return WICED_BT_GATT_INVALID_HANDLE;
  }
  // Высылаем накопленные в буфер данные
  wiced_bt_gatt_server_send_read_by_type_rsp(conn_id, opcode, pair_len,accumulated_data, buff, (wiced_bt_gatt_app_context_t)wiced_bt_free_buffer);

  return WICED_BT_GATT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  This function handles the GATT write request events from the stack

  \param conn_id Connection ID
  \param opcode  GATT opcode
  \param p_data  Write data structure

  \return wiced_bt_gatt_status_t GATT result
-----------------------------------------------------------------------------------------------------*/
static wiced_bt_gatt_status_t _Requset_attr_write_handler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode, wiced_bt_gatt_write_req_t *gatt_wr_req_ptr)
{
  wiced_bt_gatt_status_t    result   = WICED_BT_GATT_SUCCESS;
  gatt_db_lookup_table_t    *attr_ptr;


  BLELOG("BLE: GATT write handler: handle:0x%X len:%d, opcode:0x%X",gatt_wr_req_ptr->handle, gatt_wr_req_ptr->val_len, opcode);

  attr_ptr = _Get_attribute_by_handle(gatt_wr_req_ptr->handle);
  if (attr_ptr == NULL)
  {
    BLELOG("BLE: Write Handle attr not found. Handle:0x%X", gatt_wr_req_ptr->handle);
    return WICED_BT_GATT_INVALID_HANDLE;
  }

  result = WiFi_control_write_hundler(conn_id,opcode,attr_ptr,gatt_wr_req_ptr);
  // Здесь если result не равен WICED_BT_GATT_SUCCESS , то передать управление обработчикам других сервисов
  //
  // ... Здесь вставляются обработчики других сервисов
  //

  if ((opcode == GATT_REQ_WRITE) && (result == WICED_BT_GATT_SUCCESS))
  {
    wiced_bt_gatt_server_send_write_rsp(conn_id, opcode,gatt_wr_req_ptr->handle);
  }
  else
  {
    wiced_bt_gatt_server_send_error_rsp(conn_id, opcode,gatt_wr_req_ptr->handle, result);
  }

  return result;
}


/*-----------------------------------------------------------------------------------------------------
  This function redirects the GATT attribute requests to the appropriate functions

  \param p_data GATT request data structure

  \return wiced_bt_gatt_status_t GATT result
-----------------------------------------------------------------------------------------------------*/
static wiced_bt_gatt_status_t _Attribute_req_event_handler(wiced_bt_gatt_attribute_request_t *p_data)
{
  wiced_bt_gatt_status_t result  = WICED_BT_GATT_INVALID_PDU;

  switch (p_data->opcode)
  {
  case GATT_REQ_READ:
  case GATT_REQ_READ_BLOB:
    result = _Request_attr_read_handler(p_data->conn_id, p_data->opcode,&p_data->data.read_req, p_data->len_requested);
    break;

  case GATT_REQ_READ_BY_TYPE:
    result = _Request_attr_read_by_type_handler(p_data->conn_id, p_data->opcode,&p_data->data.read_by_type, p_data->len_requested);
    break;

  case GATT_REQ_WRITE:
  case GATT_CMD_WRITE:
  case GATT_CMD_SIGNED_WRITE:
    result = _Requset_attr_write_handler(p_data->conn_id, p_data->opcode,&(p_data->data.write_req));
    break;

  case GATT_REQ_MTU:
    BLELOG("BLE: Exchanged MTU from client: %d", p_data->data.remote_mtu);
    wiced_bt_gatt_server_send_mtu_rsp(p_data->conn_id, p_data->data.remote_mtu,cy_bt_cfg_settings.p_ble_cfg->ble_max_rx_pdu_size);
    result = WICED_BT_GATT_SUCCESS;
    break;

  default:
    break;
  }
  return result;
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается при изменении статуса подключения внешнего центрального стройства

  \param p_conn_status Connection or disconnection

  \return wiced_bt_gatt_status_t GATT result
-----------------------------------------------------------------------------------------------------*/
static wiced_bt_gatt_status_t _Connection_change_event_handler(wiced_bt_gatt_connection_status_t *p_conn_status)
{
  int32_t    indx;
  #ifdef ENABLE_BLE_LOG
  char                               str[64];
  #endif

  wiced_bt_gatt_status_t  status = WICED_BT_GATT_ERROR;
  wiced_result_t          result;

  if (p_conn_status != NULL)
  {
    if (p_conn_status->connected)
    {
      BLELOG("BLE: Connected: Peer Address: %s",   Buf_to_hex_str(str,6,p_conn_status->bd_addr));
      indx = Get_gat_conn_indx(p_conn_status->bd_addr);
      if (indx >= 0)
      {
        // Сохраняем данные подключения для вывода в режиме диагностикм
        gatt_conn_list[indx].active = 1;
        memcpy(&gatt_conn_list[indx].status,p_conn_status, sizeof(wiced_bt_gatt_connection_status_t));
        memcpy(gatt_conn_list[indx].addr, p_conn_status->bd_addr, sizeof(wiced_bt_device_address_t));
      }
      else
      {
        indx = _Get_free_gat_conn_indx();
        if (indx >= 0)
        {
          gatt_conn_list[indx].active = 1;
          memcpy(&gatt_conn_list[indx].status,p_conn_status, sizeof(wiced_bt_gatt_connection_status_t));
          memcpy(gatt_conn_list[indx].addr, p_conn_status->bd_addr, sizeof(wiced_bt_device_address_t));
        }
      }

      for (uint32_t i=0; i < GATT_CALLBACKS_NUM; i++)
      {
        if (gatt_cb[i] != NULL) if (gatt_cb[i]->connect != NULL) gatt_cb[i]->connect(p_conn_status);
      }

    }
    else
    {
      BLELOG("BLE: Disconnected: Peer Address: %s",   Buf_to_hex_str(str,6,p_conn_status->bd_addr));
      BLELOG("BLE: Reason for disconnection: %s",get_bt_gatt_disconn_reason_name(p_conn_status->reason));
      indx = Get_gat_conn_indx(p_conn_status->bd_addr);
      if (indx >= 0)
      {
        gatt_conn_list[indx].active = 0;
        memcpy(&gatt_conn_list[indx].status,p_conn_status, sizeof(wiced_bt_gatt_connection_status_t));
        memcpy(gatt_conn_list[indx].addr, p_conn_status->bd_addr, sizeof(wiced_bt_device_address_t));
      }


      for (uint32_t i=0; i < GATT_CALLBACKS_NUM; i++)
      {
        if (gatt_cb[i] != NULL) if (gatt_cb[i]->connect != NULL) gatt_cb[i]->disconnect(p_conn_status);
      }

      // Если прервалось соединение со стороны клиента, то возобновить адвертайсинг
      if (p_conn_status->link_role == HCI_ROLE_PERIPHERAL)
      {

        result = wiced_bt_ble_set_raw_advertisement_data(CY_BT_ADV_PACKET_DATA_SIZE,cy_bt_adv_packet_data);
        if (WICED_SUCCESS != result)
        {
          BLELOG("BLE: Set ADV data failed");
        }

        result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, BLE_ADDR_PUBLIC, NULL);
        if (WICED_SUCCESS != result)
        {
          BLELOG("BLE: Start ADV failed");
        }
      }
    }
    status = WICED_BT_GATT_SUCCESS;
  }

  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param gatt_cb_ptr
-----------------------------------------------------------------------------------------------------*/
uint32_t  Set_gatt_callbacks(T_gatt_callbacks *gatt_cb_ptr)
{
  for (uint32_t i = 0; i < GATT_CALLBACKS_NUM; i++)
  {
    if (gatt_cb[i] == NULL)
    {
      gatt_cb[i] = gatt_cb_ptr;
      return RES_OK;
    }
  }
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param gatt_cb_ptr
-----------------------------------------------------------------------------------------------------*/
uint32_t  Delete_gatt_callbacks(T_gatt_callbacks *gatt_cb_ptr)
{
  for (uint32_t i = 0; i < GATT_CALLBACKS_NUM; i++)
  {
    if (gatt_cb[i] == gatt_cb_ptr)
    {
      gatt_cb[i] = NULL;
      return RES_OK;
    }
  }
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------



  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t  Get_gat_conn_id(wiced_bt_device_address_t addr)
{
  for (uint32_t i = 0; i < MAX_BLE_CONNECTION_NUMBER; i++)
  {
    if (gatt_conn_list[i].active == 1)
    {
      if (memcmp(gatt_conn_list[i].addr , addr, sizeof(wiced_bt_device_address_t)) == 0)
      {
        return  gatt_conn_list[i].status.conn_id;
      }
    }
  }
  return -1;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t  Get_gat_conn_indx(wiced_bt_device_address_t addr)
{
  for (uint32_t i = 0; i < MAX_BLE_CONNECTION_NUMBER; i++)
  {
    if (memcmp(gatt_conn_list[i].addr , addr, sizeof(wiced_bt_device_address_t)) == 0)
    {
      return i;
    }
  }
  return -1;
}

/*-----------------------------------------------------------------------------------------------------
  This function redirects the GATT requests to the appropriate functions

  \param event  GATT request data structure
  \param p_data Pointer to BLE GATT event structures

  \return wiced_bt_gatt_status_t GATT result
-----------------------------------------------------------------------------------------------------*/
wiced_bt_gatt_status_t GATT_callback(wiced_bt_gatt_evt_t evt, wiced_bt_gatt_event_data_t *p_data)
{
  wiced_bt_gatt_status_t result = WICED_BT_GATT_INVALID_PDU;

  BLELOG("BLE: GATTS event: %s", get_bt_gatt_evt_name(evt));

  switch (evt)
  {
  case GATT_CONNECTION_STATUS_EVT:   // GATT connection status change.
    result = _Connection_change_event_handler(&p_data->connection_status);
    break;

  case GATT_ATTRIBUTE_REQUEST_EVT:   // GATT attribute request (from remote client).
    result = _Attribute_req_event_handler(&p_data->attribute_request);
    break;

  case GATT_GET_RESPONSE_BUFFER_EVT: // GATT buffer request

    p_data->buffer_request.buffer.p_app_rsp_buffer = wiced_bt_get_buffer(p_data->buffer_request.len_requested);

    p_data->buffer_request.buffer.p_app_ctxt = (void *)wiced_bt_free_buffer;

    if (NULL == p_data->buffer_request.buffer.p_app_rsp_buffer)
    {
      BLELOG("BLE: Insufficient resources");
      result = WICED_BT_GATT_INSUF_RESOURCE;
    }
    else
    {
      result = WICED_BT_GATT_SUCCESS;
    }
    break;

  case GATT_APP_BUFFER_TRANSMITTED_EVT: // GATT buffer transmitted event, indicates that the data  has been transmitted and may be released by the application
    {
      pfn_free_buffer_t pfn_free = (pfn_free_buffer_t)p_data->buffer_xmitted.p_app_ctxt; // If the buffer is dynamic, the context will point to a function to free it.
      if (pfn_free) pfn_free(p_data->buffer_xmitted.p_app_data);
      result = WICED_BT_GATT_SUCCESS;
    }
    break;

  case GATT_OPERATION_CPLT_EVT:
    // Получили по запросу данные от BLE сервера
    for (uint32_t i=0; i < GATT_CALLBACKS_NUM; i++) if (gatt_cb[i] != NULL) if (gatt_cb[i]->connect != NULL) gatt_cb[i]->op_complete(p_data);
    result = WICED_BT_GATT_SUCCESS;
    break;
  case GATT_DISCOVERY_RESULT_EVT:
    // Сообщение об очередном результате в процессе Discovery
    for (uint32_t i=0; i < GATT_CALLBACKS_NUM; i++) if (gatt_cb[i] != NULL) if (gatt_cb[i]->connect != NULL) gatt_cb[i]->discovery_result(p_data);
    result = WICED_BT_GATT_SUCCESS;
    break;
  case GATT_DISCOVERY_CPLT_EVT:
    // Сообщение о завершении процесса Discovery
    for (uint32_t i=0; i < GATT_CALLBACKS_NUM; i++) if (gatt_cb[i] != NULL) if (gatt_cb[i]->connect != NULL) gatt_cb[i]->discovery_cplt(p_data);
    result = WICED_BT_GATT_SUCCESS;
    break;
  case GATT_CONGESTION_EVT:
    result = WICED_BT_GATT_SUCCESS;
    break;


  default:
    BLELOG("BLE: GATT event not handled");
  }
  return result;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return wiced_bt_gatt_connection_status_t*
-----------------------------------------------------------------------------------------------------*/
T_gatt_connection_cbl* Get_gatt_connection_status(uint32_t n)
{
  return &gatt_conn_list[n];
}




