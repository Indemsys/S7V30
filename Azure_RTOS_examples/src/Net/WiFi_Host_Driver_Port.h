#ifndef WIFI_HOST_DRIVER_PORT_H
  #define WIFI_HOST_DRIVER_PORT_H


uint32_t     WHD_resource_size(whd_driver_t whd_drv, whd_resource_type_t resource, uint32_t *size_out);
uint32_t     WHD_get_resource_block(whd_driver_t whd_drv, whd_resource_type_t type, uint32_t blockno, const uint8_t **data, uint32_t *size_out);
uint32_t     WHD_get_resource_no_of_blocks(whd_driver_t whd_drv, whd_resource_type_t type, uint32_t *block_count);
uint32_t     WHD_get_resource_block_size(whd_driver_t whd_drv, whd_resource_type_t type, uint32_t *size_out);
uint32_t     WHD_resource_read(whd_driver_t whd_drv, whd_resource_type_t type, uint32_t offset, uint32_t size, uint32_t *size_out, void *buffer);
whd_result_t WHD_host_buffer_get(whd_buffer_t *buffer, whd_buffer_dir_t direction, uint16_t size, uint32_t timeout_ms);
void         WHD_buffer_release(whd_buffer_t buffer, whd_buffer_dir_t direction);
uint8_t*     WHD_buffer_get_current_piece_data_pointer(whd_buffer_t buffer);
uint16_t     WHD_buffer_get_current_piece_size(whd_buffer_t buffer);
whd_result_t WHD_buffer_set_size(whd_buffer_t buffer, uint16_t size);
whd_result_t WHD_buffer_add_remove_at_front(whd_buffer_t *buffer, int32_t add_remove_amount);
void         WHD_RTT_LOGs(const char *fmt_ptr, ...);

#endif



