// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.06.23
// 12:41:17
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "sdhi.h"

#define WIFI_SDIO_BLOCK_SIZE 64

extern  void WHD_thread_notify_irq(void);

//........................................................................................
//  Организация канала передачи для SDIO интерфейса Wi-Fi модуля
//  Используется модуль DMAC канал 3
//........................................................................................
SSP_VECTOR_DEFINE_CHAN(DMA3_int_isr,        DMAC,    INT,     3);

SSP_VECTOR_DEFINE_CHAN(SDHI1_accs_isr,      SDHIMMC, ACCS,    1);
SSP_VECTOR_DEFINE_CHAN(SDHI1_sdio_isr,      SDHIMMC, SDIO,    1);
SSP_VECTOR_DEFINE_CHAN(SDHI1_card_isr,      SDHIMMC, CARD,    1);


dmac_instance_ctrl_t DMA_CH3_control_block;

transfer_info_t DMA_CH3_transfer_info =
{
  .dest_addr_mode      = TRANSFER_ADDR_MODE_FIXED,
  .repeat_area         = TRANSFER_REPEAT_AREA_SOURCE,
  .irq                 = TRANSFER_IRQ_EACH,
  .chain_mode          = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode       = TRANSFER_ADDR_MODE_INCREMENTED,
  .size                = TRANSFER_SIZE_1_BYTE,
  .mode                = TRANSFER_MODE_NORMAL,
  .p_dest              = (void *) NULL,
  .p_src               = (void const *) NULL,
  .num_blocks          = 0,
  .length              = 0,
};

const transfer_on_dmac_cfg_t DMA_CH3_extended_configuration =
{
  .channel             = 3,
  .offset_byte         = 0,
};

const transfer_cfg_t DMA_CH3_configuration =
{
  .p_info              =&DMA_CH3_transfer_info,
  .activation_source   = ELC_EVENT_SDHIMMC1_DMA_REQ,
  .auto_enable         = false,
  .p_callback          = NULL,
  .p_context           =&DMA_CH3_transfer_instance,
  .irq_ipl             =(BSP_IRQ_DISABLED),           // Приоритет прерывания происходящего по окончании пересылки данных по DMA
  .p_extend            =&DMA_CH3_extended_configuration,
};

const transfer_instance_t DMA_CH3_transfer_instance =
{
  .p_ctrl        =&DMA_CH3_control_block,
  .p_cfg         =&DMA_CH3_configuration,
  .p_api         =&g_transfer_on_dmac
};





sdmmc_instance_ctrl_t g_sdhi1_ctrl;


const sdmmc_extended_cfg_t g_sdio1_ext_cfg_t =
{
  .block_size      = 512,
  .card_detect     = SDMMC_CARD_DETECT_NONE,
  .write_protect   = SDMMC_WRITE_PROTECT_NONE,
};

sdmmc_cfg_t g_sdhi1_cfg =
{
  .hw =
  {
    .media_type    = SDMMC_MEDIA_TYPE_CARD,
#ifdef SDIO1_1BIT_MODE
    .bus_width     = SDMMC_BUS_WIDTH_1_BIT,
#else
    .bus_width     = SDMMC_BUS_WIDTH_4_BITS,
#endif
    .channel       = 1,
  },
  .p_callback      = NULL,              // Указатель на функцию вызываемую из разных прерываний accs_isr, card_isr, sdio_isr в штатном драйвере
                                        // Функция централизовано реагирует на события случающиеся с картой:
  //   SDMMC_EVENT_CARD_REMOVED
  //   SDMMC_EVENT_CARD_INSERTED
  //   SDMMC_EVENT_ACCESS
  //   SDMMC_EVENT_SDIO
  //   SDMMC_EVENT_TRANSFER_COMPLETE
  //   SDMMC_EVENT_TRANSFER_ERROR
  //  Здесь не используем поскольку реализуем собственные обработчики прерываний
  //
  .p_extend        = (void *)&g_sdio1_ext_cfg_t,
  .p_lower_lvl_transfer   =&DMA_CH3_transfer_instance,
  .access_ipl      =(7),                // Приоритет прерывания возникающего после передачи команд или в случае ошибки на шине SDIO
  .sdio_ipl        =(7),                // Приоритет прерывания возникающего в момент прихода пакета прерывания от SDIO модуля
  .card_ipl        =(BSP_IRQ_DISABLED), // Приоритет прерывания происходящего при установке или извлечении SD карты
  .dma_req_ipl     =(BSP_IRQ_DISABLED),

};

