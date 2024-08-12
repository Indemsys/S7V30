#ifndef VT100_MONITOR_H
  #define VT100_MONITOR_H

  #define VT100_TASKS_MAX_NUM        5  // Максимальное количество одновременно работающих задач VT100 в системе
  #define VT100_TASK_NAME_MAX_SZ     32
  #define VT100_TASK_STACK_SIZE      (4*1024)

  #define COLCOUNT 100
  #define ROWCOUNT 24

  #define VT100_CNTLQ      0x11
  #define VT100_CNTLS      0x13
  #define VT100_DEL        0x7F
  #define VT100_BCKSP      0x08
  #define VT100_CR         0x0D  //конфликт имен с регистрами периферии
  #define VT100_LF         0x0A
  #define VT100_ESC        0x1B

/* cursor motion */
  #define VT100_CURSOR_DN   "\033D"  //
  #define VT100_CURSOR_DN_L "\033E"
  #define VT100_CURSOR_UP   "\033M"
  #define VT100_CURSOR_HOME "\033[H"
  #define VT100_CURSOR_N_UP "\033[%dA"  /* printf argument: lines */
  #define VT100_CURSOR_N_RT "\033[%dC"  /* printf argument: cols  */
  #define VT100_CURSOR_N_LT "\033[%dD"  /* printf argument: cols  */
  #define VT100_CURSOR_N_DN "\033[%dB"  /* printf argument: lines */
  #define VT100_CURSOR_SET  "\033[%d;%dH" /* printf arguments: row, col */

/* erasing the screen */
  #define VT100_CLR_FM_CRSR "\033[J"
  #define VT100_CLR_TO_CRSR "\033[1J"
  #define VT100_CLR_SCREEN  "\033[2J"

/* erasing current line */
  #define VT100_CLL_FM_CRSR "\033[K"
  #define VT100_CLL_TO_CRSR "\033[1K"
  #define VT100_CLR_LINE    "\033[2K"

/* inserting and deleting */
  #define VT100_INS_CHARS   "\033[%d"   /* printf argument: cols */
  #define VT100_DEL_CHARS   "\033[%dP"  /* printf argument: cols */
  #define VT100_INS_LINES   "\033[%dL"  /* printf argument: cols */
  #define VT100_DEL_LINES   "\033[%dM"  /* printf argument: cols */

/* character attributes */
  #define VT100_NORMAL      "\033[m"
  #define VT100_ALL_OFF     "\033[0m"
  #define VT100_BOLD_ON     "\033[1m"
  #define VT100_BOLD_OFF    "\033[22m"
  #define VT100_UNDERL_ON   "\033[4m"
  #define VT100_UNDERL_OFF  "\033[24m"
  #define VT100_BLINK_ON    "\033[5m"
  #define VT100_BLINK_OFF   "\033[25m"
  #define VT100_REVERSE_ON  "\033[7m"
  #define VT100_REVERSE_OFF "\033[27m"
  #define VT100_INVIS_ON    "\033[8m"
  #define VT100_INVIS_OFF   "\033[28m"

/* screen attributes */
  #define VT100_ECHO_ON     "\033[12l"
  #define VT100_ECHO_OFF    "\033[12h"
  #define VT100_WRAP_ON     "\033[?7l"
  #define VT100_WRAP_OFF    "\033[?7h"
  #define VT100_CURSOR_ON   "\033[?25h"
  #define VT100_CURSOR_OFF  "\033[?25l"
  #define VT100_ENQ_SIZE    "\033[?92l" /* response: "\033[?%d,%dc" rows, cols */

  #define VT100_CLEAR_AND_HOME "\033[2J\033[H\033[m\033[?25l"
  #define VT100_HOME "\033[H\033[m\033[?25l"


  #define COL        80   /* Maximum column size       */
  #define EDSTLEN    17


  #define DASH_LINE "----------------------------------------------------------------------\n\r"
  #define SCR_ITEMS_VERT_OFFS 8
  #define SCR_ITEMS_HOR_OFFS  1


  #define GET_MCBL     T_monitor_cbl    *mcbl = (T_monitor_cbl*)(tx_thread_identify()->environment); \
                       T_serial_io_driver *mdrv = (T_serial_io_driver*)(tx_thread_identify()->driver);

  #define MPRINTF      mdrv->_printf
  #define WAIT_CHAR    mdrv->_wait_char
  #define SEND_BUF     mdrv->_send_buf


  #define MN_DRIVER_MARK         0x87654321
  #define MN_RTT0_DRIVER         1
  #define MN_USBFS_VCOM0_DRIVER  2
  #define MN_USBFS_VCOM1_DRIVER  3
  #define MN_NET_TELNET_DRIVER   4
  #define MN_BLUETOOTH_DRIVER    5


