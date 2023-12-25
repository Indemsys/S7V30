// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-27
// 12:31:19
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"
#include "whd.h"


FMSTR_TSA_TABLE_BEGIN(app_vars)

FMSTR_TSA_RW_VAR(g_cpu_usage                             ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(g_aver_cpu_usage                        ,FMSTR_TSA_UINT32)

FMSTR_TSA_TABLE_END();