const sdmmc_instance_t g_sdio1 =
{
  .p_ctrl         =&g_sdhi1_ctrl,
  .p_cfg          =&g_sdhi1_cfg,
  .p_api          =&g_sdmmc_on_sdmmc
};

static IRQn_Type    DMA3_IRQn;
static IRQn_Type    SDHI1_ACCS_IRQn;    // SD Card Access Interrupt
static IRQn_Type    SDHI1_SDIO_IRQn;    // SDIO Access Interrupt
static IRQn_Type    SDHI1_CARD_IRQn;    // Card Detect Interrupt
static IRQn_Type    SDHI1_DMA_REQ_IRQn;
static elc_event_t  DMA3_event;
static elc_event_t  SDHI1_ACCS_event;
static elc_event_t  SDHI1_SDIO_event;
static elc_event_t  SDHI1_CARD_event;
static elc_event_t  SDHI1_DMA_REQ_event;

typedef struct
{
    uint32_t cnt_accs_isr;
    uint32_t cnt_resp_end;
    uint32_t cnt_access_end;
    uint32_t cnt_sdio_isr;
    uint32_t cnt_card_isr;
    uint32_t cnt_dma_req_isr;
    uint32_t cnt_CMDE;
    uint32_t cnt_CRCE;
    uint32_t cnt_ENDE;
    uint32_t cnt_DTO;
    uint32_t cnt_ILW;
    uint32_t cnt_ILR;
    uint32_t cnt_RSPTO;
    uint32_t cnt_ILA;
} T_sdio_counters;

T_sdio_counters sdc;


#define   SDIO1_TRANSFER_COMPLETE  BIT(0)
#define   SDIO1_TRANSFER_ERROR     BIT(1)

TX_EVENT_FLAGS_GROUP  sdio1_flags;

