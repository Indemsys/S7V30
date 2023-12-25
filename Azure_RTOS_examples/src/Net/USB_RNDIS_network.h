#ifndef USB_RNDIS_DRIVER_H
  #define USB_RNDIS_DRIVER_H


UINT     Register_rndis_class(void);
UINT     Register_cdc_ecm_class(void);
uint32_t RNDIS_init_network_stack(NX_IP **ip_p_ptr);
void     RNDIS_network_controller(void);
uint8_t  RNDIS_network_active_flag(void);
uint32_t RNDIS_Get_MAC(char* mac_str, uint32_t max_str_len);
uint32_t RNDIS_Get_MASK_IP(char* ip_str, char* mask_str,uint32_t max_str_len);
uint32_t RNDIS_Get_Gateway_IP(char* gate_str,uint32_t max_str_len);



#endif



