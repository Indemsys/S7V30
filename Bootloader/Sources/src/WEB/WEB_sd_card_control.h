#ifndef WEB_SD_CARD_CONTROL_H
  #define WEB_SD_CARD_CONTROL_H

uint32_t HTTP_POST_send_card_csd(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_send_card_state(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);
uint32_t HTTP_POST_card_control(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);


#endif



