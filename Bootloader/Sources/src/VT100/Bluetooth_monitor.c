// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-07-09
// 10:39:33
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BLE_main.h"

#define  BT_MAX_REMOTE_NAME_LEN    64
wiced_bt_dev_rssi_result_t         rssi_info;
char                               remote_name[BT_MAX_REMOTE_NAME_LEN+1];

// Переменные процесса обсервера

#define START_ROW                    9
#define BLE_DEV_STR_TYPE_LEN         32
#define BLE_DEV_DIG_TYPE_LEN         10
#define MAX_OSERVER_RECS_NUM         30
#define MAX_OSERVER_UUID_NUM         4

#define   FLG_BT_OBSRV_RES_NEW          BIT(0)
#define   FLG_BT_OBSRV_COMPL            BIT(1)
#define   FLG_BT_RSSI_COMPL             BIT(2)
#define   FLG_BT_NAME_COMPL             BIT(3)
#define   FLG_BT_BLE_DEV_CONNECTED      BIT(4)
#define   FLG_BT_BLE_DEV_DISCONNECTED   BIT(5)
#define   FLG_BT_GATT_OP_COMPLETED      BIT(6)
#define   FLG_BT_GATT_OP_NOTIFICATION   BIT(7)

static wiced_bt_ble_scan_results_t   g_last_scan_result;
static uint8_t                       last_scan_name[BLE_DEV_STR_TYPE_LEN+1];
static uint8_t                       last_scan_UUIDs_num;
static uint16_t                      last_scan_UUIDs[MAX_OSERVER_UUID_NUM];
static TX_EVENT_FLAGS_GROUP          bt_mon_flags;
static uint8_t                       observer_busy;


typedef struct
{
    uint8_t                         active;
    uint32_t                        cnt;
    wiced_bt_device_address_t       addr;
    uint8_t                         addr_type;
    uint8_t                         flag;
    int8_t                          rssi;
    uint8_t                         tx_power;
    uint8_t                         UUIDs_num;
    uint16_t                        UUIDs[MAX_OSERVER_UUID_NUM];
    uint8_t                         device_name[BLE_DEV_STR_TYPE_LEN+1];
    uint8_t                         model_num[BLE_DEV_STR_TYPE_LEN+1];
    uint8_t                         serial_num[BLE_DEV_STR_TYPE_LEN+1];
    uint8_t                         dig_attr[BLE_DEV_DIG_TYPE_LEN];

} T_oserver_rec;

T_oserver_rec           orecs[MAX_OSERVER_RECS_NUM];
static int32_t          curr_conn_id;
static uint8_t          curr_op_status;


