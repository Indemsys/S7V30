#ifndef BLE_STRINGS_H
  #define BLE_STRINGS_H

const char* get_bt_advert_mode_name(wiced_bt_ble_advert_mode_t mode);
const char* get_btm_event_name(wiced_bt_management_evt_t event);
const char* get_bt_gatt_status_name(wiced_bt_gatt_status_t status);
const char* get_bt_gatt_evt_name(wiced_bt_gatt_evt_t event);
const char* get_bt_gatt_disconn_reason_name(wiced_bt_gatt_disconn_reason_t reason);
const char* get_bt_stack_err_name(cybt_result_t res);

#endif



