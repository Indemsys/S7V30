#ifndef BLE_MAIN_H
  #define BLE_MAIN_H

#define PARING_KEYS_NUM               5


#define CYBT_RX_MEM_MIN_SIZE          2048 // (1040)
#define CYBT_TX_CMD_MEM_SIZE          (512)
#define CYBT_TX_HEAP_MIN_SIZE         2048 // (1040)

#define BT_HEAP_SIZE                  0x1000 //0x1000


#define MAX_LE_KEY_SIZE                 (16)   // LE Key Size



/* Macros defining the commands for WiFi control point characteristic */
#define WIFI_CONTROL_DISCONNECT      (0u)
#define WIFI_CONTROL_CONNECT         (1u)
#define WIFI_CONTROL_SCAN            (2u)

#define CY_WCM_MAX_SSID_LEN                (32)        /**< Max SSID length.                            */
#define CY_WCM_MAX_PASSPHRASE_LEN          (63)        /**< Max passphrase length.                      */
#define CY_WCM_MAC_ADDR_LEN                (6)         /**< MAC address length.                         */



#define WEP_ENABLED                        0x0001      /**< Flag to enable WEP security.                */
#define TKIP_ENABLED                       0x0002      /**< Flag to enable TKIP encryption.             */
#define AES_ENABLED                        0x0004      /**< Flag to enable AES encryption.              */
#define SHARED_ENABLED                     0x00008000  /**< Flag to enable shared key security.         */
#define WPA_SECURITY                       0x00200000  /**< Flag to enable WPA security.                */
#define WPA2_SECURITY                      0x00400000  /**< Flag to enable WPA2 security.               */
#ifndef COMPONENT_CAT5
#define WPA2_SHA256_SECURITY               0x00800000  /**< Flag to enable WPA2 SHA256 Security         */
#endif
#define WPA3_SECURITY                      0x01000000  /**< Flag to enable WPA3 PSK security.           */
#define ENTERPRISE_ENABLED                 0x02000000  /**< Flag to enable enterprise security.         */
#ifdef COMPONENT_CAT5
#define SHA256_1X                          0x04000000  /**< Flag 1X with SHA256 key derivation          */
#define SUITE_B_SHA384                     0x08000000  /**< Flag to enable Suite B-192 SHA384 Security  */
#endif
#define WPS_ENABLED                        0x10000000  /**< Flag to enable WPS security.                */
#define IBSS_ENABLED                       0x20000000  /**< Flag to enable IBSS mode.                   */
#define FBT_ENABLED                        0x40000000  /**< Flag to enable FBT.                         */


