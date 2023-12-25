#ifndef __FREEMASTER_WVAR_H
  #define __FREEMASTER_WVAR_H
#include "App.h"
#include   "freemaster_tsa.h"


FMSTR_TSA_TABLE_BEGIN(wvar_tbl)
FMSTR_TSA_RW_VAR( wvar.en_iperf                        ,FMSTR_TSA_UINT32    ) // Enable IPerf | def.val.= 0
FMSTR_TSA_TABLE_END();


#endif
