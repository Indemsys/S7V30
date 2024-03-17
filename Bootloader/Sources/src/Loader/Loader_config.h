#ifndef LOADER_CONFIG_H
  #define LOADER_CONFIG_H


#define APP_IMG_SIZE              0x00200000                     // Размер образа основной программы во Flash памяти микроконтроллера
#define APP_IMG_START_ADDRESS     0x00100000                     // Адрес хранения образа основной программы во Flash памяти микроконтроллера
#define APP_IMG_END_ADDRESS       (APP_IMG_START_ADDRESS + APP_IMG_SIZE - 1)



#define REDY_TO_BOOT_MAGIC        0x12345678
#define REDY_TO_FS_MAGIC          0x21436587


#define MONITOR_PASSWORD_LEN      10
#define FLASH_ACCESS_PASSWORD_LEN 16
#define WIFI_PASSWORD_LEN         10


#define DIGEST_ALG_CRC32          0
#define DIGEST_ALG_MD5            1
#define DIGEST_ALG_SHA256         2
#define DIGEST_ALG_SHA512         3

#define DIGEST_MD5_SZ             16
#define DIGEST_SHA256_SZ          32

#define IMG_DIGEST_BUF_SZ         64

#define MIN_DATA_UNIT_SIZE        16     //

#define FILE_CHANK_SZ             (64*1024)

//#define WRITE_EXTRACTED_IMAGE_TO_FILE Раскоментировать если нужно сохранить извлеченный образ фирмваре в файл

typedef struct
{
  uint32_t img_compressed : 1;
  uint32_t img_encrypted : 1;
  uint32_t img_signed : 1;
  uint32_t img_sz;
  uint32_t sign_sz;
  uint32_t crc32;

} T_file_header;


// Размер заголовка данных - 128 байт
typedef struct
{
  uint32_t data_block_size;    // Размер образа в данном файле в байтах
  uint32_t orig_image_size;    // Размер образа до сжатия
  uint32_t start_address;      // Адрес начала размещения образа в адресном пространстве целевого устройства
  uint32_t main_start_address; // Адрес функции __main в адресном пространстве целевого устройства
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
  uint8_t  hour;
  uint8_t  min;
  uint8_t  sec;
  uint8_t  msec;
  uint8_t  protection_type;    // Тип защщиты 0- нет защиты, 1, 2, 3 - ровни защиты
  uint8_t  version[31];        // Текстовое представление версии образа
  uint16_t compression_alg;    // Тип используемого сжатия (0-нет сжатия, 1 - SIXPACK, 2- LZSS)
  uint16_t digest_type;        // Тип контрольного блока образа (0-CRC32, 1 - MD5, 2- SHA256, 3- SHA512)
  uint8_t  digest[IMG_DIGEST_BUF_SZ];         // Контрольный блок образа - CRC или хэш
  uint32_t this_struct_crc;
} T_data_header;

typedef struct
{
  uint8_t digest[64];                     // Хэш подписываемого файла
  uint8_t AES_key[AESKey_SIZE];        // Ключ кторым файл зашифрован
  uint8_t AES_iv[AES_init_vector_SIZE]; // Инициализационный вектор ключа
  uint8_t AES_aad[AES_aad_SIZE];
} T_file_sign;



typedef struct
{
  uint32_t tag1;
  uint32_t tag2;
  uint32_t tag3;
  uint32_t tag4;

  uint8_t  rstsr0;
  uint16_t rstsr1;
  uint8_t  rstsr2;

  // Тэги для определения режима USB MassStorage. Режим включается если все тэги имеют значение REDY_TO_FS_MAGIC
  uint32_t boot_tag1;
  uint32_t boot_tag2;
  uint32_t boot_tag3;
  uint32_t boot_tag4;

} T_loader_info;

extern T_loader_info  loader_info;

#endif // LOADER_CONFIG_H



