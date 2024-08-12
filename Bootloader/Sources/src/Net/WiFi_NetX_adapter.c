// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-05-01
// 11:38:44
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"


static VOID WHD_NetXDuo_driver_entry(NX_IP_DRIVER *driver, NX_INTERFACE *intf_ptr, whd_interface_t   ifp);

/*-----------------------------------------------------------------------------------------------------


  \param ip_ptr_in
  \param packet_ptr
  \param destination_mac_msw
  \param destination_mac_lsw
  \param ethertype
-----------------------------------------------------------------------------------------------------*/
static void WHD_add_ethernet_header(NX_INTERFACE *intf_ptr, NX_PACKET *packet_ptr, ULONG destination_mac_msw, ULONG destination_mac_lsw, USHORT ethertype)
{
  ULONG *ethernet_header;

  // Make space at the front of the packet buffer for the ethernet header
  packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr - WHD_ETHERNET_SIZE;
  packet_ptr->nx_packet_length = packet_ptr->nx_packet_length + WHD_ETHERNET_SIZE;

  // Ensure ethernet header writing starts with 32 bit alignment
  ethernet_header = (ULONG *)(packet_ptr->nx_packet_prepend_ptr - 2);

  *ethernet_header       = destination_mac_msw;
  *(ethernet_header + 1) = destination_mac_lsw;
  *(ethernet_header + 2) = (intf_ptr->nx_interface_physical_address_msw << 16) | (intf_ptr->nx_interface_physical_address_lsw >> 16);
  *(ethernet_header + 3) = (intf_ptr->nx_interface_physical_address_lsw << 16) | ethertype;

  NX_CHANGE_ULONG_ENDIAN(*(ethernet_header));
  NX_CHANGE_ULONG_ENDIAN(*(ethernet_header + 1));
  NX_CHANGE_ULONG_ENDIAN(*(ethernet_header + 2));
  NX_CHANGE_ULONG_ENDIAN(*(ethernet_header + 3));
}


/*-----------------------------------------------------------------------------------------------------


  \param driver
  \param mac
-----------------------------------------------------------------------------------------------------*/
static VOID WHD_get_MAC_from_driver(NX_IP_DRIVER *driver, whd_mac_t *mac)
{
  mac->octet[0] = (uint8_t)((driver->nx_ip_driver_physical_address_msw & 0x0000ff00) >> 8);
  mac->octet[1] = (uint8_t)((driver->nx_ip_driver_physical_address_msw & 0x000000ff) >> 0);
  mac->octet[2] = (uint8_t)((driver->nx_ip_driver_physical_address_lsw & 0xff000000) >> 24);
  mac->octet[3] = (uint8_t)((driver->nx_ip_driver_physical_address_lsw & 0x00ff0000) >> 16);
  mac->octet[4] = (uint8_t)((driver->nx_ip_driver_physical_address_lsw & 0x0000ff00) >> 8);
  mac->octet[5] = (uint8_t)((driver->nx_ip_driver_physical_address_lsw & 0x000000ff) >> 0);
}

/*-----------------------------------------------------------------------------------------------------
  Функция отправки пакетов WiFi станции в WiFi драйвер

  \param driver
-----------------------------------------------------------------------------------------------------*/
VOID WHD_STA_NetXDuo_driver_entry(NX_IP_DRIVER *driver)
{
  WHD_NetXDuo_driver_entry(driver, &(Net_get_ip_ptr()->nx_ip_interface[WIFI_STA_INTF_NUM]), g_prim_whd_intf_ptr);
}

/*-----------------------------------------------------------------------------------------------------
  Функция отправки пакетов WiFi точки доступа в WiFi драйвер

  \param driver
-----------------------------------------------------------------------------------------------------*/
VOID WHD_AP_NetXDuo_driver_entry(NX_IP_DRIVER *driver)
{
  WHD_NetXDuo_driver_entry(driver, &(Net_get_ip_ptr()->nx_ip_interface[WIFI_AP_INTF_NUM]), g_secd_whd_intf_ptr);
}

