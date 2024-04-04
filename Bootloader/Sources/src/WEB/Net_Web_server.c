// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.09.22
// 11:50:10
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "WEB_server_certificate.h"






extern const NX_SECURE_TLS_CRYPTO   nx_crypto_tls_ciphers;
extern const NX_SECURE_TLS_CRYPTO   nx_crypto_tls_ciphers_synergys7;

#ifdef USE_HARDWARE_CRIPTO_ENGINE
  #define CRIPTO_TLS_CYPHERS  nx_crypto_tls_ciphers_synergys7
#else
  #define CRIPTO_TLS_CYPHERS  nx_crypto_tls_ciphers
#endif

static NX_IP              *http_ip_ptr;
NX_WEB_HTTP_SERVER         http_server;

#define    WEB_SERVER_STACK_SIZE     (2048*2)


static   uint8_t   server_stack[WEB_SERVER_STACK_SIZE];


static NX_SECURE_X509_CERT  certificate @ ".sdram";
static CHAR                 crypto_metadata_server[20000 * NX_WEB_HTTP_SERVER_SESSION_MAX] @ ".sdram";
static UCHAR                tls_packet_buffer[40000] @ ".sdram";


#define               RESPONSE_MAX_SIZE 512
static CHAR           response_buf[RESPONSE_MAX_SIZE];
static uint8_t        web_first_call;
static FX_LOCAL_PATH  web_local_path;



extern  UINT _nx_web_http_server_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code, UINT status_code_length, CHAR *information, UINT information_length, CHAR *additional_information, UINT additional_information_length);
extern  UINT _nx_web_http_server_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, CHAR *status_code, UINT status_code_length, UINT content_length, CHAR *content_type, UINT content_type_length, CHAR *additional_header, UINT additional_header_length);
extern  UINT _nx_web_http_server_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
extern  UINT _nx_web_http_server_response_packet_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, UINT wait_option);
extern  UINT _nx_web_http_server_type_get_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length, CHAR *http_type_string, UINT http_type_string_max_size, UINT *string_size);


