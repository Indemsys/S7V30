#include "App.h"
#include "Net.h"
#include "ux_hcd_synergy.h"



/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t USB_host_cdc_ecm_init(void)
{
  uint32_t res;

  res =  ux_host_stack_initialize(USB_host_change_function);
  if (res == UX_SUCCESS)
  {
    // Еше возможные варианты
    // ux_host_stack_class_register(_ux_system_host_class_hub_name, ux_host_class_hub_entry);
    // ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name, ux_host_class_cdc_acm_entry);
    // ux_host_stack_class_register(_ux_system_host_class_hid_name, ux_host_class_hid_entry);
    // ux_host_class_hid_clients_register();
    // ux_host_stack_class_register_storage();
    // ux_host_stack_class_register(_ux_system_host_class_video_name, ux_host_class_video_entry);
    // ux_host_stack_class_register(_ux_system_host_class_audio_name, ux_host_class_audio_entry);



    res =  ux_host_stack_class_register(_ux_system_host_class_cdc_ecm_name, ux_host_class_cdc_ecm_entry);
    if (res == UX_SUCCESS)
    {
      if (ivar.usd_dev_interface == USB_DEV_INTERFACE_HIGH_SPEED_INTERFACE)
      {
        res = ux_host_stack_hcd_register((UCHAR *) "g_sf_el_ux_hcd_hs_0", USB_initialize_hcd_transfer_support, R_USBHS_BASE, UX_SYNERGY_CONTROLLER);
      }
      else
      {
        res = ux_host_stack_hcd_register((UCHAR *) "g_sf_el_ux_hcd_fs_0", USB_initialize_hcd_transfer_support, R_USBFS_BASE, UX_SYNERGY_CONTROLLER);
      }
    }
  }

  return res;
}


/*-----------------------------------------------------------------------------------------------------
   Функция вызывается из функции _ux_host_class_cdc_ecm_activate в контексте задачи ux_system_host_enum_thread

  Цепочка вызовов:
   ux_system_host_enum_thread  ->
   _ux_host_stack_rh_change_process  ->
   _ux_host_stack_rh_device_insertion  ->
   _ux_host_stack_new_device_create  ->
   _ux_host_stack_class_interface_scan  ->
   _ux_host_stack_configuration_interface_scan  ->
   _ux_host_class_cdc_ecm_entry->
   _ux_host_class_cdc_ecm_activate -> USB_host_change_function

  \param event
  \param host_class
  \param instance

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT USB_host_change_function(ULONG event, UX_HOST_CLASS *host_class, VOID *instance)
{
  UX_HOST_CLASS_CDC_ECM    *ecm_host_class = (UX_HOST_CLASS_CDC_ECM *)instance;

  if (event == UX_DEVICE_INSERTION) /* Check if there is a device insertion. */
  {
    if (_ux_utility_memory_compare(_ux_system_host_class_cdc_ecm_name, host_class->ux_host_class_name ,_ux_utility_string_length_get(_ux_system_host_class_cdc_ecm_name)) == UX_SUCCESS)
    {


      g_uinf.idVendor  = ecm_host_class->ux_host_class_cdc_ecm_device->ux_device_descriptor.idVendor;
      g_uinf.idProduct = ecm_host_class->ux_host_class_cdc_ecm_device->ux_device_descriptor.idProduct;
      g_uinf.dev_state = ecm_host_class->ux_host_class_cdc_ecm_device->ux_device_state;

      g_uinf.interface_id  = ecm_host_class->ux_host_class_cdc_ecm_interface_data->ux_interface_descriptor.bInterfaceClass;
      g_uinf.interface_num = ecm_host_class->ux_host_class_cdc_ecm_interface_data->ux_interface_descriptor.bInterfaceNumber;

      NETLOG("USB ECM host: Inserted device VID=%04X PID=%04X Intf.Class=%04X Intf.Num=%d Dev.State=%d. Class=%s", g_uinf.idVendor , g_uinf.idProduct, g_uinf.interface_id, g_uinf.interface_num, g_uinf.dev_state, host_class->ux_host_class_name);

      uint8_t *m = ecm_host_class->ux_host_class_cdc_ecm_node_id;
      // К этому моменту уже была выполнена функция _ux_network_driver_activate

      NETLOG("USB ECM host: Inserted device MAC : %02X %02X %02X %02X %02X %02X", m[0],m[1],m[2],m[3],m[4],m[5]);
      Send_net_event(NET_FLG_ECM_HOST_START);
    }
  }
  else if (event == UX_DEVICE_REMOVAL) /* Check if there is a device removal. */
  {
    NETLOG("USB ECM host: Device removed");
    Send_net_event(NET_FLG_ECM_HOST_STOP);
  }
  return UX_SUCCESS;
}


