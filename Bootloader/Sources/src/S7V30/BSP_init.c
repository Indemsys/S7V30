// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.05.12
// 16:43:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


#define  VECTOR_TABLE_SIZE  (96+16)
#pragma section="VECTOR_TABLE"
void*   vector_tbl[VECTOR_TABLE_SIZE]  @ "VECTOR_TABLE";

#define   TRCKCR_REG   0x4001E03F

extern uint32_t __VECT_start__;

// Структура необходимая для инициализации пинов
// Оставляем пустой поскольку начальную инициализацию не  используем, но инициализация необходима для последующего корректного выполнения драйвера SPI
const ioport_cfg_t g_bsp_pin_cfg =
{
  .number_of_pins = 0,
  .p_pin_cfg_data =0,
};

uint32_t               g_file_system_ready;
uint32_t               g_sd_card_status;
uint32_t               g_fs_free_space_val_ready;
uint64_t               g_fs_free_space;
T_sys_timestump        g_main_thread_start_timestump;

uint8_t                trckcr;


extern void Board_pins_init(void);


/*-----------------------------------------------------------------------------------------------------
  Вызывается два раза из SystemInit,
   - до того как будет проведена инициализация окружения C и периферии
   - и после инициализации периферии, но до инициализации прерывания и SDRAM

  \param event
-----------------------------------------------------------------------------------------------------*/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
  if (BSP_WARM_START_PRE_C == event)
  {
    // Инициализация до того как будет выполнена инициализация окружения С
    // Никакие переменные не инициализированы
    // Периферия, внешние выводы и тактирование не инициализированы


    // Разрешаем запись в регистры пинов
    R_PMISC->PWPR_b.BOWI  = 0;
    R_PMISC->PWPR_b.PFSWE = 1;
    // Первым действием включаем сигнал VSEL3 чтобы питание процессора было не меньше 3 V
    R_PFS->P802PFS_b.PODR = 1;
    R_PFS->P802PFS_b.PDR  = GP_OUT;

    Jump_to_app();  // Пробуем перейти в основное приложение


    App_log_disable();

    // Разрешаем запись в регистры пинов
    R_PMISC->PWPR_b.BOWI  = 0;
    R_PMISC->PWPR_b.PFSWE = 1;

    Board_pins_init();

  }
  else if (BSP_WARM_START_POST_C == event)
  {

    R_SYSTEM->PRCR = 0xA50B; // Открываем доступ на запись к регистрам

    // Здесь после того как будет выполнена инициализация окружения С
    // Переменные инициализированы и установлены в дефолтные состояния
    // Инициализировано тактирование и периферия. Прерывания и SDRAM здесь еще не инициализированы
    SCB->VTOR = (uint32_t)TX_VECTOR_TABLE; // Устанавливаем адрес области векторов для контроллера прерываний

    trckcr = *((uint8_t*)(TRCKCR_REG));
    *((uint8_t*)(TRCKCR_REG)) = 0;
    trckcr = 0x80;
    *((uint8_t*)(TRCKCR_REG)) = trckcr; // Разрешаем трассировщик с частотй 120 МГц


    // Запускаем кварц 32768 Гц если он остановлен
    if (R_SYSTEM->SOSCCR_b.SOSTP == 1)
    {
      R_SYSTEM->SOSCCR_b.SOSTP = 1; // Для надежности устанавливаем бит остановки осциллятора еще раз. После установки или сброса этого бита необходима задержка в 5 циклов осциллятора = 153 мкс
      DELAY_ms(1); // Задержка на 1 мс при тактовой частоте 240 МГц
      // Устанавливаем драйверу кварца стандартное значение емкости нагрузки (12 pF)
      R_SYSTEM->SOMCR_b.SODRV1 = 0;
      R_SYSTEM->SOSCCR_b.SOSTP = 0; // Устанавливаем бит запуска осциллятора.
      DELAY_ms(1); // Задержка на 1 мс при тактовой частоте 240 МГц
    }

    g_ioport_on_ioport.init(&g_bsp_pin_cfg); // Инициализация пинов необходима поскольку от нее зависит управление пинами в драйвере SPI

  }
  else
  {
    /* Do nothing */
  }
}

/*-----------------------------------------------------------------------------------------------------
  Назначение вектора прерывания в таблице векторов находящейся в RAM

  \param evt
  \param isr

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* Set_irq_vector( elc_event_t evt, void* isr)
{
  unsigned int     prev;
  IRQn_Type        irq_num;
  void            *prev_isr;
  irq_num = (IRQn_Type)Find_IRQ_number_by_evt(evt);

  prev = tx_interrupt_control(TX_INT_DISABLE);

  prev_isr = (void*)vector_tbl[irq_num+16];
  vector_tbl[irq_num+16] = isr;

  tx_interrupt_control(prev);
  return prev_isr;
}

/*-----------------------------------------------------------------------------------------------------
  Reset_Handler->SystemInit->bsp_init

  \param p_args
-----------------------------------------------------------------------------------------------------*/
void bsp_init(void *p_args)
{
#ifdef ENABLE_SDRAM
  S7V30_sdram_init();
#endif
}

/*-----------------------------------------------------------------------------------------------------
  Перехватчики аппаратных исключений для каждого вида отдельно для упрощения отладки

  \param void
-----------------------------------------------------------------------------------------------------*/
void HardFault_Handler  (void)
{
  BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}
void MemManage_Handler  (void)
{
  BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}
void BusFault_Handler   (void)
{
  BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}
void UsageFault_Handler (void)
{
  BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}
void SVC_Handler        (void)
{
  BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}
void DebugMon_Handler   (void)
{
  BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}

