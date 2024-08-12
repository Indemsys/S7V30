#ifndef NET_DHCP_SERV_FACTORY_H
  #define NET_DHCP_SERV_FACTORY_H


  #define   MAX_NUM_OF_DHCP_SERVERS   2
  #define   DHCP_SERVER_STACK_SIZE    2048

  #define   MAX_NUM_OF_DHCP_CLIENTS   2


typedef struct
{

    ULONG           ip_address;
    ULONG           dns_ip_address;
    ULONG           network_mask;
    ULONG           gateway_address;
    ULONG           address_pool_sz;

} T_dhcp_server_config;

typedef VOID  (*T_Net_dhcp_state_change_notify)(NX_DHCP *dhcp_ptr, UCHAR new_state);


uint32_t        Net_DHCP_server_create(NX_IP *ip_ptr);
uint32_t        Net_DHCP_server_restart(void);
uint32_t        Net_DHCP_client_create(NX_IP *ip_ptr);
uint32_t        Net_DHCP_server_config(uint32_t intf_indx, T_dhcp_server_config *sc);
NX_DHCP        *Net_get_dhcp_client_ptr(void);
const char*     DHCP_client_state_str( UCHAR state);


#endif



