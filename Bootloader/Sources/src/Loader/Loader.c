#include "S7V30.h"
#include "nx_crypto_rsa_sce.h"
#include "nx_crypto_aes_sce.h"
#include "nx_crypto_md5_sce.h"
#include "nx_crypto_sha2_sce.h"

extern uint32_t __region_VECT_start;
extern uint32_t __region_VECT_end;
extern uint32_t __region_ROMREG_start;
extern uint32_t __region_ROMREG_end;
extern uint32_t __region_FLASH_start;
extern uint32_t __region_FLASH_end;
extern uint32_t __region_RAM_start;
extern uint32_t __region_RAM_end;
extern uint32_t __region_DataFlash_start;
extern uint32_t __region_DataFlash_end;
extern uint32_t __region_SDRAM_start;
extern uint32_t __region_SDRAM_end;


#define  STATUS_FILE_PROGRAMED            0
#define  STATUS_LOADING_FIRWARE_FAIL      1
#define  STATUS_FLASHING_FIRMWARE_FAIL    2

#define FIRMWARE_FILE_NAME                "firmware.bin"
#define FIRMWARE_FILE_NAME_RENAMED        "firmware.flashed"
#define FIRMWARE_LOADING_FAIL_RENAME      "firmware.load_err"
#define FIRMWARE_PROGRAMING_FAIL_RENAME   "firmware.flash_err"

T_sys_timestump      t1;
T_sys_timestump      t2;
uint32_t             td;
T_loader_cbl         loader_cbl;

T_file_header        file_header __attribute__((aligned(4)));
T_data_header        data_header __attribute__((aligned(4)));
T_file_sign          file_sign   __attribute__((aligned(4)));
uint8_t              AES_TAG[AES_tag_SIZE] __attribute__((aligned(4)));
uint8_t              img_digest[IMG_DIGEST_BUF_SZ] __attribute__((aligned(4)));

#ifdef WRITE_EXTRACTED_IMAGE_TO_FILE
FX_FILE      debug_out_file;
#endif


uint32_t             New_sp;
uint32_t             New_pc; /* stack pointer and program counter */