typedef enum
{
    CY_WCM_SECURITY_OPEN                = 0,                                                                   /**< Open security.                                         */
    CY_WCM_SECURITY_WEP_PSK             = WEP_ENABLED,                                                         /**< WEP PSK security with open authentication.             */
    CY_WCM_SECURITY_WEP_SHARED          = ( WEP_ENABLED   | SHARED_ENABLED ),                                  /**< WEP PSK security with shared authentication.           */
    CY_WCM_SECURITY_WPA_TKIP_PSK        = ( WPA_SECURITY  | TKIP_ENABLED ),                                    /**< WPA PSK security with TKIP.                            */
    CY_WCM_SECURITY_WPA_AES_PSK         = ( WPA_SECURITY  | AES_ENABLED ),                                     /**< WPA PSK security with AES.                             */
    CY_WCM_SECURITY_WPA_MIXED_PSK       = ( WPA_SECURITY  | AES_ENABLED | TKIP_ENABLED ),                      /**< WPA PSK security with AES and TKIP.                    */
    CY_WCM_SECURITY_WPA2_AES_PSK        = ( WPA2_SECURITY | AES_ENABLED ),                                     /**< WPA2 PSK security with AES.                            */
#ifndef COMPONENT_CAT5
    CY_WCM_SECURITY_WPA2_AES_PSK_SHA256 = ( WPA2_SECURITY | WPA2_SHA256_SECURITY | AES_ENABLED ),              /**< WPA2 PSK SHA256 Security with AES                      */
#else
    CY_WCM_SECURITY_WPA2_AES_PSK_SHA256 = ( WPA2_SECURITY | SHA256_1X | AES_ENABLED ),                         /**< WPA2 PSK SHA256 Security with AES                      */
#endif
    CY_WCM_SECURITY_WPA2_TKIP_PSK       = ( WPA2_SECURITY | TKIP_ENABLED ),                                    /**< WPA2 PSK security with TKIP.                           */
    CY_WCM_SECURITY_WPA2_MIXED_PSK      = ( WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED ),                      /**< WPA2 PSK security with AES and TKIP.                   */
    CY_WCM_SECURITY_WPA2_FBT_PSK        = ( WPA2_SECURITY | AES_ENABLED | FBT_ENABLED),                        /**< WPA2 FBT PSK security with AES and TKIP.               */
    CY_WCM_SECURITY_WPA3_SAE            = ( WPA3_SECURITY | AES_ENABLED ),                                     /**< WPA3 security with AES.                                */
    CY_WCM_SECURITY_WPA2_WPA_AES_PSK    = (WPA2_SECURITY | WPA_SECURITY | AES_ENABLED),                        /**< WPA2 WPA PSK Security with AES                         */
    CY_WCM_SECURITY_WPA2_WPA_MIXED_PSK  = (WPA2_SECURITY | WPA_SECURITY | AES_ENABLED | TKIP_ENABLED),         /**< WPA2 WPA PSK Security with AES & TKIP.                 */
    CY_WCM_SECURITY_WPA3_WPA2_PSK       = ( WPA3_SECURITY | WPA2_SECURITY | AES_ENABLED ),                     /**< WPA3 WPA2 PSK security with AES.                       */
    CY_WCM_SECURITY_WPA_TKIP_ENT        = (ENTERPRISE_ENABLED | WPA_SECURITY | TKIP_ENABLED),                  /**< WPA Enterprise Security with TKIP.                     */
    CY_WCM_SECURITY_WPA_AES_ENT         = (ENTERPRISE_ENABLED | WPA_SECURITY | AES_ENABLED),                   /**< WPA Enterprise Security with AES                       */
    CY_WCM_SECURITY_WPA_MIXED_ENT       = (ENTERPRISE_ENABLED | WPA_SECURITY | AES_ENABLED | TKIP_ENABLED),    /**< WPA Enterprise Security with AES and TKIP.             */
    CY_WCM_SECURITY_WPA2_TKIP_ENT       = (ENTERPRISE_ENABLED | WPA2_SECURITY | TKIP_ENABLED),                 /**< WPA2 Enterprise Security with TKIP.                    */
    CY_WCM_SECURITY_WPA2_AES_ENT        = (ENTERPRISE_ENABLED | WPA2_SECURITY | AES_ENABLED),                  /**< WPA2 Enterprise Security with AES.                     */
    CY_WCM_SECURITY_WPA2_MIXED_ENT      = (ENTERPRISE_ENABLED | WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED),   /**< WPA2 Enterprise Security with AES and TKIP.            */
    CY_WCM_SECURITY_WPA2_FBT_ENT        = (ENTERPRISE_ENABLED | WPA2_SECURITY | AES_ENABLED | FBT_ENABLED),    /**< WPA2 Enterprise Security with AES and FBT.             */
#ifdef COMPONENT_CAT5
    CY_WCM_SECURITY_WPA3_192BIT_ENT     = (ENTERPRISE_ENABLED | WPA3_SECURITY | SUITE_B_SHA384 | AES_ENABLED), /**< WPA3 192-BIT Enterprise Security with AES              */
    CY_WCM_SECURITY_WPA3_ENT            = (ENTERPRISE_ENABLED | WPA3_SECURITY | SHA256_1X | AES_ENABLED),      /**< WPA3 Enterprise Security with AES GCM-256              */
    CY_WCM_SECURITY_WPA3_ENT_AES_CCMP   = (ENTERPRISE_ENABLED | WPA3_SECURITY | WPA2_SECURITY | SHA256_1X | AES_ENABLED), /**< WPA3 Enterprise Security with AES CCM-128   */
#endif
    CY_WCM_SECURITY_IBSS_OPEN           = ( IBSS_ENABLED ),                                                    /**< Open security on IBSS ad hoc network.                  */
    CY_WCM_SECURITY_WPS_SECURE          = ( WPS_ENABLED | AES_ENABLED),                                        /**< WPS with AES security.                                 */

    CY_WCM_SECURITY_UNKNOWN             = -1,                                                                  /**< Returned by \ref cy_wcm_scan_result_callback_t if security is unknown. Do not pass this to the join function! */

    CY_WCM_SECURITY_FORCE_32_BIT        = 0x7fffffff                                                           /**< Exists only to force whd_security_t type to 32 bits.   */
} cy_wcm_security_t;


