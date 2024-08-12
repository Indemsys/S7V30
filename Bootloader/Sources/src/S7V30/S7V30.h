#ifndef S7V30_H
  #define S7V30_H

  #define     S7V30_SOFTWARE_VERSION   "S7V30"
  #define     S7V30_HARDWARE_VERSION   "S7V30"


  #define       ENABLE_SDRAM
  #define       USE_HARDWARE_CRIPTO_ENGINE

  #ifdef ENABLE_SDRAM
    #define     LOG_TO_ONBOARD_SDRAM      // Используем SDRAM для хранения лога
  #endif

  #define DEFAULT_PARAMS_TYPE   BOOTL_PARAMS

//  #define     ENABLE_MATLAB_CONNECTION

  #define     CAN_SPEED                      555555ul      // Скорость в системе по умолчанию
  #define     CPU_ID_STR_LEN                 33
  #define     CPU_ID_LEN                     16

//  #define     SDIO1_1BIT_MODE

  #define     RAM_END                        (0x2007FFFF)
  #define     OSIS_ADDRESS                    0x40120050  // OCD/Serial Programmer ID Setting Register (OSIS)


  #define     SDRAM_BEGIN                    (0x90000000)
  #define     SDRAM_END                      (0x91FFFFFF)
  #define     SDRAM_SIZE                     (SDRAM_END - SDRAM_BEGIN + 1)

  #define     ICLK_FREQ                      240000000ul
  #define     PCLKA_FREQ                     120000000ul
  #define     PCLKB_FREQ                     60000000ul
  #define     PCLKC_FREQ                     60000000ul
  #define     PCLKD_FREQ                     120000000ul
  #define     BCLK_FREQ                      120000000ul
  #define     FCLK_FREQ                      60000000ul

  #define     DEFAULT_HOST_IP_ADDRESS        "192.168.1.100"

  #define     WINDOWS_DIR                    "System Volume Information"

  #define     APP_LOG_FILE_PATH              "\\log.txt"
  #define     NET_LOG_FILE_PATH              "\\net_log.txt"

  #define     APP_LOG_PREV_FILE_PATH         "log_prev.txt"
  #define     NET_LOG_PREV_FILE_PATH         "net_log_prev.txt"

  #define     PARAMS_SCHEMA_FILE_NAME        "PS.json"
  #define     PARAMS_VALUES_FILE_NAME        "PV.json"
  #define     FILES_LIST_FILE_NAME           "FL.json"
  #define     PACKED_FILES_LIST_FILE_NAME    "FL.pck"
  #define     COMPRESSED_STREAM_FILE_NAME    "tmpc.dat"
  #define     UNCOMPESSED_STREAM_FILE_NAME   "tmpu.dat"

  #define     RES_OK     (0)
  #define     RES_ERROR  (1)

  #define     BIT(n) (1u << n)
  #define     LSHIFT(v,n) (((unsigned int)(v) << n))


