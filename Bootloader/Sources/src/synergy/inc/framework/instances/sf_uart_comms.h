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
 * File Name    : sf_uart_comms.h
 * Description  : UART AMS framework driver header file
 **********************************************************************************************************************/

#ifndef SF_UART_COMMS_H
#define SF_UART_COMMS_H

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_UART_COMMS UART Framework Instance
 * @brief RTOS-integrated Communications Framework UART implementation.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_uart_api.h"
#include "sf_comms_api.h"
#include "sf_uart_comms_cfg.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Version of code that implements the API defined in this file */
#define SF_UART_COMMS_CODE_VERSION_MAJOR (1U)
#define SF_UART_COMMS_CODE_VERSION_MINOR (12U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Framework UART state */
typedef enum e_sf_uart_comms_state
{
    SF_UART_COMMS_STATE_CLOSED = 0,       ///< UART port is closed.
    SF_UART_COMMS_STATE_OPENED,           ///< UART port is opened.
    SF_UART_COMMS_STATE_READING,          ///< UART port is on data reception.
    SF_UART_COMMS_STATE_WRITING,          ///< UART port is on data transmission.
} sf_uart_comms_state_t;

/** UART communications instance control structure. DO NOT INITIALIZE.  Initialization occurs when
 * sf_comms_api_t::open is called */
typedef struct st_sf_uart_comms_instance_ctrl
{
    uint32_t                state;             ///< UART status
    uart_instance_t const * p_lower_lvl_uart;  ///< Pointer to UART interface (copied from cfg)
    TX_MUTEX                mutex[2];          ///< Pointer to the mutex object for UART resource mutual exclusion
    TX_EVENT_FLAGS_GROUP    eventflag[2];      ///< Pointer to the event flag object for UART data transfer
    TX_QUEUE                queue;             ///< Queue for reading
    uint32_t                queue_mem[SF_UART_COMMS_CFG_QUEUE_SIZE_WORDS];   ///< Queue memory
} sf_uart_comms_instance_ctrl_t;

/** Configuration for RTOS integrated UART driver */
typedef struct st_sf_uart_comms_cfg
{
    uart_instance_t const  * p_lower_lvl_uart;    /**< Pointer to UART Driver instance */
} sf_uart_comms_cfg_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const sf_comms_api_t g_sf_comms_on_sf_uart_comms;
/** @endcond */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_UART_COMMS)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_UART_COMMS_H */
