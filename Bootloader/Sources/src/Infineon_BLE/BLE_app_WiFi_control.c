// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-07-10
// 19:07:32
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BLE_main.h"

wifi_details_t            wifi_details;


/*-----------------------------------------------------------------------------------------------------


  \param conn_id
  \param opcode
  \param p_data

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
wiced_bt_gatt_status_t  WiFi_control_write_hundler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode, gatt_db_lookup_table_t *attr_ptr, wiced_bt_gatt_write_req_t *p_data)
{
  wiced_bt_gatt_status_t     result = WICED_BT_GATT_SUCCESS;
  uint8_t                   *new_val_ptr;
  uint16_t                   new_val_len;

  new_val_ptr  = p_data->p_val;
  new_val_len  = p_data->val_len;

  switch (p_data->handle)
  {
    // Записываем SSID
  case HDLC_CUSTOM_SERVICE_WIFI_SSID_VALUE:

    memcpy(app_custom_service_wifi_ssid, new_val_ptr, new_val_len);
    attr_ptr->cur_len = new_val_len;

    memcpy(&wifi_details.wifi_ssid[0], new_val_ptr, new_val_len);
    wifi_details.ssid_len = new_val_len;

    BLELOG("BLE: Wi-Fi SSID: %s", app_custom_service_wifi_ssid);
    break;

    // Записываем пароль
  case HDLC_CUSTOM_SERVICE_WIFI_PASSWORD_VALUE:

    memcpy(app_custom_service_wifi_password, new_val_ptr, new_val_len);
    attr_ptr->cur_len = new_val_len;

    memcpy(&wifi_details.wifi_password[0], new_val_ptr, new_val_len);
    wifi_details.password_len = new_val_len;

    BLELOG("BLE: Wi-Fi Password: %s", app_custom_service_wifi_password);
    break;

    // Запись команды для Wi-Fi подключения: подключиться, отключиться, сканировать
  case HDLC_CUSTOM_SERVICE_WIFI_CONTROL_VALUE:
    app_custom_service_wifi_control[0] = new_val_ptr[0];
    attr_ptr->cur_len = new_val_len;

    if (app_custom_service_wifi_control[0] == WIFI_CONTROL_CONNECT)
    {
      // Команда на подключение, только если SSID имеет не нулевую длину
      if (wifi_details.ssid_len  != 0)
      {
         // Отработка команды на подключение
      }
    }
    else if (app_custom_service_wifi_control[0] == WIFI_CONTROL_SCAN )
    {
      // Проверим включена ли нотификация, иначе сообщения об обнаружении новых сетей не дойдут до коиента
      if ((conn_id != 0) && (app_custom_service_wifi_networks_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
      {
        // Здесь отрабатываем команду на начало сканирования
      }
    }
    else if (app_custom_service_wifi_control[0] == WIFI_CONTROL_DISCONNECT)
    {
      // Здесь надо отрабатывать команду отключения
    }
    break;

    // Запись в Client Characteristic Configuration для аттрибута WIFI_NETWORKS, который содержит название текущей Wi-Fi сети
    // Клиент записывает суюда чтобы включать или отключать нотификации и индакации
    // Формат двухбайтного слова CCC:
    // Bit 0: Notification Enable (Уведомления включены)
    // Bit 1: Indication Enable (Индикации включены). Тоже что уведомления , но требуют подтверждения
    // Bits 2-15: Зарезервировано для будущего использования
  case HDLD_CUSTOM_SERVICE_WIFI_NETWORKS_CLIENT_CHAR_CONFIG:
    app_custom_service_wifi_networks_client_char_config[0] = new_val_ptr[0];
    app_custom_service_wifi_networks_client_char_config[1] = new_val_ptr[1];
    break;

    // Запись в Client Characteristic Configuration для аттрибута WIFI_CONTROL, который содержит команду управления Wi-Fi и состояние подключения
    // Клиент записывает суюда чтобы включать или отключать нотификации и индакации
    // Формат двухбайтного слова CCC:
    // Bit 0: Notification Enable (Уведомления включены)
    // Bit 1: Indication Enable (Индикации включены). Тоже что уведомления , но требуют подтверждения
    // Bits 2-15: Зарезервировано для будущего использования
  case HDLD_CUSTOM_SERVICE_WIFI_CONTROL_CLIENT_CHAR_CONFIG:
    app_custom_service_wifi_control_client_char_config[0] = new_val_ptr[0];
    app_custom_service_wifi_control_client_char_config[1] = new_val_ptr[1];
    break;

  default:
    result = WICED_BT_GATT_INVALID_HANDLE;
    break;
  }

  return result;
}




