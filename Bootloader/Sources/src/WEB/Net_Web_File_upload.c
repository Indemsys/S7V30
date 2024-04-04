// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-04-04
// 11:29:11
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"

//  Пример текста HTTP заголовка при передаче файла
//
//  POST /firmware_upload HTTP/1.1
//  Host: 192.168.137.2
//  Connection: keep-alive
//  Content-Length: 1054815
//  Authorization: Basic dXNlcjoxMjM0NTY3ODk=
//  Accept: */*
//  X-Requested-With: XMLHttpRequest
//  User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0.0.0 Safari/537.36
//  Content-Type: multipart/form-data; boundary=----WebKitFormBoundary1AkBsiB9cqpOazGx  (строка заканчивается символоами 0x0D 0x0A)
//  Origin: http://192.168.137.2
//  Referer: http://192.168.137.2/
//  Accept-Encoding: gzip, deflate
//  Accept-Language: en-US,en;q=0.9,ru;q=0.8,lt;q=0.7,de;q=0.6,pl;q=0.5
//  (Здесь служебные симолы 0x0D 0x0A 0x0D 0x0A)
//  ------WebKitFormBoundary1AkBsiB9cqpOazGx
//  Content-Disposition: form-data; name="file_size"
//  (Здесь служебные симолы 0x0D 0x0A 0x0D 0x0A)
//  1054512 (Здесь служебные симолы 0x0D 0x0A)
//  ------WebKitFormBoundary1AkBsiB9cqpOazGx
//  Content-Disposition: form-data; name="file"; filename="firmware.bin"
//  Content-Type: application/octet-stream
//
//  (Здесь служебные симолы 0x0D 0x0A 0x0D 0x0A)
//  ... Далее байты передаваемого файла ...

// Стратегия поиска:
// 1. Находим вторую  запись символов 0x0D 0x0A 0x0D 0x0A и сразу после нее читаем размер файла
// 2. Находим следующую запись символов  0x0D 0x0A 0x0D 0x0A и после нее читаем данные файла

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param packet_ptr
  \param p_length
  \param left_mark
  \param right_mark

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Lookup_substr(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, uint32_t *p_length, T_marked_str_finder  *p_msf, char const *left_mark, char const *right_mark)
{

  uint32_t             res;

  p_msf->step                         = 0;
  p_msf->left_mark                    = left_mark;
  p_msf->left_mark_sz                 = strlen(left_mark);
  p_msf->right_mark                   = right_mark;
  p_msf->right_mark_sz                = strlen(right_mark);

  do
  {
    res = Find_marked_number(packet_ptr->nx_packet_prepend_ptr, packet_ptr->nx_packet_length,p_msf);
    if (res != 0)
    {
      packet_ptr->nx_packet_prepend_ptr = p_msf->next_sym_ptr;
      return RES_OK;
    }
    res = nx_web_http_server_packet_get(server_ptr,&packet_ptr);
    if (res != NX_SUCCESS) return RES_ERROR;
    if (packet_ptr->nx_packet_length >= *p_length)
    {
      nx_packet_release(packet_ptr);
      return RES_ERROR;
    }
    *p_length -= packet_ptr->nx_packet_length;
    nx_packet_release(packet_ptr);
  } while (1);
}




/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param packet_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t HTTP_firmware_upload(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, UINT request_type, char *url)
{
  char                 str[65];
  uint32_t             content_length = 0;
  uint32_t             res;
  T_marked_str_finder  msf;
  uint32_t             file_size = 0;
  FX_FILE              *p_file = 0;
  uint32_t             data_sz;


  res = nx_web_http_server_packet_content_find(server_ptr,&packet_ptr, (ULONG *)&content_length);
  if ((res != NX_SUCCESS) || (content_length == 0)) goto err_;


  // Здесь мы находимся на начале содержимого
  // Содержимое начинается с маркера границы вида  ------WebKitFormBoundary1AkBsiB9cqpOazGx
  //   packet_ptr->nx_packet_prepend_ptr  - указывает на начало блока данных
  //   packet_ptr->nx_packet_length       - указывает на колическтво данных

  // Ищем тэг вида  name="file_size"

  if (_Lookup_substr(server_ptr,packet_ptr,&content_length,&msf, "name=\"","\"") != RES_OK) goto err_;
  if (strcmp(msf.fragment, "file_size") != 0) goto err_;

  if (_Lookup_substr(server_ptr,packet_ptr,&content_length,&msf, "\r\n\r\n","\r\n") != RES_OK) goto err_;
  file_size = atoi(msf.fragment);

  if (_Lookup_substr(server_ptr,packet_ptr,&content_length,&msf, "name=\"","\"") != RES_OK) goto err_;
  if (strcmp(msf.fragment, "file") != 0) goto err_;

  if (_Lookup_substr(server_ptr,packet_ptr,&content_length,&msf, "filename=\"","\"") != RES_OK) goto err_;
  if (strlen(msf.fragment) > 64) goto err_;
  strcpy(str, "/"); // Файл будет размещен в корне файловой системы
  strcat(str, msf.fragment);

  if (_Lookup_substr(server_ptr,packet_ptr,&content_length,&msf, "Content-Type: ","\r\n\r\n") != RES_OK) goto err_;
  if (strcmp(msf.fragment, "application/octet-stream") != 0) goto err_;

  HTTPLOG("WEB | Uploading file size = %d. File name = %s", file_size, str);

  // Здесь сохраняем данные в файл


  p_file = (FX_FILE *)App_malloc(sizeof(FX_FILE));
  if (p_file == NULL) goto err_;

  fx_file_delete(&fat_fs_media, str);
  res = fx_file_create(&fat_fs_media, str);
  if (res != FX_SUCCESS) goto err_;
  // Следим за счетчиком оставшихся к чтению байт
  res = fx_file_open(&fat_fs_media,p_file, str,  FX_OPEN_FOR_WRITE);
  if (res != FX_SUCCESS) goto err1_;

  // Находим размер оставшихся данных в текущем пакете
  data_sz = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr;

  do
  {
    if (file_size < data_sz) data_sz = file_size;

    res = fx_file_write(p_file, packet_ptr->nx_packet_prepend_ptr, data_sz);
    if (res != FX_SUCCESS) goto err1_;

    file_size -= data_sz;
    if (file_size == 0) break;

    // Читаем следующий пакет с данными
    nx_packet_release(packet_ptr);
    res = nx_web_http_server_packet_get(server_ptr,&packet_ptr);
    if (res != NX_SUCCESS) goto err1_;
    data_sz = packet_ptr->nx_packet_length;

  } while (1);

  fx_file_close(p_file);

  HTTPLOG("WEB | File uploaded successfully");

  nx_packet_release(packet_ptr);
  App_free(p_file);
  strcpy(str,"{\"status\":0}");
  HTTP_send_memory_buffer(server_ptr,str, strlen(str),request_type,"application/json");
  return res;
err1_:
  fx_file_close(p_file);
err_:
  nx_packet_release(packet_ptr);
  App_free(p_file);
  strcpy(str,"{\"status\":1}");
  HTTP_send_memory_buffer(server_ptr,str, strlen(str),request_type,"application/json");
  HTTPLOG("WEB | File uploading error");
  return RES_ERROR;

}