// Назначение аппаратный приоритетов прерываниям
// У величины приоритета действующие 4 бита. Допускаются значения от 0 до 15
// Меньшие значения имеют больший приоритет

  #define     EXT_ADC_SCAN_PRIO          0
  #define     IO_EXP_SCAN_PRIO           0
  #define     INT_ADC_SCAN_PRIO          1
  #define     UART_BLE_PRIO              2  // Приоритет канала связи c BLE модулем
  #define     SPI1_PRIO                  7
  #define     AGT0_PRIO                  5  // Приоритет прерывания обслуживающего ручной энкодер

  #define     INT_AGT1_SCAN_PRIO         4
  #define     INT_DAC_DTC_PRIO           5

  #define     INT_CAN0_ERROR_PRIO        6
  #define     INT_CAN0_FIFO_RX_PRIO      6
  #define     INT_CAN0_FIFO_TX_PRIO      6
  #define     INT_CAN0_MAILBOX_RX_PRIO   6
  #define     INT_CAN0_MAILBOX_TX_PRIO   6

  #define     INT_GPT2_OVERFLOW_PRIO     6

  #define     INT_MODBUS_SCI_PRIO        6

  #define     INT_SSI0_TXI_PRIO          6
  #define     INT_SSI0_RXI_PRIO          6
  #define     INT_SSI0_INT_PRIO          6
  #define     IRQ11_INT_PRIO             6
  #define     IRQ8_INT_PRIO              6



  #define     BT3_STATE          R_PFS->P201PFS_b.PIDR
  #define     BT2_STATE          R_PFS->P200PFS_b.PIDR

  #define     RED_LED            R_PFS->P313PFS_b.PODR
  #define     GREEN_LED          R_PFS->P314PFS_b.PODR
  #define     BLUE_LED           R_PFS->P315PFS_b.PODR

  #define     RED_LED_STATE      R_PFS->P313PFS_b.PIDR
  #define     GREEN_LED_STATE    R_PFS->P314PFS_b.PIDR
  #define     BLUE_LED_STATE     R_PFS->P315PFS_b.PIDR

  #define     RF_RST             R_PFS->P909PFS_b.PODR     // Сброс происходит при низком уровне сигнала
  #define     RF_ANT_SW          R_PFS->P910PFS_b.PODR     // Управление антенным ключом через вход негативный CTRL (pin 6) При подаче 3V управление отдается только входу CTRL   (pin 4)
  #define     RF_DIO1_STATE      R_PFS->PA10PFS_b.PIDR
  #define     RF_BUSY_STATE      R_PFS->PA09PFS_b.PIDR     // Низкий уровень означает готовность к обмену командами

  #define     WIFI_REG_ON        R_PFS->P805PFS_b.PODR
  #define     BT_REG_ON          R_PFS->P810PFS_b.PODR
  #define     CODEC_PWR          R_PFS->P806PFS_b.PODR
  #define     LORA_PWR_EN        R_PFS->P808PFS_b.PODR
  #define     LORA_RESET         R_PFS->P909PFS_b.PODR

  #define     SD_CARD_PWR        R_PFS->P809PFS_b.PODR
  #define     VBUSCTRL           R_PFS->P615PFS_b.PODR
  #define     PWR_SEL            R_PFS->PA11PFS_b.PODR  // Управление ограничением входного тока зарядника.  0 - 2.4 А, 1 - 0.5 A


  #define     SD_CARD_OK           0
  #define     SD_CARD_ERROR1       1
  #define     SD_CARD_ERROR2       2
  #define     SD_CARD_ERROR3       3
  #define     SD_CARD_FS_ERROR1    4
  #define     SD_CARD_FS_ERROR2    5
  #define     SD_CARD_FS_ERROR3    6
  #define     SD_CARD_FS_ERROR4    7
  #define     SD_CARD_FS_ERROR5    8
  #define     SD_CARD_FS_ERROR6    9


  #define     STR_CRLF                       "\r\n"

  #include <ctype.h>
  #include <stdint.h>
  #include <stdlib.h>
  #include <string.h>
  #include <limits.h>
  #include <math.h>
  #include <time.h>
  #include <arm_itm.h>
  #include <stdarg.h>


typedef struct
{
    uint32_t cycles;
    uint32_t ticks;

} T_sys_timestump;


  #define  SYSTEM_CLOCK    240000000
  #define  DELAY_1us       Delay_m7(33)           // 0.992       мкс при частоте 240 МГц
  #define  DELAY_4us       Delay_m7(136)          // 3.996       мкс при частоте 240 МГц
  #define  DELAY_8us       Delay_m7(273)          // 7.992       мкс при частоте 240 МГц
  #define  DELAY_32us      Delay_m7(1096)         // 31.996      мкс при частоте 240 МГц
  #define  DELAY_100us     Delay_m7(3428)         // ~100        мкс при частоте 240 МГц
  #define  DELAY_ms(x)     Delay_m7(34285*x-1)    // 1000.008*N  мкс при частоте 240 МГц

