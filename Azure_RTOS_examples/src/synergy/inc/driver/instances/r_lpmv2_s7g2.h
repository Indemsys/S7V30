/***********************************************************************************************************************
 * Copyright [2015-2017] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : r_lpmv2_s7g2.h
 * Description  : LPMV2 HAL API header file
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup LPMV2_S7G2 LPMV2 S7G2
 * @brief Driver for Low Power Modes.
 *
 * The LPMV2 driver supports low power modes deep standby, standby, sleep, and snooze.
 *
 * @note Not all low power modes are available on all MCU's.
 * @{
 **********************************************************************************************************************/

#ifndef R_LPMV2_S7G2_H
#define R_LPMV2_S7G2_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_lpmv2_cfg.h"
#include "r_lpmv2_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define LPMV2_CODE_VERSION_MAJOR (1U)
#define LPMV2_CODE_VERSION_MINOR (4U)

#ifndef LPMV2_ERROR_RETURN
    /*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
    #define LPMV2_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &(g_lpmv2_module_name[0]), &g_lpmv2_version)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Snooze request sources */
typedef enum e_lpmv2_snooze_request
{
    LPMV2_SNOOZE_REQUEST_RXD0_FALLING       = 0x00000000U,  ///< Enable RXD0 falling edge snooze request
    LPMV2_SNOOZE_REQUEST_IRQ0               = 0x00000001U,  ///< Enable IRQ0 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ1               = 0x00000002U,  ///< Enable IRQ1 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ2               = 0x00000004U,  ///< Enable IRQ2 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ3               = 0x00000008U,  ///< Enable IRQ3 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ4               = 0x00000010U,  ///< Enable IRQ4 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ5               = 0x00000020U,  ///< Enable IRQ5 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ6               = 0x00000040U,  ///< Enable IRQ6 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ7               = 0x00000080U,  ///< Enable IRQ7 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ8               = 0x00000100U,  ///< Enable IRQ8 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ9               = 0x00000200U,  ///< Enable IRQ9 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ10              = 0x00000400U,  ///< Enable IRQ10 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ11              = 0x00000800U,  ///< Enable IRQ11 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ12              = 0x00001000U,  ///< Enable IRQ12 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ13              = 0x00002000U,  ///< Enable IRQ13 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ14              = 0x00004000U,  ///< Enable IRQ14 pin snooze request
    LPMV2_SNOOZE_REQUEST_IRQ15              = 0x00008000U,  ///< Enable IRQ15 pin snooze request
    LPMV2_SNOOZE_REQUEST_KEY                = 0x00020000U,  ///< Enable KR snooze request
    LPMV2_SNOOZE_REQUEST_ACMPHS0            = 0x00400000U,  ///< Enable High-speed analog comparator 0 snooze request
    LPMV2_SNOOZE_REQUEST_RTC_ALARM          = 0x01000000U,  ///< Enable RTC alarm snooze request
    LPMV2_SNOOZE_REQUEST_RTC_PERIOD         = 0x02000000U,  ///< Enable RTC period snooze request
    LPMV2_SNOOZE_REQUEST_AGT1_UNDERFLOW     = 0x10000000U,  ///< Enable AGT1 underflow snooze request
    LPMV2_SNOOZE_REQUEST_AGT1_COMPARE_A     = 0x20000000U,  ///< Enable AGT1 compare match A snooze request
    LPMV2_SNOOZE_REQUEST_AGT1_COMPARE_B     = 0x40000000U,  ///< Enable AGT1 compare match B snooze request
} lpmv2_snooze_request_t;

