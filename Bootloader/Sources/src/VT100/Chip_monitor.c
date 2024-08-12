// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-05
// 13:58:37
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"



const T_VT100_Menu_item MENU_S7V30_ITEMS[] =
{
  { '1', Do_S7V30_System_info           , 0                        },
  { '2', 0                              , (void *)&MENU_TESTS      },
  { 'R', 0                              , 0                        },
  { 'M', 0                              , (void *)&MENU_MAIN       },
  { 0                                                 }
};

const T_VT100_Menu      MENU_S7V30          =
{
  "S7V30 MENU",
  "\033[5C MAIN MENU \r\n"
  "\033[5C <1> - System info\r\n"
  "\033[5C <2> - Peripherial tests\r\n",
  MENU_S7V30_ITEMS,
};

