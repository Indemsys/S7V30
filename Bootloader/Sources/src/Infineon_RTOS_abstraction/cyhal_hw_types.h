/***************************************************************************//**
* \file cyhal_hw_types_template.h
*
* \brief
* Provides a template for configuration resources used by the HAL. Items
* here need to be implemented for each HAL port. It is up to the environment
* being ported into what the actual types are. There are some suggestions below
* but these are not required. All that is required is that the type is defined;
* it does not matter to the HAL what type is actually chosen for the
* implementation
* All TODOs and references to 'PORT' need to be replaced by with meaningful
* values for the device being supported.
*
********************************************************************************
* \copyright
* Copyright 2018-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/**
 * \addtogroup group_hal_hw_types PORT Hardware Types
 * \ingroup group_hal_PORT
 * \{
 * Struct definitions for configuration resources in the PORT.
 *
 * \defgroup group_hal_hw_types_data_structures Data Structures
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile uint32_t *  cyhal_gpio_t;

typedef struct
{
    void *div_type;
} cyhal_clock_divider_t;


typedef void *cyhal_sdio_t;


typedef struct
{
    void *empty;
} cyhal_spi_t;

typedef struct
{
    void *empty;
} cyhal_m2m_t;


typedef enum
{
  CYHAL_UART_PARITY_NONE ,
  CYHAL_UART_PARITY_EVEN ,
  CYHAL_UART_PARITY_ODD
}
cyhal_uart_parity_t;



typedef enum
{
  CYHAL_UART_IRQ_RX_NOT_EMPTY,
  CYHAL_UART_IRQ_RX_DONE,
  CYHAL_UART_IRQ_TX_DONE
}
cyhal_uart_event_t;


#if defined(__cplusplus)
}
#endif /* __cplusplus */


