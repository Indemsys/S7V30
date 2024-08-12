// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-02
// 15:05:57
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------
  Найти  делители для тактовой частоты SCI обеспечивающие наилучшее совпадение с заданной битовой скоростью baud в битах в  секунду

  \param baud
  \param brme   - флаг разрешения модуляции
  \param abcse  - значение бита absce
  \param cks    - значение CKS для регистра SMR (в формуле жто переменная n)
  \param N      - значение для регистра BRR
  \param M      - значение для регистра MDDR

  Предполагается что биты BGDM = 0 и ABCS = 0

  \return uint32_t возвращает RES_ERROR если подходящих делителей найти не удалось
-----------------------------------------------------------------------------------------------------*/
uint32_t SCI_Find_best_divider(uint32_t  baud, uint8_t *brme, uint8_t *abcse,  uint8_t *cks, uint8_t *N, uint8_t *M)
{
  float    NDIV = 0;
  float    f_baud;
  uint32_t vN;
  uint32_t vM;
  float    err;

  float    min_err = 100;
  uint32_t best_N;
  uint32_t best_M;

  // Подбор подходящей частоты тактирования SCI с учетом что PCLKA = 120 МГц
  if (baud == 4000000)
  {
    //  speed = PCLKA_FREQ (Mhz)/ 12*(2^(2*cks-1))(N+1)
    *abcse = 1;
    cks    = 0;
    *brme  = 0; // Disable bit rate modulation function
    *N = (uint8_t)4;
    *M = (uint8_t)0;
    return RES_OK;
  }
  if (baud == 2000000)
  {
    //  speed = PCLKA_FREQ (Mhz)/ 12*(2^(2*cks-1))(N+1)
    *abcse = 1;
    cks    = 0;
    *brme  = 0; // Disable bit rate modulation function
    *N = (uint8_t)9;
    *M = (uint8_t)0;
    return RES_OK;
  }
  if (baud == 1000000)
  {
    //  speed = PCLKA_FREQ (Mhz)/ 12*(2^(2*cks-1))(N+1)
    *abcse = 1;
    cks    = 0;
    *brme  = 0; // Disable bit rate modulation function
    *N = (uint8_t)19;
    *M = (uint8_t)0;
    return RES_OK;
  }
  if (baud < 120)
  {
    return RES_ERROR;
  }
  if (baud < 7500)
  {
    //  speed = PCLKA_FREQ (Mhz)/ (256/M)*64*(2^(2*cks-1))(N+1)
    *brme  = 1;
    *cks   = 3;
    *abcse = 0;
    NDIV = 2048.0f; //
  }
  else if (baud > 1000000)
  {
    //  speed = PCLKA_FREQ (Mhz)/ (256/M)*12*(2^(2*cks-1))(N+1)
    *brme  = 1;
    *cks   = 0;
    *abcse = 1;
    NDIV = 6.0f;   //
  }
  else // between 7500 ... 1000000
  {
    //  speed = PCLKA_FREQ (Mhz)/ (256/M)*64*(2^(2*cks-1))(N+1)
    *brme  = 1;
    *cks   = 0;
    *abcse = 0;
    NDIV = 32.0f;
  }

  for (vN = 0; vN < 256; vN++)
  {
    for (vM=128; vM < 256; vM++)
    {
      f_baud          = PCLKA_FREQ / (NDIV * (256.0f / vM) * (vN+1));
      err             = fabsf((f_baud / (float)baud)-1.0f);
      if (err < min_err)
      {
        min_err = err;
        best_N = vN;
        best_M = vM;
      }
    }
  }

  if (best_N == 255) return RES_ERROR;

  *N = (uint8_t)best_N;
  *M = (uint8_t)best_M;

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Длина

  \param suid
-----------------------------------------------------------------------------------------------------*/
void Get_CPU_UID(char *suid, uint8_t *uid, uint32_t suid_len)
{
  fmi_unique_id_t unique_id;
  g_fmi.p_api->uniqueIdGet(&unique_id);
  if (uid != 0)
  {
    memcpy(uid,&unique_id, CPU_ID_LEN);
  }

  if (suid != 0)
  {
    snprintf(suid, suid_len, "%02X%02X%02X%02X", unique_id.unique_id[0], unique_id.unique_id[1], unique_id.unique_id[2], unique_id.unique_id[3]);
  }
}


/*------------------------------------------------------------------------------
 Сброс системы
 ------------------------------------------------------------------------------*/
void Reset_SoC(void)
{
  __disable_interrupt();
  // System Control Block -> Application Interrupt and Reset Control Register
  *(uint32_t *)(0xE000ED0C)= 0x05FA0000 // Обязательный шаблон при записи в  регистр SCB_AIRCR
                            | BIT(2);  // Установка бита SYSRESETREQ
  for (;;)
  {
    __no_operation();
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param lock

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
inline uint32_t Atomic_lock(uint8_t *lock)
{
  uint32_t  err;
  uint8_t   res;
  uint8_t   retry;

  err = RES_ERROR;

  do
  {
    retry = 0;
    res = __LDREXB(lock);
    if (res == 0)
    {
      /* Lock is available. Attempt to lock it. */
      if (__STREXB(1,lock) == 0)
      {
        err = RES_OK;
      }
      // Не возвращаемся пока не захватим lock
      //else
      //{
      //  retry = 1;
      //}
    }
    else
    {
      __CLREX();
    }
  } while (retry == 1);

  return err;
}


inline void Atomic_unlock(uint8_t *lock)
{
  *lock = 0;
}

