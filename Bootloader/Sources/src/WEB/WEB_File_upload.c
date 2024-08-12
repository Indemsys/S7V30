// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-04-04
// 11:29:11
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"


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
   Ищем заданный фрагмент в потоке пакетов на заданном объеме данных
   Объем проверяемых данных определяется размером заголовка в команде POST перед началом данных самого передаваемого файла

   Внимание!!!
   Если мискомый фрагмент находится не в первом TCP пакете команды POST,
   то в случае HTTPS такой случай обрабатываться не будет и произойдет выход по ошибке.

  \param server_ptr
  \param packet_ptr
  \param p_length   - указатель на количество байт, которые осталось проверить
  \param left_mark
  \param right_mark

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Lookup_substr(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr_ptr, uint32_t *remaining_data_sz_ptr, T_marked_str_finder  *p_msf, char const *left_mark, char const *right_mark)
{

  uint32_t             res;
  uint32_t             packet_len;
  NX_PACKET           *packet_ptr;
  uint32_t            remaining_data_sz =*remaining_data_sz_ptr;

  p_msf->step                         = 0;
  p_msf->left_mark                    = left_mark;
  p_msf->left_mark_sz                 = strlen(left_mark);
  p_msf->right_mark                   = right_mark;
  p_msf->right_mark_sz                = strlen(right_mark);

  packet_ptr =*packet_ptr_ptr;
  do
  {
    packet_len = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr;

    res = Find_marked_number(packet_ptr->nx_packet_prepend_ptr, packet_len,p_msf);
    if (res != 0)
    {
      // Здесь нашли то что искали. И смещаем указатель в бефере на начало следующих за фрагментом данных
      packet_ptr->nx_packet_prepend_ptr = p_msf->next_sym_ptr;
      *packet_ptr_ptr = packet_ptr;
      *remaining_data_sz_ptr = remaining_data_sz;
      return RES_OK;
    }
    if (packet_len > remaining_data_sz)
    {
      remaining_data_sz = 0;
    }
    else
    {
      remaining_data_sz = remaining_data_sz - packet_len;
    }
    if (remaining_data_sz == 0)
    {
      // Весь заданный объем данных просмотрели и не нашли искомый фрагмент
      *packet_ptr_ptr = packet_ptr;
      *remaining_data_sz_ptr = remaining_data_sz;
      return RES_ERROR;
    }
    if (packet_ptr->nx_packet_next != 0)
    {
      // При использовании TLS пакеты организуются в цепочки. Поэтому выполняем проход по цепочке.
      packet_ptr = packet_ptr->nx_packet_next;
      packet_len = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr;
    }
    else
    {
      // Если цепочки нет, то запрашиваем следующий пакет
      nx_packet_release(packet_ptr);
      res = nx_web_http_server_packet_get(server_ptr,&packet_ptr);
      if (res != NX_SUCCESS)
      {
        // Пакеты кончились. Не нашли искомый фрагмент.
        *packet_ptr_ptr = packet_ptr;
        *remaining_data_sz_ptr = remaining_data_sz;
        return RES_ERROR;
      }
      packet_len = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr;
    }
  } while (1);
}




