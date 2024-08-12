#include "App.h"
#include "BLE_main.h"


#define HCI_VSC_UPDATE_BAUDRATE_CMD                (0xFC18)
#define HCI_VSC_UPDATE_BAUD_RATE_UNENCODED_LENGTH  (6)


typedef enum
{
  BT_POST_RESET_STATE_IDLE = 0x00,
  BT_POST_RESET_STATE_UPDATE_BAUDRATE_FOR_FW_DL,
  BT_POST_RESET_STATE_FW_DOWNLOADING,
  BT_POST_RESET_STATE_FW_DOWNLOAD_COMPLETED,
  BT_POST_RESET_STATE_UPDATE_BAUDRATE_FOR_FEATURE,
  BT_POST_RESET_STATE_DONE,
  BT_POST_RESET_STATE_FAILED
} bt_post_reset_state_e;

typedef struct
{
    bt_post_reset_state_e  state;
} bt_fw_download_cb;

bt_fw_download_cb bt_fwdl_cb = {.state = BT_POST_RESET_STATE_IDLE};


void bt_baudrate_updated_cback(wiced_bt_dev_vendor_specific_command_complete_params_t *p);
void bt_fw_download_complete_cback(cybt_prm_status_t status);
void bt_post_reset_cback(void);

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void bt_start_fw_download(void)
{
  static T_infineon_blob const *bt_blob;

  bt_fwdl_cb.state = BT_POST_RESET_STATE_FW_DOWNLOADING;

  // Ищем в массиве патчей нужный файл
  for (uint32_t i = 0; i < INFINEON_BLOBS_CNT; i++)
  {
    bt_blob = _Find_infineon_blob("CYW4373A0_BT_patch.hcd");

    if (bt_blob != NULL)
    {
      #ifdef INFINEON_BLOBS_COMPRESSED

      uint32_t uncomprees_buff_sz;
      uint8_t  *uncompressed_buff;

      memcpy(&uncomprees_buff_sz, bt_blob->data, 4);
      uncompressed_buff = SDRAM_malloc(uncomprees_buff_sz);
      if (uncompressed_buff != NULL)
      {
        if (Decompress_mem_to_mem(COMPR_ALG_SIXPACK, (void *)bt_blob->data, bt_blob->size, uncompressed_buff, uncomprees_buff_sz) > 0)
        {
          cybt_prm_download(bt_fw_download_complete_cback,uncompressed_buff,uncomprees_buff_sz,0,CYBT_PRM_FORMAT_HCD);
        }
        SDRAM_free(uncompressed_buff);
      }

      #else

      cybt_prm_download(bt_fw_download_complete_cback,bt_blob->data,bt_blob->size,0,CYBT_PRM_FORMAT_HCD);

      #endif
    }
  }


}

/*-----------------------------------------------------------------------------------------------------


  \param baudrate
-----------------------------------------------------------------------------------------------------*/
void bt_update_platform_baudrate(uint32_t baudrate)
{
  MAIN_TRACE_DEBUG("bt_update_platform_baudrate(): %d", baudrate);

  cy_rtos_delay_milliseconds(100);
  cybt_platform_hci_set_baudrate(baudrate);
  cy_rtos_delay_milliseconds(100);
}

/*-----------------------------------------------------------------------------------------------------


  \param baudrate
-----------------------------------------------------------------------------------------------------*/
void bt_update_controller_baudrate(uint32_t baudrate)
{
  uint8_t       hci_data[HCI_VSC_UPDATE_BAUD_RATE_UNENCODED_LENGTH];

  MAIN_TRACE_DEBUG("bt_update_controller_baudrate(): %d", baudrate);

  /* Baudrate is loaded LittleEndian */
  hci_data[0] = 0;
  hci_data[1] = 0;
  hci_data[2] = baudrate & 0xFF;
  hci_data[3] = (baudrate >> 8) & 0xFF;
  hci_data[4] = (baudrate >> 16) & 0xFF;
  hci_data[5] = (baudrate >> 24) & 0xFF;

  /* Send the command to the host controller */
  wiced_bt_dev_vendor_specific_command(HCI_VSC_UPDATE_BAUDRATE_CMD,HCI_VSC_UPDATE_BAUD_RATE_UNENCODED_LENGTH,hci_data,bt_baudrate_updated_cback);
}

