// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2018.09.03
// 23:06:41
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include <stdarg.h>

#define INI_STR_SIZE 512
#define TMP_BUF_SZ   512

const uint32_t  df_params_addr[PARAMS_TYPES_NUM][2] =
{
  {  DATAFLASH_APP_PARAMS_1_ADDR     , DATAFLASH_APP_PARAMS_2_ADDR     },
  {  DATAFLASH_MODULE_PARAMS_1_ADDR  , DATAFLASH_MODULE_PARAMS_2_ADDR  },
  {  DATAFLASH_BOOTL_PARAMS_1_ADDR   , DATAFLASH_BOOTL_PARAMS_2_ADDR   }
};


char  *ini_fname[PARAMS_TYPES_NUM]        = { PARAMS_APP_INI_FILE_NAME        , PARAMS_MOD_INI_FILE_NAME        , PARAMS_BOOTL_INI_FILE_NAME       };
char  *ini_used_fname[PARAMS_TYPES_NUM]   = { PARAMS_APP_USED_INI_FILE_NAME   , PARAMS_MOD_USED_INI_FILE_NAME   , PARAMS_BOOTL_USED_INI_FILE_NAME  };
char  *json_fname[PARAMS_TYPES_NUM]       = { PARAMS_APP_JSON_FILE_NAME       , PARAMS_MOD_JSON_FILE_NAME       , PARAMS_BOOTL_JSON_FILE_NAME      };
char  *json_compr_fname[PARAMS_TYPES_NUM] = { PARAMS_APP_COMPR_JSON_FILE_NAME , PARAMS_MOD_COMPR_JSON_FILE_NAME , PARAMS_BOOTL_COMPR_JSON_FILE_NAME};

uint32_t   g_setting_wr_counters[PARAMS_TYPES_NUM][2]; // Счетчики количества записей в каждую область
uint32_t   g_setting_start_condition[PARAMS_TYPES_NUM][2];  // Регистры ошибок каждой области

static uint32_t Restore_settings_from_DataFlash(const T_NV_parameters_instance  *p_pars, uint8_t ptype);

static T_settings_restore_results nvr[2];

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_file_system_init_results*
-----------------------------------------------------------------------------------------------------*/
T_settings_restore_results* Get_Setting_restoring_res(uint8_t ptype)
{
  if (ptype >= PARAMS_TYPES_NUM) return &nvr[0];
  return &nvr[ptype];
}

