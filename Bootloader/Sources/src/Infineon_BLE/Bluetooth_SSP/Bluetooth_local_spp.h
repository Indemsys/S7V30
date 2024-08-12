#ifndef __BLUETOOTH_SPP
  #define __BLUETOOTH_SPP

  #ifdef __cplusplus
extern "C"
{
  #endif


  #define SPP_TX_BUFFER_SIZE        1017
  #define SPP_MAX_PACKET_SIZE       1013  // MAX packet size that can be sent over SPP
  #define WICED_EIR_BUF_MAX_SIZE    264

  #define RFCOMM_INVALID_HANDLE     0xFFFF

  typedef uint8_t BD_ADDR[BD_ADDR_LEN];

  typedef void (*T_bt_spp_callback)(uint16_t handle);


typedef struct
{
    #define     SPP_SESSION_STATE_IDLE       0
    #define     SPP_SESSION_STATE_OPENING    1
    #define     SPP_SESSION_STATE_OPEN       2
    #define     SPP_SESSION_STATE_CLOSING    3

    uint8_t                        state;                  //  state machine state
    wiced_bool_t                   in_use;                 //  indicates if control block is in use
    uint8_t                        b_is_initiator;         //  initiator of the connection ( true ) or acceptor ( false )
    uint16_t                       rfc_serv_handle;        //  Хэндлер сервера RFCOMM получаемый из библиотеки btstack
    uint16_t                       rfc_conn_handle;        //  Хэндлер RFCOMM соединения получаемый из библиотеки btstack
    uint8_t                        server_scn;             //  server's scn
    BD_ADDR                        server_addr;            //  server's bd address
    void                           *p_sdp_discovery_db;    //  pointer to discovery database
    uint32_t                       data_flow_enabled;      //  Если 1, то принимающая сторона не может принимать данные
    wiced_bt_rfcomm_port_event_t   event_error;            //  reflect PORT_EV_ERR
} T_local_spp;



  void           BT_ssp_set_rx_callback(T_bt_spp_callback func);
  void           BT_ssp_set_tx_cmpl_callback(T_bt_spp_callback func);
  uint32_t       BT_spp_startup(void);
  uint32_t       BT_spp_rfcomm_start_server(void);
  void           BT_spp_rfcomm_control_callback(uint32_t port_status, uint16_t port_handle);

  #ifdef __cplusplus
}
  #endif

#endif
