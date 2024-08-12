#ifndef BLUETOOTH_SPP_API_H
  #define BLUETOOTH_SPP_API_H


uint8_t        BT_remote_spp_get_connection_state(BD_ADDR bd_addr);
wiced_bool_t   BT_remote_spp_send_data(uint16_t handle, uint8_t *p_data, uint32_t len);
wiced_result_t BT_remote_spp_connect(BD_ADDR bd_addr);
wiced_result_t BT_remote_spp_disconnect(uint16_t handle);
void           BT_remote_spp_rx_flow_control(uint16_t handle, wiced_bool_t enable);

#endif



