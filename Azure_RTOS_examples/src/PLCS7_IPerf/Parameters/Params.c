#include "App.h"
#include   "freemaster_tsa.h"


#define  WVAR_SIZE        3
#define  PARMNU_ITEM_NUM   2

#define  SELECTORS_NUM     2

WVAR_TYPE  wvar;


static const T_parmenu parmenu[PARMNU_ITEM_NUM]=
{
{ APP_0                       , APP_main                    , "Parameters and settings    ", "PARAMETERS          ", -1   }, // Основная категория
{ APP_main                    , APP_General                 , "General settings           ", "GENERAL_SETTINGS    ", -1   }, // 
};


static const T_NV_parameters arr_wvar[WVAR_SIZE]=
{
// N: 0
  {
    "name",
    "Product  name",
    "SYSNAM",
    (void*)&wvar.name,
    tstring,
    0,
    0,
    0,
    0,
    APP_General,
    "PLCS7",
    "%s",
    0,
    sizeof(wvar.name)-1,
    1,
    0,
  },
// N: 1
  {
    "manuf_date",
    "Manufacturing date",
    "FRMVER",
    (void*)&wvar.manuf_date,
    tstring,
    0,
    0,
    0,
    0,
    APP_General,
    "2023 12 25 ",
    "%s",
    0,
    sizeof(wvar.manuf_date)-1,
    2,
    0,
  },
// N: 2
  {
    "en_iperf",
    "Enable IPerf",
    "-",
    (void*)&wvar.en_iperf,
    tint32u,
    0,
    0,
    1,
    0,
    APP_General,
    "",
    "%d",
    0,
    sizeof(wvar.en_iperf),
    3,
    0,
  },
};
 
 
// Selector description:  Выбор между Yes и No
static const T_selector_items selector_1[2] = 
{
  { 0 , "No                                          " , 0},
  { 1 , "Yes                                         " , 1},
};
 
static const T_selectors_list selectors_list[SELECTORS_NUM] = 
{
  {"string"                      , 0    , 0             },
  {"binary"                      , 2    , selector_1    },
};
 
const T_NV_parameters_instance wvar_inst =
{
  WVAR_SIZE,
  arr_wvar,
  PARMNU_ITEM_NUM,
  parmenu,
  SELECTORS_NUM,
  selectors_list
};
