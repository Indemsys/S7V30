#ifndef S7V30_SX1262_H
  #define S7V30_SX1262_H

  #define SX_RF_FREQUENCY                            868000000 // Hz
  #define LORA_PREAMBLE_LENGTH                       108       // Same for Tx and Rx
  #define LORA_SYMBOL_TIMEOUT                        100       // Сколько символов должен принять приемник чтобы не выдать сигнал RxTimeout IRQ
  #define LORA_PAYLOAD_LEN                           4
  #define LORA_CAD_DET_PEAK                          22
  #define LORA_CAD_DET_MIN                           10
  #define LORA_CAD_TIMEOUT_MS                        ((uint32_t)(2000.0f/0.015625f))    // 2 сек
  #define RADIO_TCXO_SETUP_TIME                      ((uint32_t)(5.0f/0.015625f))       // The time needed for the 32 MHz to appear and stabilize can be controlled through the parameter delay(23:0).  Delay duration = Delay(23:0) *15.625 µs


  #define LORA_RX_TIMEOUT_MS                         ((uint32_t)(30000.0f/0.015625f))    // 30 сек (не более 262 сек)
  #define LORA_TX_TIMEOUT_MS                         ((uint32_t)(30000.0f/0.015625f))    // 30 сек (не более 262 сек). If 0  Timeout disable, Tx Single mode, the device will stay in TX Mode until the packet is transmitted and returns in STBY_RC mode upon completion



  #define LORA_MAC_PRIVATE_SYNCWORD                 0x1424  // Syncword for Private LoRa networks
  #define LORA_MAC_PUBLIC_SYNCWORD                  0x3444  // Syncword for Public LoRa networks


  #define SX_XTAL_FREQ                             ( double )32000000
  #define SX_FREQ_DIV                              ( double )(33554432.0) // 2^25
  #define SX_FREQ_STEP                             ( double )( SX_XTAL_FREQ / SX_FREQ_DIV )