/*-------------------------------------------------------------------------------------------
  Восстановление параметров по умолчанию, после сбоев системы или смены версии
---------------------------------------------------------------------------------------------*/
void Return_def_params(const T_NV_parameters_instance *p_pars)
{
  uint16_t  i;
  const     T_NV_parameters    *pp;

  // Загрузить параметры значениями по умолчанию
  for (i = 0; i < p_pars->items_num; i++)
  {
    pp = &p_pars->items_array[i];

    if ((pp->attr & VAL_NOINIT) == 0)
    {
      switch (pp->vartype)
      {
        // tint8u, tint16u, tuint32_t, tfloat, tarrofdouble, tarrofbyte
      case tint8u:
        *(uint8_t *)pp->val = (uint8_t)pp->defval;
        break;
      case tint16u:
        *(uint16_t *)pp->val = (uint16_t)pp->defval;
        break;
      case tint32u:
        *(uint32_t *)pp->val = (uint32_t)pp->defval;
        break;
      case tint32s:
        *(int32_t *)pp->val = (int32_t)pp->defval;
        break;
      case tfloat:
        *(float *)pp->val = (float)pp->defval;
        break;
      case tstring:
        {
          uint8_t *st;

          strncpy((char *)pp->val, (const char *)pp->pdefval, pp->varlen - 1);
          st = (uint8_t *)pp->val;
          st[pp->varlen - 1] = 0;
        }
        break;
      case tarrofbyte:
        memcpy(pp->val, pp->pdefval, pp->varlen);
        break;
      case tarrofdouble:
        break;
      }
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  Восстановление параметров из всех доступных ситочников: .ini файла, .json файла, dataflash
  Параметры могут относиться к модулю и к приложению.
  Эти параметры храняться в разных файлах и в разных областях Flash

  \param p_pars

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Restore_settings(const T_NV_parameters_instance *p_pars, uint8_t ptype)
{
  uint32_t res;

  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;

  nvr[ptype].dataflash_saving_error = SAVED_TO_DATAFLASH_NONE;

  // Сначала запишем в параметры значения по умолчанию
  Return_def_params(p_pars);
  nvr[ptype].settings_source = RESTORED_DEFAULT_SETTINGS;

  res = Restore_settings_from_INI_file(p_pars, ptype);
  if (res == RES_OK)
  {
    nvr[ptype].settings_source = RESTORED_SETTINGS_FROM_INI_FILE;
    res = Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0, ptype);
    if (res != RES_OK)
    {
      nvr[ptype].dataflash_saving_error = SAVED_TO_DATAFLASH_ERROR;
    }
    else
    {
      nvr[ptype].dataflash_saving_error = SAVED_TO_DATAFLASH_OK;
    }
    fx_file_rename(&fat_fs_media, ini_fname[ptype], ini_used_fname[ptype]);
    fx_media_flush(&fat_fs_media);
  }
  else
  {
    // Пытаемся восстановить параметры из файла на диске
    res = Restore_settings_from_JSON_file(p_pars, 0, ptype);
    if (res == RES_OK)
    {
      nvr[ptype].settings_source = RESTORED_SETTINGS_FROM_JSON_FILE;
      res = Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0, ptype);
      if (res != RES_OK)
      {
        nvr[ptype].dataflash_saving_error = SAVED_TO_DATAFLASH_ERROR;
      }
      else
      {
        nvr[ptype].dataflash_saving_error = SAVED_TO_DATAFLASH_OK;
      }
      Delete_app_settings_file(ptype); // Не оставляем файлов с установками на диске
    }
    else
    {
      if (Restore_settings_from_DataFlash(p_pars, ptype) != RES_OK)
      {
        nvr[ptype].dataflash_restoring_error = 1;
        Return_def_params(p_pars);
        nvr[ptype].settings_source = RESTORED_DEFAULT_SETTINGS;
        res = Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0, ptype);
        if (res != RES_OK)
        {
          nvr[ptype].dataflash_saving_error = SAVED_TO_DATAFLASH_ERROR;
        }
        else
        {
          nvr[ptype].dataflash_saving_error = SAVED_TO_DATAFLASH_OK;
        }
      }
      else
      {
        nvr[ptype].dataflash_saving_error = SAVED_TO_DATAFLASH_NONE;
        nvr[ptype].settings_source = RESTORED_DEFAULT_SETTINGS;
      }
    }
  }
  return (RES_OK);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_pars  - указатель на структуру с конфигурацией параметров
  \param ptype   - тип рараметров. Параметры модуля или приложения

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Save_settings(const T_NV_parameters_instance *p_pars, uint8_t ptype)
{
  return  Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0, ptype);
}

/*-------------------------------------------------------------------------------------------
  Восстановить серийный номер, MAC адрес и другие постоянные величины из файла
---------------------------------------------------------------------------------------------*/
uint32_t Restore_settings_from_INI_file(const T_NV_parameters_instance  *p_pars, uint8_t ptype)
{
  FX_FILE            f;
  char              *str;
  char               *inbuf;
  char              *var_alias;
  char              *val;
  int32_t            n;
  uint32_t           pcnt;
  int32_t            scan_res;

  if (g_file_system_ready == 0) return RES_ERROR;
  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;

  str = App_malloc_pending(INI_STR_SIZE + 1,10);
  if (str == NULL)
  {
    return RES_ERROR;
  }

  inbuf = App_malloc_pending(TMP_BUF_SZ + 1,10);
  if (inbuf == NULL)
  {
    App_free(str);
    return RES_ERROR;
  }
  // Открываем файл

  if (fx_file_open(&fat_fs_media,&f, ini_fname[ptype],(FX_OPEN_FOR_READ)) != FX_SUCCESS)
  {
    App_free(str);
    App_free(inbuf);
    return RES_ERROR;
  }

  pcnt = 0;
  do
  {
    // Читаем строку из файла
    if (Scanf_from_file(&f,&scan_res, inbuf, TMP_BUF_SZ, "%s\n", str) == RES_ERROR) break;
    if (scan_res == 1)
    {
      if (str[0] == 0)  continue;
      if (str[0] == ';') continue;

      // Ищем указатель на символ =
      val = strchr(str, '=');
      if (val == NULL) continue; // Если символа не найдено то это не запись параметра
      *val = 0; // Вставляем 0 по адресу симвла = чтобы отделить стороки имени и значения
      val++; // Переходим на первый символ строки значения
      var_alias = str;
      // Найти параметр по аббревиатуре
      var_alias = Trim_and_dequote_str(var_alias);
      n = Find_param_by_alias(p_pars, var_alias);
      if (n >= 0)
      {
        val = Trim_and_dequote_str(val);
        Convert_str_to_parameter(p_pars, (uint8_t *)val, n);
        pcnt++;
      }
    }
  }while (1);


  fx_file_close(&f);
  App_free(str);
  App_free(inbuf);

  return RES_OK;
}

