#ifndef  __KEY_FILE
#define  __KEY_FILE
 
#define  AESKey_SIZE             32
#define  AES_init_vector_SIZE    16
#define  AES_aad_SIZE            8
#define  AES_tag_SIZE            16
#define  RSA_public_key_SIZE     270
#define  RSA_private_key_SIZE    1193
#define  RSA_priv_exp_modul_SIZE 512
#define  Flash_access_pass_SIZE  16
#define  Monitor_pass_SIZE       16
#define  Engnr_menu_pass_SIZE    16
#define  WIFI_pass_SIZE          16
 
extern const unsigned char AESKey[AESKey_SIZE];
extern const unsigned char AES_init_vector[AES_init_vector_SIZE];
extern const unsigned char AES_aad[AES_aad_SIZE];
extern const unsigned char RSA_public_key[RSA_public_key_SIZE];
extern const unsigned char RSA_private_key[RSA_private_key_SIZE];
extern const unsigned char RSA_priv_exp_modul[RSA_priv_exp_modul_SIZE];
extern const unsigned char Flash_access_pass[Flash_access_pass_SIZE];
extern const unsigned char Monitor_pass[Monitor_pass_SIZE];
extern const unsigned char Engnr_menu_pass[Engnr_menu_pass_SIZE];
extern const unsigned char WIFI_pass[WIFI_pass_SIZE];
 
#endif  // __KEY_FILE
