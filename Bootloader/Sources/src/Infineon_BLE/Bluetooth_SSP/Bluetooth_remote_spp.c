// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-06-28
// 12:38:13
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BLE_main.h"

// Аттрибут по которому определяем наличие COM порта в подключаемом по Bluetooth удаленном устройстве
static wiced_bt_uuid_t  spp_uuid =
{
  .len = 2,
  .uu =
  {
    .uuid16 = UUID_SERVCLASS_SERIAL_PORT
  },
};

typedef struct
{
    #define     SPP_SESSION_STATE_IDLE       0
    #define     SPP_SESSION_STATE_OPENING    1
    #define     SPP_SESSION_STATE_OPEN       2
    #define     SPP_SESSION_STATE_CLOSING    3

    uint8_t                        state;                  //  state machine state
    wiced_bool_t                   in_use;                 //  indicates if control block is in use
    uint16_t                       rfc_serv_handle;        //  Хэндлер сервера RFCOMM получаемый из библиотеки btstack
    uint16_t                       rfc_conn_handle;        //  Хэндлер RFCOMM соединения получаемый из библиотеки btstack
    uint8_t                        server_scn;             //  server's scn
    BD_ADDR                        server_addr;            //  server's bd address
    void                           *p_sdp_discovery_db;    //  pointer to discovery database
    uint32_t                       data_flow_enabled;      //  Если 1, то принимающая сторона не может принимать данные
    wiced_bt_rfcomm_port_event_t   event_error;            //  reflect PORT_EV_ERR
} T_remote_spp_cbl;


static  T_remote_spp_cbl  remote_spp_cbl;