// Списо кодов команд чипа SX1262
  #define   sx_SetSleep                             0x84    // Set Chip in SLEEP mode
  #define   sx_SetStandby                           0x80    // Set Chip in STDBY_RC or STDBY_XOSC mode
  #define   sx_SetFs                                0xC1    // Set Chip in Frequency Synthesis mode
  #define   sx_SetTx                                0x83    // Set Chip in Tx mode
  #define   sx_SetRx                                0x82    // Set Chip in Rx mode
  #define   sx_StopTimerOnPreamble                  0x9F    // Stop Rx timeout on Sync Word/Header or preamble detection
  #define   sx_SetRxDutyCycle                       0x94    // Store values of RTC setup for listen mode and if period parameter is not 0, set chip into RX mode
  #define   sx_SetCad                               0xC5    // Set chip into RX mode with passed CAD parameters
  #define   sx_SetTxContinuousWave                  0xD1    // Set chip into TX mode with infinite carrier wave settings
  #define   sx_SetTxInfinitePreamble                0xD2    // Set chip into TX mode with infinite preamble settings
  #define   sx_SetRegulatorMode                     0x96    // Select LDO or DC_DC+LDO for CFG_XOSC, FS, RX or TX mode
  #define   sx_Calibrate                            0x89    // Calibrate the RC13, RC64, ADC, PLL, Image according to parameter
  #define   sx_CalibrateImage                       0x98    // Launches an image calibration at the given frequencies
  #define   sx_SetPaConfig                          0x95    // Configure the Duty Cycle, Max output power, device for the PA for SX1261 or SX1262
  #define   sx_SetRxTxFallbackMode                  0x93    // Defines into which mode the chip goes after a TX / RX done
  #define   sx_WriteRegister                        0x0D    // Write into one or several registers
  #define   sx_ReadRegister                         0x1D    // Read one or several registers
  #define   sx_WriteBuffer                          0x0E    // Write data into the FIFO
  #define   sx_ReadBuffer                           0x1E    // Read data from the FIFO
  #define   sx_SetDioIrqParams                      0x08    // Configure the IRQ and the DIOs attached to each IRQ
  #define   sx_GetIrqStatus                         0x12    // Get the values of the triggered IRQs
  #define   sx_ClearIrqStatus                       0x02    // Clear one or several of the IRQs
  #define   sx_SetDIO2AsRfSwitchCtrl                0x9D    // Configure radio to control an RF switch from DIO2
  #define   sx_SetDIO3AsTcxoCtrl                    0x97    // Configure the radio to use a TCXO controlled by DIO3
  #define   sx_SetRfFrequency                       0x86    // Set the RF frequency of the radio
  #define   sx_SetPacketType                        0x8A    // Select the packet type corresponding to the modem
  #define   sx_GetPacketType                        0x11    // Get the current packet configuration for the device
  #define   sx_SetTxParams                          0x8E    // Set output power and ramp time for the PA
  #define   sx_SetModulationParams                  0x8B    // Compute and set values in selected protocol modem for given modulation parameters
  #define   sx_SetPacketParams                      0x8C    // Set values on selected protocol modem for given packet parameters
  #define   sx_SetCadParams                         0x88    // Set the parameters which are used for performing a CAD (LoRa® only)
  #define   sx_SetBufferBaseAddress                 0x8F    // Store TX and RX base address in register of selected protocol modem
  #define   sx_SetLoRaSymbNumTimeout                0xA0    // Set the number of symbol the modem has to wait to validate a lock
  #define   sx_GetStatus                            0xC0    // Returns the current status of the device
  #define   sx_GetRssiInst                          0x15    // Returns the instantaneous measured RSSI while in Rx mode
  #define   sx_GetRxBufferStatus                    0x13    // Returns PaylaodLengthRx(7:0), RxBufferPointer(7:0)
  #define   sx_GetPacketStatus                      0x14    // Returns RssiAvg, RssiSync, PStatus2, PStaus3, PStatus4 in FSK protocol, returns RssiPkt, SnrPkt in LoRa® protocol
  #define   sx_GetDeviceErrors                      0x17    // Returns the error which has occurred in the device
  #define   sx_ClearDeviceErrors                    0x07    // Clear all the error(s). The error(s) cannot be cleared independently
  #define   sx_GetStats                             0x10    // Returns statistics on the last few received packets
  #define   sx_ResetStats                           0x00    // Resets the value read by the command GetStats

// Адреса регистров чипа SX1262
  #define   SX_1262_WHITENING_INITIAL_VALUE_MSB     0x06B8
  #define   SX_1262_WHITENING_INITIAL_VALUE_LSB     0x06B9
  #define   SX_1262_CRC_MSB_INITIAL_VALUE_0         0x06BC
  #define   SX_1262_CRC_LSB_INITIAL_VALUE_1         0x06BD
  #define   SX_1262_CRC_MSB_POLYNOMIAL_VALUE_0      0x06BE
  #define   SX_1262_CRC_LSB_POLYNOMIAL_VALUE_1      0x06BF
  #define   SX_1262_SYNCWORD_0                      0x06C0
  #define   SX_1262_SYNCWORD_1                      0x06C1
  #define   SX_1262_SYNCWORD_2                      0x06C2
  #define   SX_1262_SYNCWORD_3                      0x06C3
  #define   SX_1262_SYNCWORD_4                      0x06C4
  #define   SX_1262_SYNCWORD_5                      0x06C5
  #define   SX_1262_SYNCWORD_6                      0x06C6
  #define   SX_1262_SYNCWORD_7                      0x06C7
  #define   SX_1262_NODE_ADDRESS                    0x06CD
  #define   SX_1262_BROADCAST_ADDRESS               0x06CE
  #define   SX_1262_LORA_SYNC_WORD_MSB              0x0740
  #define   SX_1262_LORA_SYNC_WORD_LSB              0x0741
  #define   SX_1262_RANDOMNUMBERGEN_0               0x0819
  #define   SX_1262_RANDOMNUMBERGEN_1               0x081A
  #define   SX_1262_RANDOMNUMBERGEN_2               0x081B
  #define   SX_1262_RANDOMNUMBERGEN_3               0x081C
  #define   SX_1262_RX_GAIN                         0x08AC
  #define   SX_1262_OCP_CONFIGURATION               0x08E7
  #define   SX_1262_XTA_TRIM                        0x0911
  #define   SX_1262_XTB_TRIM                        0x0912



  #define   SX_TCXO_CTRL_1_6V                       0x00
  #define   SX_TCXO_CTRL_1_7V                       0x01
  #define   SX_TCXO_CTRL_1_8V                       0x02
  #define   SX_TCXO_CTRL_2_2V                       0x03
  #define   SX_TCXO_CTRL_2_4V                       0x04
  #define   SX_TCXO_CTRL_2_7V                       0x05
  #define   SX_TCXO_CTRL_3_0V                       0x06
  #define   SX_TCXO_CTRL_3_3V                       0x07



