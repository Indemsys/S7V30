#ifndef S7V30_SDHI1_H
  #define S7V30_SDHI1_H


void      DMA3_int_isr(void);
void      SDHI1_accs_isr(void);
void      SDHI1_sdio_isr(void);
void      SDHI1_card_isr(void);

void      SDIO1_irq_disable(void);
void      SDIO1_irq_enable(void);

uint32_t  Init_SDIO1(void);
uint32_t  Deinit_SDIO1(void);

uint32_t  SDIO1_wait_transfer_complete(uint32_t timeout_ms);

#endif



