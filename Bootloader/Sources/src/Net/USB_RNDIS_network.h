#ifndef USB_RNDIS_DRIVER_H
  #define USB_RNDIS_DRIVER_H

UINT          Register_rndis_class(void);
uint32_t      RNDIS_to_start(void);
uint32_t      RNDIS_to_stop(void);
NX_INTERFACE *RNDIS_link_state(void);

#endif



