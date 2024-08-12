#ifndef MAIN_MONITOR_H
  #define MAIN_MONITOR_H

extern const T_VT100_Menu      MENU_MAIN;

void         Set_monitor_func(void (*func)(unsigned char));
void         Goto_main_menu(void);
void         Return_to_prev_menu(void);
void         Menu_press_key_handler(uint8_t b);
void         Display_menu(void);

#endif
