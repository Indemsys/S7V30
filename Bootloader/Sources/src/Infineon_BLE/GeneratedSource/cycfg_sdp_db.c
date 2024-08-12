/***************************************************************************//**
* File Name: cycfg_sdp_db.c
*
* Description:
* SDP database for Single Mode BR/EDR or Dual Mode device.
* This file should not be modified. It was automatically generated by
* Bluetooth Configurator 2.90.0.2529
*
********************************************************************************
* Copyright 2024 Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
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

#include "cycfg_sdp_db.h"
#include "wiced_bt_uuid.h"
#include "wiced_bt_sdp.h"

/*************************************************************************************
* SDP Database
*************************************************************************************/

const uint8_t sdp_database[] = 
{
    SDP_ATTR_SEQUENCE_1(150),

    /* SDP Record for Device ID */
    SDP_ATTR_SEQUENCE_1(69),
        SDP_ATTR_ID(ATTR_ID_SERVICE_RECORD_HDL), SDP_ATTR_VALUE_UINT4(HDLR_DEVICE_ID),    /* Record Handle */
        SDP_ATTR_ID(ATTR_ID_SERVICE_CLASS_ID_LIST), SDP_ATTR_SEQUENCE_1(3),    /* Class ID List */
            SDP_ATTR_UUID16(UUID_SERVCLASS_PNP_INFORMATION),    /* Class ID */
        SDP_ATTR_ID(ATTR_ID_PROTOCOL_DESC_LIST), SDP_ATTR_SEQUENCE_1(13),    /* Protocol Descriptor List */
            SDP_ATTR_SEQUENCE_1(6),    /* Protocol Descriptor 0 */
                SDP_ATTR_UUID16(UUID_PROTOCOL_L2CAP),    /* Protocol ID */
                SDP_ATTR_VALUE_UINT2(0x01),    /* Parameter 0 */
            SDP_ATTR_SEQUENCE_1(3),    /* Protocol Descriptor 1 */
                SDP_ATTR_UUID16(0x01),    /* L2CAP Channel */
        SDP_ATTR_ID(ATTR_ID_SPECIFICATION_ID), SDP_ATTR_VALUE_UINT2(0x103),    /* Specification ID */
        SDP_ATTR_ID(ATTR_ID_VENDOR_ID), SDP_ATTR_VALUE_UINT2(0x0F),    /* Vendor ID */
        SDP_ATTR_ID(ATTR_ID_PRODUCT_ID), SDP_ATTR_VALUE_UINT2(0x401),    /* Product ID */
        SDP_ATTR_ID(ATTR_ID_PRODUCT_VERSION), SDP_ATTR_VALUE_UINT2(0x01),    /* Version */
        SDP_ATTR_ID(ATTR_ID_PRIMARY_RECORD), SDP_ATTR_VALUE_BOOLEAN(0x01),    /* Primary Record */
        SDP_ATTR_ID(ATTR_ID_VENDOR_ID_SOURCE), SDP_ATTR_VALUE_UINT2(0x00),    /* Vendor ID Source */

    /* SDP Record for Serial Port */
    SDP_ATTR_SEQUENCE_1(77),
        SDP_ATTR_ID(ATTR_ID_SERVICE_RECORD_HDL), SDP_ATTR_VALUE_UINT4(HDLR_SERIAL_PORT),    /* Record Handle */
        SDP_ATTR_ID(ATTR_ID_SERVICE_CLASS_ID_LIST), SDP_ATTR_SEQUENCE_1(3),    /* Class ID List */
            SDP_ATTR_UUID16(UUID_SERVCLASS_SERIAL_PORT),    /* Service Class 0 */
        SDP_ATTR_ID(ATTR_ID_PROTOCOL_DESC_LIST), SDP_ATTR_SEQUENCE_1(12),    /* Protocol Descriptor List */
            SDP_ATTR_SEQUENCE_1(3),    /* Protocol Descriptor 0 */
                SDP_ATTR_UUID16(UUID_PROTOCOL_L2CAP),    /* Protocol ID */
            SDP_ATTR_SEQUENCE_1(5),    /* Protocol Descriptor 1 */
                SDP_ATTR_UUID16(UUID_PROTOCOL_RFCOMM),    /* Protocol ID */
                SDP_ATTR_VALUE_UINT1(0x01),    /* Parameter 0 */
        SDP_ATTR_ID(ATTR_ID_LANGUAGE_BASE_ATTR_ID_LIST), SDP_ATTR_SEQUENCE_1(9),    /* Language Base Attribute ID List */
            SDP_ATTR_VALUE_UINT2(LANG_ID_CODE_ENGLISH),    /* Language Code */
            SDP_ATTR_VALUE_UINT2(LANG_ID_CHAR_ENCODE_UTF8),    /* Character Encoding */
            SDP_ATTR_VALUE_UINT2(LANGUAGE_BASE_ID),    /* Base Attribute ID */
        SDP_ATTR_ID(ATTR_ID_BROWSE_GROUP_LIST), SDP_ATTR_SEQUENCE_1(3),    /* Browse Group List */
            SDP_ATTR_UUID16(UUID_SERVCLASS_PUBLIC_BROWSE_GROUP),    /* Public Browse Root */
        SDP_ATTR_ID(ATTR_ID_BT_PROFILE_DESC_LIST), SDP_ATTR_SEQUENCE_1(8),    /* Bluetooth Profile Descriptor List */
            SDP_ATTR_SEQUENCE_1(6),    /* Profile Descriptor 0 */
                SDP_ATTR_UUID16(UUID_SERVCLASS_SERIAL_PORT),    /* Profile */
                SDP_ATTR_VALUE_UINT2(0x102),    /* Parameter for Profile: Version */
        SDP_ATTR_ID(ATTR_ID_SERVICE_NAME), SDP_ATTR_VALUE_TEXT_1(5), 'S', '7', 'V', '3', '0',     /* Service Name */
};

/* Length of the SDP database */
const uint16_t sdp_database_len = sizeof(sdp_database);

/* Device class */
/* uint8_t device_class[] = {0x00, 0x00, 0x00, }; */