/** Snooze end control */
typedef enum e_lpmv2_snooze_end
{
    LPMV2_SNOOZE_END_STANDBY_WAKE_SOURCES         = 0x00U,  ///< Transition from Snooze to Normal mode directly
    LPMV2_SNOOZE_END_AGT1_UNDERFLOW               = 0x01U,  ///< AGT1 underflow
    LPMV2_SNOOZE_END_DTC_TRANS_COMPLETE           = 0x02U,  ///< Last DTC transmission completion
    LPMV2_SNOOZE_END_DTC_TRANS_COMPLETE_NEGATED   = 0x04U,  ///< Not Last DTC transmission completion
    LPMV2_SNOOZE_END_ADC0_COMPARE_MATCH           = 0x08U,  ///< ADC Channel 0 compare match
    LPMV2_SNOOZE_END_ADC0_COMPARE_MISMATCH        = 0x10U,  ///< ADC Channel 0 compare mismatch
    LPMV2_SNOOZE_END_ADC1_COMPARE_MATCH           = 0x20U,  ///< ADC 1 compare match
    LPMV2_SNOOZE_END_ADC1_COMPARE_MISMATCH        = 0x40U,  ///< ADC 1 compare mismatch
    LPMV2_SNOOZE_END_SCI0_ADDRESS_MATCH           = 0x80U,  ///< SCI0 address mismatch
} lpmv2_snooze_end_t;

typedef uint8_t lpmv2_snooze_end_bits_t;

/** Snooze cancel control */
typedef enum e_lpmv2_snooze_cancel
{
    LPMV2_SNOOZE_CANCEL_SOURCE_ADC0_WCMPM               = (79),  ///< ADC Channel 0 window compare match
    LPMV2_SNOOZE_CANCEL_SOURCE_ADC0_WCMPUM              = (80),  ///< ADC Channel 0 window compare mismatch
    LPMV2_SNOOZE_CANCEL_SOURCE_ADC1_WCMPM               = (85),  ///< ADC Channel 1 window compare match
    LPMV2_SNOOZE_CANCEL_SOURCE_ADC1_WCMPUM              = (86),  ///< ADC Channel 1 window compare mismatch
    LPMV2_SNOOZE_CANCEL_SOURCE_SCI0_AM                  = (376), ///< SCI0 address match event
    LPMV2_SNOOZE_CANCEL_SOURCE_SCI0_RXI_OR_ERI          = (377), ///< SCI0 receive error
    LPMV2_SNOOZE_CANCEL_SOURCE_DTC_COMPLETE             = (41),  ///< DTC transfer completion
    LPMV2_SNOOZE_CANCEL_SOURCE_DOC_DOPCI                = (134), ///< Data operation circuit interrupt
    LPMV2_SNOOZE_CANCEL_SOURCE_CTSU_CTSUFN              = (18),  ///< CTSU measurement end interrupt
} lpmv2_snooze_cancel_t;

/** DTC Enable in Snooze Mode */
typedef enum e_lpmv2_snooze_dtc
{
    /**
     * Disable DTC operation
     */
    LPMV2_SNOOZE_DTC_DISABLE    = 0U,
    /**
     * Enable DTC operation
     */
    LPMV2_SNOOZE_DTC_ENABLE     = 1U,
} lpmv2_snooze_dtc_t;

