#ifndef MONITOR_BLUETOOTH_DRV_H
  #define MONITOR_BLUETOOTH_DRV_H


#define   BLUETOOTH_VT100_DRV_STR_SZ                512
#define   BLUETOOTH_VT100_DRV_RECV_THREAD_STACK_SZ  1024
#define   BLUETOOTH_VT100_DRV_BUF_QUANTITY          4           // Количество приемных буферов
#define   BLUETOOTH_VT100_DRV_BUFFER_MAX_LENGTH     256



// Ведем прием циклически в N приемных буферов
typedef struct
{
    uint32_t  actual_len;  // Длина пакета
    uint8_t   buff[BLUETOOTH_VT100_DRV_BUFFER_MAX_LENGTH]; // Буфер с пакетов

} T_bluetooth_drv_rx_cbl;


typedef struct
{
    volatile uint8_t       taken;
    int32_t                vt100_task_instance_index;
    char                   str[BLUETOOTH_VT100_DRV_STR_SZ];
    void                   *spp_ptr;
    TX_EVENT_FLAGS_GROUP   *event_flags_ptr;    // Группа флагов для взаимодействия с задачей приема

    T_serial_io_driver     serial_io_driver;
    volatile int32_t       head_buf_indx;       //  Индекс головы циклической очереди буферов приема
    volatile int32_t       tail_buf_indx;       //  Индекс хвоста циклической очереди буферов приема
    volatile int32_t       all_buffers_full;    //  Сигнал о том что все приемные буферы заняты принятыми данными
    int32_t                tail_buf_rd_pos;     //  Позиция чтения в хвостовом буфере
    T_bluetooth_drv_rx_cbl in_bufs_ring[BLUETOOTH_VT100_DRV_BUF_QUANTITY]; //  Кольцо управляющих структур приема-обработки входящих пакетов


} T_bluetooth_serial_drv_cbl;

VOID Monitor_Bluetooth_Init_callback(VOID *arg);
VOID Monitor_Bluetooth_DeInit_callback(VOID *arg);

#endif



