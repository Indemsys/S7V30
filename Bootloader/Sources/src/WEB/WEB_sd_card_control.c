// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-07-26
// 16:02:55
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t HTTP_POST_send_card_csd(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr)
{
  uint32_t       status;
  uint32_t       sz = 0;

  // Выделяем память для буфера с данными
  char *out_buffer = App_malloc_pending(2048,10);
  if (out_buffer == NULL) return RES_ERROR;

  sdmmc_priv_csd_reg_t *p_csd_reg = s7_Get_csd_reg();
  sz = Get_card_csd_text(p_csd_reg, out_buffer, 2048);

  status = HTTP_send_data(server_ptr,out_buffer, sz,NX_WEB_HTTP_SERVER_POST_REQUEST,"application/text");

  App_free(out_buffer);
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t HTTP_POST_send_card_state(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr)
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

  status = HTTP_send_data(server_ptr,str, strlen(str),NX_WEB_HTTP_SERVER_POST_REQUEST,"application/text");
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t HTTP_POST_card_control(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr)
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
  HTTP_send_data(server_ptr,str, strlen(str),NX_WEB_HTTP_SERVER_POST_REQUEST,"application/text");
  return res;
err_:
  json_decref(root);
  App_free(input_buffer);
  strcpy(str,"An operation error occurred");
  HTTP_send_data(server_ptr,str, strlen(str),NX_WEB_HTTP_SERVER_POST_REQUEST,"application/text");
  return RES_ERROR;
}





