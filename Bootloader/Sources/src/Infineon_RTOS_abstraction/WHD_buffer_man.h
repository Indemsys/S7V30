#ifndef WHD_BUFFER_MAN_H
  #define WHD_BUFFER_MAN_H


whd_result_t WHD_host_buffer_get(whd_buffer_t *buffer, whd_buffer_dir_t direction, uint16_t size, uint32_t timeout_ms);
void         WHD_buffer_release(whd_buffer_t buffer, whd_buffer_dir_t direction);
uint8_t*     WHD_buffer_get_current_piece_data_pointer(whd_buffer_t buffer);
uint16_t     WHD_buffer_get_current_piece_size(whd_buffer_t buffer);
whd_result_t WHD_buffer_set_size(whd_buffer_t buffer, uint16_t size);
whd_result_t WHD_buffer_add_remove_at_front(whd_buffer_t *buffer, int32_t add_remove_amount);



#endif // WHD_BUFFER_MAN_H



