#ifndef NET_WEB_SERVER_H
  #define NET_WEB_SERVER_H

#define ENABLE_WEB_LOG

#ifdef ENABLE_WEB_LOG
  #define HTTPLOG  NET_LOG
#else
  #define HTTPLOG(...)
#endif

typedef uint32_t (*T_post_handler)(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr);


typedef struct
{
  const char      *url;
  T_post_handler  handler_func;

} T_post_handler_rec;

#define  AUTH_USER_NAME              "user"
#define  REALM_NAME                  "IoT_realm"
#define  WEB_FILES_DIR_NAME          "www"

#define  GET_GEN_LOG_FILE            "/log"
#define  GET_GEN_NETLOG_FILE         "/netlog"

#define  POST_SAVE_MODULE_PARAMS     "/post_save_module_params"
#define  POST_GET_MODULE_PARAMS      "/post_get_module_params"
#define  POST_DEVICE_INFO            "/post_device_info"
#define  POST_DEV_STATUS             "/post_status"
#define  POST_CARD_CSD               "/post_sd_csd"
#define  POST_CARD_STATE             "/post_sd_state"
#define  POST_CARD_CONTROL           "/post_sd_card_control"
#define  POST_UPLOAD_TIME            "/upload_time"
#define  POST_FIRMWARE_UPLOAD        "/firmware_upload"
#define  POST_MODULE_PRAMS           "/post_module_params"
#define  POST_RESET_DEVICE           "/reset"
#define  POST_CLEAR_GEN_LOG_FILE     "/clear_log"
#define  POST_CLEAR_NET_LOG_FILE     "/clear_netlog"
#define  POST_START_WIFI_SCAN        "/start_wifi_scan"
#define  POST_WIFI_SCAN_RESULTS      "/get_wifi_scan_results"
#define  INDEX_URL                   "/"

#define  MAX_PUT_DATA_SIZE   1024*30

#include   "WEB_files.h"
#include   "WEB_File_upload.h"
#include   "WEB_sd_card_control.h"
#include   "WEB_module_control.h"

extern  UINT _nx_web_http_server_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code, UINT status_code_length, CHAR *information, UINT information_length, CHAR *additional_information, UINT additional_information_length);
extern  UINT _nx_web_http_server_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, CHAR *status_code, UINT status_code_length, UINT content_length, CHAR *content_type, UINT content_type_length, CHAR *additional_header, UINT additional_header_length);
extern  UINT _nx_web_http_server_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
extern  UINT _nx_web_http_server_response_packet_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, UINT wait_option);
extern  UINT _nx_web_http_server_type_get_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length, CHAR *http_type_string, UINT http_type_string_max_size, UINT *string_size);


uint32_t HTTP_send_data(NX_WEB_HTTP_SERVER *server_ptr, char *data_buf, uint32_t data_buf_size, UINT request_type, char *mime_type_str);
uint32_t  Net_HTTP_server_create(NX_IP *ip_ptr);

#endif // NET_WEB_SERVER_H