/** Wake from standby mode sources, does not apply to sleep or deep standby modes */
typedef enum e_lpmv2_standby_wake_source
{
    LPMV2_STANDBY_WAKE_SOURCE_IRQ0      = 0x00000001U,        ///< IRQ0
    LPMV2_STANDBY_WAKE_SOURCE_IRQ1      = 0x00000002U,        ///< IRQ1
    LPMV2_STANDBY_WAKE_SOURCE_IRQ2      = 0x00000004U,        ///< IRQ2
    LPMV2_STANDBY_WAKE_SOURCE_IRQ3      = 0x00000008U,        ///< IRQ3
    LPMV2_STANDBY_WAKE_SOURCE_IRQ4      = 0x00000010U,        ///< IRQ4
    LPMV2_STANDBY_WAKE_SOURCE_IRQ5      = 0x00000020U,        ///< IRQ5
    LPMV2_STANDBY_WAKE_SOURCE_IRQ6      = 0x00000040U,        ///< IRQ6
    LPMV2_STANDBY_WAKE_SOURCE_IRQ7      = 0x00000080U,        ///< IRQ7
    LPMV2_STANDBY_WAKE_SOURCE_IRQ8      = 0x00000100U,        ///< IRQ8
    LPMV2_STANDBY_WAKE_SOURCE_IRQ9      = 0x00000200U,        ///< IRQ9
    LPMV2_STANDBY_WAKE_SOURCE_IRQ10     = 0x00000400U,        ///< IRQ10
    LPMV2_STANDBY_WAKE_SOURCE_IRQ11     = 0x00000800U,        ///< IRQ11
    LPMV2_STANDBY_WAKE_SOURCE_IRQ12     = 0x00001000U,        ///< IRQ12
    LPMV2_STANDBY_WAKE_SOURCE_IRQ13     = 0x00002000U,        ///< IRQ13
    LPMV2_STANDBY_WAKE_SOURCE_IRQ14     = 0x00004000U,        ///< IRQ14
    LPMV2_STANDBY_WAKE_SOURCE_IRQ15     = 0x00008000U,        ///< IRQ15
    LPMV2_STANDBY_WAKE_SOURCE_IWDT      = 0x00010000U,        ///< Independent watchdog interrupt
    LPMV2_STANDBY_WAKE_SOURCE_KEY       = 0x00020000U,        ///< Key interrupt
    LPMV2_STANDBY_WAKE_SOURCE_LVD1      = 0x00040000U,        ///< Low Voltage Detection 1 interrupt
    LPMV2_STANDBY_WAKE_SOURCE_LVD2      = 0x00080000U,        ///< Low Voltage Detection 2 interrupt
    LPMV2_STANDBY_WAKE_SOURCE_ACMPHS0   = 0x00400000U,        ///< Analog Comparator High-speed 0 interrupt
    LPMV2_STANDBY_WAKE_SOURCE_RTCALM    = 0x01000000U,        ///< RTC Alarm interrupt
    LPMV2_STANDBY_WAKE_SOURCE_RTCPRD    = 0x02000000U,        ///< RTC Period interrupt
    LPMV2_STANDBY_WAKE_SOURCE_USBHS     = 0x04000000U,        ///< USB High-speed interrupt
    LPMV2_STANDBY_WAKE_SOURCE_USBFS     = 0x08000000U,        ///< USB Full-speed interrupt
    LPMV2_STANDBY_WAKE_SOURCE_AGT1UD    = 0x10000000U,        ///< AGT1 underflow interrupt
    LPMV2_STANDBY_WAKE_SOURCE_AGT1CA    = 0x20000000U,        ///< AGT1 compare match A interrupt
    LPMV2_STANDBY_WAKE_SOURCE_AGT1CB    = 0x40000000U,        ///< AGT1 compare match B interrupt
    LPMV2_STANDBY_WAKE_SOURCE_IIC0      = 0x80000000U,        ///< I2C 0 interrupt
} lpmv2_standby_wake_source_t;

typedef uint32_t lpmv2_standby_wake_source_bits_t;

/** I/O port state after Deep Software Standby mode */
typedef enum e_lpmv2_io_port
{
    /**
     * When the Deep Software Standby mode is canceled, the I/O ports are in the reset state
     */
    LPMV2_IO_PORT_RESET                         = 0U,
    /**
     * When the Deep Software Standby mode is canceled, the I/O ports are in the same state as
     * in the Deep Software Standby mode
     */
    LPMV2_IO_PORT_NO_CHANGE                     = 1U,
} lpmv2_io_port_t;

