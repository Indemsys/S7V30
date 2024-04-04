#ifndef NET_WEB_SERVER_H
  #define NET_WEB_SERVER_H

#define ENABLE_WEB_LOG

#ifdef ENABLE_WEB_LOG
  #define HTTPLOG  APPLOG
#else
  #define HTTPLOG(...)
#endif


#define  AUTH_USER_NAME        "user"
#define  REALM_NAME            "IoT_realm"
#define  WEB_FILES_DIR_NAME    "www"
#define  SETTINGS_URL          "/data.json"
#define  POST_DEV_STATUS       "/post_status"
#define  POST_CARD_CSD         "/post_sd_csd"
#define  POST_CARD_STATE       "/post_sd_state"
#define  POST_CARD_CONTROL     "/post_sd_card_control"
#define  POST_UPLOAD_TIME      "/upload_time"
#define  POST_FIRMWARE_UPLOAD  "/firmware_upload"
#define  DEV_LOG_URL           "/log.txt"
#define  RESET_CMD_URL         "/reset"
#define  RESET_LOG_CMD_URL     "/reset_log"
#define  INDEX_URL             "/"
#define  MAX_PUT_DATA_SIZE   1024*30

void     HTTP_server_controller(void);
uint32_t HTTP_send_memory_buffer(NX_WEB_HTTP_SERVER *server_ptr, char *data_buf, uint32_t data_buf_size, UINT request_type, char *mime_type_str);

#endif // NET_WEB_SERVER_H