/*-----------------------------------------------------------------------------------------------------


  \param v

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* To_Str_Yes_No(uint32_t v)
{
  if (v != 0)
  {
    return "Yes";
  }
  else
  {
    return "No ";
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* To_Str_Addr_type(uint8_t v)
{
  switch (v)
  {
  case BLE_ADDR_PUBLIC   : //  0x00
    return "PUBLIC  ";
  case BLE_ADDR_RANDOM   : //  0x01
    return "RANDOM   ";
  case BLE_ADDR_PUBLIC_ID: //  0x02
    return "PUBLIC_ID";
  case BLE_ADDR_RANDOM_ID: //  0x03
    return "RANDOM_ID";
  }
  return "UNKNOWN  ";
}

/*-----------------------------------------------------------------------------------------------------

  \param v

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* To_Str_Link_Role(uint8_t v)
{
  switch (v)
  {
  case HCI_ROLE_CENTRAL   :
    return "CENTRAL   ";
  case HCI_ROLE_PERIPHERAL   :
    return "PERIPHERAL";
  case HCI_ROLE_UNKNOWN:
    return "UNKNOWN   ";
  }
  return "UNKNOWN  ";
}

/*-----------------------------------------------------------------------------------------------------
  BT_TRANSPORT_BR_EDR
  BT_TRANSPORT_LE
  \param v

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* To_Str_Bluetooth_transport(uint8_t v)
{
  switch (v)
  {
  case BT_TRANSPORT_BR_EDR   :
    return "BR_EDR";
  case BT_TRANSPORT_LE   :
    return "LE    ";
  }
  return "UNKNOWN  ";
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* To_Str_BLE_sec_level(uint8_t v)
{
  switch (v)
  {
  case SMP_SEC_NONE   :
    return "NONE          ";
  case SMP_SEC_UNAUTHENTICATE   :
    return "UNAUTHENTICATE";
  case SMP_SEC_AUTHENTICATED   :
    return "AUTHENTICATED ";
  }
  return "UNKNOWN  ";
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* To_Str_SMP_status(uint8_t v)
{
  switch (v)
  {
  case SMP_SUCCESS                 :
    return "SUCCESS                ";
  case SMP_PASSKEY_ENTRY_FAIL      :
    return "PASSKEY_ENTRY_FAIL     ";
  case SMP_OOB_FAIL                :
    return "OOB_FAIL               ";
  case SMP_PAIR_AUTH_FAIL          :
    return "PAIR_AUTH_FAIL         ";
  case SMP_CONFIRM_VALUE_ERR       :
    return "CONFIRM_VALUE_ERR      ";
  case SMP_PAIR_NOT_SUPPORT        :
    return "PAIR_NOT_SUPPORT       ";
  case SMP_ENC_KEY_SIZE            :
    return "ENC_KEY_SIZE           ";
  case SMP_INVALID_CMD             :
    return "INVALID_CMD            ";
  case SMP_PAIR_FAIL_UNKNOWN       :
    return "PAIR_FAIL_UNKNOWN      ";
  case SMP_REPEATED_ATTEMPTS       :
    return "REPEATED_ATTEMPTS      ";
  case SMP_INVALID_PARAMETERS      :
    return "INVALID_PARAMETERS     ";
  case SMP_DHKEY_CHK_FAIL          :
    return "DHKEY_CHK_FAIL         ";
  case SMP_NUMERIC_COMPAR_FAIL     :
    return "NUMERIC_COMPAR_FAIL    ";
  case SMP_BR_PAIRING_IN_PROGR     :
    return "BR_PAIRING_IN_PROGR    ";
  case SMP_XTRANS_DERIVE_NOT_ALLOW :
    return "XTRANS_DERIVE_NOT_ALLOW";
  case SMP_PAIR_INTERNAL_ERR       :
    return "PAIR_INTERNAL_ERR      ";
  case SMP_UNKNOWN_IO_CAP          :
    return "UNKNOWN_IO_CAP         ";
  case SMP_INIT_FAIL               :
    return "INIT_FAIL              ";
  case SMP_CONFIRM_FAIL            :
    return "CONFIRM_FAIL           ";
  case SMP_BUSY                    :
    return "BUSY                   ";
  case SMP_ENC_FAIL                :
    return "ENC_FAIL               ";
  case SMP_STARTED                 :
    return "STARTED                ";
  case SMP_RSP_TIMEOUT             :
    return "RSP_TIMEOUT            ";
  case SMP_FAIL                    :
    return "FAIL                   ";
  case SMP_CONN_TOUT               :
    return "CONN_TOUT              ";
  }
  return "UNKNOWN  ";
}

/*-----------------------------------------------------------------------------------------------------


  \param p_data
-----------------------------------------------------------------------------------------------------*/
static void  _Accept_RSSI_callback(void *p_data)
{
  memcpy(&rssi_info,  (wiced_bt_dev_rssi_result_t *)p_data, sizeof(wiced_bt_dev_rssi_result_t));
  tx_event_flags_set(&bt_mon_flags, FLG_BT_RSSI_COMPL, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_remote_name_result
-----------------------------------------------------------------------------------------------------*/
static void _Accept_remote_name(wiced_bt_dev_remote_name_result_t *p_remote_name_result)
{
  uint32_t n;
  if (p_remote_name_result->status == 0)
  {
    n = BT_MAX_REMOTE_NAME_LEN;
    if (p_remote_name_result->length < BT_MAX_REMOTE_NAME_LEN) n = p_remote_name_result->length;
    memcpy(remote_name, p_remote_name_result->remote_bd_name, n);
    remote_name[n] = 0;
  }
  else
  {
    remote_name[0] = 0;
  }
  tx_event_flags_set(&bt_mon_flags, FLG_BT_NAME_COMPL, TX_OR);

}


/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексе задачи cybt_hci_rx_task

  \param p_scan_result
  \param p_adv_data
-----------------------------------------------------------------------------------------------------*/
static void ble_scan_result_callback(wiced_bt_ble_scan_results_t *p_scan_result, uint8_t *p_adv_data)
{
  uint8_t *ptr;
  uint8_t length;

  if (p_scan_result!= NULL)
  {
    if (observer_busy == 0)
    {
      last_scan_name[0] = 0;
      memcpy(&g_last_scan_result, p_scan_result, sizeof(wiced_bt_ble_scan_results_t));

      // Из пакета advertising извлекаем полное имя 
      ptr = wiced_bt_ble_check_advertising_data(p_adv_data, BTM_BLE_ADVERT_TYPE_NAME_COMPLETE,&length);
      if (ptr != 0)
      {
        if (length > BLE_DEV_STR_TYPE_LEN) length = BLE_DEV_STR_TYPE_LEN;
        memcpy(last_scan_name, ptr, length);
        last_scan_name[length]  = 0;
      }
      else
      {
        // Если не удалось извлечь полное имя извлекаем короткое имя 
        ptr = wiced_bt_ble_check_advertising_data(p_adv_data, BTM_BLE_ADVERT_TYPE_NAME_SHORT,&length);
        if (ptr != 0)
        {
          if (length > BLE_DEV_STR_TYPE_LEN) length = BLE_DEV_STR_TYPE_LEN;
          memcpy(last_scan_name, ptr, length);
          last_scan_name[length]  = 0;
        }
      }

      // Извлекаем полный список 16-битных UUIDs поддерживаемых сервисов 
      ptr = wiced_bt_ble_check_advertising_data(p_adv_data, BTM_BLE_ADVERT_TYPE_16SRV_PARTIAL,&length);
      if (ptr != 0)
      {
        last_scan_UUIDs_num = length / 2;
        if (last_scan_UUIDs_num > MAX_OSERVER_UUID_NUM) last_scan_UUIDs_num  = MAX_OSERVER_UUID_NUM;
        for (uint32_t i = 0; i < last_scan_UUIDs_num; i++)
        {
          memcpy(&last_scan_UUIDs[i],ptr + 2 * i,2);
        }
      }
      else
      {
        // Если не удалось извлечь полный список то извлекаем частичный список 16-битных UUIDs поддерживаемых сервисов 
        ptr = wiced_bt_ble_check_advertising_data(p_adv_data, BTM_BLE_ADVERT_TYPE_16SRV_COMPLETE,&length);
        if (ptr != 0)
        {
          last_scan_UUIDs_num = length / 2;
          if (last_scan_UUIDs_num > MAX_OSERVER_UUID_NUM) last_scan_UUIDs_num  = MAX_OSERVER_UUID_NUM;
          for (uint32_t i = 0; i < last_scan_UUIDs_num; i++)
          {
            memcpy(&last_scan_UUIDs[i],ptr + 2 * i,2);
          }
        }
        else
        {
          last_scan_UUIDs_num = 0;
        }
      }


      tx_event_flags_set(&bt_mon_flags, FLG_BT_OBSRV_RES_NEW, TX_OR); // Сообщаем о получении нового пакета advertising
    }
  }
  else
  {
    tx_event_flags_set(&bt_mon_flags, FLG_BT_OBSRV_COMPL, TX_OR); // Сообщаем о завершении сканирования 
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param uuid
  \param buf
  \param buf_len
-----------------------------------------------------------------------------------------------------*/
uint32_t _Read_attr_by_16bit_uuid(uint16_t uuid, uint8_t *buf, uint16_t buf_len)
{
  GET_MCBL;
  ULONG            actual_flags;
  wiced_bt_uuid_t  uuid_cbl;

  uuid_cbl.len = LEN_UUID_16;
  uuid_cbl.uu.uuid16 = uuid;

  // После вызова этой функции будет получено событие GATT_OPERATION_CPLT_EVT с идентификатором операции = GATTC_OPTYPE_READ_HANDLE
  wiced_bt_gatt_client_send_read_by_type(curr_conn_id, 0x0001, 0xFFFF,&uuid_cbl,buf, buf_len, GATT_AUTH_REQ_NONE);
  if (tx_event_flags_get(&bt_mon_flags, FLG_BT_GATT_OP_COMPLETED, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(10000)) == TX_SUCCESS)
  {
    if (curr_op_status != WICED_BT_GATT_SUCCESS)
    {
      MPRINTF(" UUID read %04X error= %04X",uuid, curr_op_status);
      return RES_ERROR;
    }
  }
  else
  {
    MPRINTF(" UUID %04X read timeout", uuid);
    return RES_ERROR;
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Accept_observer_result(uint32_t start_row)
{
  GET_MCBL;
  char          str[64];
  uint32_t      i;
  uint32_t      new_rec;
  uint32_t      row;

  observer_busy = 1;

  new_rec = 1;
  for (i = 0; i < MAX_OSERVER_RECS_NUM; i++)
  {
    if (orecs[i].active)
    {
      if (memcmp(orecs[i].addr, g_last_scan_result.remote_bd_addr, 6) == 0)
      {
        // Обнаружена уже существующая запись об этом адресе
        orecs[i].cnt++;
        orecs[i].addr_type = g_last_scan_result.ble_addr_type;
        orecs[i].flag      = g_last_scan_result.flag;
        orecs[i].rssi      = g_last_scan_result.rssi;
        orecs[i].tx_power  = g_last_scan_result.tx_power;
        if (last_scan_UUIDs_num != 0)
        {
          orecs[i].UUIDs_num = last_scan_UUIDs_num;
          memcpy(orecs[i].UUIDs, last_scan_UUIDs, sizeof(last_scan_UUIDs));
        }
        if (last_scan_name[0] != 0)
        {
          memcpy(orecs[i].device_name, last_scan_name, sizeof(last_scan_name));
        }
        new_rec = 0;
        break;
      }
    }
    else break;
  }

  if (new_rec == 1)
  {
    orecs[i].active = 1;
    memcpy(orecs[i].addr, g_last_scan_result.remote_bd_addr,6);
    orecs[i].addr_type = g_last_scan_result.ble_addr_type;
    orecs[i].flag      = g_last_scan_result.flag;
    orecs[i].rssi      = g_last_scan_result.rssi;
    orecs[i].tx_power  = g_last_scan_result.tx_power;
    orecs[i].UUIDs_num = last_scan_UUIDs_num;
    memcpy(orecs[i].UUIDs, last_scan_UUIDs, sizeof(last_scan_UUIDs));
    memcpy(orecs[i].device_name, last_scan_name, sizeof(last_scan_name));
  }

  row = start_row;

  // Выводим на дисплей все активные записи
  for (i = 0; i < MAX_OSERVER_RECS_NUM; i++)
  {
    VT100_set_cursor_pos(row,0);
    if (orecs[i].active)
    {
      MPRINTF(VT100_CLR_LINE"<%02d> Cnt=%06d, Type=%02X, Flag=%02X,  %s, RSSI=%04d",
              i,
              orecs[i].cnt,
              orecs[i].addr_type,
              orecs[i].flag,
              Buf_to_hex_str(str,6,orecs[i].addr),
              orecs[i].rssi);

      if (orecs[i].UUIDs_num != 0)
      {
        MPRINTF(" UUIDs = ");
        for (uint32_t n = 0; n < orecs[i].UUIDs_num; n++)
        {
          MPRINTF("%04X ",orecs[i].UUIDs[n]);
        }
      }
      MPRINTF(" %s",orecs[i].device_name);
    }
    else
    {
      MPRINTF(VT100_CLR_LINE);
    }
    row++;
  }

  observer_busy = 0;
}

/*-----------------------------------------------------------------------------------------------------
  Режим обсервера или пассивное сканирование эфира 
  Устройство слушает все advertising пакеты и сообщает о них в функции ble_scan_result_callback 

  \param void

  \return void _Print_connection
-----------------------------------------------------------------------------------------------------*/
static void _Passive_scaning_mode(void)
{
  GET_MCBL;
  ULONG         actual_flags;
  uint8_t       b;
  uint32_t      row;

  memset(orecs, 0, sizeof(orecs));

  row = START_ROW;
  VT100_set_cursor_pos(row,0);
  MPRINTF(VT100_CLR_FM_CRSR); // Очищаем экран от текущей строки
  MPRINTF("BLE Device list. Press <H> to filter out Heart Rate sensors.");


  // Стартуем сканирование
  wiced_bt_ble_observe(WICED_TRUE, 0, ble_scan_result_callback);
  do
  {
    if (tx_event_flags_get(&bt_mon_flags, 0xFFFFFFFF, TX_OR_CLEAR,&actual_flags, 10) == TX_SUCCESS)
    {
      if (actual_flags & FLG_BT_OBSRV_RES_NEW) 
      {
        // Здесь обрабатываем результат каждого полученного advertising пакета при сканировании
        // Чтобы не нагружать задачу приемника протокола Bluetooth HCI 
        _Accept_observer_result(row + 2); 
      }
      if (actual_flags & FLG_BT_OBSRV_COMPL)
      {
        break;
      }
    }
    if (WAIT_CHAR(&b, 0) == RES_OK)
    {
      // Останавливаем сканирование
      wiced_bt_ble_observe(WICED_FALSE, 0, ble_scan_result_callback);
    }
  } while (1);

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Print_identity_keys(void)
{
  GET_MCBL;
  char          str[64];
  uint8_t       b;
  uint32_t      row = START_ROW;

  VT100_set_cursor_pos(row,0);
  MPRINTF(VT100_CLR_FM_CRSR); // Очищаем экран от текущей строки
  MPRINTF("\n\r\n\r--- Identity_keys ----\n\r");
  MPRINTF("Key type mask               : %02X\n\r",bt_nv.identity_keys.key.key_type_mask);
  MPRINTF("Identity Root Key (IR)      : %s\n\r",Buf_to_hex_str(str,16, bt_nv.identity_keys.key.id_keys.ir));
  MPRINTF("Identity Resolving Key (IRK): %s\n\r",Buf_to_hex_str(str,16, bt_nv.identity_keys.key.id_keys.irk));
  MPRINTF("Diversifying Key (DHK)      : %s\n\r",Buf_to_hex_str(str,16, bt_nv.identity_keys.key.id_keys.dhk));
  MPRINTF("LE encryption key (ER)      : %s\n\r",Buf_to_hex_str(str,16, bt_nv.identity_keys.key.er));
  WAIT_CHAR(&b, 1000000);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Print_pairing_keys(void)
{
  GET_MCBL;
  char          str[64];
  uint8_t       b;
  uint32_t      row = START_ROW;

  VT100_set_cursor_pos(row,0);
  MPRINTF(VT100_CLR_FM_CRSR); // Очищаем экран от текущей строки
  MPRINTF("\n\r\n\r--- Pairing_keys ----\n\r");
  for (uint32_t i = 0; i < PARING_KEYS_NUM; i++)
  {
    MPRINTF("\n\rPairing  %d\n\r",i);
    MPRINTF("Address type            : %s, mask: %02X\n\r",To_Str_Addr_type(bt_nv.paring_keys.link_keys[i].addr_type), bt_nv.paring_keys.link_keys[i].key_mask);
    MPRINTF("BD Address              : %s\n\r",Buf_to_hex_str(str,6, bt_nv.paring_keys.link_keys[i].key.bd_addr));
    MPRINTF("Connecton Address       : %s\n\r",Buf_to_hex_str(str,6, bt_nv.paring_keys.link_keys[i].key.conn_addr));
    MPRINTF("BLE address type        : %s\n\r",To_Str_Addr_type(bt_nv.paring_keys.link_keys[i].key.key_data.ble_addr_type));
    MPRINTF("LE keys available mask  : %02X\n\r",bt_nv.paring_keys.link_keys[i].key.key_data.le_keys_available_mask);
    MPRINTF("BR EDR key type         : %02X\n\r",bt_nv.paring_keys.link_keys[i].key.key_data.br_edr_key_type);
    MPRINTF("BR EDR key              : %s\n\r",Buf_to_hex_str(str,16, bt_nv.paring_keys.link_keys[i].key.key_data.br_edr_key));
    MPRINTF("BLE address type        : %s\n\r",To_Str_Addr_type(bt_nv.paring_keys.link_keys[i].key.key_data.ble_addr_type));
    MPRINTF("BLE keys data:\n\r");
    MPRINTF("  Identity Resolving Key (IRK)                    : %s\n\r", Buf_to_hex_str(str,16, bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.irk));
    MPRINTF("  Peer long term key (PLTK)                       : %s\n\r", Buf_to_hex_str(str,16, bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.pltk));
    MPRINTF("  Peer Connection Signature Resolving Key (PCSRK) : %s\n\r", Buf_to_hex_str(str,16, bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.pcsrk));
    MPRINTF("  Local long term key (LLTK)                      : %s\n\r", Buf_to_hex_str(str,16, bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.lltk));
    MPRINTF("  Local Connection Signature Resolving Key (LCSRK): %s\n\r", Buf_to_hex_str(str,16, bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.lcsrk));
    MPRINTF("  Random vector for LTK generation (RAND)         : %s\n\r", Buf_to_hex_str(str,16, bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.rand));
    MPRINTF("  Encrypted LTK Diversifier (EDIV)                : %04X\n\r", bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.ediv);
    MPRINTF("  Local Div. to generate local LTK and CSRK (DIV) : %04X\n\r", bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.div);
    MPRINTF("  Local pairing sec.level : %02X\n\r", bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.sec_level);
    MPRINTF("  Key size                : %d\n\r",   bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.key_size);
    MPRINTF("  SRK sec. level          : %02X\n\r", bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.srk_sec_level);
    MPRINTF("  Local CSRK sec. level   : %02X\n\r", bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.local_csrk_sec_level);
    MPRINTF("  Counter                 : %d\n\r",   bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.counter);
    MPRINTF("  Local_counter           : %d\n\r",   bt_nv.paring_keys.link_keys[i].key.key_data.le_keys.local_counter);
  }
  WAIT_CHAR(&b, 1000000);

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Print_connections(void)
{
  GET_MCBL;
  char                     str[64];
  uint8_t                  b;
  ULONG                    actual_flags;
  uint32_t                 row = START_ROW;
  T_gatt_connection_cbl   *cons;
  T_bt_dev_pairing_info   *pinf;

  VT100_set_cursor_pos(row,0);
  MPRINTF(VT100_CLR_FM_CRSR); // Очищаем экран от текущей строки

  do
  {
    VT100_set_cursor_pos(row,0);

    MPRINTF("\n\r\n\r----------------------------- Last connections --------------------------------\n\r");
    for (uint32_t i = 0; i < MAX_BLE_CONNECTION_NUMBER; i++)
    {
      cons = Get_gatt_connection_status(i);
      if (cons->active)
      {
        wiced_bt_dev_get_remote_name(cons->addr,_Accept_remote_name);
        tx_event_flags_get(&bt_mon_flags, FLG_BT_NAME_COMPL, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(5000));

        wiced_bt_dev_read_rssi(cons->addr, cons->status.transport, _Accept_RSSI_callback);
        tx_event_flags_get(&bt_mon_flags, FLG_BT_RSSI_COMPL, TX_OR_CLEAR,&actual_flags, MS_TO_TICKS(5000));

        MPRINTF(VT100_CLR_LINE"Connection ID           : %d  \n\r",  cons->status.conn_id);
        MPRINTF(VT100_CLR_LINE"Connected device address: %s. Type: %s. Link Role: %s, Remote name : %s\n\r",  Buf_to_hex_str(str,6,cons->addr), To_Str_Addr_type(cons->status.addr_type), To_Str_Link_Role(cons->status.link_role), remote_name);
        MPRINTF(VT100_CLR_LINE"RSSI                    : %d dB, status: %d, HCI status: %d, Bluetooth transport : %s\n\r\n\r",rssi_info.rssi, rssi_info.status, rssi_info.hci_status, To_Str_Bluetooth_transport(cons->status.transport));
      }
      else
      {
        MPRINTF(VT100_CLR_LINE"Connection ID           : ---\n\r");
        MPRINTF(VT100_CLR_LINE"Connected device address: ---\n\r");
        MPRINTF(VT100_CLR_LINE"RSSI                    : ---\n\r\n\r");
      }
    }

    MPRINTF(VT100_CLR_LINE"\n\r----------------------------- Last pairing -----------------------------------\n\r");

    pinf = BLE_get_last_pairing_info();
    MPRINTF(VT100_CLR_LINE"Paired peer  address    : %s\n\r", Buf_to_hex_str(str,6,pinf->addr));
    MPRINTF(VT100_CLR_LINE"Bluetooth transport     : %s\n\r",To_Str_Bluetooth_transport(pinf->info.transport));
    MPRINTF(VT100_CLR_LINE"Bluetooth EDR status    : %d\n\r",pinf->info.pairing_complete_info.br_edr.status);
    MPRINTF(VT100_CLR_LINE"Bluetooth LE status     : %04X\n\r",pinf->info.pairing_complete_info.ble.status);
    MPRINTF(VT100_CLR_LINE"Bluetooth LE resolv.addr: %s Type: %s\n\r",Buf_to_hex_str(str,6,pinf->info.pairing_complete_info.ble.resolved_bd_addr), To_Str_Addr_type(pinf->info.pairing_complete_info.ble.resolved_bd_addr_type));
    MPRINTF(VT100_CLR_LINE"Bluetooth LE sec.level  : %s\n\r",To_Str_BLE_sec_level(pinf->info.pairing_complete_info.ble.sec_level));
    MPRINTF(VT100_CLR_LINE"Bluetooth LE is pair canceled : %s  Failure reason: %s\n\r",To_Str_Yes_No(pinf->info.pairing_complete_info.ble.is_pair_cancel), To_Str_SMP_status(pinf->info.pairing_complete_info.ble.reason));

    if (WAIT_CHAR(&b, 100) == RES_OK) return;

  }while (1);

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Print_header(void)
{
  GET_MCBL;
  char            str[64];

  wiced_bt_dev_local_addr_ext_t       ext_data;


  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  Bluetooth monitor ===\n\r");
  MPRINTF("ESC - exit,  [L] - Print connections. [K]- Print identity keys, [B] - Print clients bindings, [O] - Observer mode, [D] - Erase bindings data\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");

  wiced_bt_dev_read_local_addr_ext(&ext_data);


  // Здесь наблюдаем подключения по BLE
  MPRINTF("Device local    address: %s\n\r", Buf_to_hex_str(str,6,ext_data.local_addr));
  MPRINTF("Device private  address: %s. Type:%s\n\r", Buf_to_hex_str(str,6,ext_data.private_addr), To_Str_Addr_type(ext_data.private_addr_type));
  MPRINTF("LE Privacy is enabled  : %s.  Static random address is used : %s\n\r", To_Str_Yes_No(ext_data.is_privacy_enabled), To_Str_Yes_No(ext_data.is_static_rand_addr_used));


}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_Bluetooth_control(uint8_t keycode)
{
  GET_MCBL;
  uint8_t                    b;

  if (BLE_is_initialised() == 0)
  {
    MPRINTF("Bluetooth stack not initialised. Abort!\n\r");
    Wait_ms(1000);
    return;
  }
  tx_event_flags_create(&bt_mon_flags, "bt_mon");
  _Print_header();

  do
  {
    if (WAIT_CHAR(&b, 500) == RES_OK)
    {
      switch (b)
      {
      case 'L':
      case 'l':
        _Print_connections();
        break;
      case 'K':
      case 'k':
        _Print_identity_keys();
        break;
      case 'B':
      case 'b':
        _Print_pairing_keys();
        break;
      case 'D':
      case 'd':
        memset(&bt_nv.paring_keys, 0, sizeof(bt_nv.paring_keys));
        bt_nv.oldest_paring_keys_indx = 0;
        Request_save_bt_nv_data();
        break;
      case 'O':
      case 'o':
        _Passive_scaning_mode();
        break;

      case VT100_ESC:
        tx_event_flags_delete(&bt_mon_flags);
        return;
      }
    }
    else
    {
      _Print_header();
    }

  }while (1);


}