// Command status
  #define  SX_CMD_STAT_RESERVED                      0x0
  #define  SX_CMD_STAT_DATA_IS_AVAILABLE_TO_HOST1    0x2
  #define  SX_CMD_STAT_COMMAND_TIMEOUT2              0x3
  #define  SX_CMD_STAT_COMMAND_PROCESSING_ERROR3     0x4
  #define  SX_CMD_STAT_FAILURE_TO_EXECUTE_COMMAND4   0x5
  #define  SX_CMD_STAT_COMMAND_TX_DONE5              0x6

// Chip mode
  #define  SX_CHPM_UNUSED                            0x0
  #define  SX_CHPM_STBY_RC                           0x2
  #define  SX_CHPM_STBY_XOSC                         0x3
  #define  SX_CHPM_FS                                0x4
  #define  SX_CHPM_RX                                0x5
  #define  SX_CHPM_TX                                0x6


// Standby modes
  #define  SX_STDBY_RC   0
  #define  SX_STDBY_XOSC 1

// Regulator mode
  #define  SX_REG_LDO_ONLY  0  // Only LDO used for all modes
  #define  SX_REG_DC_DC_LDO 1  // DC_DC+LDO used for STBY_XOSC,FS, RX and TX modes

// PA Ramp Time
  #define  SX_RADIO_RAMP_10_US                      0x00
  #define  SX_RADIO_RAMP_20_US                      0x01
  #define  SX_RADIO_RAMP_40_US                      0x02
  #define  SX_RADIO_RAMP_80_US                      0x03
  #define  SX_RADIO_RAMP_200_US                     0x04
  #define  SX_RADIO_RAMP_800_US                     0x05
  #define  SX_RADIO_RAMP_1700_US                    0x06
  #define  SX_RADIO_RAMP_3400_US                    0x07

// IRQ Flags
  #define  SX_IRQ_RADIO_NONE                        0x0000
  #define  SX_IRQ_TX_DONE                           0x0001
  #define  SX_IRQ_RX_DONE                           0x0002
  #define  SX_IRQ_PREAMBLE_DETECTED                 0x0004
  #define  SX_IRQ_SYNCWORD_VALID                    0x0008
  #define  SX_IRQ_HEADER_VALID                      0x0010
  #define  SX_IRQ_HEADER_ERROR                      0x0020
  #define  SX_IRQ_CRC_ERROR                         0x0040
  #define  SX_IRQ_CAD_DONE                          0x0080
  #define  SX_IRQ_CAD_ACTIVITY_DETECTED             0x0100
  #define  SX_IRQ_RX_TX_TIMEOUT                     0x0200
  #define  SX_IRQ_RADIO_ALL                         0xFFFF

  #define  SX_RX_INTS_SET  ( SX_IRQ_RX_DONE | SX_IRQ_PREAMBLE_DETECTED | SX_IRQ_SYNCWORD_VALID | SX_IRQ_HEADER_VALID | SX_IRQ_HEADER_ERROR | SX_IRQ_CRC_ERROR | SX_IRQ_RX_TX_TIMEOUT)
  #define  SX_TX_INTS_SET  ( SX_IRQ_TX_DONE | SX_IRQ_RX_TX_TIMEOUT)