static uint32_t HTTP_serv_send_file(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, char *file_name);
/*-----------------------------------------------------------------------------------------------------
   Define the application's authentication check.  This is called by the HTTP server whenever a new request is received.

  \param server_ptr
  \param request_type
  \param resource
  \param name
  \param password
  \param realm

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  _HTTP_authentication_check(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm)
{

  // Just use a simple name, password, and realm for all requests and resources.
  *name =     AUTH_USER_NAME;
  *password = (CHAR *)ivar.HTTP_server_password;
  *realm =    REALM_NAME;

  /* Request basic authentication.  */
  return (NX_WEB_HTTP_BASIC_AUTHENTICATE);
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param data_buf
  \param data_buf_size
  \param request_type
  \param mime_type_str

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t HTTP_send_memory_buffer(NX_WEB_HTTP_SERVER *server_ptr, char *data_buf, uint32_t data_buf_size, UINT request_type, char *mime_type_str)
{
  uint32_t       status;
  char          *ptr;
  NX_PACKET     *new_packet_ptr;
  ULONG          block_sz;
  NX_TCP_SOCKET *socket_ptr = &server_ptr->nx_web_http_server_current_session_ptr->nx_tcp_session_socket;


  // Формируем заголовок отправляемого файла
  status = _nx_web_http_server_generate_response_header(
                                                        server_ptr,                         //  Pointer to HTTP server
                                                        &new_packet_ptr,                    //  Pointer to packet
                                                        NX_WEB_HTTP_STATUS_OK,              //  Status-code and reason-phrase
                                                        sizeof(NX_WEB_HTTP_STATUS_OK) - 1,   //  Length of status-code
                                                        data_buf_size,                      //  Length of content
                                                        mime_type_str,                      //  Type of content
                                                        strlen(mime_type_str),              //  Length of content type
                                                        NX_NULL,                            //  Other HTTP headers
                                                        0);
  if (status != NX_SUCCESS)
  {
    _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                      sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                      "NetX HTTP Request Aborted",
                                      sizeof("NetX HTTP Request Aborted") - 1, NX_NULL, 0);

    return RES_ERROR;
  }

  if (request_type == NX_WEB_HTTP_SERVER_HEAD_REQUEST)
  {
    status = _nx_web_http_server_send(server_ptr, new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);
    if (status != NX_SUCCESS)
    {
      nx_packet_release(new_packet_ptr);
      return RES_ERROR;
    }
    return RES_OK;
  }

  // Получаем длину пакета
  block_sz = new_packet_ptr->nx_packet_length;
  ptr = data_buf;

  while (data_buf_size)
  {
    // Определим нужно ли выделять новый пакет
    if (block_sz == 0)
    {
      status =  _nx_web_http_server_response_packet_allocate(server_ptr,&new_packet_ptr, NX_WAIT_FOREVER);
      if (status != NX_SUCCESS)
      {
        server_ptr->nx_web_http_server_allocation_errors++; // Считаем ошибки
        break;
      }
    }

    // Вычисляем свободную длину пакета
    block_sz = ((ULONG)(new_packet_ptr->nx_packet_data_end - new_packet_ptr->nx_packet_append_ptr)) - NX_PHYSICAL_TRAILER;

    // Корректируем длину чтобы не превысить допустимое значение
    if (block_sz > socket_ptr->nx_tcp_socket_connect_mss) block_sz =  socket_ptr->nx_tcp_socket_connect_mss;

    if (block_sz > data_buf_size)  block_sz = data_buf_size;

    // Читаем данные в пакет
    memcpy(new_packet_ptr->nx_packet_append_ptr, ptr, block_sz);
    ptr += block_sz;

    // Обновим информацию пакета
    new_packet_ptr->nx_packet_length =  new_packet_ptr->nx_packet_length + block_sz;
    new_packet_ptr->nx_packet_append_ptr =  new_packet_ptr->nx_packet_append_ptr + block_sz;

    // Вышлем пакет
    status =  _nx_web_http_server_send(server_ptr, new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

    if (status != NX_SUCCESS)
    {
      nx_packet_release(new_packet_ptr);
      return RES_ERROR;
    }

    // Считаем отправленные байты
    server_ptr->nx_web_http_server_total_bytes_sent =  server_ptr->nx_web_http_server_total_bytes_sent + block_sz;

    // Вычислим сколько еще осталось отправить
    data_buf_size =  data_buf_size - block_sz;

    block_sz = 0; // Отметим необходимость в выделении нового пакета
  }
  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param request_type

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_send_device_settings(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, char *url)
{
  uint32_t       status;
  char          *json_str = NULL;
  uint32_t       json_str_sz;

  if (Serialze_settings_to_mem(&ivar_inst,&json_str,&json_str_sz, JSON_INDENT(1) | JSON_ENSURE_ASCII) != RES_OK)
  {
    // Возвращаем ответ о неудаче если файл не открылся
    _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_NOT_FOUND,
                                      sizeof(NX_WEB_HTTP_STATUS_NOT_FOUND) - 1,
                                      "NetX HTTP Server unable to find file: ",
                                      sizeof("NetX HTTP Server unable to find file: ") - 1,
                                      url,
                                      strlen(url));
    return RES_ERROR;
  }

  status = HTTP_send_memory_buffer(server_ptr,json_str, json_str_sz,request_type,"application/json");

  App_free(json_str);
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param request_type

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_send_device_status(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, char *url)
{
  uint32_t       status;
  uint32_t       sz = 0;
  char           str[128];
  char           str1[128];
  char          *out_buffer;
  rtc_time_t     curr_time;


  // Выделяем память для буфера с данными
  out_buffer = App_malloc_pending(1024,10);
  if (out_buffer == NULL) return RES_ERROR;

  strcpy(&out_buffer[sz], "{");
  sz++;

  sprintf(str, "\"cpu_usage\":%d,", g_aver_cpu_usage);
  strcpy(&out_buffer[sz], str);
  sz += strlen(str);

  rtc_cbl.p_api->calendarTimeGet(rtc_cbl.p_ctrl,&curr_time);
  curr_time.tm_mon++;

  sprintf(str, "\"local_time\":\"%04d.%02d.%02d  %02d:%02d:%02d\",", curr_time.tm_year + 1900, curr_time.tm_mon, curr_time.tm_mday, curr_time.tm_hour, curr_time.tm_min, curr_time.tm_sec);
  strcpy(&out_buffer[sz], str);
  sz += strlen(str);

  if (Get_device_ip_addr(str1, 48) == RES_OK)
  {
    sprintf(str, "\"ip_address\":\"%s\",",str1);
    strcpy(&out_buffer[sz], str);
    sz += strlen(str);

    if (Get_device_MAC_addr(str1, 48) == RES_OK)
    {
      sprintf(str, "\"mac_address\":\"%s\",",str1);
      strcpy(&out_buffer[sz], str);
      sz += strlen(str);
    }
    if (Get_connected_SSID(str1, 48) == RES_OK)
    {
      sprintf(str, "\"ssid\":\"%s\",",str1);
      strcpy(&out_buffer[sz], str);
      sz += strlen(str);
    }
  }

  // Последняя запись в JSON блоке. Запятая не ставится
  sprintf(str, "\"up_time\":%d", _tx_time_get() / TX_TIMER_TICKS_PER_SECOND);
  strcpy(&out_buffer[sz], str);
  sz += strlen(str);

  strcpy(&out_buffer[sz], "}");
  sz++;
  status = HTTP_send_memory_buffer(server_ptr,out_buffer, sz,request_type,"application/json");

  App_free(out_buffer);
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_send_card_csd(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, char *url)
{
  uint32_t       status;
  uint32_t       sz = 0;

  // Выделяем память для буфера с данными
  char *out_buffer = App_malloc_pending(2048,10);
  if (out_buffer == NULL) return RES_ERROR;

  sdmmc_priv_csd_reg_t *p_csd_reg = s7_Get_csd_reg();
  sz = Get_card_csd_text(p_csd_reg, out_buffer, 2048);

  status = HTTP_send_memory_buffer(server_ptr,out_buffer, sz,request_type,"application/text");

  App_free(out_buffer);
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_send_card_state(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, char *url)
{
  uint32_t            status;
  char                str[64];
  T_sd_unlock_status *p_st;

  str[0] = 0;

  if (g_file_system_ready == 0)
  {
    sprintf(&str[strlen(str)],"SD card not detected");
  }
  else
  {
    p_st = s7_Get_sd_status();
    if (p_st->pass_exist)
    {
      sprintf(&str[strlen(str)],"SD card password is set. ");
    }
    else
    {
      sprintf(&str[strlen(str)],"SD card password not set. ");
    }
    if ((p_st->lock_detected == 1) &&  (p_st->unlock_executed == 1))
    {
      sprintf(&str[strlen(str)],"SD card unlocked successfully.");
    }
    else if ((p_st->lock_detected == 1) &&  (p_st->unlock_executed == 0))
    {
      sprintf(&str[strlen(str)],"SD card is locked.");
    }
  }

  status = HTTP_send_memory_buffer(server_ptr,str, strlen(str),request_type,"application/text");
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_card_control(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, UINT request_type, char *url)
{
  uint32_t             res;
  uint8_t             *input_buffer = NULL;
  UINT                 actual_size;

  json_t              *root = 0;
  json_error_t         error;

  char                *action_str = 0;
  char                *password_str = 0;
  char                 str[64];

  sdmmc_priv_card_status_t  response = {0};



  ULONG content_length;
  res = nx_web_http_server_content_length_get(packet_ptr,&content_length);
  if ((res == NX_SUCCESS) && (content_length > 0))
  {
    // Выделяем память для поступающих данных
    input_buffer = App_malloc_pending(content_length + 1,10);
    if (input_buffer == NULL) goto err_;

    // Получаем данные из первого пакета.
    res = nx_web_http_server_content_get(server_ptr, packet_ptr, 0, (CHAR *)input_buffer, content_length,&actual_size);
    if (res != NX_SUCCESS) goto err_;

    //Данные должны быть в формате JSON

    root = json_loads((char const *)input_buffer, 0,&error);
    if (!root) goto err_;

    json_unpack(root, "{s:s, s:s}", "action",&action_str, "password",&password_str);
    if (action_str == NULL) goto err_;
    if (password_str == NULL) goto err_;

    if (strcmp(action_str, "set_pass") == 0)
    {
      if (strlen(password_str) == 0) goto err_;
      if (Validate_password_symbols(password_str, strlen(password_str)) != RES_OK) goto err_;

      HTTPLOG("WEB | Set SD card password %s",password_str);

      res = Set_SD_card_password(SD_SET_PASSWORD, (char *)password_str, strlen(password_str),&response);
      if (res == 0)
      {
        // В случае успеха копируем новый пароль в настройки
        strncpy((char *)ivar.sd_card_password,password_str,MAX_SD_PASS_LEN);
        // Отправляем запрос на сохранение параметров
        Request_save_nv_parameters(Get_mod_params_instance(), BOOTL_PARAMS);
        Wait_ms(50);

        HTTPLOG("WEB | The new password was set successfully");
      }
      else
      {
        HTTPLOG("WEB | An error occurred while setting a new password");
        goto err_;
      }
    }
    else if (strcmp(action_str, "clear_pass") == 0)
    {
      HTTPLOG("WEB | Clear SD card password %s", ivar.sd_card_password);
      res = Set_SD_card_password(SD_CLEAR_PASSWORD, (char *)ivar.sd_card_password, strlen((char *)ivar.sd_card_password),&response);
      if (res == 0)
      {
        HTTPLOG("WEB | The new password was cleared successfully");
      }
      else
      {
        HTTPLOG("WEB | An error occurred while clearing a new password");
        goto err_;
      }
    }
    else goto err_;
  }
  else goto err_;


  json_decref(root);
  App_free(input_buffer);
  strcpy(str,"Operation completed successfully");
  HTTP_send_memory_buffer(server_ptr,str, strlen(str),request_type,"application/text");
  return res;
err_:
  json_decref(root);
  App_free(input_buffer);
  strcpy(str,"An operation error occurred");
  HTTP_send_memory_buffer(server_ptr,str, strlen(str),request_type,"application/text");
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param packet_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_upload_time(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, UINT request_type, char *url)
{
  uint32_t             res;
  uint8_t             *input_buffer = NULL;
  UINT                 actual_size;

  json_t              *root = 0;
  json_error_t         error;

  uint32_t             year = 0;
  uint32_t             month= 0;
  uint32_t             day  = 0;
  uint32_t             hours= 0;
  uint32_t             mins = 0;
  uint32_t             secs = 0;
  rtc_time_t           rt_time = {0};
  char                 str[64];

  ULONG content_length;
  res = nx_web_http_server_content_length_get(packet_ptr,&content_length);
  if ((res == NX_SUCCESS) && (content_length > 0))
  {
    // Выделяем память для поступающих данных
    input_buffer = App_malloc_pending(content_length + 1,10);
    if (input_buffer == NULL) goto err_;

    // Получаем данные из первого пакета.
    res = nx_web_http_server_content_get(server_ptr, packet_ptr, 0, (CHAR *)input_buffer, content_length,&actual_size);
    if (res != NX_SUCCESS) goto err_;

    //Данные должны быть в формате JSON

    root = json_loads((char const *)input_buffer, 0,&error);
    if (!root) goto err_;

    json_unpack(root, "{s:i, s:i, s:i, s:i, s:i, s:i}", "year",&year, "month", &month, "day", &day, "hours", &hours, "mins", &mins, "secs", &secs);
    rt_time.tm_year  = year - 1900;
    rt_time.tm_mon   = month;
    rt_time.tm_mday  = day   ;
    rt_time.tm_hour  = hours ;
    rt_time.tm_min   = mins  ;
    rt_time.tm_sec   = secs  ;

    RTC_set_system_DateTime(&rt_time);
  }

  json_decref(root);
  App_free(input_buffer);
  strcpy(str,"{\"status\":0}");
  HTTP_send_memory_buffer(server_ptr,str, strlen(str),request_type,"application/json");
  return res;

err_:
  json_decref(root);
  App_free(input_buffer);
  strcpy(str,"{\"status\":1}");
  HTTP_send_memory_buffer(server_ptr,str, strlen(str),request_type,"application/json");
  return RES_ERROR;
}
/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param resource
  \param packet_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_serv_get_request(NX_WEB_HTTP_SERVER *server_ptr, CHAR *resource, NX_PACKET *packet_ptr)
{
  if (strcmp(resource, "/") == 0)
  {
    HTTP_serv_send_file(server_ptr, NX_WEB_HTTP_SERVER_GET_REQUEST, "index.html");
    return (NX_WEB_HTTP_CALLBACK_COMPLETED);
  }
  else if (strcmp(resource, SETTINGS_URL) == 0)
  {
    HTTP_send_device_settings(server_ptr, NX_WEB_HTTP_SERVER_GET_REQUEST, SETTINGS_URL);
    return (NX_WEB_HTTP_CALLBACK_COMPLETED);
  }
  else if (strcmp(resource, DEV_LOG_URL) == 0)
  {
    char path[sizeof LOG_FILE_PATH  + 2];
    strcpy(path, "\\");
    strcat(path,LOG_FILE_PATH);
    HTTP_serv_send_file(server_ptr, NX_WEB_HTTP_SERVER_GET_REQUEST, path);
    return (NX_WEB_HTTP_CALLBACK_COMPLETED);
  }

  server_ptr->nx_web_http_server_request_resource[0] = ' ';
  return NX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_serv_post_request(NX_WEB_HTTP_SERVER *server_ptr, CHAR *resource, NX_PACKET *packet_ptr)
{
  uint32_t   status;
  uint8_t   *input_buffer = 0;
  uint32_t   received_size = 0;
  NX_PACKET *recv_packet_ptr;

  if (strcmp(resource, SETTINGS_URL) == 0)
  {
    // Принимаем из браузера json файл с отредактированными параметрами для сохранения

    ULONG content_length;
    status = nx_web_http_server_content_length_get(packet_ptr,&content_length);
    if (status != NX_SUCCESS) goto error_answer_;
    if (content_length > 0)
    {
      uint32_t actual_size;

      // Слишком большие блоки данных принять не можем
      if (content_length > MAX_PUT_DATA_SIZE) goto error_answer_;

      // Выделяем память для поступающих данных
      input_buffer = App_malloc_pending(content_length + 1,10);
      if (input_buffer == NULL) goto error_answer_;

      // Получаем данные из первого пакета
      status = nx_web_http_server_content_get(server_ptr, packet_ptr, 0, (CHAR *)input_buffer, content_length,&actual_size);
      if (status != NX_SUCCESS) goto error_answer_;

      received_size = actual_size;

      // Принимаем остальные пакеты с данными
      while (received_size < content_length)
      {
        ULONG packet_length;
        status = nx_web_http_server_packet_get(server_ptr,&recv_packet_ptr);
        if (status != NX_SUCCESS) goto error_answer_;
        nx_packet_length_get(recv_packet_ptr,&packet_length);

        uint32_t remain = content_length - received_size;
        if (packet_length > remain) packet_length = remain;
        nx_packet_data_retrieve(recv_packet_ptr,&input_buffer[received_size],&packet_length);
        nx_packet_release(recv_packet_ptr);

        received_size += packet_length;

      }

    }
    else goto error_answer_;

    input_buffer[received_size] = 0; // Завершаем нулем json блок данных.

    if (JSON_Deser_settings(&ivar_inst,(char *)input_buffer) == RES_OK)
    {
      App_free(input_buffer);

      if (Save_settings_to(&ivar_inst, MEDIA_TYPE_DATAFLASH, 0, BOOTL_PARAMS) == RES_OK)
      {

        // Передаем ответ об успешной загрузке
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_OK, sizeof(NX_WEB_HTTP_STATUS_OK) - 1, "Request Accepted", sizeof("Request Accepted") - 1, NX_NULL, 0);
      }
      else goto error_answer_;
    }
    else goto error_answer_;
  }
  else if (strcmp(resource, RESET_CMD_URL) == 0)
  {
    // Передаем ответ об успешной загрузке
    _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_OK, sizeof(NX_WEB_HTTP_STATUS_OK) - 1, "Request Accepted", sizeof("Request Accepted") - 1, NX_NULL, 0);
    Wait_ms(300);
    Reset_SoC();
  }
  else if (strcmp(resource, RESET_LOG_CMD_URL) == 0)
  {
    // Передаем ответ об успешной загрузке
    _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_OK, sizeof(NX_WEB_HTTP_STATUS_OK) - 1, "Request Accepted", sizeof("Request Accepted") - 1, NX_NULL, 0);
    Req_to_reset_log_file();
  }
  else if (strcmp(resource, POST_DEV_STATUS) == 0)
  {
    HTTP_send_device_status(server_ptr, NX_WEB_HTTP_SERVER_POST_REQUEST, POST_DEV_STATUS);
  }
  else if (strcmp(resource, POST_CARD_CSD) == 0)
  {
    HTTP_send_card_csd(server_ptr, NX_WEB_HTTP_SERVER_POST_REQUEST, POST_CARD_CSD);
  }
  else if (strcmp(resource, POST_CARD_STATE) == 0)
  {
    HTTP_send_card_state(server_ptr, NX_WEB_HTTP_SERVER_POST_REQUEST, POST_CARD_STATE);
  }
  else if (strcmp(resource, POST_CARD_CONTROL) == 0)
  {
    HTTP_card_control(server_ptr, packet_ptr, NX_WEB_HTTP_SERVER_POST_REQUEST, POST_CARD_CONTROL);
  }
  else if (strcmp(resource, POST_UPLOAD_TIME) == 0)
  {
    HTTP_upload_time(server_ptr, packet_ptr, NX_WEB_HTTP_SERVER_POST_REQUEST, POST_UPLOAD_TIME);
  }
  else if (strcmp(resource, POST_FIRMWARE_UPLOAD) == 0)
  {
    HTTP_firmware_upload(server_ptr, packet_ptr, NX_WEB_HTTP_SERVER_POST_REQUEST, POST_FIRMWARE_UPLOAD);
  }
  else goto error_answer_;

  return NX_WEB_HTTP_CALLBACK_COMPLETED;

error_answer_:
  App_free(input_buffer);

  _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR, sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1, "NetX HTTP Request Aborted", sizeof("NetX HTTP Request Aborted") - 1, NX_NULL, 0);

  return NX_WEB_HTTP_ERROR;

}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param file_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t HTTP_serv_send_file(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, char *file_name)
{
  uint32_t       status;
  CHAR           mime_type_str[48];
  UINT           file_name_len;
  UINT           mime_type_str_len;
  uint32_t       content_length =  0;
  NX_PACKET      *new_packet_ptr;
  ULONG          temp;
  NX_TCP_SOCKET *socket_ptr = &server_ptr->nx_web_http_server_current_session_ptr->nx_tcp_session_socket;

  // Проверяем длину строки ресурса
  if (_nx_utility_string_length_check(file_name,&file_name_len, NX_WEB_HTTP_MAX_RESOURCE) != NX_SUCCESS) return RES_ERROR;

  // Открываем файл на чтение
  status =  fx_file_open(server_ptr->nx_web_http_server_media_ptr,&(server_ptr->nx_web_http_server_file), file_name, FX_OPEN_FOR_READ);
  if (status != NX_SUCCESS)
  {
    // Возвращаем ответ о неудаче если файл не открылся
    _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_NOT_FOUND,
                                      sizeof(NX_WEB_HTTP_STATUS_NOT_FOUND) - 1,
                                      "NetX HTTP Server unable to find file: ",
                                      sizeof("NetX HTTP Server unable to find file: ") - 1,
                                      file_name,
                                      file_name_len);
    return RES_ERROR;
  }

  // Получаем длину файла
  content_length = server_ptr->nx_web_http_server_file.fx_file_current_file_size;

  // Получаем тип MIME
  _nx_web_http_server_type_get_extended(server_ptr, file_name, file_name_len, mime_type_str, sizeof(mime_type_str),&mime_type_str_len);


  mime_type_str[mime_type_str_len] = 0;

  // Формируем заголовок отправляемого файла
  status = _nx_web_http_server_generate_response_header(
                                                        server_ptr,                         //  Pointer to HTTP server
                                                        &new_packet_ptr,                    //  Pointer to packet
                                                        NX_WEB_HTTP_STATUS_OK,              //  Status-code and reason-phrase
                                                        sizeof(NX_WEB_HTTP_STATUS_OK) - 1,   //  Length of status-code
                                                        content_length,                     //  Length of content
                                                        mime_type_str,                      //  Type of content
                                                        mime_type_str_len,                  //  Length of content type
                                                        0,                                  //  Other HTTP headers
                                                        0);                                 //  Length of other HTTP headers


  if (status != NX_SUCCESS)
  {
    _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                      sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                      "NetX HTTP Request Aborted",
                                      sizeof("NetX HTTP Request Aborted") - 1, NX_NULL, 0);
    fx_file_close(&(server_ptr->nx_web_http_server_file));
    return RES_ERROR;
  }


  if ((!content_length) || (request_type == NX_WEB_HTTP_SERVER_HEAD_REQUEST))
  {
    fx_file_close(&(server_ptr->nx_web_http_server_file));
    status = _nx_web_http_server_send(server_ptr, new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);
    if (status != NX_SUCCESS)
    {
      nx_packet_release(new_packet_ptr);
      return RES_ERROR;
    }
    return RES_OK;
  }

  // Получаем длину пакета
  temp = new_packet_ptr->nx_packet_length;

  while (content_length)
  {
    // Определим нужно ли выделять новый пакет
    if (temp == 0)
    {
      status =  _nx_web_http_server_response_packet_allocate(server_ptr,&new_packet_ptr, NX_WAIT_FOREVER);
      if (status != NX_SUCCESS)
      {
        server_ptr->nx_web_http_server_allocation_errors++; // Считаем ошибки
        break;
      }
    }

    // Вычисляем свободную длину пакета
    temp = ((ULONG)(new_packet_ptr->nx_packet_data_end - new_packet_ptr->nx_packet_append_ptr)) - NX_PHYSICAL_TRAILER;

    // Корректируем длину чтобы не превысить допустимое значение
    if (temp > socket_ptr->nx_tcp_socket_connect_mss) temp =  socket_ptr->nx_tcp_socket_connect_mss;

    // Читаем данные в пакет
    status =  fx_file_read(&(server_ptr->nx_web_http_server_file), new_packet_ptr->nx_packet_append_ptr,temp,&temp);

    if (status != NX_SUCCESS)
    {
      nx_packet_release(new_packet_ptr);
      fx_file_close(&(server_ptr->nx_web_http_server_file));
      return RES_ERROR;
    }

    // Обновим информацию пакета
    new_packet_ptr->nx_packet_length =  new_packet_ptr->nx_packet_length + temp;
    new_packet_ptr->nx_packet_append_ptr =  new_packet_ptr->nx_packet_append_ptr + temp;

    // Вышлем пакет
    status =  _nx_web_http_server_send(server_ptr, new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

    if (status != NX_SUCCESS)
    {
      nx_packet_release(new_packet_ptr);
      fx_file_close(&(server_ptr->nx_web_http_server_file));
      return RES_ERROR;
    }

    // Считаем отправленные байты
    server_ptr->nx_web_http_server_total_bytes_sent =  server_ptr->nx_web_http_server_total_bytes_sent + temp;

    // Вычислим сколько еще осталось отправить
    content_length =  content_length - temp;

    temp = 0; // Отметим необходимость в выделении нового пакета
  }

  fx_file_close(&(server_ptr->nx_web_http_server_file));
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------


  \param server_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t HTTP_serv_send_not_found(NX_WEB_HTTP_SERVER *server_ptr)
{
  uint32_t    status;
  NX_PACKET   *response_pkt;
  int32_t      response_len;

  response_len = sprintf(response_buf,"<!DOCTYPE html><html xmlns=\"http://www.w3.org/1999/xhtml\"><head><title>404 Not found</title></head><body><h1>Not found</h1></body></html>\r\n");

  status = nx_web_http_server_callback_generate_response_header_extended(
                                                                         server_ptr,
                                                                         &response_pkt,
                                                                         NX_WEB_HTTP_STATUS_OK,         // status_code.              Indicate status of resource.
                                                                         strlen(NX_WEB_HTTP_STATUS_OK), // status_code_length.
                                                                         response_len,                  // content_length.           Size of content in bytes
                                                                         "text/html",                   // content_type.             Type of HTTP e.g. "text/plain"
                                                                         strlen("text/html"),           // content_type_length.      String length of content type
                                                                         NX_NULL,                       // additional_header.        Pointer to additional header text
                                                                         0                              // additional_header_length. Length of additional header text
                                                                        );
  if (status != NX_SUCCESS) return (status);

  nx_packet_data_append(response_pkt, response_buf, response_len, server_ptr->nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

  status = nx_web_http_server_callback_packet_send(server_ptr, response_pkt);
  if (status != NX_SUCCESS)
  {
    nx_packet_release(response_pkt);
    return (status);
  }

  return (NX_WEB_HTTP_CALLBACK_COMPLETED);

}

/*-----------------------------------------------------------------------------------------------------


  \param resource
  \param max_age
  \param last_modified

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT cache_info_callback(CHAR *resource, UINT *max_age, NX_WEB_HTTP_SERVER_DATE *last_modified)
{
  rtc_time_t rt_time;
  *max_age = 1;

  RTC_get_system_DateTime(&rt_time);
  last_modified->nx_web_http_server_year   = rt_time.tm_year + 1900;
  last_modified->nx_web_http_server_month  = rt_time.tm_mon + 1;
  last_modified->nx_web_http_server_day    = rt_time.tm_mday;
  last_modified->nx_web_http_server_hour   = rt_time.tm_hour;
  last_modified->nx_web_http_server_minute = rt_time.tm_min;
  last_modified->nx_web_http_server_second = rt_time.tm_sec;
  last_modified->nx_web_http_server_weekday = rt_time.tm_wday;
  return NX_TRUE;
}

/*-----------------------------------------------------------------------------------------------------


  \param last_modified
-----------------------------------------------------------------------------------------------------*/
void gmt_get_callback(NX_WEB_HTTP_SERVER_DATE *last_modified)
{
  rtc_time_t rt_time;
  RTC_get_system_DateTime(&rt_time);
  last_modified->nx_web_http_server_year   = rt_time.tm_year + 1900 - 1;
  last_modified->nx_web_http_server_month  = rt_time.tm_mon + 1;
  last_modified->nx_web_http_server_day    = rt_time.tm_mday;
  last_modified->nx_web_http_server_hour   = rt_time.tm_hour;
  last_modified->nx_web_http_server_minute = rt_time.tm_min;
  last_modified->nx_web_http_server_second = rt_time.tm_sec;
  last_modified->nx_web_http_server_weekday = rt_time.tm_wday;
}
/*-----------------------------------------------------------------------------------------------------
  Вызывается в функции _nx_tcpserver_data_process -> __nx_web_http_server_receive_data -> nx_web_http_server_get_process
  путем вызова server_ptr -> nx_web_http_server_request_notify
  в контексте задачи TCPSERVER Thread создаваемой в функции _nx_tcpserver_create

  \param server_ptr
  \param request_type
  \param resource
  \param packet_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT _HTTP_server_request_callback(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{
  uint32_t res;
  // Создаем локальный контекст задачи если он еще не создан
  if (web_first_call)
  {
    // Устаноавливаем директорию по умолчанию для файлов WEB сервера
    fx_directory_local_path_set(server_ptr->nx_web_http_server_media_ptr,&web_local_path, WEB_FILES_DIR_NAME);
    web_first_call = 0;

    nx_web_http_server_cache_info_callback_set(server_ptr, cache_info_callback);
    nx_web_http_server_gmt_callback_set(server_ptr,gmt_get_callback);
  }

  HTTPLOG("WEB | Req.type %d | url = %s", request_type, resource)
  if (request_type == NX_WEB_HTTP_SERVER_HEAD_REQUEST)
  {
    return (NX_SUCCESS);
  }
  if (request_type == NX_WEB_HTTP_SERVER_GET_REQUEST)
  {
    res = HTTP_serv_get_request(server_ptr, resource, packet_ptr);
    return (res);
  }
  else if (request_type == NX_WEB_HTTP_SERVER_POST_REQUEST)
  {
    res = HTTP_serv_post_request(server_ptr, resource, packet_ptr);
    return res;
  }
  else
  {
    // Символ '/' или '\' означает что путь к файлу абсолютный и поэтому не будет работать  назначение fx_directory_local_path_set
    // В этом случае надо откорректировать содержание запроса чтобы в нем отсутствовал некорректный первый символ
    server_ptr->nx_web_http_server_request_resource[0] = ' ';
    // Возвращаем статус о том что запрос не обработан и требуется дальнейшая обработка
    return (NX_SUCCESS);
  }


}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t  Net_HTTP_server_delete(void)
{
  uint32_t res = NX_SUCCESS;
  if (http_ip_ptr != 0)
  {
    res = nx_web_http_server_stop(&http_server);
    if (res == NX_SUCCESS)
    {
      APPLOG("HTTP server stopped successfully");
    }
    else
    {
      APPLOG("Failed to stop HTTP server. Error %d", res);
    }

    res = nx_web_http_server_delete(&http_server);
    if (res == NX_SUCCESS)
    {
      APPLOG("HTTP server deleted successfully");
    }
    else
    {
      APPLOG("Failed to delete HTTP server. Error %d", res);
    }

    http_ip_ptr = 0;
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t  Net_HTTP_server_create(NX_IP *ip_ptr)
{
  UINT status;
  UINT port;

  if (ivar.enable_HTTP_server == 0) return RES_ERROR;
  if (http_ip_ptr != 0) return RES_ERROR;


  if (ivar.enable_HTTPS != 0)
  {
    port = NX_WEB_HTTPS_SERVER_PORT;
  }
  else
  {
    port = NX_WEB_HTTP_SERVER_PORT;
  }

  status = nx_web_http_server_create(&http_server,
                                     "HTTP Server",
                                     ip_ptr,
                                     port,
                                     &fat_fs_media,
                                     server_stack,
                                     WEB_SERVER_STACK_SIZE,
                                     &net_packet_pool,
                                     _HTTP_authentication_check,
                                     _HTTP_server_request_callback);

  if (NX_SUCCESS != status)
  {
    APPLOG("HTTP server create error %d", status);
    return RES_ERROR;
  }
  APPLOG("HTTP server created successfully.");

  if (ivar.enable_HTTPS)
  {
    memset(&certificate, 0, sizeof(certificate));
    memset(crypto_metadata_server, 0, 20000 * NX_WEB_HTTP_SERVER_SESSION_MAX);
    memset(tls_packet_buffer, 0, 40000);
    nx_secure_x509_certificate_initialize(&certificate, (UCHAR *)SERVER_CERTIFICATE, sizeof(SERVER_CERTIFICATE), NX_NULL, 0, SERVER_KEY, sizeof(SERVER_KEY), NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER);

    // Setup TLS session data for the TCP server.
    status = nx_web_http_server_secure_configure(
                                                 &http_server,
                                                 &CRIPTO_TLS_CYPHERS,
                                                 crypto_metadata_server,
                                                 sizeof(crypto_metadata_server),
                                                 tls_packet_buffer,
                                                 sizeof(tls_packet_buffer),
                                                 &certificate,
                                                 NX_NULL, 0, NX_NULL, 0, NX_NULL, 0
                                                );
    if (status != NX_SUCCESS)
    {
      APPLOG("HTTP server secure configuration error %d", status);
    }

  }

  web_first_call = 1;
  status = nx_web_http_server_start(&http_server);
  if (status == NX_SUCCESS)
  {
    http_ip_ptr = ip_ptr;
    APPLOG("HTTP server started successfully.");
    return RES_OK;
  }
  else
  {
    APPLOG("HTTP server starting  error %d", status);
    nx_web_http_server_delete(&http_server);
    return RES_ERROR;
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void HTTP_server_controller(void)
{
  NX_IP   *ip_ptr = NULL;

  switch (g_network_type)
  {
  case NET_BY_WIFI_STA:
    if (WIFI_STA_network_active_flag()) ip_ptr = wifi_sta_ip_ptr;
    break;
  case NET_BY_WIFI_AP:
    if (WIFI_AP_network_active_flag())  ip_ptr = wifi_ap_ip_ptr;
    break;
  case NET_BY_RNDIS:
    if (RNDIS_network_active_flag())    ip_ptr = rndis_ip_ptr;
    break;
  case NET_BY_ECM:
    if (ECM_Host_network_active_flag()) ip_ptr = ecm_host_ip_ptr;
    break;
  }

  if (ip_ptr != NULL)
  {
    Net_HTTP_server_create(ip_ptr);
  }
  else
  {
    Net_HTTP_server_delete();
  }

}