typedef void (*T_menu_func)(uint8_t  keycode);

typedef struct
{
    uint8_t      but;       // Кнопка нажатие которой вызывает данный пункт
    T_menu_func  func;      // Функция вызываемая данным пунктом меню
    void        *psubmenu;  // Аргумент. Для подменю указатель на запись подменю
}
T_VT100_Menu_item;

typedef struct
{
    const uint8_t            *menu_header; // Заголовок вверху экрана посередине
    const uint8_t            *menu_body;   // Содержание меню
    const T_VT100_Menu_item  *menu_items;  // Массив структур с аттрибутами пунктов меню
}
T_VT100_Menu;

  #define MENU_MAX_DEPTH   20
  #define MAX_ITEMS_COUNT  16
  #define MONIT_STR_MAXLEN 127

typedef struct
{
    const uint32_t   mark;
    const int        driver_type;
    int              (*_init)(void **pcbl, void *pdrv);
    int              (*_send_buf)(const void *buf, unsigned int len);
    int              (*_wait_char)(unsigned char *b,  int ticks); // ticks - время ожидания выражается в тиках (если 0 то без ожидания)
    int              (*_printf)(const char *, ...);               // Возвращает неопределенный результат
    int              (*_deinit)(void **pcbl);
    void              *drv_cbl_ptr;                               // Указатель на управляющую структуру необходимую для работы драйвера
} T_serial_io_driver;


typedef struct
{
    T_VT100_Menu                    *menu_trace[MENU_MAX_DEPTH];
    uint32_t                         menu_nesting;
    uint32_t                         curr_pos;                      // Текущая позиция в области редактирования
    uint32_t                         beg_pos;                       // Начальная позиция области редактирования
    void                             (*Monitor_func)(unsigned char);

    uint32_t                         item_indexes[MAX_ITEMS_COUNT]; // Массив индексов пунктов меню. Предназначены для быстрого поиска пунктов в списках
    uint32_t                         current_menu_submenus_count;   // Количество субменю в текущем меню
    uint32_t                         current_menu_items_count;      // Общее количество пунктов всех типов в текущем меню
    uint32_t                         current_level;
    int32_t                          current_parameter_indx;        // Индекс редактируемого параметра
    uint8_t                          param_str[MONIT_STR_MAXLEN+1]; // Строка хранения параметра
    uint8_t                          out_str[MONIT_STR_MAXLEN+1];   // Буфферная строка для вывода
    uint32_t                         firstrow;                      // Позиция первой  строки области редактирования переменной
    uint32_t                         current_row;
    uint32_t                         current_col;
    uint32_t                         current_pos;
    int32_t                          g_access_to_spec_menu;
    const T_NV_parameters_instance  *p_pinst;                        // Указатель на структуру параметров
    uint8_t                          ptype;                          // Тип параметров
    T_serial_io_driver              *pdrv;                           // Указатель на драйвер

} T_monitor_cbl;

typedef struct
{
    uint32_t   row;
    uint8_t    b;
    char      *str1;
    char      *str2;
    uint32_t   key_mode;
    uint32_t   keys_mask;

}
T_sys_diagn_term;


typedef struct
{
    uint32_t       taken;       // Флаг занятого записи
    TX_THREAD      *VT100_thread_prt;
    uint8_t        *VT100_thread_stack;
    T_monitor_cbl  *monitor_cbl_ptr;
    char           task_name[VT100_TASK_NAME_MAX_SZ];

}  T_VT100_task_cbl;

#define VT100_MAG_QUEUE_BUF_LEN  10


typedef struct
{
  uint32_t arg1;
  uint32_t arg2;
  uint32_t arg3;
} T_vt100_man_msg;

typedef void (*T_vt100_man_callback)(T_vt100_man_msg *msg_ptr);


uint32_t     VT100_task_manager_initialization(void);
void         Send_message_to_VT100_task_manager(T_vt100_man_msg *msg_ptr);
uint32_t     Task_VT100_create(T_serial_io_driver *serial_drv_ptr, int32_t *task_instance_index_ptr);
uint32_t     Task_VT100_start(int32_t instance_indx);
uint32_t     Task_VT100_suspend(int32_t instance_indx);
uint32_t     Task_VT100_delete(int32_t instance_indx);


  #include "Main_menu_monitor.h"
  #include "Params_editor_monitor.h"
  #include "SD_card_monitor.h"
  #include "System_info_monitor.h"
  #include "Flash_control_monitor.h"
  #include "Bluetooth_monitor.h"
  #include "Tests_monitor.h"
  #include "Monitor_USB_drv.h"
  #include "Monitor_utilites.h"
  #include "Monitor_access_control.h"
  #include "Monitor_Telnet_driver.h"
  #include "Monitor_Bluetooth_drv.h"

#endif