// Represents the possible spreading factor values in LoRa packet types
  #define  SX_LORA_SF5                              0x05
  #define  SX_LORA_SF6                              0x06
  #define  SX_LORA_SF7                              0x07
  #define  SX_LORA_SF8                              0x08
  #define  SX_LORA_SF9                              0x09
  #define  SX_LORA_SF10                             0x0A
  #define  SX_LORA_SF11                             0x0B
  #define  SX_LORA_SF12                             0x0C

// Represents the bandwidth values for LoRa packet type
  #define  SX_LORA_BW_500                           6
  #define  SX_LORA_BW_250                           5
  #define  SX_LORA_BW_125                           4
  #define  SX_LORA_BW_062                           3
  #define  SX_LORA_BW_041                           10
  #define  SX_LORA_BW_031                           2
  #define  SX_LORA_BW_020                           9
  #define  SX_LORA_BW_015                           1
  #define  SX_LORA_BW_010                           8
  #define  SX_LORA_BW_007                           0

// Represents the coding rate values for LoRa packet type
  #define  SX_LORA_CR_4_5                           0x01
  #define  SX_LORA_CR_4_6                           0x02
  #define  SX_LORA_CR_4_7                           0x03
  #define  SX_LORA_CR_4_8                           0x04



  #define SX_PACKET_TYPE_GFSK                       0x00
  #define SX_PACKET_TYPE_LORA                       0x01



  #define LORA_EXPLICIT_HEADER                      0
  #define LORA_IMPLICIT_HEADER                      1

  #define LORA_CRC_OFF                              0
  #define LORA_CRC_ON                               1

  #define LORA_INVERT_IQ                            1
  #define LORA_NOT_INVERT_IQ                        0


// Represents the Channel Activity Detection actions after the CAD operation is finished
  #define LORA_CAD_ONLY                             0x00  // Выход из CAD сразу как только детектированли активность в канале
  #define LORA_CAD_RX                               0x01  // Выход после того как детектировали активность и приняли пакет или закончился таймаут ожидания приема
  #define LORA_CAD_LBT                              0x10


// Represents the number of symbols to be used for channel activity detection operation
  #define LORA_CAD_01_SYMBOL                        0x00
  #define LORA_CAD_02_SYMBOL                        0x01
  #define LORA_CAD_04_SYMBOL                        0x02
  #define LORA_CAD_08_SYMBOL                        0x03
  #define LORA_CAD_16_SYMBOL                        0x04

// Fallback Mode Definition
  #define LORA_FALLBACK_TO_FS                       0x40   // The radio goes into FS mode after Tx or Rx
  #define LORA_FALLBACK_TO_STDBY_XOSC               0x30   // The radio goes into STDBY_XOSC mode after Tx or Rx
  #define LORA_FALLBACK_TO_STDBY_RC                 0x20   // The radio goes into STDBY_RC mode after Tx or Rx


  #define FLAG_LORA_READY                           BIT(0)
  #define FLAG_LORA_INT                             BIT(1)   // Флаг взводимый в процедуре прерывания
  #define FLAG_LORA_TERMINATE                       BIT(2)
  #define FLAG_LORA_CMD                             BIT(3)
  #define FLAG_LORA_CMD_DONE                        BIT(4)
  #define FLAG_LORA_TX_DONE                         BIT(5)
  #define FLAG_LORA_RX_DONE                         BIT(6)



extern const sf_spi_instance_t g_sf_spi_SX1262;


