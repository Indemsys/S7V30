#ifndef APP_MONITOR_H
  #define APP_MONITOR_H


#define APP_MENU        (void *)&MENU_APPLICATION
#define APP_MENU_STR    "\033[5C <2> - APP menu\r\n"

#define   MON_VTYPE__BOOL  1
#define   MON_VTYPE_INT32  2
#define   MON_VTYPE_FLOAT  3
#define   MON_VTYPE_STRNG  4

typedef   float   (*T_monitor_conv_func)(int32_t);
typedef   int32_t (*T_monitor_get_func)(void);
typedef   void    (*T_monitor_set_func)(int32_t);


typedef struct
{
    uint8_t                   row;
    uint8_t                   col;
    char                      cmd_sym;  // Символ редактирования если редактирование доступно
    uint8_t                   vtype;    // Представлять символ как бинарный
    const char *const         fmt;
    const char *const         name;
    T_monitor_get_func        getv;     // Функция чтения значения
    T_monitor_set_func        setv;     // Функция запис значения
    void                     *cfunc;     // Функция конвертирования значения или нетипизированная функция
    const char *const         cfmt;

} T_monitor_sigs_map;

extern const T_VT100_Menu MENU_APPLICATION;


#endif //



