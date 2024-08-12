#ifndef BLE_APP_WIFI_CONTROL_H
  #define BLE_APP_WIFI_CONTROL_H


/* Structure to store WiFi details that goes into EEPROM */
typedef __PACKED_STRUCT
{
    uint8_t wifi_ssid[CY_WCM_MAX_SSID_LEN];
    uint8_t ssid_len;
    uint8_t wifi_password[CY_WCM_MAX_PASSPHRASE_LEN];
    uint8_t password_len;

}wifi_details_t;

wiced_bt_gatt_status_t  WiFi_control_write_hundler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode, gatt_db_lookup_table_t *attr_ptr, wiced_bt_gatt_write_req_t *p_data);


#endif



