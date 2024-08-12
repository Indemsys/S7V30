#ifndef BLE_CONFIG_H
  #define BLE_CONFIG_H


#define HCI_UART_DEFAULT_BAUDRATE            (115200) // Скорость на которой работает интерфейс HCI на старте
#define CYBSP_BT_PLATFORM_CFG_BAUD_DOWNLOAD (4000000) // Скорость на которую переводится интерфейс BLE при загрузке фирмваре
#define CYBSP_BT_PLATFORM_CFG_BAUD_FEATURE  (4000000) // Скорость на которую переводится интерфейс BLE после загрузки фирмваре


//#define ENABLE_BLE_STACK_TRACE  // Разрешение получения хостом трассировочных сообщений BTHOST стека
//#define ENABLE_BLE_RTT_LOG      // Разрешение вывода в отладочный RTT интерфейс сообщений из слоя портирования
//#define ENABLE_BLE_LOG          // Разрешение записи сообщений от стека BLE в общий лог приложения


#ifdef ENABLE_BLE_LOG
#define BLELOG   APPLOG
#else
#define BLELOG(...)
#endif

#endif



