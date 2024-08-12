#ifndef NET_DNS_H
  #define NET_DNS_H


UINT     DNS_get_host_address(UCHAR *host_name_ptr, ULONG *dns_address, ULONG wait_option);
uint32_t Is_DNS_created(void);
uint32_t Net_DNS_client_create(NX_IP  *ip_ptr);

#endif // NET_DNS_H