/*-----------------------------------------------------------------------------------------------------


  \param driver
  \param ip
  \param ifp
-----------------------------------------------------------------------------------------------------*/
static VOID WHD_NetXDuo_driver_entry(NX_IP_DRIVER *driver, NX_INTERFACE *intf_ptr, whd_interface_t   ifp)
{
  NX_PACKET         *packet_ptr;

  whd_mac_t         mac;

  packet_ptr                           = driver->nx_ip_driver_packet;
  driver->nx_ip_driver_status          = NX_NOT_SUCCESSFUL;

  // Process commands which are valid independent of the link state
  switch (driver->nx_ip_driver_command)
  {
  case NX_LINK_INITIALIZE:
    // Инициализация IP STA
    intf_ptr->nx_interface_ip_mtu_size             = (ULONG) WHD_PAYLOAD_MTU;
    intf_ptr->nx_interface_address_mapping_needed  = (UINT) NX_TRUE;
    intf_ptr->nx_interface_physical_address_msw    = (ULONG)((ifp->mac_addr.octet[0] << 8) + ifp->mac_addr.octet[1]);
    intf_ptr->nx_interface_physical_address_lsw    = (ULONG)((ifp->mac_addr.octet[2] << 24) + (ifp->mac_addr.octet[3] << 16) + (ifp->mac_addr.octet[4] << 8) + ifp->mac_addr.octet[5]);
    intf_ptr->nx_interface_link_up                 = NX_FALSE;
    driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
    return;

  case NX_LINK_UNINITIALIZE:
    driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
    return;

  case NX_LINK_ENABLE:

    intf_ptr->nx_interface_link_up    = (UINT) NX_TRUE;
    driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
    return;

  case NX_LINK_DISABLE:
    intf_ptr->nx_interface_link_up    = NX_FALSE;
    driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
    break;

  case NX_LINK_MULTICAST_JOIN:

    WHD_get_MAC_from_driver(driver,&mac);

    if (whd_wifi_register_multicast_address(ifp,&mac) != WHD_SUCCESS)
    {
      driver->nx_ip_driver_status = (UINT) NX_NOT_SUCCESSFUL;
    }
    driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
    return;

  case NX_LINK_MULTICAST_LEAVE:

    WHD_get_MAC_from_driver(driver,&mac);

    if (whd_wifi_unregister_multicast_address(ifp,&mac) != WHD_SUCCESS)
    {
      driver->nx_ip_driver_status = (UINT) NX_NOT_SUCCESSFUL;
    }
    driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
    return;

  case NX_LINK_GET_STATUS:
    // Здесь мы должны вернуть реальное состояние подключения физического адаптера
    *(driver->nx_ip_driver_return_ptr) = (ULONG) intf_ptr->nx_interface_link_up; // Signal status through return pointer
    driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
    return;

  case NX_LINK_PACKET_SEND:
  case NX_LINK_ARP_RESPONSE_SEND:
  case NX_LINK_ARP_SEND:
  case NX_LINK_RARP_SEND:
  case NX_LINK_PACKET_BROADCAST:
    break;

  case NX_LINK_DEFERRED_PROCESSING:
  default:

    driver->nx_ip_driver_status = (UINT) NX_UNHANDLED_COMMAND; // Invalid driver request
    break;
  }

  if ((intf_ptr->nx_interface_link_up == NX_TRUE) && (whd_wifi_is_ready_to_transceive(ifp) == WHD_SUCCESS))
  {
    switch (driver->nx_ip_driver_command)
    {
    case NX_LINK_PACKET_SEND:
      if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4)
      {
        WHD_add_ethernet_header(intf_ptr, packet_ptr, driver->nx_ip_driver_physical_address_msw, driver->nx_ip_driver_physical_address_lsw, (USHORT) WHD_ETHERTYPE_IPv4);
      }
      else if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6)
      {
        WHD_add_ethernet_header(intf_ptr, packet_ptr, driver->nx_ip_driver_physical_address_msw, driver->nx_ip_driver_physical_address_lsw, (USHORT) WHD_ETHERTYPE_IPv6);
      }
      else
      {
        nx_packet_release(packet_ptr);
        break;
      }
      whd_network_send_ethernet_data(ifp, (whd_buffer_t) packet_ptr);
      driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
      break;

    case NX_LINK_ARP_RESPONSE_SEND:
      WHD_add_ethernet_header(intf_ptr, packet_ptr, driver->nx_ip_driver_physical_address_msw, driver->nx_ip_driver_physical_address_lsw, (USHORT) WHD_ETHERTYPE_ARP);
      whd_network_send_ethernet_data(ifp, (whd_buffer_t) packet_ptr);
      driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
      break;

    case NX_LINK_ARP_SEND:
      WHD_add_ethernet_header(intf_ptr, packet_ptr, (ULONG) 0xFFFF, (ULONG) 0xFFFFFFFF, (USHORT) WHD_ETHERTYPE_ARP);
      whd_network_send_ethernet_data(ifp, (whd_buffer_t) packet_ptr);
      driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
      break;

    case NX_LINK_RARP_SEND:
      WHD_add_ethernet_header(intf_ptr, packet_ptr, (ULONG) 0xFFFF, (ULONG) 0xFFFFFFFF, (USHORT) WHD_ETHERTYPE_RARP);
      whd_network_send_ethernet_data(ifp, (whd_buffer_t) packet_ptr);
      driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
      break;

    case NX_LINK_PACKET_BROADCAST:
      if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4)
      {
        WHD_add_ethernet_header(intf_ptr, packet_ptr, (ULONG) 0xFFFF, (ULONG) 0xFFFFFFFF, (USHORT) WHD_ETHERTYPE_IPv4);
      }
      else if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6)
      {
        WHD_add_ethernet_header(intf_ptr, packet_ptr, (ULONG) 0xFFFF, (ULONG) 0xFFFFFFFF, (USHORT) WHD_ETHERTYPE_IPv6);
      }
      else
      {
        nx_packet_release(packet_ptr);
        break;
      }
      whd_network_send_ethernet_data(ifp, (whd_buffer_t) packet_ptr);
      driver->nx_ip_driver_status = (UINT) NX_SUCCESS;
      break;

    case NX_LINK_MULTICAST_JOIN:
    case NX_LINK_MULTICAST_LEAVE:
    case NX_LINK_INITIALIZE:
    case NX_LINK_ENABLE:
    case NX_LINK_DISABLE:
    case NX_LINK_GET_STATUS:
    case NX_LINK_DEFERRED_PROCESSING:
    default:
      driver->nx_ip_driver_status =  NX_UNHANDLED_COMMAND;
      break;
    }
  }
  else
  {
    if (packet_ptr != NULL)
    {
      switch (driver->nx_ip_driver_command)
      {
      case NX_LINK_PACKET_BROADCAST:
      case NX_LINK_RARP_SEND:
      case NX_LINK_ARP_SEND:
      case NX_LINK_ARP_RESPONSE_SEND:
      case NX_LINK_PACKET_SEND:
        nx_packet_release(packet_ptr);
        break;

      default:
        break;
      }
    }
    else
    {
    }
  }
}


