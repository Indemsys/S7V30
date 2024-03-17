#ifndef ELC_SOFTWARE_EVENT_1_H
  #define ELC_SOFTWARE_EVENT_1_H

typedef   void   (*T_ELS_soft_event_callback)(void);

void ELS_soft_event_set_and_enable_interrupts(void);
void Set_ELS_soft_event_callback(T_ELS_soft_event_callback func);


#endif



