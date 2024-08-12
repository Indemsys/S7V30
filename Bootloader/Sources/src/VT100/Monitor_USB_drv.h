#ifndef MONITOR_USB_DRV_H
  #define MONITOR_USB_DRV_H

#define   USB_VT100_DRV_INSTANCES_MAX_NUM     2

#define   USB_VT100_DRV_STR_SZ                512
#define   USB_VT100_DRV_RECV_THREAD_STACK_SZ  1024
#define   USB_VT100_DRV_BUF_QUANTITY          2           // Количество приемных буферов
#define   USB_VT100_DRV_BUFFER_MAX_LENGTH     512



// Ведем прием циклически в N приемных буферов
typedef struct
{
    uint32_t  actual_len;  // Длина пакета
    uint8_t   buff[USB_VT100_DRV_BUFFER_MAX_LENGTH]; // Буфер с пакетов

} T_usb_drv_rx_cbl;


typedef struct
{
    volatile uint8_t       taken;
    int32_t                vt100_task_instance_index;
    UX_SLAVE_CLASS_CDC_ACM *cdc;
    char                   str[USB_VT100_DRV_STR_SZ];
    char                   recv_task_name[VT100_TASK_NAME_MAX_SZ];
    TX_THREAD              *recv_thread_ptr; // Задача прием
    uint8_t                *recv_thread_stack;
    TX_EVENT_FLAGS_GROUP   *event_flags_ptr;         // Группа флагов для взаимодействия с задачей приема
    char                   event_flags_name[VT100_TASK_NAME_MAX_SZ];
    void                  *dbuf;        // Указатель на буфер с принимаемыми данными
    uint32_t               dsz;         // Количество принимаемых байт

    T_serial_io_driver     serial_io_driver;
    volatile int32_t       head_buf_indx;       //  Индекс головы циклической очереди буферов приема
    volatile int32_t       tail_buf_indx;       //  Индекс хвоста циклической очереди буферов приема
    volatile int32_t       all_buffers_full;    //  Сигнал о том что все приемные буферы заняты принятыми данными
    int32_t                tail_buf_rd_pos;     //  Позиция чтения в хвостовом буфере
    T_usb_drv_rx_cbl       in_bufs_ring[USB_VT100_DRV_BUF_QUANTITY]; // Кольцо управляющих структур приема-обработки входящих пакетов


} T_usb_serial_drv_cbl;

VOID Monitor_USB_Init_callback(VOID *cdc_instance);
VOID Monitor_USB_DeInit_callback(VOID *cdc_instance);


#endif