/*-----------------------------------------------------------------------------------------------------


  \param info2
  \param info1
-----------------------------------------------------------------------------------------------------*/
static void SDHI1_errors_count(uint32_t info2, uint32_t info1)
{
  if (info2 & BIT(0)) sdc.cnt_CMDE++;
  if (info2 & BIT(1)) sdc.cnt_CRCE++;
  if (info2 & BIT(2)) sdc.cnt_ENDE++;
  if (info2 & BIT(3)) sdc.cnt_DTO++;
  if (info2 & BIT(4)) sdc.cnt_ILW++;
  if (info2 & BIT(5)) sdc.cnt_ILR++;
  if (info2 & BIT(6))
  {
    sdc.cnt_RSPTO++;
  }
  if (info2 & BIT(15)) sdc.cnt_ILA++;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_ctrl
-----------------------------------------------------------------------------------------------------*/
static void SDHI1_transfer_end(void)
{
  g_sdhi1_ctrl.transfer_block_current = 0U;
  g_sdhi1_ctrl.transfer_blocks_total  = 0U;
  g_sdhi1_ctrl.p_transfer_data        = NULL;
  g_sdhi1_ctrl.transfer_dir           = SDMMC_TRANSFER_DIR_NONE;
  g_sdhi1_ctrl.transfer_block_size    = 0U;
  g_sdhi1_ctrl.p_lower_lvl_transfer->p_api->close(g_sdhi1_ctrl.p_lower_lvl_transfer->p_ctrl);
  R_SDHI1->SD_DMAEN &= (uint32_t) ~0x02;
  g_sdhi1_ctrl.transfer_in_progress = false;
}


/*-----------------------------------------------------------------------------------------------------
  Обслуживание прерывания DMA канала 3
  Происходит по окончании цикла DMA
-----------------------------------------------------------------------------------------------------*/
void DMA3_int_isr(void)
{
  R_ICU->IELSRn_b[(uint32_t)DMA3_IRQn].IR = 0U;

  // Здесь перенос даных между целевым буфером и промежуточным буфером драйвера SDIO в случае если целевой буфер данных или его размер не выровнен по границе 4 байта
  if (g_sdhi1_ctrl.transfer_blocks_total != g_sdhi1_ctrl.transfer_block_current)
  {
    if (g_sdhi1_ctrl.transfer_dir == SDMMC_TRANSFER_DIR_READ)
    {
      memcpy(g_sdhi1_ctrl.p_transfer_data, (void *)&g_sdhi1_ctrl.aligned_buff[0],  g_sdhi1_ctrl.transfer_block_size);
      //r_sdmmc_software_copy((void *)&g_sdhi1_ctrl.aligned_buff[0], g_sdhi1_ctrl.transfer_block_size, g_sdhi1_ctrl.p_transfer_data);
    }
    if (g_sdhi1_ctrl.transfer_dir == SDMMC_TRANSFER_DIR_WRITE)
    {
      memcpy((void *)&g_sdhi1_ctrl.aligned_buff[0], g_sdhi1_ctrl.p_transfer_data, g_sdhi1_ctrl.transfer_block_size);
      //r_sdmmc_software_copy(g_sdhi1_ctrl.p_transfer_data, g_sdhi1_ctrl.transfer_block_size, (void *)&g_sdhi1_ctrl.aligned_buff[0]);
    }
    g_sdhi1_ctrl.transfer_block_current++;
    g_sdhi1_ctrl.p_transfer_data += g_sdhi1_ctrl.transfer_block_size;
  }
  if (g_sdhi1_ctrl.transfer_blocks_total == g_sdhi1_ctrl.transfer_block_current)
  {
    if (g_sdhi1_ctrl.transfer_dir == SDMMC_TRANSFER_DIR_READ)
    {
      SDHI1_transfer_end();
      tx_event_flags_set(&sdio1_flags, SDIO1_TRANSFER_COMPLETE, TX_OR);
    }
  }

  // Transfer is disabled during the interrupt if an interrupt is requested after each block. If not all transfers are complete, reenable transfer here.
  if (R_DMAC3->DMCRB > 0U)
  {
    R_DMAC3->DMCNT_b.DTE = 1;
  }
  __DSB();
}


/*-----------------------------------------------------------------------------------------------------
  Обслуживание прерываний SD Card Access Interrupt
  Прерывание возникает во время ожидания выполнения команды r_sdmmc_command_send

  В микроконтроллере это прерывание возникает как реакция на события:

  ----------------------------------------------------------
  Регистр SD Card Interrupt Flag Register 1 (SD_INFO1)
  ----------------------------------------------------------
  бит  мнемоника  описание
  0    RSPEND     Response End Detection Flag
  2    ACEND      Access End Detection Flag


  ----------------------------------------------------------
  Регистр SD Card Interrupt Flag Register 2 (SD_INFO2)
  ----------------------------------------------------------
  бит  мнемоника  описание
  0    CMDE       Command Error Detection Flag
  1    CRCE       CRC Error Detection Flag
  2    ENDE       End Bit Error Detection Flag
  3    DTO        Data Timeout Detection Flag
  4    ILW        SD_BUF0 Illegal Write Access Detection Flag
  5    ILR        SD_BUF0 Illegal Read Access Detection Flag
  6    RSPTO      Response Timeout Detection Flag
  15   ILA        Illegal Access Error Detection Flag


  Это прерывание возникает слишком часто при пересылке данных по SDIO. Надо выяснить причину!
-----------------------------------------------------------------------------------------------------*/
void SDHI1_accs_isr(void)
{
  uint32_t       info1;
  uint32_t       info2;

  sdhi_event_t   flags = {0U};

  flags.word = g_sdhi1_ctrl.sdhi_event.word;

  /* Clear stop register after access end. */
  R_SDHI1->SD_STOP_b.STP = 0;

  /* Read interrupt flag registers. */
  info1        = R_SDHI1->SD_INFO1;
  info2        = R_SDHI1->SD_INFO2;

  info1       &= SDMMC_SDHI_INFO1_ACCESS_MASK;
  info2       &= SDMMC_SDHI_INFO2_MASK;

  sdc.cnt_accs_isr++;

  /* Combine all flags in one 32 bit word. */
  flags.word |=(info1 | (info2 << 16));



  /* Combine only errors in one bit. */
  if (flags.word & SDMMC_ACCESS_ERROR_MASK)
  {
    flags.bit.event_error = 1U;
    SDHI1_errors_count(info2,info1);
  }

  g_sdhi1_ctrl.sdhi_event.word |= flags.word;

  // Проверка флага RSPEND Response End Detection Flag  полученного из регистра SD_INFO1 бита 0
  // Установку этого флага проверяет функция r_sdmmc_command_send, как условие возврата. Эта функция общаая для всех обменов по SDMMC
  //
  if (flags.bit.response_end)
  {
    /* Disable response end interrupt (set the bit) and enable access end interrupt (clear the bit). */
    uint32_t mask = R_SDHI1->SD_INFO1_MASK;
    mask &=(~SDMMC_SDHI_INFO1_ACCESS_END);
    mask |= SDMMC_SDHI_INFO1_RESPONSE_END;
    R_SDHI1->SD_INFO1_MASK = mask;
    sdc.cnt_resp_end++;
  }

  /* Clear interrupt flags */
  R_SDHI1->SD_INFO1 =(~info1);
  R_SDHI1->SD_INFO2 =(~info2);
  R_ICU->IELSRn_b[(uint32_t)SDHI1_ACCS_IRQn].IR = 0U;

  if (g_sdhi1_ctrl.transfer_in_progress)
  {
    // Проверка единого флага всех ошибок
    if (g_sdhi1_ctrl.sdhi_event.bit.event_error)
    {
      R_SDHI1->SD_STOP_b.STP = 1;
      SDHI1_transfer_end();
      tx_event_flags_set(&sdio1_flags, SDIO1_TRANSFER_ERROR, TX_OR);
    }
    else
    {
      // Проверка флага ACEND Access End Detection Flag полученного из регистра SD_INFO1 бита 2
      if (g_sdhi1_ctrl.sdhi_event.bit.access_end)
      {
        /* Read transfer end and callback performed in transfer interrupt. */
        if (g_sdhi1_ctrl.transfer_dir != SDMMC_TRANSFER_DIR_READ)
        {

          SDHI1_transfer_end();
          tx_event_flags_set(&sdio1_flags, SDIO1_TRANSFER_COMPLETE, TX_OR);
        }
        sdc.cnt_access_end++;
      }
    }
  }

  R_SDHI1->SD_INFO2_MASK |=(BIT(8) | BIT(9)); // bit 8 BREM - Mask read enable interrupt request for the SD buffer
                                              // bit 9 BWEM - Mask write enable interrupt request for the SD_BUF0 register.

  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Прерывание возникает в момент прихода пакета прерывания от SDIO модуля

-----------------------------------------------------------------------------------------------------*/
void SDHI1_sdio_isr(void)
{
  uint32_t info1 = R_SDHI1->SDIO_INFO1;

  R_SDHI1->SDIO_INFO1 = 0x00000006;  // Данный паттерн сбрасывает все флаги прерываний в регистре R_SDHI1->SDIO_INFO1
  R_ICU->IELSRn_b[(uint32_t)SDHI1_SDIO_IRQn].IR = 0U;

  // Проверяем флаги:
  //  EXWT    (bit 15) EXWT Status Flag.      Indicates the status of the EXWT.    While the last block in the CMD53 (multiple block) read sequence is transferred, RWREQ in SDIO_MODE is set to 1
  //  EXPUB52 (bit 14) EXPUB52 Status Flag.   Indicates the status of the EXPUB52. While the last block in the CMD53 (multiple block) sequence is transferred, C52PUB in SDIO_MODE is set to 1
  // if (info1 & 0xC000)
  // {
  //   // Здесь реакция на событие SDMMC_EVENT_TRANSFER_COMPLETE : A multi-block CMD53 transfer is complete
  // }

  // Проверяем бит IOIRQ. SDIO Interrupt Status Flag. 1: SDIO interrupt detected
  if (info1 & BIT(0))
  {
    SDIO1_irq_disable();
    WHD_thread_notify_irq();                  // Передаем семафор wwd_transceive_semaphore в функцию wwd_thread_func
  }


  sdc.cnt_sdio_isr++;
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Это прерывание не используется при работе с SDIO

-----------------------------------------------------------------------------------------------------*/
void SDHI1_card_isr(void)
{
  uint32_t info1 = {0U};
  info1  = R_SDHI1->SD_INFO1;
  info1 &= SDMMC_SDHI_INFO1_CARD_MASK;

  if (0U != (info1 & SDMMC_SDHI_INFO1_CARD_REMOVED_MASK))
  {
    g_sdhi1_ctrl.status.ready = false;
    // Здесь реакция на  SDMMC_EVENT_CARD_REMOVED;
  }
  if (0U != (info1 & SDMMC_SDHI_INFO1_CARD_INSERTED_MASK))
  {
    // Здесь реакция на  SDMMC_EVENT_CARD_INSERTED;
  }

  /* Clear interrupt flags */
  R_SDHI1->SD_INFO1 =(~info1);

  /* Clear the IR flag in the ICU */
  /* Clearing the IR bit must be done after clearing the interrupt source in the the peripheral */
  R_ICU->IELSRn_b[(uint32_t)SDHI1_CARD_IRQn].IR = 0U;
  sdc.cnt_card_isr++;
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void  SDIO1_irq_disable(void)
{
  R_SDHI1->SDIO_MODE = 0;                                            // Запрещаем прерывания от SDIO
  R_SDHI1->SDIO_INFO1_MASK = 0x00000006u + BIT(15)+ BIT(14)+ BIT(0); // Маскируем прерываня EXPUB52, EXWT (прерывания при использованим CMD52 в течении работы CMD53 )
                                                                     // и прерывания от SDIO чтобы их флаги не появлялись в регитстре SDIO_INFO1
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void  SDIO1_irq_enable(void)
{
  R_SDHI1->SD_INFO1  = 6;                                              // Сбрасываем флаги прерываний IOIRQ, EXPUB52, EXWT, если они были
  R_SDHI1->SDIO_INFO1_MASK = 0x00000006u + BIT(15)+ BIT(14);          // Маскируем прерываня EXPUB52, EXWT (прерывания при использованим CMD52 в течении работы CMD53 )
                                                                      // и не маскируем прерывания от SDIO
  R_SDHI1->SDIO_MODE = BIT(0);                                         // Разрешаем прерывания от SDIO
}

/*-----------------------------------------------------------------------------------------------------


  \param timeout_ms

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SDIO1_wait_transfer_complete(uint32_t timeout_ms)
{
  ULONG actual_flags;
  if (tx_event_flags_get(&sdio1_flags, SDIO1_TRANSFER_COMPLETE | SDIO1_TRANSFER_ERROR , TX_OR_CLEAR,&actual_flags, ms_to_ticks(timeout_ms)) == TX_SUCCESS)
  {
    if (actual_flags == SDIO1_TRANSFER_COMPLETE) return RES_OK;
  }
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Init_SDIO1(void)
{
  ssp_err_t    ssp_res;
  uint32_t     block_size;
  uint32_t     err_line;


  if (tx_event_flags_create(&sdio1_flags, "SDIO1") != TX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  // Получение вектора прерывания и номера ивента для прерывний DMA3
  Find_IRQ_number_by_attrs(SSP_IP_DMAC, 3, 0, SSP_SIGNAL_DMAC_INT,&DMA3_IRQn,&DMA3_event);
  // Получение вектора прерывания и номера ивента для прерывний SDHI1_ACCS
  Find_IRQ_number_by_attrs(SSP_IP_SDHIMMC, 1, 0, SSP_SIGNAL_SDHIMMC_ACCS,&SDHI1_ACCS_IRQn,&SDHI1_ACCS_event);
  // Получение вектора прерывания и номера ивента для прерывний SDHI1_SDIO
  Find_IRQ_number_by_attrs(SSP_IP_SDHIMMC, 1, 0, SSP_SIGNAL_SDHIMMC_SDIO,&SDHI1_SDIO_IRQn,&SDHI1_SDIO_event);
  // Получение вектора прерывания и номера ивента для прерывний SDHI1_CARD
  Find_IRQ_number_by_attrs(SSP_IP_SDHIMMC, 1, 0, SSP_SIGNAL_SDHIMMC_CARD,&SDHI1_CARD_IRQn,&SDHI1_CARD_event);
  // Получение вектора прерывания и номера ивента для прерывний SDHI1_DMA_REQ
  Find_IRQ_number_by_attrs(SSP_IP_SDHIMMC, 1, 0, SSP_SIGNAL_SDHIMMC_DMA_REQ,&SDHI1_DMA_REQ_IRQn,&SDHI1_DMA_REQ_event);

  // Выводим модуль SDHI/MMC1 из STOP режима если он там находился
  R_MSTP->MSTPCRC_b.MSTPC11 = 0;

  ssp_res = g_sdio1.p_api->open(&g_sdhi1_ctrl,&g_sdhi1_cfg);
  if (ssp_res != SSP_SUCCESS)
  {
    tx_event_flags_delete(&sdio1_flags);
    err_line = __LINE__;
    goto _err;
  }

  // Размер блока равен 64 байта для SDIO WiFi модуля
  block_size = WIFI_SDIO_BLOCK_SIZE;
  ssp_res = g_sdio1.p_api->control(&g_sdhi1_ctrl,SSP_COMMAND_SET_BLOCK_SIZE,&block_size);
  if (ssp_res != SSP_SUCCESS)
  {
    g_sdio1.p_api->close(&g_sdhi1_ctrl);
    tx_event_flags_delete(&sdio1_flags);
    err_line = __LINE__;
    goto _err;
  }

  SDIO1_irq_disable();

  APPLOG("SDIO1 initialized");
  return RES_OK;
_err:
  APPLOG("SDIO1 initialization error in line %d. Result=%08X", err_line, ssp_res);
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t DeInit_SDIO1(void)
{
  SDIO1_irq_disable();
  g_sdio1.p_api->close(&g_sdhi1_ctrl);
  tx_event_flags_delete(&sdio1_flags);
  return RES_OK;
}

