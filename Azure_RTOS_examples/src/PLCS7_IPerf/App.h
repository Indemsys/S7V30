#ifndef APP_H
  #define APP_H

  #include "S7V30.h"

  #define     SOFTWARE_VERSION         "APP"
  #define     HARDWARE_VERSION         "APP"


  #include "App_task.h"
  #include "Params.h"
  #include "App_monitor.h"
  #include "App_freemaster.h"


  #define     ENC_LED_RD          R_PFS->P514PFS_b.PODR
  #define     ENC_LED_GR          R_PFS->P513PFS_b.PODR


void      S7V30_board_pins_init(void);
uint32_t  S7V30_get_board_pin_count(void);
void      S7V30_get_board_pin_conf_str(uint32_t pin_num, char *dstr);

#endif