typedef struct
{
    uint8_t   cmd_status;
    uint8_t   chip_mode;
    uint8_t   ImageCalibrated;



    uint32_t  wr_rd_err_cnt;            // Счетчик ошибок коммуникации
    uint32_t  wr_err_cnt;               // Счетчик ошибок коммуникации
    uint32_t  rd_err_cnt;               // Счетчик ошибок коммуникации
    uint32_t  ready_wait_err_cnt;       // Счетчик ошибок ожидания готовности

    uint8_t   cad_symbols;
    uint8_t   cad_det_peak;
    uint8_t   cad_det_min;
    uint8_t   cad_exit;
    uint32_t  cad_timeout;

    uint16_t  NbPktReceived;
    uint16_t  NbPktCrcError;
    uint16_t  NbPktHeaderErr;


} T_SX1262_cbl;


typedef struct
{
    uint16_t      reg_addr;
    char  const  *reg_name;

} T_SX1262_reg_descr;


extern TX_EVENT_FLAGS_GROUP   SX1262_flags;

uint32_t            SX1262_Init(void);
uint32_t            SX1262_Init_interrupts(void);
uint32_t            SX1262_get_status(void);

uint32_t            SX1262_Set_chip_ready(void);
uint32_t            SX1262_wait_flags(uint32_t flags, ULONG *actual_flags, ULONG wait_option);
uint32_t            SX1262_Wait_ready_flag(ULONG wait_option);
void                SX1262_Clear_ready_flag(void);


uint32_t            SX1262_reg_count(void);
T_SX1262_reg_descr const* SX1262_get_reg_descr(uint8_t indx);
ssp_err_t           SX1262_open(void);
ssp_err_t           SX1262_close(void);
ssp_err_t           SX1262_write(uint8_t *buf, uint32_t sz);
ssp_err_t           SX1262_read(uint8_t *buf, uint32_t sz);
ssp_err_t           SX1262_write_read(uint8_t *buf, uint32_t sz);

uint32_t            SX1262_ReadRegister(uint16_t addr, uint8_t *val);
uint32_t            SX1262_WriteRegister(uint16_t reg_addr, uint8_t reg_val);
uint32_t            SX1262_WriteBuffer(uint8_t offs, uint8_t *data, uint8_t sz);
uint32_t            SX1262_ReadBuffer(uint8_t offs, uint8_t *data, uint8_t sz);

uint32_t            SX1262_SetCad(void);
uint32_t            SX1262_SetRx(uint32_t timeout);
uint32_t            SX1262_SetTx(uint32_t timeout);
uint32_t            SX1262_SetStandby(uint8_t mode);


uint32_t            SX1262_GetIrqStatus(uint8_t *p_status, uint16_t *p_irq_status);
uint32_t            SX1262_ClearIrqStatus(uint16_t irq_status);
T_SX1262_cbl*       SX1262_Get_cbl(void);

const char*         SX1262_Get_chip_mode_string(void);
const char*         SX1262_Get_last_command_status_string(void);
uint32_t            SX1262_GetStats(void);
uint32_t            SX1262_ResetStats(void);

uint32_t            SX1262_GetRxBufferStatus(uint8_t *PayloadLengthRx, uint8_t *RxStartBufferPointer);
uint32_t            SX1262_GetPacketStatus(uint8_t *RssiPkt, uint8_t *SnrPkt, uint8_t *SignalRssiPkt);
uint32_t            SX1262_GetRssiInst(uint8_t *RssiInst);
uint32_t            SX1262_SetRxTxFallbackMode(uint8_t mode);


uint32_t            SX1262_SetDioIrqParams(uint16_t Irq_Mask, uint16_t DIO1Mask, uint16_t DIO2Mask, uint16_t DIO3Mask);
uint32_t            SX1262_SetCadParams(uint8_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, uint8_t cadExitMode, uint32_t cadTimeout);


void                SX1262_Set_TX_done(void);
void                SX1262_Set_RX_done(void);

#endif





