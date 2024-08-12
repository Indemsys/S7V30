#ifndef THREADS_PRIORITIES_H
  #define THREADS_PRIORITIES_H

// Приоритеты задач
// Чем меньше значение тем выше приоритет
// Дожно быть не более 32 уровней приоритетов
// Если нужно больше, то следует переопределить макрос TX_MAX_PRIORITIES, но замедлится переключение контекста RTOS

  #define     THREAD_HIGHER_PRIO               5
  #define     MAIN_THREAD_PRIO                 5

  #define     MAIN_TASK_PRIO                   6
  #define     CAN_TASK_PRIO                    7
  #define     PLAYER_TASK_PRIO                 8


  #define     HCI_RX_TASK_PRIORITY             9   // Приоритет задачи приемника Bluetooth
  #define     HCI_TX_TASK_PRIORITY             10  // Приоритет задачи передатчика Bluetooth
  #define     WHD_TASK_PRIO                    11  // Приоритет задачи WiFi драйвера занимающейся приемом пакетов
  #define     UX_THREAD_PRIORITY_HCD           12
  #define     UX_THREAD_PRIORITY_DCD           12
  #define     UX_THREAD_PRIORITY_CLASS         12  // Приоритет классов USB устройств
  #define     UX_THREAD_PRIORITY_ENUM          12
  #define     IP_INTERFACE_PRIORITY            13
  #define     THREAD_BSD_PRIORITY              13
  #define     FREEMASTER_TASK_PRIO             14
  #define     MATLAB_TASK_PRIO                 14
  #define     LOGGER_TASK_PRIO                 14
  #define     THREAD_NET_MAN_PRIORITY          15  // Приоритет задачи менеджера сетевых интерфейсов
  #define     VT100_MANAGER_TASK_PRIO          16  // Приоритет задачи создающей и удаляющей сессии терминала VT100. А такдже приорите задач приемников данных для VT100
  #define     VT100_TASK_PRIO                  16
  #define     NX_WEB_HTTP_SERVER_PRIORITY      16
  #define     NX_FTP_SERVER_PRIORITY           17
  #define     NX_TELNET_SERVER_PRIORITY        17
  #define     NX_DHCP_THREAD_PRIORITY          18  // Приоритет клиентов DHCP
  #define     NX_DHCP_SERVER_THREAD_PRIORITY   18  // Приоритет серверов DHCP
  #define     MDNS_PRIORITY                    19
  #define     NX_SNTP_CLIENT_THREAD_PRIORITY   19  // Приоритет клиента сервиса времени
  #define     MQTT_TASK_PRIO                   19
  #define     THREAD_WIFI_MAN_PRIORITY         20  // Приоритет задачи управления модулем WiFi
  #define     BLE_INIT_TASK_PRIO               20
  #define     CHARGER_TASK_PRIO                21
  #define     BACKGROUND_TASK_PRIO             22  // Низший приоритет для процесса IDLE измеряющего нагрузку процессора (следить за приоритетами сетевых задач!)


#endif

