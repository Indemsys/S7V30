#ifndef APP_HOST_CDC_ECM_H
  #define APP_HOST_CDC_ECM_H

uint32_t USB_host_cdc_ecm_init(void);
UINT     USB_host_change_function(ULONG event, UX_HOST_CLASS *host_class, VOID *instance);

#endif



