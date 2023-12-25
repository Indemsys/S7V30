#ifndef __PARAMS_WVAR_H
  #define __PARAMS_WVAR_H

#define  APP_0                          0
#define  APP_main                       1
#define  APP_General                    2

typedef struct
{
  uint32_t       en_iperf;                      // Enable IPerf | def.val.= 0
  uint8_t        manuf_date[64+1];              // Manufacturing date | def.val.= 2023 12 25 
  uint8_t        name[64+1];                    // Product  name | def.val.= PLCS7
} WVAR_TYPE;


#endif



// Selector description:  Выбор между Yes и No
#define BINARY_NO                                 0
#define BINARY_YES                                1


extern WVAR_TYPE  wvar;
extern const T_NV_parameters_instance wvar_inst;