/** Power supply control */
typedef enum e_lpmv2_power_supply
{
    /**
     * Power to the standby RAM, Low-speed on-chip oscillator, AGTn, and USBFS/HS resume
     * detecting unit is supplied in deep software standby mode
     */
    LPMV2_POWER_SUPPLY_DEEPCUT0 = 0U,
    /**
     * Power to the standby RAM, Low-speed on-chip oscillator, AGTn, and USBFS/HS resume
     * detecting unit is not supplied in deep software standby mode
     */
    LPMV2_POWER_SUPPLY_DEEPCUT1 = 1U,

    /**
     * Power to the standby RAM, Low-speed on-chip oscillator, AGTn, and USBFS/HS resume
     * detecting unit is not supplied in deep software standby mode. In addition, LVD is
     * disabled and the low power function in a poweron reset circuit is enabled
     */
    LPMV2_POWER_SUPPLY_DEEPCUT3 = 3UL,
} lpmv2_power_supply_t;

/** Deep Standby Interrupt Edge */
typedef enum e_lpmv2_deep_standby_cancel_edge
{
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_EDGE_NONE      =  0U,            ///< No options for a deep standby cancel source
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ0_RISING    =  0x00000001U,   ///< IRQ0-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ0_FALLING   =           0U,   ///< IRQ0-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ1_RISING    =  0x00000002U,   ///< IRQ1-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ1_FALLING   =           0U,   ///< IRQ1-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ2_RISING    =  0x00000004U,   ///< IRQ2-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ2_FALLING   =           0U,   ///< IRQ2-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ3_RISING    =  0x00000008U,   ///< IRQ3-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ3_FALLING   =           0U,   ///< IRQ3-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ4_RISING    =  0x00000010U,   ///< IRQ4-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ4_FALLING   =           0U,   ///< IRQ4-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ5_RISING    =  0x00000020U,   ///< IRQ5-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ5_FALLING   =           0U,   ///< IRQ5-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ6_RISING    =  0x00000040U,   ///< IRQ6-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ6_FALLING   =           0U,   ///< IRQ6-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ7_RISING    =  0x00000080U,   ///< IRQ7-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ7_FALLING   =           0U,   ///< IRQ7-DS Pin Falling Edge

    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ8_RISING    =  0x00000100U,   ///< IRQ8-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ8_FALLING   =           0U,   ///< IRQ8-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ9_RISING    =  0x00000200U,   ///< IRQ9-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ9_FALLING   =           0U,   ///< IRQ9-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ10_RISING   =  0x00000400U,   ///< IRQ10-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ10_FALLING  =           0U,   ///< IRQ10-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ11_RISING   =  0x00000800U,   ///< IRQ11-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ11_FALLING  =           0U,   ///< IRQ11-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ12_RISING   =  0x00001000U,   ///< IRQ12-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ12_FALLING  =           0U,   ///< IRQ12-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ13_RISING   =  0x00002000U,   ///< IRQ13-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ13_FALLING  =           0U,   ///< IRQ13-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ14_RISING   =  0x00004000U,   ///< IRQ14-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ14_FALLING  =           0U,   ///< IRQ14-DS Pin Falling Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ15_RISING   =  0x00008000U,   ///< IRQ15-DS Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ15_FALLING  =           0U,   ///< IRQ15-DS Pin Falling Edge

    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_LVD1_RISING    =  0x00010000U,   ///< LVD1 Rising Slope
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_LVD1_FALLING   =           0U,   ///< LVD1 Falling Slope
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_LVD2_RISING    =  0x00020000U,   ///< LVD2 Rising Slope
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_LVD2_FALLING   =           0U,   ///< LVD2 Falling Slope
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_NMI_RISING     =  0x00100000U,   ///< NMI Pin Rising Edge
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_NMI_FALLING    =           0U,   ///< NMI Pin Falling Edge
} lpmv2_deep_standby_cancel_edge_t;

typedef uint32_t lpmv2_deep_standby_cancel_edge_bits_t;

