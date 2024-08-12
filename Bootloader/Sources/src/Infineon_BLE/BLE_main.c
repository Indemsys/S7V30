// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-05-20
// 17:29:19
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "App.h"
#include "BLE_main.h"

extern wiced_bt_cfg_settings_t cy_bt_cfg_settings;

wiced_bt_heap_t          *app_heap_pointer; // Переменная мспользуется в где-то в библиотека btstack. Удалять нельзя.

T_bt_nv_data bt_nv @ ".stanby_ram";

#define BLE_INIT_TASK_STACK_SIZE   2048

static TX_THREAD       *ble_init_thread_ptr;
static uint8_t         *ble_init_stack;          // Указатель на стек задачи BLE_init_task
static uint32_t         ble_init_task_state;

uint16_t min_interval = 6; // TODO: Magic number from BTSDK implementation
uint16_t max_interval = 6; // TODO: Magic number from BTSDK implementation

wiced_bt_ble_conn_params_t   bt_conn_params;               // Bluetooth® connection parameters
uint8_t                      bt_peer_addr[BD_ADDR_LEN];    // Host Bluetooth® address

static uint8_t               bt_local_keys_valid;
T_bt_dev_pairing_info        bt_last_pairing_info;
uint8_t                      bt_advert_type_flag;

static uint8_t               bt_initialised;

