// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-05
// 13:43:56
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


const T_VT100_Menu_item MENU_TESTS_ITEMS[] =
{
  { '1', Do_SDRAM_test,    0},
  { '2', Do_BQ25619_test,  0},
  { '4', Do_ISM330_test,   0},
  { '5', Do_SX1262_test,   0},
  { '6', Do_MAX17262_test, 0},
  { '7', Do_AB1815_test,   0},
  { 'R', 0, 0 },
  { 'M', 0, (void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_TESTS  =
{
  "MONITOR Ver.23002002",
  "\033[5C Perepherial tests \r\n"
  "\033[5C <1> - SDRAM     test\r\n"
  "\033[5C <2> - BQ25619   test\r\n"
  "\033[5C <4> - ISM330    test\r\n"
  "\033[5C <5> - SX1262    test\r\n"
  "\033[5C <6> - MAX17262  test\r\n"
  "\033[5C <7> - AB1815    test\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_TESTS_ITEMS,
};