/** Deep Standby cancel sources */
typedef enum e_lpmv2_deep_standby_cancel_source
{
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_RESET_ONLY     =           0U,   ///< Cancel deep standby only by reset

    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ0           =  0x00000001U,   ///< IRQ0
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ1           =  0x00000002U,   ///< IRQ1
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ2           =  0x00000004U,   ///< IRQ2
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ3           =  0x00000008U,   ///< IRQ3
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ4           =  0x00000010U,   ///< IRQ4
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ5           =  0x00000020U,   ///< IRQ5
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ6           =  0x00000040U,   ///< IRQ6
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ7           =  0x00000080U,   ///< IRQ7

    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ8           =  0x00000100U,   ///< IRQ8
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ9           =  0x00000200U,   ///< IRQ9
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ10          =  0x00000400U,   ///< IRQ10
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ11          =  0x00000800U,   ///< IRQ11
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ12          =  0x00001000U,   ///< IRQ12
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ13          =  0x00002000U,   ///< IRQ13
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ14          =  0x00004000U,   ///< IRQ14
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_IRQ15          =  0x00008000U,   ///< IRQ15

    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_LVD1           =  0x00010000U,   ///< LVD1
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_LVD2           =  0x00020000U,   ///< LVD2
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_RTC_INTERVAL   =  0x00040000U,   ///< RTC Interval Interrupt
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_RTC_ALARM      =  0x00080000U,   ///< RTC Alarm Interrupt
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_NMI            =  0x00100000U,   ///< NMI

    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_USBFS          =  0x01000000U,   ///< USBFS Suspend/Resume
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_USBHS          =  0x02000000U,   ///< USBHS Suspend/Resume
    LPMV2_DEEP_STANDBY_CANCEL_SOURCE_AGT1           =  0x04000000U,   ///< AGT1 Underflow
} lpmv2_deep_standby_cancel_source_t;

typedef uint32_t lpmv2_deep_standby_cancel_source_bits_t;

/** Output port enable */
typedef enum e_lpmv2_output_port_enable
{
    /**
     * 0: In Software Standby Mode or Deep Software Standby Mode, the
     * address output pins, data output pins, and other bus control signal
     * output pins are set to the high-impedance state. In Snooze, the
     * status of the address bus and bus control signals are same as
     * before entering Software Standby Mode.
     */
    LPMV2_OUTPUT_PORT_ENABLE_HIGH_IMPEDANCE     = 0U,
    /**
     * 1: In Software Standby Mode, the address output pins, data output
     * pins, and other bus control signal output pins retain the
     * output state.
     */
    LPMV2_OUTPUT_PORT_ENABLE_RETAIN             = 1U,
} lpmv2_output_port_enable_t;

/** S7G2 specific configuration structure */
typedef struct s_lpmv2_mcu_cfg
{
    /** Bitwise list of sources to wake from standby */
    lpmv2_standby_wake_source_bits_t        standby_wake_sources;
    /** Snooze request source */
    lpmv2_snooze_request_t                  snooze_request_source;
    /** Bitwise list of snooze end sources */
    lpmv2_snooze_end_bits_t                 snooze_end_sources;
    /** list of snooze cancel sources */
    lpmv2_snooze_cancel_t                   snooze_cancel_sources;
    /** State of DTC in snooze mode, enabled or disabled */
    lpmv2_snooze_dtc_t                      dtc_state_in_snooze;
    /** Output port enabled/disabled in standby and deep standby */
    lpmv2_output_port_enable_t              output_port_enable;
    /** IO port state in deep standby (maintained or reset) */
    lpmv2_io_port_t                         io_port_state;
    /** Internal power supply state in standby and deep standby (deepcut) */
    lpmv2_power_supply_t                    power_supply_state;
    /** Sources that can trigger exit from deep standby */
    lpmv2_deep_standby_cancel_source_bits_t deep_standby_cancel_source;
    /** Signal edges for the sources that can trigger exit from deep standby */
    lpmv2_deep_standby_cancel_edge_bits_t   deep_standby_cancel_edge;
}lpmv2_mcu_cfg_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const lpmv2_api_t g_lpmv2_on_lpmv2;
/** @endcond */

#endif /* R_LPMV2_S7G2_H */

/*******************************************************************************************************************//**
 * @} (end defgroup LPMV2_S7G2)
 **********************************************************************************************************************/