/*-----------------------------------------------------------------------------------------------------
  Проверяем целостность ключей пайринга в памяти
  Если ключи повреждены, то обнулить все

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void _Check_bt_keys_validity(void)
{
  uint16_t crc;

  crc = Get_CRC16_of_block(bt_nv.paring_keys.link_keys , sizeof(bt_nv.paring_keys.link_keys), 0xFFFF);
  if (crc != bt_nv.paring_keys.crc)
  {
    memset(&bt_nv.paring_keys, 0, sizeof(bt_nv.paring_keys));
  }

  crc = Get_CRC16_of_block(&bt_nv.identity_keys.key , sizeof(wiced_bt_local_identity_keys_t), 0xFFFF);
  if (crc != bt_nv.identity_keys.crc)
  {
    bt_local_keys_valid = 0;
  }
  else
  {
    bt_local_keys_valid = 1;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Сохранить в память заданный ключ пайринга в ячейку с заданным индексом

  \param link_keys
  \param n

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
wiced_result_t _Write_paring_keys_to_list(wiced_bt_device_link_keys_t *link_keys, uint8_t addr_type, uint8_t  key_mask, uint32_t indx)
{
  uint16_t crc;
  memcpy(&bt_nv.paring_keys.link_keys[indx].key, link_keys, sizeof(wiced_bt_device_link_keys_t));
  // Скорректировать контрольную сумму всего блока ключей пайринга
  crc = Get_CRC16_of_block(bt_nv.paring_keys.link_keys , sizeof(bt_nv.paring_keys.link_keys), 0xFFFF);
  bt_nv.paring_keys.crc = crc;
  return WICED_BT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param link_keys

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t _Search_paring_keys(wiced_bt_device_address_t bd_addr)
{
  for (int32_t i = 0; i < PARING_KEYS_NUM; i++)
  {
    if (memcmp(bd_addr, bt_nv.paring_keys.link_keys[i].key.bd_addr, sizeof(wiced_bt_device_address_t)) == 0)
    {
      return i;
    }
  }
  return -1;
}

/*-----------------------------------------------------------------------------------------------------


  \param link_keys

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
wiced_result_t _Store_paring_keys(wiced_bt_device_link_keys_t *link_keys, uint8_t addr_type, uint8_t  key_mask)
{
  int32_t indx;

  // Ищем в массиве уже имеющуюся запись для данного идентификатора
  indx = _Search_paring_keys(link_keys->bd_addr);
  if (indx >= 0)
  {
    // Если нашли, то обновляем ключи в записи
    _Write_paring_keys_to_list(link_keys, addr_type, key_mask, indx);
    Request_save_bt_nv_data(); // Дать команду на сохранение всей NV информации bluetooth во Flash
    BLELOG("BLE: Link Keys saved to rec.%d", indx);
    return WICED_SUCCESS;
  }

  // Если не нашли, то ищем пустую запись
  for (indx = 0; indx < PARING_KEYS_NUM; indx++)
  {
    if (Is_All_Zeros(bt_nv.paring_keys.link_keys[indx].key.bd_addr, 6) == 0)
    {
      // Если нашли пустую запись, то размещаем в ней ключи
      _Write_paring_keys_to_list(link_keys, addr_type, key_mask, indx);
      bt_nv.oldest_paring_keys_indx = indx + 1;
      if (bt_nv.oldest_paring_keys_indx >= PARING_KEYS_NUM) bt_nv.oldest_paring_keys_indx = 0;
      Request_save_bt_nv_data(); // Дать команду на сохранение всей NV информации bluetooth во Flash
      BLELOG("BLE: Link Keys saved to rec.%d", indx);
      return WICED_SUCCESS;
    }
  }

  // Если все записи заняты, то переписываем самую старую запись
  _Write_paring_keys_to_list(link_keys, addr_type, key_mask, bt_nv.oldest_paring_keys_indx);
  bt_nv.oldest_paring_keys_indx++;
  if (bt_nv.oldest_paring_keys_indx >= PARING_KEYS_NUM) bt_nv.oldest_paring_keys_indx = 0;
  Request_save_bt_nv_data(); // Дать команду на сохранение всей NV информации bluetooth во Flash

  BLELOG("BLE: Link Keys saved to rec.%d", bt_nv.oldest_paring_keys_indx);
  return WICED_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------


  \param link_keys

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
wiced_result_t _Load_paring_keys(wiced_bt_device_link_keys_t *link_keys)
{
  uint32_t i;
  for (i = 0; i < PARING_KEYS_NUM; i++)
  {
    if (memcmp(link_keys->bd_addr,bt_nv.paring_keys.link_keys[i].key.bd_addr, sizeof(wiced_bt_device_address_t)) == 0)
    {
      memcpy(link_keys,&bt_nv.paring_keys.link_keys[i].key, sizeof(wiced_bt_device_link_keys_t));
      BLELOG("BLE: Link Keys found!");
      return WICED_BT_SUCCESS;
    }
  }
  BLELOG("BLE: Link Keys NOT found!");
  return WICED_BT_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_enabled(wiced_bt_management_evt_data_t *p_event_data)
{
  #ifdef ENABLE_BLE_LOG
  char                       str[64];
  #endif
  wiced_result_t             result      = WICED_BT_SUCCESS;
  wiced_bt_gatt_status_t     gatt_status = WICED_BT_GATT_INVALID_CONNECTION_ID;
  wiced_bt_device_address_t  bda         = {0};


  app_heap_pointer = wiced_bt_create_heap("app", NULL, BT_HEAP_SIZE, NULL, WICED_TRUE); // Create a buffer heap, make it the default heap

  wiced_bt_smp_module_init();
  wiced_bt_init_resolution();

  wiced_bt_set_local_bdaddr((uint8_t *)cy_bt_device_address,BLE_ADDR_PUBLIC); // Bluetooth is enabled
  wiced_bt_dev_read_local_addr(bda); // Read and print the BD address

  BLELOG("BLE: Local Address: %s",   Buf_to_hex_str(str,6,bda));

  BT_spp_startup();

  gatt_status = wiced_bt_gatt_register(GATT_callback); // Register with stack to receive GATT callback
  if (gatt_status != WICED_BT_GATT_SUCCESS)
  {
    BLELOG("BLE: GATT register failed. Status: %s", get_bt_gatt_status_name(gatt_status));
  }

  gatt_status = wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL); //  Inform the stack to use our GATT database
  if (gatt_status != WICED_BT_GATT_SUCCESS)
  {
    BLELOG("BLE: GATT db init failed. Status :%s",get_bt_gatt_status_name(gatt_status));
  }

  // Загружаем базу данных спаренных устройств
  for (uint32_t i = 0; i < PARING_KEYS_NUM; i++)
  {
    if (Is_All_Zeros(bt_nv.paring_keys.link_keys[i].key.bd_addr, 6) != 0)
    {
      wiced_bt_dev_add_device_to_address_resolution_db(&bt_nv.paring_keys.link_keys[i].key);
    }
  }


  wiced_bt_set_pairable_mode(WICED_TRUE, WICED_FALSE); // Allow peer to pair

  result = wiced_bt_ble_set_raw_advertisement_data(CY_BT_ADV_PACKET_DATA_SIZE,cy_bt_adv_packet_data); // Set BLE advertisement data
  if (WICED_SUCCESS != result)
  {
    BLELOG("BLE: Set ADV data failed");
  }

  result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,BLE_ADDR_PUBLIC, NULL);
  if (result != WICED_SUCCESS)
  {
    BLELOG("BLE: Start ADV failed");
  }
  if (result == WICED_SUCCESS) bt_initialised = 1;
  return result;
}

/*-----------------------------------------------------------------------------------------------------
  Реакция на сообщение о факте изменеия параметров соединения

  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_connection_param_update(wiced_bt_management_evt_data_t *p_event_data)
{

  #ifdef ENABLE_BLE_LOG
  char                       str[64];
  #endif
  wiced_bt_dev_status_t res = WICED_SUCCESS;

  BLELOG("BLE: ble_connection_param_update.bd_addr: %s",   Buf_to_hex_str(str,6,p_event_data->ble_connection_param_update.bd_addr));
  BLELOG("BLE: ble_connection_param_update.conn_interval       : %d", p_event_data->ble_connection_param_update.conn_interval);
  BLELOG("BLE: ble_connection_param_update.conn_latency        : %d", p_event_data->ble_connection_param_update.conn_latency);
  BLELOG("BLE: ble_connection_param_update.status              : %d", p_event_data->ble_connection_param_update.status);
  BLELOG("BLE: ble_connection_param_update.supervision_timeout : %d", p_event_data->ble_connection_param_update.supervision_timeout);

  return res;
}
/*-----------------------------------------------------------------------------------------------------


  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_pairing_io_capabilities_ble_request(wiced_bt_management_evt_data_t *p_event_data)
{
  wiced_bt_dev_ble_io_caps_req_t    *pairing_io_caps = &(p_event_data->pairing_io_capabilities_ble_request);

  if (Check_BT_pairing_mode())
  {
    pairing_io_caps->local_io_cap = BTM_IO_CAPABILITIES_KEYBOARD_ONLY; //BTM_IO_CAPABILITIES_NONE; //BTM_IO_CAPABILITIES_DISPLAY_ONLY;
  }
  else
  {
    pairing_io_caps->local_io_cap = BTM_IO_CAPABILITIES_NONE; //BTM_IO_CAPABILITIES_NONE; //BTM_IO_CAPABILITIES_DISPLAY_ONLY;
  }
  pairing_io_caps->oob_data     = BTM_OOB_NONE;
  // Если здесь не выбрать BTM_LE_AUTH_REQ_SC_BOND, то бондинг будет требоваться при каждом запуске программы на телефоне
  // С опцией BTM_LE_AUTH_REQ_H7 не происходит пайринг с Android
  pairing_io_caps->auth_req     = BTM_LE_AUTH_REQ_SC_BOND | BTM_LE_AUTH_REQ_MITM;
  pairing_io_caps->max_key_size = MAX_LE_KEY_SIZE;
  pairing_io_caps->init_keys    = BTM_LE_KEY_PENC | BTM_LE_KEY_PID; //BTM_LE_KEY_PENC | BTM_LE_KEY_PID | BTM_LE_KEY_PCSRK | BTM_LE_KEY_LENC;


  return WICED_BT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_passkey_notification(wiced_bt_management_evt_data_t *p_event_data)
{
  BLELOG("BLE: PassKey: %lu", p_event_data->user_passkey_notification.passkey);

  wiced_bt_dev_confirm_req_reply(WICED_BT_SUCCESS, p_event_data->user_passkey_notification.bd_addr);
  return WICED_BT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_pairing_complete(wiced_bt_management_evt_data_t *p_event_data)
{
  memcpy(&bt_last_pairing_info.info,&p_event_data->pairing_complete, sizeof(p_event_data->pairing_complete));
  memcpy(bt_last_pairing_info.addr,p_event_data->pairing_complete.bd_addr, sizeof(wiced_bt_device_address_t));
  if (p_event_data->pairing_complete.pairing_complete_info.ble.status == WICED_SUCCESS)
  {
    BLELOG("BLE: Pairing Complete: SUCCESS");
  }
  else
  {
    BLELOG("BLE: Pairing Complete: FAILED");
  }
  return WICED_BT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------


  \param p_event_data
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_local_identity_keys_update(wiced_bt_management_evt_data_t *p_event_data)
{
  #ifdef ENABLE_BLE_LOG
  char                               str[64];
  #endif
  uint8_t                           *key_buf;

  BLELOG("BLE: Local Keys mask   : %02X", p_event_data->local_identity_keys_update.key_type_mask);
  BLELOG("BLE: Local id_keys.dhk : %s",  Buf_to_hex_str(str,16,p_event_data->local_identity_keys_update.id_keys.dhk));
  BLELOG("BLE: Local id_keys.ir  : %s",  Buf_to_hex_str(str,16,p_event_data->local_identity_keys_update.id_keys.ir));
  BLELOG("BLE: Local id_keys.irk : %s",  Buf_to_hex_str(str,16,p_event_data->local_identity_keys_update.id_keys.irk));
  BLELOG("BLE: Local er          : %s",  Buf_to_hex_str(str,16,p_event_data->local_identity_keys_update.er));

  key_buf = (uint8_t *)&(p_event_data->local_identity_keys_update);
  bt_nv.identity_keys.crc = Get_CRC16_of_block(key_buf, sizeof(wiced_bt_local_identity_keys_t), 0xFFFF);
  memcpy(&bt_nv.identity_keys.key, key_buf, sizeof(wiced_bt_local_identity_keys_t));
  Request_save_bt_nv_data();
  return WICED_BT_SUCCESS;

}

/*-----------------------------------------------------------------------------------------------------
  Извлекаем локальные ключи из энергонезависимой памяти и проверяем их целостность.

  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_local_identity_keys_request(wiced_bt_management_evt_data_t *p_event_data)
{
  if (bt_local_keys_valid == 0)
  {
    BLELOG("BLE: New identity keys need to be generated by the stack!");
    return WICED_BT_ERROR;
  }
  memcpy(&(p_event_data->local_identity_keys_request),&bt_nv.identity_keys.key, sizeof(wiced_bt_local_identity_keys_t));
  BLELOG("BLE: Local identity resolving key read from NV");
  return WICED_BT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  Здесь нужно сохранить новый ключ соединения с удаленным устройством

  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_paired_device_link_keys_update(wiced_bt_management_evt_data_t *p_event_data)
{
  wiced_result_t  res;
  #ifdef ENABLE_BLE_LOG
  char                               str[64];
  #endif

  wiced_bt_ble_address_type_t   addr_type =  p_event_data->paired_device_link_keys_update.key_data.ble_addr_type;
  wiced_bt_dev_le_key_type_t    key_mask  =  p_event_data->paired_device_link_keys_update.key_data.le_keys_available_mask;
  BLELOG("BLE: Updated addr type: 0x%02X, keys mask: 0x%02X",addr_type, key_mask);
  BLELOG("BLE: Updated paired device address: %s",   Buf_to_hex_str(str,6,p_event_data->paired_device_link_keys_update.bd_addr));
  BLELOG("BLE: Updated conn_addr            : %s",   Buf_to_hex_str(str,6,p_event_data->paired_device_link_keys_update.conn_addr));
  _Store_paring_keys(&(p_event_data->paired_device_link_keys_update), addr_type, key_mask);
  res = wiced_bt_dev_add_device_to_address_resolution_db(&p_event_data->paired_device_link_keys_update);
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_paired_device_link_keys_request(wiced_bt_management_evt_data_t *p_event_data)
{
  #ifdef ENABLE_BLE_LOG
  char                               str[64];
  #endif

  BLELOG("BLE: Requested addr type: 0x%02X, keys mask: 0x%02X",p_event_data->paired_device_link_keys_request.key_data.ble_addr_type, p_event_data->paired_device_link_keys_request.key_data.le_keys_available_mask)
  BLELOG("BLE: Requested paired device address: %s",   Buf_to_hex_str(str,6,p_event_data->paired_device_link_keys_request.bd_addr));
  BLELOG("BLE: Requested conn_addr            : %s",   Buf_to_hex_str(str,6,p_event_data->paired_device_link_keys_request.conn_addr));
  return _Load_paring_keys(&(p_event_data->paired_device_link_keys_request));
  ;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_encryption_status(wiced_bt_management_evt_data_t *p_event_data)
{
  if (WICED_SUCCESS == p_event_data->encryption_status.result)
  {
    BLELOG("BLE: Encryption Status Event: SUCCESS");
  }
  else
  {
    BLELOG("BLE: Encryption Status Event: FAILED");
  }
  return WICED_BT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t _Do_advert_state_changed(wiced_bt_management_evt_data_t *p_event_data)
{
  wiced_result_t result = WICED_BT_SUCCESS;
  BLELOG("BLE: Advertisement state changed to %s", get_bt_advert_mode_name(p_event_data->ble_advert_state_changed));
  //if (p_event_data->ble_advert_state_changed == BTM_BLE_ADVERT_UNDIRECTED_LOW)
  //{
  //  result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,BLE_ADDR_PUBLIC, NULL);
  //  if (result != WICED_BT_SUCCESS)
  //  {
  //    BLELOG("BLE: Start ADV failed");
  //  }
  //}
  return result;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_remote_name_result
-----------------------------------------------------------------------------------------------------*/
static void _Get_remote_name_callback(wiced_bt_dev_remote_name_result_t *p_remote_name_result)
{
  #ifdef ENABLE_BLE_LOG
  char                       str[64];
  #endif

  BLELOG("Pairing with: BdAddr:%s Status:%d Len:%d Name:%s", Buf_to_hex_str(str,6,p_remote_name_result->bd_addr), p_remote_name_result->status,p_remote_name_result->length, p_remote_name_result->remote_bd_name);
}