/*-----------------------------------------------------------------------------------------------------
  Free discovery database.

  \param p_scb
-----------------------------------------------------------------------------------------------------*/
static void _spp_sdp_free_db(T_remote_spp_cbl *p_scb)
{
  if (p_scb->p_sdp_discovery_db != NULL)
  {
    wiced_bt_free_buffer(p_scb->p_sdp_discovery_db);
    p_scb->p_sdp_discovery_db = NULL;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Process SDP discovery results to find requested attributes for requested service.
  Returns WICED_TRUE if results found, WICED_FALSE otherwise.

  \param p_scb

  \return wiced_bool_t
-----------------------------------------------------------------------------------------------------*/
wiced_bool_t _spp_sdp_find_attr(T_remote_spp_cbl *p_scb)
{
  wiced_bt_sdp_discovery_record_t     *p_rec = (wiced_bt_sdp_discovery_record_t *) NULL;
  wiced_bt_sdp_protocol_elem_t        pe;
  wiced_bool_t                        result = WICED_TRUE;


  p_rec = wiced_bt_sdp_find_service_uuid_in_db((wiced_bt_sdp_discovery_db_t *)p_scb->p_sdp_discovery_db,&spp_uuid, p_rec);
  if (p_rec == NULL)
  {
    return (WICED_FALSE);
  }

  // Look up the server channel number in the protocol list element
  if (wiced_bt_sdp_find_protocol_list_elem_in_rec(p_rec, UUID_PROTOCOL_RFCOMM,&pe))
  {
    if (pe.num_params > 0)
    {
      p_scb->server_scn = (uint8_t)pe.params[0];
    }
    else result = WICED_FALSE;
  }
  else
  {
    result = WICED_FALSE;
  }
  return result;
}


/*-----------------------------------------------------------------------------------------------------
  SDP callback function.

  \param sdp_status
-----------------------------------------------------------------------------------------------------*/
static void _spp_sdp_callback(uint16_t sdp_status)
{

  if ((sdp_status == WICED_BT_SDP_SUCCESS) || (sdp_status == WICED_BT_SDP_DB_FULL))
  {
    if (_spp_sdp_find_attr(&remote_spp_cbl))
    {
      wiced_bt_rfcomm_create_connection(UUID_SERVCLASS_SERIAL_PORT,
                                        remote_spp_cbl.server_scn, WICED_FALSE,
                                        SPP_TX_BUFFER_SIZE,
                                        remote_spp_cbl.server_addr,
                                        &remote_spp_cbl.rfc_conn_handle,
                                        (wiced_bt_port_mgmt_cback_t *)BT_spp_rfcomm_control_callback);
    }
    else
    {
      /* reopen server and notify app of the failure */
      remote_spp_cbl.state = SPP_SESSION_STATE_IDLE;
    }
  }
  else
  {
    remote_spp_cbl.state = SPP_SESSION_STATE_IDLE;
  }
  _spp_sdp_free_db(&remote_spp_cbl);
}

/*-----------------------------------------------------------------------------------------------------
  Do service discovery.

  \param p_scb
-----------------------------------------------------------------------------------------------------*/
static void _spp_sdp_start_discovery(T_remote_spp_cbl *p_scb)
{
  uint16_t        attr_list[4];
  uint8_t         num_attr;
  wiced_bool_t    result;

  /* We need to get Service Class (to compare UUID and Protocol Description to get SCN to connect */
  attr_list[0] = ATTR_ID_SERVICE_CLASS_ID_LIST;
  attr_list[1] = ATTR_ID_PROTOCOL_DESC_LIST;
  num_attr = 2;

  /* allocate buffer for sdp database */
  p_scb->p_sdp_discovery_db = wiced_bt_get_buffer(1024);

  /* set up service discovery database; attr happens to be attr_list len */
  result = wiced_bt_sdp_init_discovery_db((wiced_bt_sdp_discovery_db_t *)p_scb->p_sdp_discovery_db, 1024, 1,&spp_uuid, num_attr, attr_list);

  /* initiate service discovery */
  if ((result == WICED_FALSE) || (!wiced_bt_sdp_service_search_attribute_request(p_scb->server_addr, (wiced_bt_sdp_discovery_db_t *)p_scb->p_sdp_discovery_db,&_spp_sdp_callback)))
  {
    /* Service discovery not initiated - free discover db, reopen server, tell app  */
    _spp_sdp_free_db(p_scb);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Подключаемся к COM порту удаленного устройства заданного своим MAC адресом

-----------------------------------------------------------------------------------------------------*/
wiced_result_t BT_remote_spp_connect(BD_ADDR bd_addr)
{
  int                          i;

  if (remote_spp_cbl.in_use == WICED_FALSE)
  {
    /* Initialize SCB and link callbacks to spp_scb[0] */
    memset(&remote_spp_cbl, 0, sizeof(T_remote_spp_cbl));
    remote_spp_cbl.in_use = WICED_TRUE;
  }

  if (remote_spp_cbl.state != SPP_SESSION_STATE_IDLE) return WICED_BT_ERROR;

  remote_spp_cbl.state = SPP_SESSION_STATE_OPENING;

  /* store parameters */
  for (i = 0; i < BD_ADDR_LEN; i++) remote_spp_cbl.server_addr[i] = bd_addr[BD_ADDR_LEN - 1 - i];

  /* close RFCOMM server, if listening on this SCB */
  if (remote_spp_cbl.rfc_serv_handle)
  {
    wiced_bt_rfcomm_remove_connection(remote_spp_cbl.rfc_serv_handle, WICED_TRUE);
    remote_spp_cbl.rfc_serv_handle = 0;
  }
  else
  {
    /* Setting server handle to invalid to prevent incorrectly starting server upon disconnection */
    remote_spp_cbl.rfc_serv_handle = RFCOMM_INVALID_HANDLE;
  }

  /* do service search */
  _spp_sdp_start_discovery(&remote_spp_cbl);

  return WICED_BT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  Отключаемся от COM порта удаленного устройства
-----------------------------------------------------------------------------------------------------*/
wiced_result_t BT_remote_spp_disconnect(uint16_t handle)
{
  if ((remote_spp_cbl.state == SPP_SESSION_STATE_OPENING) || (remote_spp_cbl.state == SPP_SESSION_STATE_OPEN))
  {
    remote_spp_cbl.state = SPP_SESSION_STATE_CLOSING;
    if (remote_spp_cbl.rfc_conn_handle)
    {
      wiced_bt_rfcomm_remove_connection(remote_spp_cbl.rfc_conn_handle, WICED_FALSE);
    }
  }
  return WICED_BT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
 SPP application may use this call to disable or reenable the RX data flow

 @param[in]      handle : Connection handle indicated in the connection up callback
 @param[in]      enable : If true, data flow is enabled
 @return          none
-----------------------------------------------------------------------------------------------------*/
void BT_remote_spp_rx_flow_control(uint16_t handle, wiced_bool_t enable)
{
  wiced_bt_rfcomm_flow_control(remote_spp_cbl.rfc_conn_handle, enable);
}


/*-----------------------------------------------------------------------------------------------------
  SPP application may use this function to get the connection state when rfcomm connection made

  \param bd_addr

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t BT_remote_spp_get_connection_state(BD_ADDR bd_addr)
{
  return remote_spp_cbl.state;
}


/*-----------------------------------------------------------------------------------------------------
 Send data over the established External Accessory connection.
 The session must be SPP_EA_SESSION_ID.
 The first 2 octets of the p_data must be the handle passed to the application in the wiced_bt_spp_connection_up_callback in the big endian format.

 @param[in]      handle : Connection handle indicated in the connection up callback
 @param[in]      p_data  : Pointer to buffer with data to send.
 @param[in]      len :     Length of the data + handle
 @return          WICED_TRUE: if data is scheduled for transmission, otherwise WICED_FALSE

-----------------------------------------------------------------------------------------------------*/
wiced_bool_t BT_remote_spp_send_data(uint16_t handle, uint8_t *p_data, uint32_t length)
{
  wiced_bt_rfcomm_result_t  result;

  result = wiced_bt_rfcomm_write_data(handle, ( char * ) p_data, length);

  // port_write error is not returned from firmware, so check event history
  if (!result)
  {
    result |= remote_spp_cbl.event_error;
  }
  return (result == 0) ? WICED_TRUE : WICED_FALSE;
}

