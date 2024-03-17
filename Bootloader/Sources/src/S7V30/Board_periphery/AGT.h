#ifndef S7V30_AGT_H
  #define S7V30_AGT_H

#define AGT_CLOCK_FREQ  PCLKB_FREQ

void Init_AGT0(uint16_t rate);
void Init_AGT1_ticks_DAC_to_DTC(uint16_t rate);

#endif



