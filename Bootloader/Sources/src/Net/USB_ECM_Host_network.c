// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-07-10
// 14:09:24
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"

NX_IP                      *ecm_host_ip_ptr;
NX_DHCP                    *ecm_dhcp_client_ptr;



/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t возвращаем 1 если линк активен
-----------------------------------------------------------------------------------------------------*/
NX_INTERFACE *ECM_host_link_state(void)
{
  NX_IP *ip = Net_get_ip_ptr();
  if (ip->nx_ip_interface[NET_USB_INTF_NUM].nx_interface_link_up)
  {
    return &(ip->nx_ip_interface[NET_USB_INTF_NUM]);
  }
  return NULL;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t        ECM_HOST_accept_connection(void)
{
  uint32_t                  res = NX_SUCCESS;
  ULONG                     ret_val;
  UX_HOST_CLASS_CDC_ECM    *ecm_host_class = (UX_HOST_CLASS_CDC_ECM *)_ux_system_host->ux_system_host_class_array[0].ux_host_class_first_instance;
  USB_NETWORK_DEVICE_TYPE  *net_dev;
  NX_IP                    *ip_ptr;

  uint32_t ip_address  ;
  uint32_t network_mask;

  net_dev = (USB_NETWORK_DEVICE_TYPE *)ecm_host_class->ux_host_class_cdc_ecm_network_handle;
  net_dev->ux_network_device_link_status = 1;

  // Здесь надо отдать команду драйверу на инициализацию, поскольку еще не согласован MAC адрес в WiFi модуле и драйвере интерфейса
  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_INITIALIZE, NET_USB_INTF_NUM,&ret_val);
  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_ENABLE, NET_USB_INTF_NUM,&ret_val);

  Str_to_IP_v4((char const *)ivar.usb_default_ip_addr,  (uint8_t *)&ip_address  );
  Str_to_IP_v4((char const *)ivar.usb_default_net_mask, (uint8_t *)&network_mask);

  ip_ptr = Net_get_ip_ptr();

  nx_ip_interface_address_set(ip_ptr, NET_USB_INTF_NUM, ip_address, network_mask);
  nx_ip_gateway_address_set(ip_ptr, ip_address);

  if (ivar.en_ecm_host_dhcp_client)
  {
    res = nx_dhcp_interface_start(Net_get_dhcp_client_ptr(), NET_USB_INTF_NUM);
    if (res != NX_SUCCESS)
    {
      NETLOG("USB ECM host DHCP client error %04X", res);
    }
    else
    {
      NETLOG("USB ECM host DHCP client started");
    }
  }
  Net_mDNS_enable(NET_USB_INTF_NUM);

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t        ECM_HOST_accept_disconnection(void)
{
  ULONG ret_val;
  UX_HOST_CLASS_CDC_ECM    *ecm_host_class = (UX_HOST_CLASS_CDC_ECM *)_ux_system_host->ux_system_host_class_array[0].ux_host_class_first_instance;
  USB_NETWORK_DEVICE_TYPE  *net_dev;

  Net_mDNS_disable(NET_USB_INTF_NUM);

  nx_dhcp_interface_stop(Net_get_dhcp_client_ptr(), NET_USB_INTF_NUM);

  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_DISABLE, NET_USB_INTF_NUM,&ret_val);

  net_dev = (USB_NETWORK_DEVICE_TYPE *)ecm_host_class->ux_host_class_cdc_ecm_network_handle;
  net_dev->ux_network_device_link_status = 0;

  return RES_OK;
}