/*-------------------------------------------------------------------------------------------
   Процедура сохранения в ini-файл параметров
---------------------------------------------------------------------------------------------*/
uint32_t Save_settings_to_INI_file(const T_NV_parameters_instance *p_pars, uint8_t ptype)
{
  FX_FILE            f;
  uint32_t           res;
  uint32_t           i,n;
  char               *str;
  char               *inbuf;
  uint32_t           offs;
  char               *tmp_str;
  char               *name;
  char               *prev_name;
  const T_NV_parameters    *pp;

  if (g_file_system_ready == 0) return RES_ERROR;
  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;

  str = App_malloc_pending(INI_STR_SIZE + 1,10);
  if (str == NULL)
  {
    return RES_ERROR;
  }

  inbuf = App_malloc_pending(TMP_BUF_SZ + 1,10);
  if (inbuf == NULL)
  {
    App_free(str);
    return RES_ERROR;
  }
  // Открываем файл


  fx_file_delete(&fat_fs_media,ini_fname[ptype]);
  res = fx_file_create(&fat_fs_media,ini_fname[ptype]);
  if (res != FX_SUCCESS)
  {
    App_free(str);
    App_free(inbuf);
    return RES_ERROR;
  }
  res = fx_file_open(&fat_fs_media,&f, ini_fname[ptype],  FX_OPEN_FOR_WRITE);
  if (res != FX_SUCCESS)
  {
    App_free(str);
    App_free(inbuf);
    return RES_ERROR;
  }

  n = 0;
  name      = 0;
  prev_name = 0;
  for (i = 0; i < p_pars->items_num; i++)
  {
    pp = &p_pars->items_array[i];

    if ((pp->attr & 1) == 0) // сохраняем только если параметр для записи
    {
      offs = 0;

      // Параметр должен быть сохранен
      name = (char *)Get_mn_name(p_pars, pp->parmnlev);
      if (name != prev_name)
      {
        sprintf(inbuf + offs, ";--------------------------------------------------------\r\n");
        offs = offs + strlen(inbuf + offs);
        sprintf(inbuf + offs, "; %s\r\n", name);
        offs = offs + strlen(inbuf + offs);
        sprintf(inbuf + offs, ";--------------------------------------------------------\r\n\r\n");
        offs = offs + strlen(inbuf + offs);
        prev_name = name;
      }
      sprintf(inbuf + offs, "; N=%03d %s\r\n%s=", n++, pp->var_description, pp->var_alias);
      offs = offs + strlen(inbuf + offs);
      if (pp->vartype == tstring)
      {
        Convert_parameter_to_str(p_pars, (uint8_t *)(str + 1), MAX_PARAMETER_STRING_LEN, i);
        *str = '"';
        tmp_str = str + strlen(str);
        *tmp_str = '"';
        tmp_str++;
      }
      else
      {
        Convert_parameter_to_str(p_pars, (uint8_t *)str, MAX_PARAMETER_STRING_LEN, i);
        tmp_str = str + strlen(str);
      }
      *tmp_str = '\r';
      tmp_str++;
      *tmp_str = '\n';
      tmp_str++;
      *tmp_str = '\r';
      tmp_str++;
      *tmp_str = '\n';
      tmp_str++;
      *tmp_str = '\0';
      sprintf(inbuf + offs, "%s", str);
      offs = offs + strlen(inbuf + offs);

      if (fx_file_write(&f, inbuf, offs) != FX_SUCCESS)
      {
        fx_file_close(&f);
        App_free(str);
        App_free(inbuf);
        return RES_ERROR;
      }
    }
  }
  fx_file_close(&f);
  fx_media_flush(&fat_fs_media);
  App_free(str);
  App_free(inbuf);
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param file_name
  \param bud
  \param buf_sz
-----------------------------------------------------------------------------------------------------*/
static uint32_t Save_settings_buf_to_file(char *file_name, uint8_t *buf, ULONG buf_sz, uint8_t ptype)
{
  uint32_t   res = RES_ERROR;

  FX_FILE    f;
  char      *fname;

  if (g_file_system_ready == 0) return RES_ERROR;
  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;

  f.fx_file_id = 0;

  fname = file_name;
  // Открываем файл на запись
  if (file_name == 0)
  {
    if (ivar.en_compress_settins)
    {
      fname = json_compr_fname[ptype];
    }
    else
    {
      fname = json_fname[ptype];
    }
  }
  res = Recreate_file_for_write(&f, fname);
  if (res != FX_SUCCESS)
  {
    goto EXIT_ON_ERROR;
  }

  if (fx_file_write(&f, buf, buf_sz) != FX_SUCCESS) goto EXIT_ON_ERROR;

  res = RES_OK;
EXIT_ON_ERROR:
  if (f.fx_file_id == FX_FILE_ID) fx_file_close(&f);
  fx_media_flush(&fat_fs_media);

  return res;
}


/*-----------------------------------------------------------------------------------------------------
  Сохраняем настройки в две области памяти DataFlash

   Формат хранения параметров во Flash

  -------------
  4-е байта - размер данных N
  -------------
  4-е байта - счетчик количества модификаций данных сопровождавшихся перезаписью в DataFlash : g_setting_recordc_counter
  -------------
  N байт    - данные в виде сжатого JSON
  -------------
  4-е байта - контрольная сумма
  -------------


  \param buf
  \param buf_sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Save_settings_buf_to_DataFlash(uint8_t *buf, uint32_t buf_sz, uint8_t ptype)
{
  uint8_t   *tmp_buf = 0;
  uint32_t   tmp_buf_sz;
  uint16_t   crc;
  uint32_t   csz;

  uint32_t err_step = 1;

  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;


  if ((buf_sz > (DATAFLASH_PARAMS_AREA_SIZE - DATAFLASH_SUPLIMENT_AREA_SIZE)) || (buf_sz < 8)) goto EXIT_ON_ERROR;
  csz = buf_sz;
  if ((csz & 0x3) != 0) csz = (csz & 0xFFFFFFFC) + 4; // Выравниваем размер буфера данных по 4

  tmp_buf_sz = csz + DATAFLASH_SUPLIMENT_AREA_SIZE; // Добавляем размер буфера данных, номер записи и CRC

  err_step = 2;
  tmp_buf = App_malloc_pending(tmp_buf_sz,10);
  if (tmp_buf == NULL) goto EXIT_ON_ERROR;

  // Записываем служебные данные
  memcpy(&tmp_buf[0],&csz, 4);
  memcpy(&tmp_buf[8], buf, buf_sz);


  err_step = 3;
  // Стираем область памяти 1 хранения настроек
  if (DataFlash_bgo_EraseArea(df_params_addr[ptype][0],DATAFLASH_PARAMS_AREA_SIZE) != RES_OK) goto EXIT_ON_ERROR;
  err_step = 4;

  g_setting_wr_counters[ptype][0]++;
  memcpy(&tmp_buf[4],&g_setting_wr_counters[ptype][0], 4);
  crc = Get_CRC16_of_block(tmp_buf,tmp_buf_sz - 4, 0xFFFF);
  memcpy(&tmp_buf[8 + csz],&crc, 2);
  memcpy(&tmp_buf[10 + csz],&crc, 2);

  if (DataFlash_bgo_WriteArea(df_params_addr[ptype][0], tmp_buf, tmp_buf_sz) != RES_OK) goto EXIT_ON_ERROR;
  err_step = 5;
  // Стираем область памяти 2 хранения настроек
  if (DataFlash_bgo_EraseArea(df_params_addr[ptype][1],DATAFLASH_PARAMS_AREA_SIZE) != RES_OK) goto EXIT_ON_ERROR;
  err_step = 6;

  g_setting_wr_counters[ptype][1]++;
  memcpy(&tmp_buf[4],&g_setting_wr_counters[ptype][1], 4);
  crc = Get_CRC16_of_block(tmp_buf,tmp_buf_sz - 4, 0xFFFF);
  memcpy(&tmp_buf[8 + csz],&crc, 2);
  memcpy(&tmp_buf[10 + csz],&crc, 2);

  if (DataFlash_bgo_WriteArea(df_params_addr[ptype][1], tmp_buf, tmp_buf_sz) != RES_OK) goto EXIT_ON_ERROR;

  App_free(tmp_buf);
  return RES_OK;
EXIT_ON_ERROR:
  App_free(tmp_buf);
  return err_step;

}
/*-----------------------------------------------------------------------------------------------------
  Запись утстановок в JSON файл опционально с компрессией.
  Если имя файла не задано, то приеняется имена по умолчанию

  \param file_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Save_settings_to(const T_NV_parameters_instance *p_pars, uint8_t media_type,  char *file_name, uint8_t ptype)
{
  uint32_t   res;
  char      *json_str = NULL;
  uint32_t   json_str_sz;
  uint8_t   *compessed_data_ptr = 0;
  uint32_t   compessed_data_sz;
  uint8_t   *buf;
  uint32_t   buf_sz;
  uint32_t   flags;

  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;

  if (media_type == MEDIA_TYPE_DATAFLASH)
  {
    flags = JSON_COMPACT;
  }
  else if (ivar.en_formated_settings)
  {
    flags = JSON_INDENT(1) | JSON_ENSURE_ASCII;
  }
  else
  {
    flags = JSON_COMPACT;
  }


  if (Serialze_settings_to_mem(p_pars,&json_str,&json_str_sz, flags) != RES_OK) goto EXIT_ON_ERROR;

  if ((ivar.en_compress_settins) || (media_type == MEDIA_TYPE_DATAFLASH))
  {
    // Выделить память для сжатого файла
    compessed_data_ptr = App_malloc_pending(json_str_sz,10);
    if (compessed_data_ptr == NULL) goto EXIT_ON_ERROR;

    compessed_data_sz = json_str_sz;
    res = Compress_mem_to_mem(COMPR_ALG_SIXPACK, json_str, json_str_sz,compessed_data_ptr,&compessed_data_sz);
    if (res != RES_OK) goto EXIT_ON_ERROR;
    // Добавляем контрольную сумму
    uint16_t crc = Get_CRC16_of_block(compessed_data_ptr,compessed_data_sz, 0xFFFF);
    buf = compessed_data_ptr;
    buf_sz = compessed_data_sz;
    buf[buf_sz] = crc & 0xFF;
    buf[buf_sz + 1] = (crc >> 8) & 0xFF;
    buf_sz += 2;
  }
  else
  {
    buf = (uint8_t *)json_str;
    buf_sz = json_str_sz;
  }

  res = RES_OK;
  switch (media_type)
  {
  case MEDIA_TYPE_FILE:
    res = Save_settings_buf_to_file(file_name,buf, buf_sz, ptype);
    break;
  case MEDIA_TYPE_DATAFLASH:
    res = Save_settings_buf_to_DataFlash(buf, buf_sz, ptype);
    break;
  }

  App_free(compessed_data_ptr);
  if (json_str != 0) App_free(json_str);
  return res;
EXIT_ON_ERROR:
  App_free(compessed_data_ptr);
  if (json_str != 0) App_free(json_str);
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Восстановление установок из JSON файла, опционально с декомпрессией.
  Если имя файла не задано, то приеняется имена по умолчанию


  \param p_pars
  \param file_name
  \param ptype        - тип параметров

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Restore_settings_from_JSON_file(const T_NV_parameters_instance  *p_pars, char *file_name, uint8_t ptype)
{
  uint8_t   *data_buf = NULL;
  uint32_t   data_buf_sz;
  uint8_t   *decompessed_data_ptr = NULL;
  uint32_t   decompessed_data_sz;
  FX_FILE    *pf = NULL;
  ;
  uint32_t   res;
  uint32_t   actual_sz;
  uint8_t    compressed = 0;

  if (g_file_system_ready == 0) return RES_ERROR;
  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;

  pf = App_malloc_pending(sizeof(FX_FILE),10);
  if (pf == NULL) goto EXIT_ON_ERROR;

  // Открываем файл
  if (file_name == 0)
  {
    file_name  = json_compr_fname[ptype];
    res = fx_file_open(&fat_fs_media,pf, file_name,  FX_OPEN_FOR_READ);
    if (res != FX_SUCCESS)
    {
      file_name  = json_fname[ptype];
      res = fx_file_open(&fat_fs_media,pf, file_name,  FX_OPEN_FOR_READ);
      if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
    }
    else compressed = 1;
  }
  else
  {
    res = fx_file_open(&fat_fs_media,pf, file_name,  FX_OPEN_FOR_READ);
    if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
  }

  data_buf_sz = pf->fx_file_current_file_size;
  data_buf = App_malloc_pending(data_buf_sz + 1,10);
  if (data_buf == NULL) goto EXIT_ON_ERROR;

  actual_sz = 0;
  res = fx_file_read(pf, data_buf, data_buf_sz,(ULONG *)&actual_sz);
  if ((res != FX_SUCCESS) || (actual_sz != data_buf_sz)) goto EXIT_ON_ERROR;
  fx_file_close(pf);

  if (compressed)
  {
    // Проверка контрольной суммы
    uint16_t crc = Get_CRC16_of_block(data_buf,data_buf_sz - 2, 0xFFFF);
    uint16_t ecrc = data_buf[data_buf_sz - 2] + (data_buf[data_buf_sz - 1] << 8);
    if (crc != ecrc) goto EXIT_ON_ERROR; // Выход если не совпала контрольная сумма

    decompessed_data_sz = data_buf[0] + (data_buf[1] << 8) + (data_buf[2] << 16) + (data_buf[3] << 24);
    if (decompessed_data_sz > 65536) goto EXIT_ON_ERROR; // Выход если после декомпрессии объем данных слишком большой
    decompessed_data_ptr = App_malloc_pending(decompessed_data_sz + 1,10);
    if (decompessed_data_ptr == NULL) goto EXIT_ON_ERROR;
    // Декомпрессия
    if (Decompress_mem_to_mem(COMPR_ALG_SIXPACK, data_buf, data_buf_sz - 2, decompessed_data_ptr, decompessed_data_sz) != decompessed_data_sz) goto EXIT_ON_ERROR;

    App_free(data_buf);
    data_buf = 0;
    data_buf = decompessed_data_ptr;
    data_buf_sz = decompessed_data_sz;
    decompessed_data_ptr = 0;
  }

  data_buf[data_buf_sz] = 0; // Дополняем строку завершающим нулем

  // Парсим JSON формат данных
  res = JSON_Deser_settings(p_pars, (char *)data_buf);
  if (res == RES_OK)
  {
    fx_file_delete(&fat_fs_media, file_name);
  }
  App_free(decompessed_data_ptr);
  App_free(data_buf);
  App_free(pf);
  return res;

EXIT_ON_ERROR:
  if (pf->fx_file_id == FX_FILE_ID) fx_file_close(pf);
  App_free(decompessed_data_ptr);
  App_free(data_buf);
  App_free(pf);
  return RES_ERROR;

}

/*-----------------------------------------------------------------------------------------------------

  Формат хранения параметров во Flash

  -------------
  4-е байта - размер данных N
  -------------
  4-е байта - счетчик количества модификаций данных сопровождавшихся перезаписью в DataFlash : g_setting_recordc_counter
  -------------
  N байт    - данные в виде сжатого JSON
  -------------
  4-е байта - контрольная сумма
  -------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Restore_settings_from_DataFlash(const T_NV_parameters_instance  *p_pars, uint8_t ptype)
{
  uint32_t    done;
  uint32_t    sz;
  uint32_t    flash_addr;
  uint8_t    *buf = NULL;
  uint32_t    buf_sz;
  uint16_t    crc1, crc2;
  uint8_t    *decompessed_data_ptr = NULL;
  uint32_t    decompessed_data_sz;

  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;

  done = 0;
  // Проходим по всем областям в поисках годной
  for (uint32_t i = 0; i < 2; i++)
  {
    g_setting_start_condition[ptype][i] = 0;
    flash_addr = df_params_addr[ptype][i];
    // Читаем из  DataFlash размер блока данных
    DataFlash_bgo_ReadArea(flash_addr, (uint8_t *)&sz , 4);

    if ((sz > (DATAFLASH_PARAMS_AREA_SIZE - DATAFLASH_SUPLIMENT_AREA_SIZE)) || (sz < 8))
    {
      g_setting_start_condition[ptype][i] = 1;
      continue; // Ошибка - неправильный размер данных
    }

    buf_sz = sz + 8;
    buf = App_malloc_pending(buf_sz,10);
    if (buf != NULL)
    {
      // Читаем данные
      DataFlash_bgo_ReadArea(flash_addr  ,(uint8_t *)buf  ,  buf_sz);

      // Читаем записанную контрольную сумму
      DataFlash_bgo_ReadArea(flash_addr + 8 + sz,(uint8_t *)&crc1  ,  4);
      // Расчитываем фактическую контрольную сумму
      crc2 = Get_CRC16_of_block(buf, buf_sz, 0xFFFF);
      if (crc1 == crc2)
      {
        // Контрольные суммы совпали, значит величины номера записи и количества данных верны
        memcpy(&decompessed_data_sz,&buf[8], 4);
        if (decompessed_data_sz < 65536)
        {
          decompessed_data_ptr = App_malloc_pending(decompessed_data_sz + 1,10);
          if (decompessed_data_ptr != NULL)
          {
            // Декомпрессия
            if (Decompress_mem_to_mem(COMPR_ALG_SIXPACK,&buf[8], sz , decompessed_data_ptr, decompessed_data_sz) == decompessed_data_sz)
            {
              memcpy(&g_setting_wr_counters[ptype][i],&buf[4],4);
              App_free(buf);
              buf = 0;
              decompessed_data_ptr[decompessed_data_sz] = 0; // Дополняем нулем поскольку строка JSON должна завершаться всегда нулем
              if (JSON_Deser_settings(p_pars, (char *)decompessed_data_ptr) == RES_OK)
              {
                done = 1;
              }
              else
              {
                g_setting_start_condition[ptype][i] = 6;
              }
            }
            else
            {
              g_setting_start_condition[ptype][i] = 5;
            }
            App_free(decompessed_data_ptr);
          }
          else
          {
            g_setting_start_condition[ptype][i] = 4;
          }
        }
        else
        {
          g_setting_start_condition[ptype][i] = 3;
        }
      }
      else
      {
        g_setting_start_condition[ptype][i] = 2;
      }
      App_free(buf);
    }
  }

  if (done == 0)
  {
    return RES_ERROR;
  }
  else
  {
    return RES_OK;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param ptype
  \param sstate

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Check_settings_in_DataFlash(uint8_t ptype, T_settings_state *sstate)
{
  uint32_t    sz;
  uint32_t    flash_addr;
  uint8_t    *buf = NULL;
  uint32_t    buf_sz;
  uint16_t    crc1, crc2;


  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;


  for (uint32_t i = 0; i < 2; i++)
  {
    sstate->area_start_condition[i] = g_setting_start_condition[ptype][i];

    flash_addr = df_params_addr[ptype][i];

    if (DataFlash_bgo_BlankCheck(flash_addr, 8)== RES_OK)
    {
      sstate->area_sz[i]     = sz;
      sstate->area_state[i]  = SETT_IS_BLANK; // Ошибка - неправильный размер данных
      sstate->area_wr_cnt[i] = 0;
    }
    else
    {

      // Читаем из  DataFlash размер блока данных
      DataFlash_bgo_ReadArea(flash_addr ,(uint8_t *)&sz  ,  4);
      DataFlash_bgo_ReadArea(flash_addr + 4 ,(uint8_t *)&sstate->area_wr_cnt[i]  ,  4);

      if ((sz > (DATAFLASH_PARAMS_AREA_SIZE - DATAFLASH_SUPLIMENT_AREA_SIZE)) || (sz < 8))
      {
        sstate->area_sz[i] = sz;
        sstate->area_state[i] = SETT_WRONG_SIZE; // Ошибка - неправильный размер данных
      }
      else
      {
        buf_sz = sz + 8;
        sstate->area_sz[i] = buf_sz;
        buf = App_malloc_pending(buf_sz,10);
        if (buf != NULL)
        {
          // Читаем данные
          DataFlash_bgo_ReadArea(flash_addr  ,(uint8_t *)buf  ,  buf_sz);
          // Читаем записанную контрольную сумму
          DataFlash_bgo_ReadArea(flash_addr + 8 + sz,(uint8_t *)&crc1  ,  4);
          // Расчитываем фактическую контрольную сумму
          crc2 = Get_CRC16_of_block(buf, buf_sz, 0xFFFF);
          if (crc1 != crc2)
          {
            sstate->area_state[i] = SETT_WRONG_CRC;
          }
          else
          {
            sstate->area_state[i] = SETT_OK;
          }
          App_free(buf);
        }
        else
        {
          sstate->area_state[i] = SETT_WRONG_CHECK;
        }
      }
    }
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void Reset_settings_wr_counters(void)
{
  for (uint32_t i = 0; i < PARAMS_TYPES_NUM; i++)
  {
    for (uint32_t j = 0; j < 2; j++)
    {
      g_setting_wr_counters[i][j] = 0;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  Принять сертификат из файла

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Accept_certificates_from_file(void)
{
  uint8_t   *data_buf = NULL;
  uint32_t   file_size;
  FX_FILE    f;
  uint32_t   res;
  uint32_t   actual_sz;
  uint8_t   *ptr_sz;


  res = fx_file_open(&fat_fs_media,&f, CA_CERTIFICATE_FILE_NAME,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS)
  {
    goto EXIT_ON_ERROR;
  }

  file_size = f.fx_file_current_file_size;
  if (file_size > (DATAFLASH_CA_CERT_AREA_SIZE - 4)) goto EXIT_ON_ERROR;
  data_buf = App_malloc_pending(DATAFLASH_CA_CERT_AREA_SIZE,10);
  if (data_buf == NULL) goto EXIT_ON_ERROR;


  res = fx_file_read(&f, data_buf, file_size,(ULONG *)&actual_sz);
  if ((res != FX_SUCCESS) || (actual_sz != file_size)) goto EXIT_ON_ERROR;
  fx_file_close(&f);

  ptr_sz = (uint8_t *)(&data_buf[DATAFLASH_CA_CERT_AREA_SIZE - 4]);
  memcpy(ptr_sz,&file_size, 4);
  if (DataFlash_bgo_EraseArea(DATAFLASH_CA_CERT_ADDR,DATAFLASH_CA_CERT_AREA_SIZE) != RES_OK) goto EXIT_ON_ERROR;
  if (DataFlash_bgo_WriteArea(DATAFLASH_CA_CERT_ADDR, data_buf, DATAFLASH_CA_CERT_AREA_SIZE) != RES_OK) goto EXIT_ON_ERROR;

  App_free(data_buf);
  fx_file_delete(&fat_fs_media,CA_CERTIFICATE_FILE_NAME);
  return res;

EXIT_ON_ERROR:
  if (f.fx_file_id == FX_FILE_ID) fx_file_close(&f);
  App_free(data_buf);
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Delete_app_settings_file(uint8_t ptype)
{

  if (ptype >= PARAMS_TYPES_NUM) return RES_ERROR;

  fx_file_delete(&fat_fs_media, json_fname[ptype]);
  fx_file_delete(&fat_fs_media, json_compr_fname[ptype]);

  fx_media_flush(&fat_fs_media);
  Wait_ms(10);
  return RES_OK;
}