typedef uint8_t cy_wcm_ssid_t[CY_WCM_MAX_SSID_LEN + 1];              /**< SSID name (AP name in null-terminated string format). */
typedef uint8_t cy_wcm_mac_t[CY_WCM_MAC_ADDR_LEN];                   /**< Unique 6-byte MAC address represented in network byte order. */
typedef uint8_t cy_wcm_passphrase_t[CY_WCM_MAX_PASSPHRASE_LEN + 1];  /**< Passphrase in null-terminated string format. */


#define    BLE_INIT_TASK_NOT_STARTED  0
#define    BLE_INIT_TASK_STARTED      1
#define    BLE_INIT_TASK_FINISHED     2

#include   "BLE_config.h"
#include   "hcidefs.h"
#include   "wiced_bt_stack.h"
#include   "wiced_bt_l2c.h"
#include   "wiced_bt_rfcomm.h"
#include   "wiced_bt_sdp.h"
#include   "wiced_bt_ble.h"
#include   "wiced_bt_gatt.h"
#include   "wiced_bt_stack_platform.h"
#include   "wiced_memory.h"

#include   "cybsp.h"
#include   "cycfg_bt_settings.h"

#include   "cybt_platform_task.h"
#include   "cybt_platform_trace.h"
#include   "cybt_platform_hci.h"
#include   "cybt_platform_interface.h"
#include   "cybt_platform_util.h"
#include   "cybt_prm.h"
#include   "cycfg_gap.h"
#include   "cycfg_sdp_db.h"


#include   "BLE_main.h"
#include   "BLE_strings.h"
#include   "BLE_app_GATT.h"
#include   "BLE_app_WiFi_control.h"
#include   "Bluetooth_local_spp.h"
#include   "Bluetooth_remote_spp.h"


typedef struct
{
    uint8_t         *p_rx_mem;
    uint8_t         *p_tx_cmd_mem;
    wiced_bt_heap_t *p_tx_data_heap;

} cybt_task_mem_cb_t;

typedef struct
{
    wiced_bt_local_identity_keys_t   key;
    uint16_t                         crc;
} T_bt_identity_key;

typedef struct
{
  wiced_bt_device_link_keys_t  key;
  uint8_t                      addr_type;
  uint8_t                      key_mask;

} T_link_keys_rec;

typedef struct
{
    T_link_keys_rec             link_keys[PARING_KEYS_NUM];
    uint16_t                    crc;
} T_bt_paring_key;

typedef struct
{
  T_bt_identity_key             identity_keys;
  T_bt_paring_key               paring_keys;
  uint32_t                      oldest_paring_keys_indx; // Индекс последнего записанного ключа. Инкрементируется по кругу

} T_bt_nv_data;

typedef struct
{
  wiced_bt_device_address_t          addr;
  wiced_bt_dev_pairing_cplt_t        info;
}
T_bt_dev_pairing_info;

extern T_bt_nv_data bt_nv;

typedef void (*pfn_free_buffer_t)(uint8_t *);
typedef uint32_t cy_rslt_t;

uint32_t                     BLE_init(void);
T_bt_dev_pairing_info       *BLE_get_last_pairing_info(void);
uint32_t                     BLE_init_task_state(void);
uint8_t                      BLE_is_initialised(void);
uint32_t                     BLE_init_delete(void);
uint8_t                      Check_BT_pairing_mode(void);

#endif