/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param packet_ptr
  \param request_type
  \param url

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t HTTP_POST_firmware_upload(NX_WEB_HTTP_SERVER *server_ptr, CHAR *url, NX_PACKET *packet_ptr)
{
  char                 str[65];
  uint32_t             content_length = 0;
  uint32_t             res = 0;
  T_marked_str_finder  msf;
  uint32_t             remaining_data_sz = 0;
  FX_FILE              *p_file = 0;
  uint32_t             data_sz;
  uint32_t             hdr_sz;
  uint32_t             chank_sz;
  NX_PACKET           *origin_packet_ptr;
  uint32_t            err_line;
  uint8_t             *file_buff = NULL;
  uint8_t             *file_buff_ptr = NULL;
  uint32_t             file_buff_size = 0;
  uint8_t              upload_to_buf;

  res = nx_web_http_server_packet_content_find(server_ptr,&packet_ptr, (ULONG *)&content_length);
  if ((res != NX_SUCCESS) || (content_length == 0))
  {
    err_line = __LINE__;
    goto err_;
  }

  origin_packet_ptr = packet_ptr;
  // Здесь мы находимся на начале содержимого
  // Содержимое начинается с маркера границы вида  ------WebKitFormBoundary1AkBsiB9cqpOazGx
  //   packet_ptr->nx_packet_prepend_ptr  - указывает на начало блока данных
  //   packet_ptr->nx_packet_length       - указывает на колическтво данных

  // Ищем тэг вида  name="file_size"

  if (_Lookup_substr(server_ptr,&packet_ptr,&content_length,&msf, "name=\"","\"") != RES_OK)
  {
    err_line = __LINE__;
    goto err_;
  }
  if (strcmp(msf.fragment, "file_size") != 0)
  {
    err_line = __LINE__;
    goto err_;
  }

  if (_Lookup_substr(server_ptr,&packet_ptr,&content_length,&msf, "\r\n\r\n","\r\n") != RES_OK)
  {
    err_line = __LINE__;
    goto err_;
  }
  remaining_data_sz = atoi(msf.fragment);

  if (_Lookup_substr(server_ptr,&packet_ptr,&content_length,&msf, "name=\"","\"") != RES_OK)
  {
    err_line = __LINE__;
    goto err_;
  }
  if (strcmp(msf.fragment, "file") != 0)
  {
    err_line = __LINE__;
    goto err_;
  }

  if (_Lookup_substr(server_ptr,&packet_ptr,&content_length,&msf, "filename=\"","\"") != RES_OK)
  {
    err_line = __LINE__;
    goto err_;
  }
  if (strlen(msf.fragment) > 64)
  {
    err_line = __LINE__;
    goto err_;
  }
  strcpy(str, "/"); // Файл будет размещен в корне файловой системы
  strcat(str, msf.fragment);

  if (_Lookup_substr(server_ptr,&packet_ptr,&content_length,&msf, "Content-Type: ","\r\n\r\n") != RES_OK)
  {
    err_line = __LINE__;
    goto err_;
  }
//  if (strcmp(msf.fragment, "application/octet-stream") != 0)
//  {
//    if (strcmp(msf.fragment, "text/plain") != 0)
//    {
//      err_line = __LINE__;
//      goto err_;
//    }
//  }

  HTTPLOG("WEB | Uploading file size = %d. File name = %s", remaining_data_sz, str);

  // Здесь сохраняем данные в файл
  upload_to_buf = 1;

  if (upload_to_buf)
  {
    file_buff_size = remaining_data_sz;
    file_buff = SDRAM_malloc(file_buff_size);
    if (file_buff == NULL)
    {
      upload_to_buf = 0;
    }
    else
    {
      file_buff_ptr = file_buff;
    }
  }

  if (upload_to_buf == 0)
  {

    p_file = (FX_FILE *)App_malloc(sizeof(FX_FILE));
    if (p_file == NULL)
    {
      err_line = __LINE__;
      goto err_;
    }

    fx_file_delete(&fat_fs_media, str);
    res = fx_file_create(&fat_fs_media, str);
    if (res != FX_SUCCESS)
    {
      err_line = __LINE__;
      goto err_;
    }
    // Следим за счетчиком оставшихся к чтению байт
    res = fx_file_open(&fat_fs_media,p_file, str,  FX_OPEN_FOR_WRITE);
    if (res != FX_SUCCESS)
    {
      err_line = __LINE__;
      goto err_;
    }
  }

  // Внимание!!!
  // Для случая работы по HTTPS.
  // Если при вызове поиска маркеров в заголовке функцией _Lookup_substr был осуществлен переход на следующий пакет в цепочке,
  // то origin_packet_ptr уже не будет указывать на первый пакет и поле nx_packet_length будет равно 0. Такие ситуации не обрабатываем.
  if (packet_ptr->nx_packet_length == 0)
  {
    err_line = __LINE__;
    goto err1_;
  }

  origin_packet_ptr = packet_ptr;
  // Находим размер оставшихся данных в текущей цепочке пакетов и сохраняем в файл
  hdr_sz            = origin_packet_ptr->nx_packet_prepend_ptr - origin_packet_ptr->nx_packet_data_start;
  if (origin_packet_ptr->nx_packet_next == 0)
  {
    // Для случая HTTP размер данных определяем по адресам начала и конца пакета
    // Данные передаются по одному пакету. Поле nx_packet_length здесь нельзя применять
    data_sz = origin_packet_ptr->nx_packet_append_ptr - origin_packet_ptr->nx_packet_prepend_ptr;
  }
  else
  {
    // Для случая HTTPS размер данных определяем по полю nx_packet_length
    // Данные передаются TLS записями, каждая состоит из цепочки пакетов.
    // Поле nx_packet_length содержит полную длинну данных включая заголовок запроса POST
    data_sz = origin_packet_ptr->nx_packet_length - hdr_sz; // Все количество данных принятых в цепочку пакетов
  }
  remaining_data_sz = remaining_data_sz - data_sz;

  // Пробегаем по цепочке пакетов. Цепочки пакетов встречаются, когда применяется HTTPS
  do
  {
    chank_sz = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr; // Количество данных в текущем пакете
    if (data_sz < chank_sz) chank_sz = data_sz;
    if (upload_to_buf)
    {
      memcpy(file_buff_ptr, packet_ptr->nx_packet_prepend_ptr, chank_sz);
      file_buff_ptr += chank_sz;
    }
    else
    {
      res = fx_file_write(p_file, packet_ptr->nx_packet_prepend_ptr, chank_sz);
      if (res != FX_SUCCESS)
      {
        err_line = __LINE__;
        goto err1_;
      }
    }
    data_sz = data_sz - chank_sz;
    if (data_sz == 0) break;
    if (packet_ptr->nx_packet_next == 0)
    {
      err_line = __LINE__;
      goto err1_;
    }
    packet_ptr = packet_ptr->nx_packet_next;

  } while (1);

  nx_packet_release(origin_packet_ptr);

  // Начинаем прием и запись следующих пакетов
  do
  {
    res = nx_web_http_server_packet_get(server_ptr,&packet_ptr);
    if (res != NX_SUCCESS)
    {
      err_line = __LINE__;
      goto err1_;
    }
    origin_packet_ptr = packet_ptr;
    if (origin_packet_ptr->nx_packet_next == 0)
    {
      data_sz = origin_packet_ptr->nx_packet_append_ptr - origin_packet_ptr->nx_packet_prepend_ptr;
    }
    else
    {
      data_sz = origin_packet_ptr->nx_packet_length;
    }
    if (remaining_data_sz < data_sz) data_sz = remaining_data_sz;
    remaining_data_sz = remaining_data_sz - data_sz;

    // Пробегаем по цепочке пакетов. Цепочки пакетов встречаются, когда применяется HTTPS
    do
    {
      chank_sz = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr; // Количество данных в текущем пакете
      if (data_sz < chank_sz) chank_sz = data_sz;
      if (upload_to_buf)
      {
        memcpy(file_buff_ptr, packet_ptr->nx_packet_prepend_ptr, chank_sz);
        file_buff_ptr += chank_sz;
      }
      else
      {
        res = fx_file_write(p_file, packet_ptr->nx_packet_prepend_ptr, chank_sz);
        if (res != FX_SUCCESS)
        {
          err_line = __LINE__;
          goto err1_;
        }
      }
      data_sz = data_sz - chank_sz;
      if (data_sz == 0) break;
      if (packet_ptr->nx_packet_next == 0)
      {
        err_line = __LINE__;
        goto err1_;
      }
      packet_ptr = packet_ptr->nx_packet_next;

    } while (1);


    if (remaining_data_sz == 0) break;

    // Читаем следующий пакет с данными
    nx_packet_release(origin_packet_ptr);

  } while (1);

  if (upload_to_buf)
  {
    p_file = (FX_FILE *)App_malloc(sizeof(FX_FILE));
    if (p_file == NULL)
    {
      err_line = __LINE__;
      goto err_;
    }

    fx_file_delete(&fat_fs_media, str);
    res = fx_file_create(&fat_fs_media, str);
    if (res != FX_SUCCESS)
    {
      err_line = __LINE__;
      goto err_;
    }
    // Следим за счетчиком оставшихся к чтению байт
    res = fx_file_open(&fat_fs_media,p_file, str,  FX_OPEN_FOR_WRITE);
    if (res != FX_SUCCESS)
    {
      err_line = __LINE__;
      goto err_;
    }

    res = fx_file_write(p_file, file_buff, file_buff_size);
    if (res != FX_SUCCESS)
    {
      err_line = __LINE__;
      goto err1_;
    }

    fx_file_close(p_file);

    SDRAM_free(file_buff);
  }
  else
  {
    fx_file_close(p_file);
  }

  HTTPLOG("WEB | File uploaded successfully");

  nx_packet_release(origin_packet_ptr);
  App_free(p_file);
  strcpy(str,"{\"status\":0}");
  HTTP_send_data(server_ptr,str, strlen(str),NX_WEB_HTTP_SERVER_POST_REQUEST,"application/json");
  return res;
err1_:
  fx_file_close(p_file);
err_:
  nx_packet_release(origin_packet_ptr);
  SDRAM_free(file_buff);
  App_free(p_file);
  strcpy(str,"{\"status\":1}");
  HTTP_send_data(server_ptr,str, strlen(str),NX_WEB_HTTP_SERVER_POST_REQUEST,"application/json");
  HTTPLOG("WEB | File uploading error. Line %d, Res=0x%04X", err_line, res);
  return RES_ERROR;

}


