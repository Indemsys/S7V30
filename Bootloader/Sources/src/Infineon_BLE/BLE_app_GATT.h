#ifndef BLE_APP_GATT_H
  #define BLE_APP_GATT_H

  #define  MAX_BLE_CONNECTION_NUMBER      (CY_BT_CLIENT_MAX_LINKS + CY_BT_SERVER_MAX_LINKS)

typedef struct
{
    uint8_t                            active;
    wiced_bt_device_address_t          addr;    // Адресс отдельно храним, поскольку при передаче статуса адрес передается по ссылке, которая сразу же теряет актуальность
    wiced_bt_gatt_connection_status_t  status;
}
T_gatt_connection_cbl;


typedef void (*T_ble_gatt_conn_callback)(wiced_bt_gatt_connection_status_t *p_conn_status);
typedef void (*T_ble_gatt_callback)(wiced_bt_gatt_event_data_t *p_event_data);

#define  GATT_CALLBACKS_NUM   5

typedef struct
{
  T_ble_gatt_conn_callback    connect;
  T_ble_gatt_conn_callback    disconnect;
  T_ble_gatt_callback         op_complete;
  T_ble_gatt_callback         discovery_result;
  T_ble_gatt_callback         discovery_cplt;

} T_gatt_callbacks;


int32_t                  Get_gat_conn_id(wiced_bt_device_address_t addr);
int32_t                  Get_gat_conn_indx(wiced_bt_device_address_t addr);
wiced_bt_gatt_status_t   GATT_callback(wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_data);
T_gatt_connection_cbl*   Get_gatt_connection_status(uint32_t n);
uint32_t                 Set_gatt_callbacks(T_gatt_callbacks *gatt_cb_ptr);
uint32_t                 Delete_gatt_callbacks(T_gatt_callbacks *gatt_cb_ptr);

#endif // BLE_APP_GATT_H



