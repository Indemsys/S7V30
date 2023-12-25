#include "S7V30.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"

FMSTR_TSA_TABLE_LIST_BEGIN()

FMSTR_TSA_TABLE(wvar_tbl)
FMSTR_TSA_TABLE(ivar_tbl)
FMSTR_TSA_TABLE(app_vars)

FMSTR_TSA_TABLE_LIST_END()