/*-----------------------------------------------------------------------------------------------------
   Функция вызывается в контексте задачи WHD при приеме пакетов из сети WiFi

  \param ifp
  \param buffer
-----------------------------------------------------------------------------------------------------*/
void WHD_network_process_ethernet_data(whd_interface_t ifp, whd_buffer_t buffer)
{
  USHORT          ethertype;
  NX_PACKET      *packet_ptr = (NX_PACKET *) buffer;
  unsigned char  *buff       = packet_ptr->nx_packet_prepend_ptr;


  ethertype = (USHORT)(buff[12] << 8 | buff[13]);


  // Check if this is an 802.1Q VLAN tagged packet
  if (ethertype == WHD_ETHERTYPE_8021Q)
  {
    //  Need to remove the 4 octet VLAN Tag, by moving src and dest addresses 4 octets to the right, and then read the actual ethertype.
    //  The VLAN ID and priority fields are currently ignored.
    //
    uint8_t temp_buffer[12];
    memcpy(temp_buffer, packet_ptr->nx_packet_prepend_ptr, 12);
    memcpy(packet_ptr->nx_packet_prepend_ptr + 4, temp_buffer, 12);

    packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr + 4;
    packet_ptr->nx_packet_length      = packet_ptr->nx_packet_length - 4;

    buff      = packet_ptr->nx_packet_prepend_ptr;
    ethertype = (USHORT)(buff[12] << 8 | buff[13]);
  }

  // Remove the ethernet header, so packet is ready for reading by NetX
  packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr + WHD_ETHERNET_SIZE;
  packet_ptr->nx_packet_length      = packet_ptr->nx_packet_length - WHD_ETHERNET_SIZE;

  // Здесь присваиваем указатель на интерфейс к которому принадлежит пакет  
  if (ifp == g_prim_whd_intf_ptr)
  {
    packet_ptr->nx_packet_address.nx_packet_interface_ptr = &(Net_get_ip_ptr()->nx_ip_interface[WIFI_STA_INTF_NUM]);
  }
  else
  {
    packet_ptr->nx_packet_address.nx_packet_interface_ptr = &(Net_get_ip_ptr()->nx_ip_interface[WIFI_AP_INTF_NUM]);
  }

  if ((ethertype == WHD_ETHERTYPE_IPv4) || ethertype == WHD_ETHERTYPE_IPv6)
  {
    _nx_ip_packet_deferred_receive(Net_get_ip_ptr(), packet_ptr);
  }
  else if (ethertype == WHD_ETHERTYPE_ARP)
  {
    _nx_arp_packet_deferred_receive(Net_get_ip_ptr(), packet_ptr);
  }
  else if (ethertype == WHD_ETHERTYPE_RARP)
  {
    _nx_rarp_packet_deferred_receive(Net_get_ip_ptr(), packet_ptr);
  }
  else
  {
    nx_packet_release(packet_ptr);
  }

}