/*-----------------------------------------------------------------------------------------------------
  Проверить режим пайринга Bluetooth

  \param void

  \return uint8_t  Возвращает 0 если пайринг разрешен без ввода пин кода.
                   Возвращает 1 если пайринг возможен только после ввода пин кода на модуле.
-----------------------------------------------------------------------------------------------------*/
uint8_t Check_BT_pairing_mode(void)
{
  if (BT3_STATE == 0)
  {
    // Нажата кнопка BT3 на модуле S7V30
    return 0;
  }
  return 1;
}


/*-----------------------------------------------------------------------------------------------------


  \param event
  \param p_event_data

  \return wiced_result_t
-----------------------------------------------------------------------------------------------------*/
static wiced_result_t BLE_app_management_callback(wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data)
{
  wiced_result_t                     result        = WICED_BT_SUCCESS;


  BLELOG("BLE: Event: %s", get_btm_event_name(event));

  switch (event)
  {
  case BTM_ENABLED_EVT:
    result = _Do_enabled(p_event_data);
    break;

  case BTM_DISABLED_EVT:
    break;

  case BTM_USER_CONFIRMATION_REQUEST_EVT:
    // Event requests user confirmation for the numeric value to continue the App is expected to respond with using #wiced_bt_dev_confirm_req_reply  typically by confirming via a display to the user
    // Это событие не вызывается если установлена опция BTM_IO_CAPABILITIES_NONE
    wiced_bt_dev_confirm_req_reply(WICED_BT_SUCCESS, p_event_data->user_confirmation_request.bd_addr);
    wiced_bt_dev_get_remote_name(p_event_data->user_confirmation_request.bd_addr, _Get_remote_name_callback);
    break;

  case BTM_PASSKEY_NOTIFICATION_EVT:
    // Это событие не вызывается если установлена опция BTM_IO_CAPABILITIES_NONE
    result = _Do_passkey_notification(p_event_data);
    break;

  case BTM_PAIRING_IO_CAPABILITIES_BR_EDR_REQUEST_EVT: // Event requests BR/EDR IO capabilities for BR/EDR pairing from app
    if (Check_BT_pairing_mode())
    {
      p_event_data->pairing_io_capabilities_br_edr_request.local_io_cap   = BTM_IO_CAPABILITIES_KEYBOARD_ONLY;
    }
    else
    {
      p_event_data->pairing_io_capabilities_br_edr_request.local_io_cap   = BTM_IO_CAPABILITIES_NONE;
    }
    p_event_data->pairing_io_capabilities_br_edr_request.auth_req       = BTM_AUTH_SINGLE_PROFILE_GENERAL_BONDING_NO;
    break;

  case BTM_BLE_CONNECTION_PARAM_UPDATE:
    // Event notifies LE connection parameter update to app
    result = _Do_connection_param_update(p_event_data);
    break;


  case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
    // Event requests LE IO capabilities for LE pairing from app. Peripheral can check peer io capabilities in event data before updating with local io capabilities.
    result = _Do_pairing_io_capabilities_ble_request(p_event_data);
    break;

  case BTM_PAIRING_COMPLETE_EVT:
    // Event notifies simple pairing complete to app
    result = _Do_pairing_complete(p_event_data);
    break;

  case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
    // Если здесь не вызывать _Do_paired_device_link_keys_update и возвратить WICED_BT_ERROR, то это не помешает текущему пайрингу.
    result = _Do_paired_device_link_keys_update(p_event_data); // Paired Device Link Keys update
    break;

  case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
    result = _Do_paired_device_link_keys_request(p_event_data);
    break;

  case BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT:
    result = _Do_local_identity_keys_update(p_event_data); // Local identity Keys Update
    break;

  case BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT:
    result = _Do_local_identity_keys_request(p_event_data); // Local identity Keys Request
    break;

  case BTM_ENCRYPTION_STATUS_EVT:
    result = _Do_encryption_status(p_event_data);
    break;

  case BTM_SECURITY_REQUEST_EVT:
    wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr,WICED_BT_SUCCESS);
    break;

  case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
    result = _Do_advert_state_changed(p_event_data);
    break;

  case BTM_POWER_MANAGEMENT_STATUS_EVT: //Event notifies Power management status change.
    break;

  case BTM_RE_START_EVT:  // Event notifies Bluetooth controller and host stack re-enabled.
    break;

  case BTM_PIN_REQUEST_EVT: // Event requests app for the PIN to be used for pairing (legacy pairing only).
    break;

  case BTM_PASSKEY_REQUEST_EVT: // Event requests user passkey from app
    break;

  case BTM_KEYPRESS_NOTIFICATION_EVT:
    break;

  case BTM_PAIRING_IO_CAPABILITIES_BR_EDR_RESPONSE_EVT:
    break;

  case BTM_SECURITY_FAILED_EVT: // Event notifies Security procedure/authentication failed to app
    break;

  case BTM_SECURITY_ABORTED_EVT:
    break;

  case BTM_READ_LOCAL_OOB_DATA_COMPLETE_EVT: // Event notifies result of reading local OOB data from the controller
    break;

  case BTM_SMP_REMOTE_OOB_DATA_REQUEST_EVT:
    break;

  case BTM_SMP_SC_REMOTE_OOB_DATA_REQUEST_EVT:
    break;

  case BTM_REMOTE_OOB_DATA_REQUEST_EVT:
    break;

  case BTM_SMP_SC_LOCAL_OOB_DATA_NOTIFICATION_EVT:
    break;

  case BTM_SCO_CONNECTED_EVT:
    break;

  case BTM_SCO_DISCONNECTED_EVT:
    break;

  case BTM_SCO_CONNECTION_REQUEST_EVT:
    break;

  case BTM_SCO_CONNECTION_CHANGE_EVT:
    break;

  case BTM_BLE_PHY_UPDATE_EVT:
    break;

  case BTM_LPM_STATE_LOW_POWER:
    break;

  case BTM_MULTI_ADVERT_RESP_EVENT:
    break;

  case BTM_BLE_DATA_LENGTH_UPDATE_EVENT:
    break;

  case BTM_BLE_SUBRATE_CHANGE_EVENT:
    break;

  case BTM_BLE_DEVICE_ADDRESS_UPDATE_EVENT:
    break;

  case BTM_BLE_SCAN_STATE_CHANGED_EVT: // Event notifies LE scan state change to app
    break;

  default:
    break;
  }

  return result;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void BLE_init_task(ULONG arg)
{
  uint32_t  actual_flags;

  ble_init_task_state  = BLE_INIT_TASK_STARTED;

  // Здесь ждем пока не запустится точка доступа Wi-Fi
  Wait_wifi_event(NET_EVT_WIFI_INIT_DONE, &actual_flags, TX_WAIT_FOREVER);

  BLELOG("BLE init started.");

  WiFi_module_switch_on_Bluetooth();

  // Конструируем MAC адрес Renesas Electronics Corporation
  cy_bt_device_address[0] = 0x74;
  cy_bt_device_address[1] = 0x90;
  cy_bt_device_address[2] = 0x50;
  cy_bt_device_address[3] = g_cpu_id[1];
  cy_bt_device_address[4] = g_cpu_id[2];
  cy_bt_device_address[5] = g_cpu_id[3];

  cy_bt_cfg_settings.device_name = ivar.ble_device_name;

  // Устанавливаем имя устройства в базе данных GATT
  app_gatt_db_ext_attr_tbl[0].cur_len = strlen((char *)ivar.ble_device_name);
  app_gatt_db_ext_attr_tbl[0].p_data = ivar.ble_device_name;

  // Устанавливаем имя устройства в базе данных Advertisement
  cy_bt_adv_packet_data[1].len = strlen((char *)ivar.ble_device_name);
  cy_bt_adv_packet_data[1].p_data = ivar.ble_device_name;

  // Исправляем объявление флагов устройства. Устанавливаем: LE General Discoverable Mode, LE and BR/EDR Capable (Host)
  bt_advert_type_flag = 0x12;
  cy_bt_adv_packet_data[0].p_data = &bt_advert_type_flag;

  // Устанавливаем имя устройства в базе данных Scan Response
  cy_bt_scan_resp_packet_data[0].len = strlen((char *)ivar.ble_device_name);
  cy_bt_scan_resp_packet_data[0].p_data = ivar.ble_device_name;


  Restore_buf_from_DataFlash(DATAFLASH_BLUETOOTH_DATA_ADDR, (uint8_t *)&bt_nv, sizeof(bt_nv));
  _Check_bt_keys_validity();


  #ifdef ENABLE_BLE_RTT_LOG
  cybt_platform_set_trace_level(CYBT_TRACE_ID_ALL, CYBT_TRACE_LEVEL_MAX);
  #endif

  UART_BLE_init(HCI_UART_DEFAULT_BAUDRATE, SCI_NO_PRITY, 1); // Инициализируем UART для связи с модулем BLE

  wiced_bt_stack_init(BLE_app_management_callback,&cy_bt_cfg_settings);

  ble_init_task_state  = BLE_INIT_TASK_FINISHED;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return wiced_bt_dev_pairing_cplt_t*
-----------------------------------------------------------------------------------------------------*/
T_bt_dev_pairing_info* BLE_get_last_pairing_info(void)
{
  return &bt_last_pairing_info;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t BLE_init_task_state(void)
{
  return ble_init_task_state;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint8_t BLE_is_initialised(void)
{
  return bt_initialised;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t BLE_init_delete(void)
{
  uint32_t res;
  ble_init_task_state = BLE_INIT_TASK_NOT_STARTED;
  res = tx_thread_terminate(ble_init_thread_ptr);
  res |= tx_thread_delete(ble_init_thread_ptr);
  App_free(ble_init_thread_ptr);
  App_free(ble_init_stack);
  APPLOG("BLE init task deleted. Result = %04X", res);
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Запускаем задачу инициализации BLE

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t BLE_init(void)
{
  ULONG     res = 0;
  uint32_t  err_line;

  if ((ivar.en_ble==0) || (ivar.en_wifi_module==0))
  {
    return RES_ERROR;
  }


  ble_init_thread_ptr = NULL;
  ble_init_stack      = NULL;

  ble_init_task_state = BLE_INIT_TASK_NOT_STARTED;

  ble_init_thread_ptr =  (TX_THREAD *)App_malloc(sizeof(TX_THREAD));
  if (ble_init_thread_ptr == NULL)
  {
    err_line = __LINE__;
    goto _err;
  }
  ble_init_stack =  App_malloc(BLE_INIT_TASK_STACK_SIZE);
  if (ble_init_stack == NULL)
  {
    err_line = __LINE__;
    goto _err;
  }


  res = tx_thread_create(
                         ble_init_thread_ptr,
                         (CHAR *)"BLE init",
                         BLE_init_task,
                         0,
                         ble_init_stack,
                         BLE_INIT_TASK_STACK_SIZE,
                         BLE_INIT_TASK_PRIO,
                         BLE_INIT_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  if (res != TX_SUCCESS)
  {
    err_line = __LINE__;
    goto _err;
  }

  APPLOG("BLE init task created");
  return RES_OK;


_err:
  App_free(ble_init_thread_ptr);
  App_free(ble_init_stack);
  APPLOG("BLE init task creationn error in line %d. Result=%04X", err_line, res);
  return RES_ERROR;
}

