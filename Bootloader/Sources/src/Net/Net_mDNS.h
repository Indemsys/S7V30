#ifndef NET_MDNS_H
  #define NET_MDNS_H


uint32_t Net_mDNS_server_create(NX_IP *ip_ptr);
uint32_t Net_mDNS_enable(uint32_t intf_indx);
uint32_t Net_mDNS_disable(uint32_t intf_indx);

#endif // NET_MDNS_H