/*-----------------------------------------------------------------------------------------------------


  \param p
-----------------------------------------------------------------------------------------------------*/
void bt_baudrate_updated_cback(wiced_bt_dev_vendor_specific_command_complete_params_t *p)
{
  switch (bt_fwdl_cb.state)
  {
  case BT_POST_RESET_STATE_UPDATE_BAUDRATE_FOR_FW_DL:
    {
      MAIN_TRACE_DEBUG("bt_baudrate_updated_cback(): Baudrate is updated for FW downloading");
      bt_update_platform_baudrate(CYBSP_BT_PLATFORM_CFG_BAUD_DOWNLOAD);
      bt_start_fw_download();
    }
    break;
  case BT_POST_RESET_STATE_UPDATE_BAUDRATE_FOR_FEATURE:
    {
      MAIN_TRACE_DEBUG("bt_baudrate_updated_cback(): Baudrate is updated for feature");
      bt_update_platform_baudrate(CYBSP_BT_PLATFORM_CFG_BAUD_FEATURE);
      MAIN_TRACE_DEBUG("bt_baudrate_updated_cback(): post-reset process is done");
      bt_fwdl_cb.state = BT_POST_RESET_STATE_DONE;
      wiced_bt_continue_reset();
    }
    break;
  default:
    MAIN_TRACE_ERROR("bt_baudrate_updated_cback(): unknown state(%d)",bt_fwdl_cb.state);
    break;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param status
-----------------------------------------------------------------------------------------------------*/
void bt_fw_download_complete_cback(cybt_prm_status_t status)
{
  MAIN_TRACE_DEBUG("bt_patch_download_complete_cback(): status = %d", status);

  if (CYBT_PRM_STS_COMPLETE == status)
  {
    bt_fwdl_cb.state = BT_POST_RESET_STATE_FW_DOWNLOAD_COMPLETED;

    // After patch ram is launched, the baud rate of BT chip is reset to 115200
    if (HCI_UART_DEFAULT_BAUDRATE != CYBSP_BT_PLATFORM_CFG_BAUD_DOWNLOAD)
    {
      MAIN_TRACE_DEBUG("bt_fw_download_complete_cback(): Reset baudrate to %d",HCI_UART_DEFAULT_BAUDRATE);
      bt_update_platform_baudrate(HCI_UART_DEFAULT_BAUDRATE);
    }

    if (HCI_UART_DEFAULT_BAUDRATE != CYBSP_BT_PLATFORM_CFG_BAUD_FEATURE)
    {
      MAIN_TRACE_DEBUG("bt_fw_download_complete_cback(): Changing baudrate to %d",CYBSP_BT_PLATFORM_CFG_BAUD_FEATURE);

      bt_fwdl_cb.state = BT_POST_RESET_STATE_UPDATE_BAUDRATE_FOR_FEATURE;
      bt_update_controller_baudrate(CYBSP_BT_PLATFORM_CFG_BAUD_FEATURE);
    }
    else
    {
      MAIN_TRACE_DEBUG("bt_fw_download_complete_cback(): post-reset process is Done");

      bt_fwdl_cb.state = BT_POST_RESET_STATE_DONE;
      wiced_bt_continue_reset();
    }
  }
  else
  {
    MAIN_TRACE_ERROR("bt_patch_download_complete_cback(): Failed (%d)",status);
    bt_fwdl_cb.state = BT_POST_RESET_STATE_FAILED;
  }

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void bt_post_reset_cback(void)
{
  MAIN_TRACE_DEBUG("bt_post_reset_cback()");

  bt_fwdl_cb.state = BT_POST_RESET_STATE_IDLE;


  if (CYBSP_BT_PLATFORM_CFG_BAUD_DOWNLOAD != HCI_UART_DEFAULT_BAUDRATE)
  {
    MAIN_TRACE_DEBUG("bt_post_reset_cback(): Change baudrate (%d) for FW downloading",CYBSP_BT_PLATFORM_CFG_BAUD_DOWNLOAD);
    bt_fwdl_cb.state = BT_POST_RESET_STATE_UPDATE_BAUDRATE_FOR_FW_DL;
    bt_update_controller_baudrate(CYBSP_BT_PLATFORM_CFG_BAUD_DOWNLOAD);
  }
  else
  {
    MAIN_TRACE_DEBUG("bt_post_reset_cback(): Starting FW download...");
    bt_start_fw_download();
  }
}

