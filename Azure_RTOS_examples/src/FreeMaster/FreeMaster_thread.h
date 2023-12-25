#ifndef __FREEMASTER_LOOP
  #define __FREEMASTER_LOOP

  #define FMCMD_RESET_DEVICE                   0x01
  #define FMCMD_CHECK_LOG_PIPE                 0x07
  #define FMCMD_SAVE_APP_PARAMS                0x08
  #define FMCMD_SAVE_MODULE_PARAMS             0x09


  #define FREEMASTER_ON_NET                    0
  #define FREEMASTER_ON_SERIAL                 1

extern   T_serial_io_driver *frm_serial_drv;

uint32_t Thread_FreeMaster_create(void);
void     FreeMaster_task_delete(void);
void     Task_FreeMaster(uint32_t initial_data);
void     Determine_FreeMaster_interface_type(void);

#endif

