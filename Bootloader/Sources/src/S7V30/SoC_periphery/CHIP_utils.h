#ifndef CHIP_UTILS_H
  #define CHIP_UTILS_H



#define SCI_NO_PRITY    0
#define SCI_EVEN_PRITY  2
#define SCI_ODD_PRITY   3


#define SCI_1_STOP_BIT  0
#define SCI_2_STOP_BIT  1




uint32_t SCI_Find_best_divider(uint32_t  baud, uint8_t *brme, uint8_t *abcse,  uint8_t *cks, uint8_t *N, uint8_t *M);
void     Get_CPU_UID(char *suid, uint8_t *uid, uint32_t suid_len);
void     Reset_SoC(void);

extern  uint32_t Atomic_lock(uint8_t *lock);
extern  void     Atomic_unlock(uint8_t *lock);

#endif // RESET_H



