// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-09-30
// 14:40:09
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"
#include   "USB_descriptors.h"
#include   "ux_device_class_rndis.h"
#include   "ux_device_class_cdc_ecm.h"


static UX_SLAVE_CLASS_RNDIS_PARAMETER   rndis_parameter;


NX_IP                      *rndis_ip_ptr;

static void RNDIS_instance_activate_callback(void *arg);
static void RNDIS_instance_deactivate_callback(void *arg);


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t возвращаем 1 если линк активен
-----------------------------------------------------------------------------------------------------*/
NX_INTERFACE* RNDIS_link_state(void)
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
-----------------------------------------------------------------------------------------------------*/
UINT Register_rndis_class(void)
{
  UINT               ret = UX_SUCCESS;
  rndis_parameter.ux_slave_class_rndis_instance_activate   = RNDIS_instance_activate_callback;
  rndis_parameter.ux_slave_class_rndis_instance_deactivate = RNDIS_instance_deactivate_callback;

  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[0] = 0x00;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[1] = 0x1e;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[2] = 0x58;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[3] = 0x41;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[4] = 0xb8;
  rndis_parameter.ux_slave_class_rndis_parameter_local_node_id[5] = 0x78;

  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[0] = 0x00;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[1] = 0x1e;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[2] = 0x58;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[3] = 0x41;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[4] = 0xb8;
  rndis_parameter.ux_slave_class_rndis_parameter_remote_node_id[5] = 0x79;

  rndis_parameter.ux_slave_class_rndis_parameter_vendor_id      = 0x045B;
  rndis_parameter.ux_slave_class_rndis_parameter_driver_version = 0x0003;

  g_uinf.idVendor  = DEVICE_VID2;
  g_uinf.idProduct = DEVICE_PID2;


  ux_utility_memory_copy(rndis_parameter.ux_slave_class_rndis_parameter_vendor_description,"RNDIS", 5);

  ret = ux_device_stack_class_register(_ux_system_slave_class_rndis_name, ux_device_class_rndis_entry, 1, USBD_RNDIS_INTERFACE_INDEX,&rndis_parameter);
  //ux_device_class_rndis_entry();
  return ret;
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте прерывания usbfs_int_isr в момент активизации класса RNDIS (подключения разъема или после подачи питания с уже подключенным разъемом)

  \param arg
-----------------------------------------------------------------------------------------------------*/
static void RNDIS_instance_activate_callback(void *arg)
{
  Send_net_event(NET_FLG_RNDIS_START);
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте прерывания usbfs_int_isr в момент деактивизации класса RNDIS (отключения разъема)

  \param arg
-----------------------------------------------------------------------------------------------------*/
static void RNDIS_instance_deactivate_callback(void *arg)
{
  Send_net_event(NET_FLG_RNDIS_STOP);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t RNDIS_to_start(void)
{
  ULONG ret_val;
  UX_SLAVE_CLASS_RNDIS* rndis      = (UX_SLAVE_CLASS_RNDIS* )_ux_system_slave->ux_system_slave_class_array[0].ux_slave_class_instance;
  USB_NETWORK_DEVICE_TYPE* net_dev = (USB_NETWORK_DEVICE_TYPE*)rndis->ux_slave_class_rndis_network_handle;

  net_dev->ux_network_device_usb_link_up = 1;

  // Здесь надо отдать команду драйверу на инициализацию, поскольку еще не согласован MAC адрес в WiFi модуле и драйвере интерфейса
  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_INITIALIZE, NET_USB_INTF_NUM, &ret_val);
  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_ENABLE, NET_USB_INTF_NUM, &ret_val);

  Net_mDNS_enable(NET_USB_INTF_NUM);

  NETLOG("RNDIS IP activated");
  return NX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  От устройства отключился RNDIS интерфейс
  Нужно остановить работу сервера  DHCP.
  Поскольку клиент может быть только один, то надо очистить список от предыдущего клиента.

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t RNDIS_to_stop(void)
{
  ULONG ret_val;
  UX_SLAVE_CLASS_RNDIS*     rndis  = (UX_SLAVE_CLASS_RNDIS* )_ux_system_slave->ux_system_slave_class_array[0].ux_slave_class_instance;
  USB_NETWORK_DEVICE_TYPE* net_dev = (USB_NETWORK_DEVICE_TYPE*)rndis->ux_slave_class_rndis_network_handle;

  Net_mDNS_disable(NET_USB_INTF_NUM);

  nx_ip_driver_interface_direct_command(Net_get_ip_ptr(), NX_LINK_DISABLE, NET_USB_INTF_NUM, &ret_val);

  net_dev->ux_network_device_usb_link_up = 0;

  NETLOG("RNDIS IP deactivated");
  return NX_SUCCESS;
}