__no_init uint8_t    rstsr0;
__no_init uint16_t   rstsr1;
__no_init uint8_t    rstsr2;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Check_boot_MassStorage_mode(void)
{
  if ((loader_info.boot_tag1 == REDY_TO_FS_MAGIC) &&  (loader_info.boot_tag2 == REDY_TO_FS_MAGIC) &&  (loader_info.boot_tag3 == REDY_TO_FS_MAGIC) &&  (loader_info.boot_tag4 == REDY_TO_FS_MAGIC))
  {
    return RES_OK;
  }
  else
  {
    return RES_ERROR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Clear_boot_MassStorage_mode(void)
{
  loader_info.boot_tag1 = 0;
  loader_info.boot_tag2 = 0;
  loader_info.boot_tag3 = 0;
  loader_info.boot_tag4 = 0;
}
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Set_boot_MassStorage_mode(void)
{
  loader_info.boot_tag1 = REDY_TO_FS_MAGIC;
  loader_info.boot_tag2 = REDY_TO_FS_MAGIC;
  loader_info.boot_tag3 = REDY_TO_FS_MAGIC;
  loader_info.boot_tag4 = REDY_TO_FS_MAGIC;
}

/*-----------------------------------------------------------------------------------------------------
  Выпоолняем это код после каждого сброса, до любых инициализаций периферии и окружения языка C

  \param void
-----------------------------------------------------------------------------------------------------*/
void Jump_to_app(void)
{
  rstsr0 = R_SYSTEM->RSTSR0; // Запоминаем состояния регистров описывающий причину рестарта
  rstsr1 = R_SYSTEM->RSTSR1;
  rstsr2 = R_SYSTEM->RSTSR2;

  R_SYSTEM->RSTSR0 = 0;
  R_SYSTEM->RSTSR1 = 0;
  R_SYSTEM->RSTSR2 = 0;

  // Проверяем требование перегрузки в режим загрузчика с включением USB MassStorage

  if (Check_boot_MassStorage_mode() == RES_ERROR)
  {
    // Проверяем установлины ли тэги.
    // Если они установлены то переходим к исполнению приложения
    if ((loader_info.tag1 == REDY_TO_BOOT_MAGIC) &&  (loader_info.tag2 == REDY_TO_BOOT_MAGIC) &&  (loader_info.tag3 == REDY_TO_BOOT_MAGIC) &&  (loader_info.tag4 == REDY_TO_BOOT_MAGIC))
    {
      // Здесь мы на втором сбросе последовавшем после первого, когда бутлоадер определил наличие приложения

      // Стираем тэги. Это служит признаком того что мы переходим к исполнению приложения
      loader_info.tag1 = 0;
      loader_info.tag2 = 0;
      loader_info.tag3 = 0;
      loader_info.tag4 = 0;


      // !!! От этого метода решено отказаться. Мапирование кода не работает с мапированием данных из Flash. Извлекаемые данные из области Flash не мапируются
      // Мапируем область с адреса 0x02000000 на Flash с адреса APP_IMG_START_ADDRESS
      //R_MMF->MMEN  =(0xDB << 24) | 1; // Разрешаем мапинг
      //R_MMF->MMSFR =(0xDB << 24) | APP_IMG_START_ADDRESS; // Мапируем

      /* Get PC and SP of application region */
      New_sp  = ((uint32_t *)(APP_IMG_START_ADDRESS))[0];
      New_pc  = ((uint32_t *)(APP_IMG_START_ADDRESS))[1];


      asm("mov32   r4,New_sp");
      asm("ldr     sp,[r4]");
      asm("mov32   r4,New_pc");
      asm("ldr     r5, [r4]");
      asm("blx     r5");
    }
    else
    {
      // Здесь если мы на первом сбросе

      // Запомним причину первого сброса
      loader_info.rstsr0  = rstsr0;
      loader_info.rstsr1  = rstsr1;
      loader_info.rstsr2  = rstsr2;
    }
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param status
-----------------------------------------------------------------------------------------------------*/
static void Rename_firmware_file(uint8_t status)
{
  // Переименовываем файл в соответствии с исходом программирования
  char *new_fname;
  if (status == STATUS_FILE_PROGRAMED)
  {
    new_fname =  FIRMWARE_FILE_NAME_RENAMED;
  }
  else if (status == STATUS_LOADING_FIRWARE_FAIL)
  {
    new_fname =  FIRMWARE_LOADING_FAIL_RENAME;
  }
  else if (status == STATUS_FLASHING_FIRMWARE_FAIL)
  {
    new_fname =  FIRMWARE_PROGRAMING_FAIL_RENAME;
  }
  else
  {
    return;
  }
  fx_file_delete(&fat_fs_media,new_fname);
  fx_file_rename(&fat_fs_media, FIRMWARE_FILE_NAME, new_fname);
  fx_media_flush(&fat_fs_media);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Open_firmware_file(FX_FILE *file_ptr)
{
  ULONG     actual_size;
  uint32_t  crc32;

  if (fx_file_open(&fat_fs_media, file_ptr, FIRMWARE_FILE_NAME,  FX_OPEN_FOR_READ) != FX_SUCCESS)
  {
    loader_cbl.flasher_err = FIRMWARE_FILE_NOT_FOUND;
    goto error;
  }

  if (file_ptr->fx_file_current_file_size <= (sizeof(T_file_header) + sizeof(T_data_header)))
  {
    fx_file_close(file_ptr);
    Rename_firmware_file(STATUS_LOADING_FIRWARE_FAIL);
    loader_cbl.flasher_err = IMAGE_SIZE_ERROR;
    goto error;
  }

  // Читаем заголовок файла и проверяем его

  if (fx_file_read(file_ptr,&file_header, sizeof(file_header),&actual_size) != FX_SUCCESS)
  {
    fx_file_close(file_ptr);
    loader_cbl.flasher_err = FIRMWARE_FILE_READ_ERROR;
    goto error;
  }
  if (actual_size != sizeof(file_header))
  {
    fx_file_close(file_ptr);
    Rename_firmware_file(STATUS_LOADING_FIRWARE_FAIL);
    loader_cbl.flasher_err = FIRMWARE_FILE_READ_ERROR;
    goto error;
  }
  crc32 = Get_CRC32((unsigned char *)&file_header, sizeof(file_header) - 4);
  if (crc32 != file_header.crc32)
  {
    fx_file_close(file_ptr);
    Rename_firmware_file(STATUS_LOADING_FIRWARE_FAIL);
    loader_cbl.flasher_err = FIRMWARE_FILE_HEADER_CRC_ERROR;
    goto error;
  }

  return RES_OK;
error:
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void Reverse_buf(uint32_t *dest_buf, uint32_t *src_buf, uint32_t sz_in_bytes)
{
  uint8_t *src = (uint8_t *)src_buf;
  uint8_t *dst = (uint8_t *)dest_buf;

  for (uint32_t i = 0; i < sz_in_bytes; i++)
  {
    dst[i] = src[sz_in_bytes - i - 1];
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Decrypt_image_sign(FX_FILE *file_ptr, T_file_header  *fh_ptr, T_file_sign *fs_prt)
{
  uint32_t  res;
  void     *data_block      = NULL;
  void     *decrypted_block = NULL;
  ULONG     actual_size;
  uint32_t  signed_block_size;


  signed_block_size = fh_ptr->sign_sz;

  res = fx_file_extended_seek(file_ptr,sizeof(T_file_header) + fh_ptr->img_sz);
  if (res != FX_SUCCESS) goto error;

  // Выделить память для блока данных подписи
  data_block =  App_malloc(signed_block_size * 2); // Выделяем место сразу для входного блока и для выходного
  if (data_block == NULL) goto error;

  res =  fx_file_read(file_ptr,data_block, signed_block_size,&actual_size);
  if (res != FX_SUCCESS) goto error;
  if (actual_size != signed_block_size) goto error;


  decrypted_block = (void *)&((uint8_t *)data_block)[signed_block_size];
  res = g_sce_rsa_2048.p_api->decrypt(g_sce_rsa_2048.p_ctrl, (uint32_t *)RSA_priv_exp_modul, NULL, signed_block_size / 4, data_block, decrypted_block);
  if (res != SSP_SUCCESS) goto error;

  memcpy((uint8_t *)fs_prt,&((uint8_t *)decrypted_block)[signed_block_size - sizeof(T_file_sign)] , sizeof(T_file_sign));

  App_free(data_block);
  return RES_OK;
error:
  App_free(data_block);
  loader_cbl.flasher_err = DECRYPT_IMAGE_SIGN_ERROR;
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param file_ptr
  \param fh_ptr
  \param fs_prt

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Decrypt_image_data_header(FX_FILE *file_ptr, T_file_header  *fh_ptr, T_file_sign *fs_prt, T_data_header *dh_ptr)
{
  uint32_t        res;
  ULONG           actual_size;
  uint32_t        cryptotext_sz;
  uint8_t        *cryptotext = NULL;
  uint32_t        crc;


  // Позиционируемся на начале зашифрованного заголовка
  res = fx_file_extended_seek(file_ptr,sizeof(T_file_header));
  if (res != FX_SUCCESS) goto error;

  cryptotext_sz = sizeof(T_data_header);
  cryptotext = (uint8_t *)App_malloc(cryptotext_sz);
  if (cryptotext == NULL) goto error;

  // Читаем зашифрованный блок
  res =  fx_file_read(file_ptr,cryptotext, cryptotext_sz,&actual_size);
  if (res != FX_SUCCESS) goto error;
  if (actual_size != cryptotext_sz) goto error;

  // Позиционируемся на позицию записи тэга
  res = fx_file_extended_seek(file_ptr,sizeof(T_file_header) + fh_ptr->img_sz - AES_tag_SIZE);
  if (res != FX_SUCCESS) goto error;
  // Читаем тэг
  res =  fx_file_read(file_ptr,AES_TAG, AES_tag_SIZE,&actual_size);
  if (res != FX_SUCCESS) goto error;
  if (actual_size != AES_tag_SIZE) goto error;

  if (fh_ptr->img_signed == 0)
  {
    memcpy(fs_prt->AES_key, AESKey         , AESKey_SIZE);
    memcpy(fs_prt->AES_iv , AES_init_vector, AES_init_vector_SIZE); // IV должен быть в RAM поскольку он модияфицируется движком шифрования
    memcpy(fs_prt->AES_aad, AES_aad        , AES_aad_SIZE);
  }

  // Расшифровываем блок

  // Модуль g_sce_aes_256_gcm уже открыт в функции sce_initialize
  //res = g_sce_aes_256_gcm.p_api->open(g_sce_aes_256_gcm.p_ctrl,g_sce_aes_256_gcm.p_cfg);
  //if (res != SSP_SUCCESS) goto error;
  res = g_sce_aes_256_gcm.p_api->setGcmTag(g_sce_aes_256_gcm.p_ctrl, AES_tag_SIZE / 4, (uint32_t *)AES_TAG);
  if (res != SSP_SUCCESS) goto error;
  res = g_sce_aes_256_gcm.p_api->zeroPaddingDecrypt(g_sce_aes_256_gcm.p_ctrl,(uint32_t const*)fs_prt->AES_key, (uint32_t*)fs_prt->AES_iv, AES_aad_SIZE , (uint32_t*)fs_prt->AES_aad, NULL);
  if (res != SSP_SUCCESS) goto error;
  res = g_sce_aes_256_gcm.p_api->zeroPaddingDecrypt(g_sce_aes_256_gcm.p_ctrl,(uint32_t const*)fs_prt->AES_key, (uint32_t*)fs_prt->AES_iv, cryptotext_sz, (uint32_t *)cryptotext, (uint32_t *)dh_ptr);
  if (res != SSP_SUCCESS) goto error;

  // Проверяем контрольную сумму заголовка данных
  crc = Get_CRC32((uint8_t *)dh_ptr, sizeof(T_data_header));
  if (crc != 0xFFFFFFFF) goto error;

  App_free(cryptotext);
  return RES_OK;
error:
  App_free(cryptotext);
  loader_cbl.flasher_err = DECRYPT_IMAGE_DATA_HEADER_ERROR;
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param file_ptr
  \param fh_ptr
  \param fs_prt
  \param dh_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Read_image_data_header(FX_FILE *file_ptr, T_data_header *dh_ptr)
{
  uint32_t        res;
  ULONG           actual_size;
  // Позиционируемся на начале зашифрованного заголовка
  res = fx_file_extended_seek(file_ptr,sizeof(T_file_header));
  if (res != FX_SUCCESS) goto error;

  // Читаем зашифрованный блок
  res =  fx_file_read(file_ptr,(void *)dh_ptr, sizeof(T_data_header),&actual_size);
  if (res != FX_SUCCESS) goto error;
  if (actual_size != sizeof(T_data_header)) goto error;
  return RES_OK;
error:
  loader_cbl.flasher_err = IMAGE_DATA_HEADER_READ_ERROR;
  return RES_ERROR;


}


/*-----------------------------------------------------------------------------------------------------


  \param file_ptr
  \param fh_ptr
  \param fs_prt
  \param dh_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Check_encrypted_file_digest(FX_FILE *file_ptr, T_file_header  *fh_ptr, T_file_sign *fs_prt, T_data_header *dh_ptr)
{
  uint32_t                 res;
  ULONG                    actual_size;
  uint32_t                 data_buf_sz;
  uint8_t                 *data_buf    = NULL;
  uint8_t                 *cypher_text = NULL;
  uint8_t                 *plain_text  = NULL;
  int32_t                  data_sz = 0;
  sf_crypto_data_handle_t  dh;



  // Позиционируемся на начале зашифрованных данных
  res = fx_file_extended_seek(file_ptr,sizeof(T_file_header) + sizeof(T_data_header));
  if (res != FX_SUCCESS) goto error;

  data_buf_sz = (FILE_CHANK_SZ * 2);
  data_buf = (uint8_t *)App_malloc(data_buf_sz);
  if (data_buf == NULL) goto error;

  cypher_text = data_buf;
  plain_text  = cypher_text + FILE_CHANK_SZ;

  data_sz = fh_ptr->img_sz - sizeof(T_data_header) - AES_tag_SIZE;
  memset(img_digest, 0, IMG_DIGEST_BUF_SZ);

  // Готовимся к вычислению хэша
  switch (dh_ptr->digest_type)
  {
  case DIGEST_ALG_CRC32  :
    Get_Init_CRC32((uint32_t *)img_digest);
    break;
  case DIGEST_ALG_MD5    :
    res = g_sf_crypto_hash_md5.p_api->open(g_sf_crypto_hash_md5.p_ctrl, g_sf_crypto_hash_md5.p_cfg);
    if (res != SSP_SUCCESS) goto error;
    res = g_sf_crypto_hash_md5.p_api->hashInit(g_sf_crypto_hash_md5.p_ctrl);
    if (res != SSP_SUCCESS) goto error;
    break;
  case DIGEST_ALG_SHA256 :
    res = g_sf_crypto_hash_sha256.p_api->open(g_sf_crypto_hash_sha256.p_ctrl, g_sf_crypto_hash_sha256.p_cfg);
    if (res != SSP_SUCCESS) goto error;
    res = g_sf_crypto_hash_sha256.p_api->hashInit(g_sf_crypto_hash_sha256.p_ctrl);
    if (res != SSP_SUCCESS) goto error;
    break;
  case DIGEST_ALG_SHA512 :
    goto error;
  }

  // Читаем зашифрованные данные, расшифровываем и вычисляем хэш
  while (data_sz > 0)
  {
    actual_size = FILE_CHANK_SZ;
    if (actual_size > data_sz) actual_size = data_sz;
    res =  fx_file_read(file_ptr,cypher_text, actual_size,&actual_size);
    if (res != FX_SUCCESS) goto error1;
    if (actual_size == 0) break;
    if ((actual_size % 4) != 0) goto error1;

    res = g_sce_aes_256_gcm.p_api->zeroPaddingDecrypt(g_sce_aes_256_gcm.p_ctrl,(uint32_t const *)fs_prt->AES_key, (uint32_t*)fs_prt->AES_iv, actual_size, (uint32_t *)cypher_text, (uint32_t *)plain_text);
    if (res != SSP_SUCCESS) goto error1;

    // Вычисляем хэш
    switch (dh_ptr->digest_type)
    {
    case DIGEST_ALG_CRC32  :
      Update_CRC32((uint32_t *)img_digest,plain_text,actual_size);
      break;
    case DIGEST_ALG_MD5    :
      dh.data_length = actual_size;
      dh.p_data      = plain_text;
      res = g_sf_crypto_hash_md5.p_api->hashUpdate(g_sf_crypto_hash_md5.p_ctrl,&dh);
      if (res != SSP_SUCCESS) goto error1;
      break;
    case DIGEST_ALG_SHA256 :
      dh.data_length = actual_size;
      dh.p_data      = plain_text;
      res = g_sf_crypto_hash_sha256.p_api->hashUpdate(g_sf_crypto_hash_sha256.p_ctrl,&dh);
      if (res != SSP_SUCCESS) goto error1;
      break;
    case DIGEST_ALG_SHA512 :
      break;
    }

    data_sz -= actual_size;
  }

  // Завершаем вычисление хэша
  switch (dh_ptr->digest_type)
  {
  case DIGEST_ALG_CRC32  :
    Get_finish_CRC32((uint32_t *)img_digest);
    break;
  case DIGEST_ALG_MD5    :
    dh.data_length = DIGEST_MD5_SZ;
    dh.p_data      = img_digest;
    res = g_sf_crypto_hash_md5.p_api->hashFinal(g_sf_crypto_hash_md5.p_ctrl,&dh, (uint32_t *)&actual_size);
    if (res != SSP_SUCCESS) goto error1;
    g_sf_crypto_hash_md5.p_api->close(g_sf_crypto_hash_md5.p_ctrl);
    break;
  case DIGEST_ALG_SHA256 :
    dh.data_length = DIGEST_SHA256_SZ;
    dh.p_data      = img_digest;
    res = g_sf_crypto_hash_sha256.p_api->hashFinal(g_sf_crypto_hash_sha256.p_ctrl,&dh, (uint32_t *)&actual_size);
    if (res != SSP_SUCCESS) goto error1;
    g_sf_crypto_hash_sha256.p_api->close(g_sf_crypto_hash_sha256.p_ctrl);
    break;
  case DIGEST_ALG_SHA512 :
    break;
  }

  // Сравниваем хеши
  if (memcmp(img_digest, dh_ptr->digest, IMG_DIGEST_BUF_SZ) != 0) goto error;


  App_free(data_buf);
  return RES_OK;

  error1:

  // Закрываем движки хэшей
  switch (dh_ptr->digest_type)
  {
  case DIGEST_ALG_MD5    :
    g_sf_crypto_hash_md5.p_api->close(g_sf_crypto_hash_md5.p_ctrl);
    break;
  case DIGEST_ALG_SHA256 :
    g_sf_crypto_hash_sha256.p_api->close(g_sf_crypto_hash_sha256.p_ctrl);
    break;
  }

  error:
  loader_cbl.flasher_err = DIGEST_CHECK_ERROR;
  App_free(data_buf);
  return RES_ERROR;

}

/*-----------------------------------------------------------------------------------------------------


  \param file_ptr
  \param fh_ptr
  \param fs_prt
  \param dh_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Check_plain_file_digest(FX_FILE *file_ptr, T_file_header  *fh_ptr, T_data_header *dh_ptr)
{
  uint32_t                 res;
  ULONG                    actual_size;
  uint32_t                 data_buf_sz;
  uint8_t                 *data_buf    = NULL;
  uint8_t                 *chank_buf = NULL;
  int32_t                  data_sz = 0;
  sf_crypto_data_handle_t  dh;



  // Позиционируемся на начале зашифрованных данных
  res = fx_file_extended_seek(file_ptr,sizeof(T_file_header) + sizeof(T_data_header));
  if (res != FX_SUCCESS) goto error;

  data_buf_sz = (FILE_CHANK_SZ);
  data_buf = (uint8_t *)App_malloc(data_buf_sz);
  if (data_buf == NULL) goto error;

  chank_buf = data_buf;


  data_sz = fh_ptr->img_sz - sizeof(T_data_header);
  memset(img_digest, 0, IMG_DIGEST_BUF_SZ);

  // Готовимся к вычислению хэша
  switch (dh_ptr->digest_type)
  {
  case DIGEST_ALG_CRC32  :
    Get_Init_CRC32((uint32_t *)img_digest);
    break;
  case DIGEST_ALG_MD5    :
    res = g_sf_crypto_hash_md5.p_api->open(g_sf_crypto_hash_md5.p_ctrl, g_sf_crypto_hash_md5.p_cfg);
    if (res != SSP_SUCCESS) goto error;
    res = g_sf_crypto_hash_md5.p_api->hashInit(g_sf_crypto_hash_md5.p_ctrl);
    if (res != SSP_SUCCESS) goto error;
    break;
  case DIGEST_ALG_SHA256 :
    res = g_sf_crypto_hash_sha256.p_api->open(g_sf_crypto_hash_sha256.p_ctrl, g_sf_crypto_hash_sha256.p_cfg);
    if (res != SSP_SUCCESS) goto error;
    res = g_sf_crypto_hash_sha256.p_api->hashInit(g_sf_crypto_hash_sha256.p_ctrl);
    if (res != SSP_SUCCESS) goto error;
    break;
  case DIGEST_ALG_SHA512 :
    goto error;
  }

  // Читаем незашифрованные данные и вычисляем хэш
  while (data_sz > 0)
  {
    actual_size = FILE_CHANK_SZ;
    if (actual_size > data_sz) actual_size = data_sz;
    res =  fx_file_read(file_ptr,chank_buf, actual_size,&actual_size);
    if (res != FX_SUCCESS) goto error1;
    if (actual_size == 0) break;
    if ((actual_size % 4) != 0) goto error1;

    // Вычисляем хэш
    switch (dh_ptr->digest_type)
    {
    case DIGEST_ALG_CRC32  :
      Update_CRC32((uint32_t *)img_digest,chank_buf,actual_size);
      break;
    case DIGEST_ALG_MD5    :
      dh.data_length = actual_size;
      dh.p_data      = chank_buf;
      res = g_sf_crypto_hash_md5.p_api->hashUpdate(g_sf_crypto_hash_md5.p_ctrl,&dh);
      if (res != SSP_SUCCESS) goto error1;
      break;
    case DIGEST_ALG_SHA256 :
      dh.data_length = actual_size;
      dh.p_data      = chank_buf;
      res = g_sf_crypto_hash_sha256.p_api->hashUpdate(g_sf_crypto_hash_sha256.p_ctrl,&dh);
      if (res != SSP_SUCCESS) goto error1;
      break;
    case DIGEST_ALG_SHA512 :
      break;
    }

    data_sz -= actual_size;
  }

  // Завершаем вычисление хэша
  switch (dh_ptr->digest_type)
  {
  case DIGEST_ALG_CRC32  :
    Get_finish_CRC32((uint32_t *)img_digest);
    break;
  case DIGEST_ALG_MD5    :
    dh.data_length = DIGEST_MD5_SZ;
    dh.p_data      = img_digest;
    res = g_sf_crypto_hash_md5.p_api->hashFinal(g_sf_crypto_hash_md5.p_ctrl,&dh, (uint32_t *)&actual_size);
    if (res != SSP_SUCCESS) goto error1;
    g_sf_crypto_hash_md5.p_api->close(g_sf_crypto_hash_md5.p_ctrl);
    break;
  case DIGEST_ALG_SHA256 :
    dh.data_length = DIGEST_SHA256_SZ;
    dh.p_data      = img_digest;
    res = g_sf_crypto_hash_sha256.p_api->hashFinal(g_sf_crypto_hash_sha256.p_ctrl,&dh, (uint32_t *)&actual_size);
    if (res != SSP_SUCCESS) goto error1;
    g_sf_crypto_hash_sha256.p_api->close(g_sf_crypto_hash_sha256.p_ctrl);
    break;
  case DIGEST_ALG_SHA512 :
    break;
  }

  // Сравниваем хеши
  if (memcmp(img_digest, dh_ptr->digest, IMG_DIGEST_BUF_SZ) != 0) goto error;


  App_free(data_buf);
  return RES_OK;

error1:

  // Закрываем движки хэшей
  switch (dh_ptr->digest_type)
  {
  case DIGEST_ALG_MD5    :
    g_sf_crypto_hash_md5.p_api->close(g_sf_crypto_hash_md5.p_ctrl);
    break;
  case DIGEST_ALG_SHA256 :
    g_sf_crypto_hash_sha256.p_api->close(g_sf_crypto_hash_sha256.p_ctrl);
    break;
  }

error:
  App_free(data_buf);
  loader_cbl.flasher_err = DIGEST_CHECK_ERROR;
  return RES_ERROR;

}


/*-----------------------------------------------------------------------------------------------------


   Схема использования динамической памяти:

   Без шифрования и сжатия:
   File -> BUF1(64K) -> Flash

   С шифрованием без сжатия
   File -> BUF1(64K) -> расшифровка -> BUF2(64K) -> Flash

   С сжатием без шифрования
   File -> BUF1(64K+10K) -> распаковка  -> BUF2(64K) -> Flash

   С шифрованием и сжатием
   File -> BUF1(64K+10K) -> засшифровка -> BUF2(64K+10K) -> распаковка -> BUF1(64K) ->Flash


   Результаты тестирования для загрузчика скомпилированного без оптимизации по скорости:
   Загрузка, расшифровка и распаковка образа размером 0x00200000 (2097152) байт

   - с сжатием по алгоритму SIXPACK (данные сжаты до размера 1053024), с шифрованием AES256 GCM, с хэшем SHA256, с подписью RSA 2048 длится   10.80  сек
   - с сжатием по алгоритму SIXPACK (данные сжаты до размера 1053024), с шифрованием AES256 GCM, с хэшем SHA256, без подписи длится           10.30  сек
   - с сжатием по алгоритму LZSS (данные сжаты до размера 1245808), с шифрованием AES256 GCM, с хэшем SHA256, с подписью RSA 2048 длится       1.57  сек
   - без сжатия, с шифрованием AES256 GCM, с хэшем MD5, с подписью RSA 2048 длится                                                             0.89  сек
   - без сжатия, с шифрованием AES256 GCM, с хэшем SHA256, с подписью RSA 2048 длится                                                          1.04  сек
   - без сжатия, с шифрованием AES256 GCM, с хэшем CRC32, с подписью RSA 2048 длится                                                           1.37  сек
   - без сжатия, без шифрования, с хэшем MD5, без подписи длится                                                                               0.84  сек

   Результаты тестирования для загрузчика скомпилированного с максимальной оптимизации по размеру:
   Загрузка, расшифровка и распаковка образа размером 0x00200000 (2097152) байт

   - с сжатием по алгоритму SIXPACK (данные сжаты до размера 1053024), с шифрованием AES256 GCM, с хэшем SHA256, с подписью RSA 2048 длится    6.53  сек

   Результаты тестирования для загрузчика скомпилированного с максимальной оптимизации по скорости:
   Загрузка, расшифровка и распаковка образа размером 0x00200000 (2097152) байт

   - с сжатием по алгоритму SIXPACK (данные сжаты до размера 1053024), с шифрованием AES256 GCM, с хэшем SHA256, с подписью RSA 2048 длится    5.88  сек
   - без сжатия, без шифрования, с хэшем MD5, без подписи длится                                                                               0.52  сек


  \param file_ptr
  \param fh_ptr
  \param dh_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Flash_image(FX_FILE *file_ptr,  T_file_sign *fs_prt, T_file_header  *fh_ptr, T_data_header *dh_ptr)
{
  uint8_t   *buf1     = NULL;
  uint8_t   *buf2     = NULL;
  int32_t    compr_block_sz;
  int32_t    decompr_block_sz;
  uint32_t   n;
  uint32_t   tail_sz  = 0;
  uint32_t   prev_tail_sz  = 0;
  uint8_t    tail[32];
  uint32_t   res;
  ULONG      actual_size;
  int32_t    sz;

  uint8_t     *inp_ptr   = NULL;
  uint8_t     *out_ptr   = NULL;
  uint8_t     *pack_ptr  = NULL;

  uint8_t   *flash_buf_prt;
  uint32_t   flash_buf_size;
  uint32_t   flashed_sz = 0;

  uint32_t   calculated_crc;
  uint32_t   flash_crc;



  #ifdef WRITE_EXTRACTED_IMAGE_TO_FILE
  res = Recreate_file_for_write(&debug_out_file, "firmware.ext");
  if (res != FX_SUCCESS) goto error;
  #endif


  // Выделем блок памяти для распакованных данных

  buf1 = (uint8_t *)App_malloc(FILE_CHANK_SZ + 10 * 1024);
  if (buf1 == NULL) goto error;
  buf2 = (uint8_t *)App_malloc(FILE_CHANK_SZ + 10 * 1024);
  if (buf2 == NULL) goto error;


  // Подготавливаем движок шифрования
  if (fh_ptr->img_encrypted == 1)
  {
    res = g_sce_aes_256_gcm.p_api->close(g_sce_aes_256_gcm.p_ctrl);
    if (res != SSP_SUCCESS) goto error;
    res = g_sce_aes_256_gcm.p_api->open(g_sce_aes_256_gcm.p_ctrl,g_sce_aes_256_gcm.p_cfg);
    if (res != SSP_SUCCESS) goto error;
    res = g_sce_aes_256_gcm.p_api->setGcmTag(g_sce_aes_256_gcm.p_ctrl, AES_tag_SIZE / 4, (uint32_t *)AES_TAG);
    if (res != SSP_SUCCESS) goto error;
    fs_prt->AES_iv[12] = 0;
    fs_prt->AES_iv[13] = 0;
    fs_prt->AES_iv[14] = 0;
    fs_prt->AES_iv[15] = 1;
    res = g_sce_aes_256_gcm.p_api->zeroPaddingDecrypt(g_sce_aes_256_gcm.p_ctrl,(uint32_t const*)fs_prt->AES_key, (uint32_t*)fs_prt->AES_iv, AES_aad_SIZE , (uint32_t*)fs_prt->AES_aad, NULL);
    if (res != SSP_SUCCESS) goto error;

    // Поскольку расшифровка валидна только если расшифровывать с самого начала, то расшифруем заголовок данных

    // Позиционируемся на начале заголовка  данных
    res = fx_file_extended_seek(file_ptr,sizeof(T_file_header));
    if (res != FX_SUCCESS) goto error;
    res =  fx_file_read(file_ptr,(void *)buf1, sizeof(T_data_header),&actual_size);
    if (res != FX_SUCCESS) goto error;
    if (actual_size != sizeof(T_data_header)) goto error;

    res = g_sce_aes_256_gcm.p_api->zeroPaddingDecrypt(g_sce_aes_256_gcm.p_ctrl,(uint32_t const *)fs_prt->AES_key, (uint32_t*)fs_prt->AES_iv, sizeof(T_data_header), (uint32_t *)buf1, (uint32_t *)buf2);
    if (res != SSP_SUCCESS) goto error;
  }
  else
  {
    // Позиционируемся на начале блока данных
    res = fx_file_extended_seek(file_ptr,sizeof(T_file_header) + sizeof(T_data_header));
    if (res != FX_SUCCESS) goto error;
  }

  do
  {
    GREEN_LED   = 0;
    //.............................................................................................................
    // Считывание блоков данных из файла
    //.............................................................................................................

    if ((fh_ptr->img_compressed == 1) && (fh_ptr->img_encrypted == 1))
    {
      // Схема перемещения данных: File -> BUF1(64K+10K) -> засшифровка -> BUF2(64K+10K) -> распаковка -> BUF1(64K) ->Flash

      if (prev_tail_sz < 8)
      {
        // Предыдцщий хвост слишком мал и включает не все служебные данные о размере сжатого и несжатого представлений данных
        // поэтому читаем дополнительно MIN_DATA_UNIT_SIZE байт чтобы получить все служебные данные для распаковки
        inp_ptr = buf1;
        out_ptr = buf2 + MIN_DATA_UNIT_SIZE;  // В выходном буфере оставляем спереди место для хвоста от предыдущего блока
        res =  fx_file_read(file_ptr,(void *)inp_ptr, MIN_DATA_UNIT_SIZE ,&actual_size);
        if (res != FX_SUCCESS) goto error;
        if (actual_size != MIN_DATA_UNIT_SIZE) goto error;

        // Расшифровываем MIN_DATA_UNIT_SIZE байт в BUF2
        res = g_sce_aes_256_gcm.p_api->zeroPaddingDecrypt(g_sce_aes_256_gcm.p_ctrl,(uint32_t const *)fs_prt->AES_key, (uint32_t*)fs_prt->AES_iv, MIN_DATA_UNIT_SIZE, (uint32_t *)inp_ptr, (uint32_t *)out_ptr);
        if (res != SSP_SUCCESS) goto error;

        // Хвост содержит служебные данные о размерах упакованного и распакованного блока
        // Дописываем расшифрованные данные к хвосту и извлекаем служебные данные находящиеся в начале хвоста
        memcpy(tail+prev_tail_sz, out_ptr, MIN_DATA_UNIT_SIZE);

        memcpy(& compr_block_sz, tail, 4);
        if (compr_block_sz == 0) break;    // Нулевое значение сжатого блока обозначает окончание данных
        memcpy(& decompr_block_sz, tail+4, 4);
        if (decompr_block_sz > FILE_CHANK_SZ) goto error;

        // Остаток хвоста с упакованными данными перемещаем в начало блока из которого будет происходить распаковывка
        out_ptr = buf2 + MIN_DATA_UNIT_SIZE -(prev_tail_sz + 8);
        memcpy(out_ptr, tail + 8 , prev_tail_sz + 8);

        // Определяем сколько данных надо прочитать с учетом выравнивания по границе MIN_DATA_UNIT_SIZE байт, уже прочитанных данных и отстатка данных от предыдущего блока
        n = compr_block_sz -(8 + prev_tail_sz);
        if ((n % MIN_DATA_UNIT_SIZE) != 0)
        {
          tail_sz = MIN_DATA_UNIT_SIZE - (n % MIN_DATA_UNIT_SIZE);
        }
        else
        {
          tail_sz = 0;
        }
        n = n + tail_sz;

        // Подготавливаем указатели
        inp_ptr  = buf1;     // В этот адрес читаем зашифрованные данные
        pack_ptr = out_ptr;  // С этого адреса начинается распаковка
        out_ptr  = buf2 + MIN_DATA_UNIT_SIZE;  // В этот адрес производится расшифровка
      }
      else
      {
        memcpy(&compr_block_sz, tail, 4);
        if (compr_block_sz == 0) break;    // Нулевое значение сжатого блока обозначает окончание данных
        memcpy(&decompr_block_sz, tail + 4, 4);
        if (decompr_block_sz > FILE_CHANK_SZ) goto error;

        // Остаток хвоста с упакованными данными перемещаем в начало блока из которого будет происходить распаковывка
        out_ptr = buf2 + MIN_DATA_UNIT_SIZE - (prev_tail_sz - 8);
        memcpy(out_ptr, tail + 8 ,(prev_tail_sz - 8));

        // Определяем сколько данных надо прочитать с учетом выравнивания по границе MIN_DATA_UNIT_SIZE байт и отстатка данных от предыдущего блока
        n = compr_block_sz - (prev_tail_sz - 8);
        if ((n % MIN_DATA_UNIT_SIZE) != 0)
        {
          tail_sz = MIN_DATA_UNIT_SIZE - (n % MIN_DATA_UNIT_SIZE);
        }
        else
        {
          tail_sz = 0;
        }
        n = n + tail_sz;

        // Подготавливаем указатели
        inp_ptr  = buf1;     // В этот адрес читаем зашифрованные данные
        pack_ptr = out_ptr;  // С этого адреса начинается распаковка
        out_ptr  = buf2 + MIN_DATA_UNIT_SIZE;  // В этот адрес производится расшифровка
      }

      // Читаем зашифрованные и запакованные данные в BUF1
      res =  fx_file_read(file_ptr,(void *)inp_ptr, n,&actual_size);
      if (res != FX_SUCCESS) goto error;
      if (actual_size != n) goto error;

      // Расшифровываем прочитанный блок в BUF2
      res = g_sce_aes_256_gcm.p_api->zeroPaddingDecrypt(g_sce_aes_256_gcm.p_ctrl,(uint32_t const *)fs_prt->AES_key,(uint32_t*)fs_prt->AES_iv, n,(uint32_t *)inp_ptr,(uint32_t *)out_ptr);
      if (res != SSP_SUCCESS) goto error;

      // Копируем расшифрованный хвост во временный буфер
      if (tail_sz != 0) memcpy(tail, buf2 + MIN_DATA_UNIT_SIZE + n - tail_sz, tail_sz);

      //memset(buf1, 0, decompr_block_sz);
      sz = Decompress_mem_to_mem(dh_ptr->compression_alg, pack_ptr , compr_block_sz, buf1, decompr_block_sz);
      if (sz != decompr_block_sz) goto error;

      prev_tail_sz = tail_sz;

      flash_buf_prt  = buf1;
      flash_buf_size = decompr_block_sz;

    }
    else if ((fh_ptr->img_compressed == 1) && (fh_ptr->img_encrypted == 0))
    {
      res =  fx_file_read(file_ptr,(void *)buf1, 8,&actual_size);
      if (res != FX_SUCCESS) goto error;
      if (actual_size != 8) goto error;

      memcpy(&compr_block_sz, buf1, 4);
      if (compr_block_sz == 0) break;    // Нулевое значение сжатого блока обозначает окончание данных
      memcpy(&decompr_block_sz, buf1 + 4, 4);
      if (decompr_block_sz > FILE_CHANK_SZ) goto error;

      res =  fx_file_read(file_ptr,(void *)buf1, compr_block_sz,&actual_size);
      if (res != FX_SUCCESS) goto error;
      if (actual_size != compr_block_sz) goto error;

      sz = Decompress_mem_to_mem(dh_ptr->compression_alg, buf1 , compr_block_sz, buf2, decompr_block_sz);
      if (sz != decompr_block_sz) goto error;

      flash_buf_prt  = buf2;
      flash_buf_size = decompr_block_sz;
    }
    else if ((fh_ptr->img_compressed == 0) && (fh_ptr->img_encrypted == 1))
    {
      if (dh_ptr->orig_image_size <= flashed_sz) goto error;
      if ((dh_ptr->orig_image_size - flashed_sz) < FILE_CHANK_SZ)
      {
        decompr_block_sz = dh_ptr->orig_image_size - flashed_sz;
        if ((decompr_block_sz % MIN_DATA_UNIT_SIZE) != 0) tail_sz = MIN_DATA_UNIT_SIZE - (n % MIN_DATA_UNIT_SIZE);
        decompr_block_sz += tail_sz;
      }
      else
      {
        decompr_block_sz = FILE_CHANK_SZ;
      }
      res =  fx_file_read(file_ptr,(void *)buf1, decompr_block_sz,&actual_size);
      if (res != FX_SUCCESS) goto error;
      if (actual_size != decompr_block_sz) goto error;

      // Расшифровываем прочитанный блок в BUF2
      res = g_sce_aes_256_gcm.p_api->zeroPaddingDecrypt(g_sce_aes_256_gcm.p_ctrl,(uint32_t const *)fs_prt->AES_key,(uint32_t*)fs_prt->AES_iv, decompr_block_sz,(uint32_t *)buf1,(uint32_t *)buf2);
      if (res != SSP_SUCCESS) goto error;

      flash_buf_prt  = buf2;
      flash_buf_size = decompr_block_sz;
    }
    else if ((fh_ptr->img_compressed == 0) && (fh_ptr->img_encrypted == 0))
    {
      if (dh_ptr->orig_image_size <= flashed_sz) goto error;
      if ((dh_ptr->orig_image_size - flashed_sz) < FILE_CHANK_SZ)
      {
        decompr_block_sz = dh_ptr->orig_image_size - flashed_sz;
        if ((decompr_block_sz % MIN_DATA_UNIT_SIZE) != 0) tail_sz = MIN_DATA_UNIT_SIZE - (n % MIN_DATA_UNIT_SIZE);
        decompr_block_sz += tail_sz;
      }
      else
      {
        decompr_block_sz = FILE_CHANK_SZ;
      }
      res =  fx_file_read(file_ptr,(void *)buf1, decompr_block_sz,&actual_size);
      if (res != FX_SUCCESS) goto error;
      if (actual_size != decompr_block_sz) goto error;

      flash_buf_prt  = buf1;
      flash_buf_size = decompr_block_sz;
    }

    // Здесь имеем буфер с данными для программирования flash_buf_prt, flash_buf_size


    #ifdef WRITE_EXTRACTED_IMAGE_TO_FILE
    res = fx_file_write(&debug_out_file,flash_buf_prt,flash_buf_size);
    if (res != FX_SUCCESS)
    {
      fx_file_close(&debug_out_file);
      goto error;
    }
    #endif

    GREEN_LED   = 1;

    //.............................................................................................................
    // Флеширование блока данных
    //.............................................................................................................
    Switch_Flash_driver_to_no_bgo();

    int32_t              section_sz;
    flash_result_t       flash_res;

    sz                     = flash_buf_size;
    uint32_t flash_address = dh_ptr->start_address + flashed_sz;

    // Стираем и программируем секциями размером CODE_FLASH_EBLOCK_SZ, как указано в спецификации на Flash память чипа
    // Предполагаем, что начало данных всегда выровнено по границе CODE_FLASH_EBLOCK_SZ
    // Получаемые блоки всегда размером FILE_CHANK_SZ кроме последнего
    do
    {
      section_sz = CODE_FLASH_EBLOCK_SZ;
      if (section_sz > sz) section_sz = sz;

      // Проверяем на чистоту
      __disable_interrupt();
      res = Flash_blank_check(flash_address, section_sz,&flash_res);
      __enable_interrupt();
      if (flash_res != FLASH_RESULT_BLANK)
      {
        BLUE_LED   = 0;
        __disable_interrupt();
        res = Flash_erase_block(flash_address, 1);
        __enable_interrupt();
        if (res != SSP_SUCCESS)
        {
          Switch_Flash_driver_to_bgo();
          goto error;
        }
        BLUE_LED   = 1;
      }
      // Проверяем данные на чистоту, если данные чистые, то не программируем
      for (uint32_t i = 0; i < section_sz; i++)
      {
        if (flash_buf_prt[i] != 0xFF)
        {
          // Программируем
          // Размер программируемых данных должен быть кратным размеру CODE_FLASH_WR_SZ
          if ((section_sz % CODE_FLASH_WR_SZ) != 0)
          {
            // Дописываем в буфер значение 0xFF
            memset(&flash_buf_prt[section_sz], 0xFF, CODE_FLASH_WR_SZ - (section_sz % CODE_FLASH_WR_SZ));
            // Корректируем размер секции
            section_sz = section_sz + (CODE_FLASH_WR_SZ - (section_sz % CODE_FLASH_WR_SZ));
          }
          RED_LED   = 0;
          __disable_interrupt();
          res = Flash_write_block((uint32_t)flash_buf_prt, flash_address, section_sz);
          __enable_interrupt();
          if (res != SSP_SUCCESS)
          {
            Switch_Flash_driver_to_bgo();
            goto error;
          }
          RED_LED   = 1;
          break;
        }
      }

      flash_buf_prt += section_sz;
      flash_address += section_sz;
      sz = sz - section_sz;
    }while (sz > 0);


    Switch_Flash_driver_to_bgo();
    //.............................................................................................................
    // Конец флеширование блока данных
    //.............................................................................................................

    flashed_sz += flash_buf_size;
    if (flashed_sz >= dh_ptr->orig_image_size) break;

  } while (1);



  #ifdef WRITE_EXTRACTED_IMAGE_TO_FILE
  fx_file_close(& debug_out_file);
  fx_media_flush(& fat_fs_media);
  #endif

  // В заключении проверяем контрольную сумму флешированных данных
  calculated_crc = Get_CRC32((uint8_t*)dh_ptr->start_address, dh_ptr->orig_image_size-4);
  memcpy(& flash_crc, (void*)(dh_ptr->start_address + dh_ptr->orig_image_size-4), 4);
  if (flash_crc != calculated_crc) goto error;


  App_free(buf1);
  App_free(buf2);
  return RES_OK;

  error:
  App_free(buf1);
  App_free(buf2);

  loader_cbl.flasher_err = FIRMWARE_FLASHING_FAIL;
  return RES_ERROR;


}

/*-----------------------------------------------------------------------------------------------------

   Алгоритм получения образа:
   - Исходный файл сжимается если надо
   - Добавляются дополнительные случайные данные до набора заданного размера если надо
   - Вычисляется хэш и помещается в заголовок данных
   - Все данные шифруются включая заголовок данных. В конце блока зашифрованных данных добавляется тэг
   - Создается блок подписи данных включающий ключи шифрования и хэш
   - Блок подписи шифруется и добавляется к файлу
   - К файлу добавляется загловок файла определяющий размер блока данных и размер блока подписи

-----------------------------------------------------------------------------------------------------*/
uint32_t Load_and_Flash_Image_File(void)
{
  FX_FILE         file;
  T_sys_timestump tt1,tt2;
  ssp_err_t       res;

  loader_cbl.flasher_err = LOADING_FIRMWARE_UNKNOWN_ERROR;
  loader_cbl.load_duration_us = 0;
  Get_hw_timestump(&tt1);

  // Включаем все светодиоды
  RED_LED   = 0;
  GREEN_LED = 0;
  BLUE_LED  = 0;

  if (Open_firmware_file(&file) != RES_OK) return RES_ERROR;

  memset(&file_sign, 0, sizeof(T_file_sign));
  memset(&data_header, 0, sizeof(T_data_header));

  if (file_header.img_signed)
  {
    // Расшифровать подпись файла
    if (Decrypt_image_sign(&file,&file_header,&file_sign) != RES_OK) goto error;
  }
  if (file_header.img_encrypted)
  {
    // Расшифровываем заголовок данных
    if (Decrypt_image_data_header(&file,&file_header,&file_sign,&data_header) != RES_OK) goto error;
    // Расшифровываем файл и проверяем хэш файла
    if (Check_encrypted_file_digest(&file,&file_header,&file_sign,&data_header) != RES_OK) goto error;
  }
  else
  {
    if (Read_image_data_header(&file,&data_header) != RES_OK) goto error;
    // Расшифровываем файл и проверяем хэш файла
    if (Check_plain_file_digest(&file,&file_header,&data_header) != RES_OK) goto error;
  }

  // Соответствует ли адрес начала данных адресу начала фирмваре
  if (data_header.start_address != APP_IMG_START_ADDRESS)
  {
    loader_cbl.flasher_err = INCORECT_FIRMWARE_START_ADDRESS;
    goto error;
  }

  // Выключаем светодиоды
  RED_LED   = 1;
  GREEN_LED = 1;
  BLUE_LED  = 1;

  //.................................................................
  // Программируем данные во Flash
  //.................................................................
  if (Flash_image(&file,&file_sign,&file_header,&data_header) != RES_OK)
  {
    fx_file_close(&file);
    Rename_firmware_file(STATUS_FLASHING_FIRMWARE_FAIL);
    return RES_ERROR;
  }


  fx_file_close(&file);
  Rename_firmware_file(STATUS_FILE_PROGRAMED);

  Get_hw_timestump(&tt2);

  loader_cbl.load_duration_us = Hw_timestump_diff32_us(&tt1,&tt2);
  loader_cbl.flasher_err = FIRMWARE_FLASHED_SUCCESFULLY;

  //.................................................................
  // Заменяем дефолтные пароли если устанавливается защита 3 уровня
  //.................................................................
  if (data_header.protection_type == 3)
  {
    // Переустанавливаем пароли коммуникационных сервисов
    memcpy(ivar.wifi_ap_key, Monitor_pass, Monitor_pass_SIZE);
    ivar.wifi_ap_key[Monitor_pass_SIZE] = 0;
    memcpy(ivar.ftp_serv_password, Monitor_pass, Monitor_pass_SIZE);
    ivar.ftp_serv_password[Monitor_pass_SIZE] = 0;

    if (Save_settings_to(&ivar_inst, MEDIA_TYPE_DATAFLASH, 0, BOOTL_PARAMS) != RES_OK)
    {
      loader_cbl.settings_err  = 1;
    }
    else
    {
      loader_cbl.settings_err  = 0;
    }
    //Set_SD_card_password(SD_SET_PASSWORD, (char *)Monitor_pass, Monitor_pass_SIZE);
  }

  //.................................................................
  // Перепрограммируем тип защиты от считывания
  //.................................................................
  loader_cbl.protection_err = (uint32_t)SSP_SUCCESS;
  loader_cbl.protection_type = data_header.protection_type;
  if (loader_cbl.protection_type <= 3)
  {

    uint8_t buf[Flash_access_pass_SIZE];
    memcpy(buf, Flash_access_pass, Flash_access_pass_SIZE);
    res = Set_Flash_protection(buf, (flash_id_code_mode_t)data_header.protection_type);
    if (res != SSP_SUCCESS)
    {
      loader_cbl.protection_err = (uint32_t)res;
    }
  }
  else
  {
    loader_cbl.protection_err = (uint32_t)SSP_ERR_UNSUPPORTED;
  }


  // Выключаем светодиоды
  RED_LED   = 1;
  GREEN_LED = 1;
  BLUE_LED  = 1;

  return RES_OK;

error:
  fx_file_close(&file);
  Rename_firmware_file(STATUS_LOADING_FIRWARE_FAIL);


  // Выключаем светодиоды
  RED_LED   = 1;
  GREEN_LED = 1;
  BLUE_LED  = 1;

  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Erase_firmware_area(void)
{
  uint32_t res  = 0;
  uint32_t n = APP_IMG_SIZE/CODE_FLASH_EBLOCK_SZ;
  Switch_Flash_driver_to_no_bgo();
  __disable_interrupt();
  res = Flash_erase_block(APP_IMG_START_ADDRESS, n);
  __enable_interrupt();
  Switch_Flash_driver_to_bgo();
  return  res;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Clear_app_DataFlash(void)
{
  uint32_t res = 0;
  // Стираем области
  //   DATAFLASH_MODULE_PARAMS_1_ADDR
  //   DATAFLASH_MODULE_PARAMS_2_ADDR
  //   DATAFLASH_BOOTL_PARAMS_1_ADDR
  //   DATAFLASH_BOOTL_PARAMS_2_ADDR

  res |= DataFlash_bgo_EraseArea(DATAFLASH_APP_PARAMS_1_ADDR       ,DATAFLASH_PARAMS_AREA_SIZE);
  res |= DataFlash_bgo_EraseArea(DATAFLASH_APP_PARAMS_2_ADDR       ,DATAFLASH_PARAMS_AREA_SIZE);
  res |= DataFlash_bgo_EraseArea(DATAFLASH_MODULE_PARAMS_1_ADDR    ,DATAFLASH_PARAMS_AREA_SIZE);
  res |= DataFlash_bgo_EraseArea(DATAFLASH_MODULE_PARAMS_2_ADDR    ,DATAFLASH_PARAMS_AREA_SIZE);

  return res;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void Auto_protection(void)
{
  ssp_err_t       res;

  if (ivar.en_auto_protection)
  {
    uint8_t buf[Flash_access_pass_SIZE];
    Read_OSIS_to_buf(buf, Flash_access_pass_SIZE);
    if (memcmp(buf, Flash_access_pass, Flash_access_pass_SIZE) == 0)
    {
      APPLOG("Flash protection is already activated");
      return;
    }

    memcpy(buf, Flash_access_pass, Flash_access_pass_SIZE);
    res = Set_Flash_protection(buf, FLASH_ID_CODE_MODE_LOCKED_WITH_ALL_ERASE_SUPPORT);
    if (res != SSP_SUCCESS)
    {
      APPLOG("Flash protection activation fault (%d)", (uint32_t)res);
    }
    else
    {
      APPLOG("Flash protection activated successfully");
    }
  }
  else
  {
    APPLOG("Flash protection activation skipped");
  }
  return;
}
/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Integrity_check_App_firmware(void)
{
  uint32_t flash_crc;
  uint32_t img_crc;

  Get_hw_timestump(&t1);
  // Проверяем целостность контрольной суммы образа основной программы во Flash
  // Время вычисления контролько суммы на объему в 2 Мбайта = 193 мс
  flash_crc = Get_CRC32((uint8_t *)(APP_IMG_START_ADDRESS), APP_IMG_SIZE - 4);
  memcpy(&img_crc, (void *)(APP_IMG_END_ADDRESS - 3),4);
  Get_hw_timestump(&t2);
  td = Hw_timestump_diff32_us(&t1,&t2);
  if (flash_crc != img_crc)
  {
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Restart_to_App_firmware(void)
{
  //    Это попытка использовать механизм переключения банков Flash
  //    Настраиваемся на временное переключение на альтернативный банк флэш и делаем сброс.
  //    Выявлена проблема. Альтернативный банк переключается сразу, но после сброса это переключение не сохраняется
  //    flash_cbl.p_api->open(flash_cbl.p_ctrl,flash_cbl.p_cfg);
  //    __disable_irq();
  //    flash_cbl.p_api->startupAreaSelect(flash_cbl.p_ctrl,FLASH_STARTUP_AREA_BLOCK1,true);
  //    flash_cbl.p_api->close(flash_cbl.p_ctrl);

  // Записываем контрольные числа по которым бутлодер при следующем перезапуске определит, что надо передать управление основному приложению.
  loader_info.tag1 =  REDY_TO_BOOT_MAGIC;
  loader_info.tag2 =  REDY_TO_BOOT_MAGIC;
  loader_info.tag3 =  REDY_TO_BOOT_MAGIC;
  loader_info.tag4 =  REDY_TO_BOOT_MAGIC;
  Reset_SoC();
}