extern void Delay_m7(int cnt); // Задержка на (cnt+1)*7 тактов . Передача нуля не допускается


  #include "bsp_api.h"
  #include "tx_api.h"
  #include "fx_api.h"
  #include "ux_api.h"
  #include "nx_api.h"
  #include "nx_ip.h"
  #include "nxd_mdns.h"
  #include "tx_timer.h"
  #include "hal_data.h"
  #include "r_hash_api.h"
  #include "r_aes_api.h"
  #include "r_rsa_api.h"
  #include "sf_el_fx.h"
  #include "sf_block_media_api.h"
  #include "sf_block_media_sdmmc.h"
  #include "sf_spi_api.h"
  #include "sf_i2c.h"
  #include "sf_i2c_api.h"
  #include "sf_spi.h"
  #include "ux_dcd_synergy.h"
  #include "ux_device_class_cdc_acm.h"
  #include "ux_device_class_storage.h"
  #include "ux_host_class_cdc_acm.h"
  #include "ux_host_class_cdc_ecm.h"
  #include "s7_sdmmc.h"
  #include "s7_sdmmc_utils.h"

  #include "r_timer_api.h"
  #include "r_dtc.h"
  #include "r_transfer_api.h"


  #include "sdram.h"
  #include "ELC_software_event_1.h"
  #include "SDIO1.h"
  #include "SCI8.h"
  #include "BQ25619.h"
  #include "ISM330.h"
  #include "pins.h"
  #include "SX1262.h"
  #include "MAX17262.h"
  #include "AB1815.h"
  #include "ADS7028.h"
  #include "CAN0.h"
  #include "I2C0.h"
  #include "DAC.h"
  #include "AGT.h"
  #include "UART_BLE.h"
  #include "ELC_tbl.h"
  #include "Flasher.h"
  #include "LowPower.h"
  #include "CHIP_utils.h"
  #include "Transfer_channels.h"
  #include "Memory_manager.h"
  #include "SDRAM_mem_pool.h"
  #include "Realtime_clock.h"
  #include "Watchdog_controller.h"
  #include "WiFi_module_control.h"

  #include "SEGGER_RTT.h"

  #include "jansson.h"

  #include "String_utils.h"
  #include "Time_utils.h"
  #include "CRC_utils.h"
  #include "compress.h"

  #include "FS_init.h"
  #include "FS_utils.h"
  #include "USB_descriptors.h"
  #include "USB_init.h"
  #include "USB_storage.h"

  #include "Params_Types.h"
  #include "Logger.h"
  #include "Start_report.h"
  #include "Background_Task.h"

  #include "S7V30_params.h"
  #include "NV_store.h"

  #include "JSON_serializer.h"
  #include "JSON_deserializer.h"
  #include "ParamsSchema_serializer.h"
  #include "DeviceInfo_serializer.h"
  #include "WiFi_scan_res_serializer.h"

  #include "DSP_Filters.h"

  #include "Infineon_BLOBs.h"

  #include "Net.h"
  #include "WEB_server.h"

  #include "VT100_monitor.h"

  #ifdef ENABLE_MATLAB_CONNECTION
    #include "Net_MATLAB_connection.h"
  #endif

  #include "..\..\Keys\Keys.h"
  #include "Loader_config.h"
  #include "Loader.h"
  #include "Charger_task.h"
  #include "BLE_main.h"


// Биты событий обрабатываемых основным приложением
  #define EVENT_APP_TICK                BIT(0)
  #define EVENT_CAN_TASK_READY          BIT(1)
  #define EVENT_INT_ADC_RES_READY       BIT(2)
  #define EVENT_PLAYER_TASK_READY       BIT(3)
  #define EVENT_CHARGER_TICK            BIT(4)
  #define LAST_GENERAL_EVNT_NUM         5


extern volatile ULONG           _tx_timer_system_clock;

extern  char                    g_cpu_id_str[CPU_ID_STR_LEN];
extern  uint8_t                 g_cpu_id[CPU_ID_LEN];
extern  uint8_t                 rstsr0;
extern  uint16_t                rstsr1;
extern  uint8_t                 rstsr2;
extern  uint32_t                g_file_system_ready;
extern  uint32_t                g_sd_card_status;
extern  uint32_t                g_fs_free_space_val_ready;
extern  uint64_t                g_fs_free_space;
extern  T_sys_timestump         g_main_thread_start_timestump;

extern const uint8_t            FIRMWARE_4373A0_CLM[];
extern const uint8_t            FIRMWARE_4373A0[];

void                            bsp_init(void *p_args);
void*                           Set_irq_vector(elc_event_t evt, void *isr);

void                            Set_app_event(uint32_t events_mask);
uint32_t                        Wait_app_event(uint32_t events_mask, uint32_t opt, uint32_t wait);
uint32_t                        Clear_app_event(void);
uint32_t                        Get_app_events(uint32_t events_mask, ULONG *p_flags,  uint32_t opt, uint32_t wait);
const T_NV_parameters_instance* Get_mod_params_instance(void);
const char*                     Get_build_date(void);
const char*                     Get_build_time(void);


  #include   "thread_priorities.h"
#endif



