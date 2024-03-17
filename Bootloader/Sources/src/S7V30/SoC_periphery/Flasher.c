// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020-01-30
// 10:51:25
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"



#define  FLAG_FLASH_OP_DONE   BIT(0)      // Флаг завершения операции FLASH

flash_hp_instance_ctrl_t flash_ctrl;
flash_hp_instance_ctrl_t flash_bgo_ctrl;

extern const flash_instance_t flash_bgo_cbl;
extern const flash_instance_t flash_cbl;

static void  Flash_bgo_callback(flash_callback_args_t *p_args);

const flash_cfg_t flash_bgo_cfg =
{
  .data_flash_bgo      = true,
  .p_callback          = Flash_bgo_callback,
  .p_context           = &flash_bgo_cbl,
  .irq_ipl             = (10),
  .err_irq_ipl         = (10),
};

const flash_instance_t flash_bgo_cbl =
{
  .p_ctrl        = &flash_bgo_ctrl,
  .p_cfg         = &flash_bgo_cfg,
  .p_api         = &g_flash_on_flash_hp
};


static TX_EVENT_FLAGS_GROUP   flash_bgo_flags;
static uint32_t               flash_bgo_status;

const flash_cfg_t flash_cfg =
{
  .data_flash_bgo      = false,
  .p_callback          = NULL,
  .p_context           = &flash_cbl,
  .irq_ipl             = (0xFF),
  .err_irq_ipl         = (0xFF),
};

const flash_instance_t flash_cbl =
{
  .p_ctrl        = &flash_ctrl,
  .p_cfg         = &flash_cfg,
  .p_api         = &g_flash_on_flash_hp
};


/*-----------------------------------------------------------------------------------------------------
  Инициализация драйвера записи стирания Flash с синхронно выполняемыми операциями
  без использованиея прерываний

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_driver_init(void)
{
  return flash_cbl.p_api->open(&flash_ctrl,&flash_cfg);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_driver_deinit(void)
{
  return flash_cbl.p_api->close(&flash_ctrl);
}


/*-----------------------------------------------------------------------------------------------------
  Инициализация драйвера записи стирания Flash с асинхронно выполняемыми операциями Background Operation
  с использованием прерываний

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_driver_bgo_init(void)
{
  UINT     res;

  // Создаем объект синхронизации
  res = tx_event_flags_create(&flash_bgo_flags, "Flash");
  if (res != TX_SUCCESS)
  {
    APPLOG("Failed to create event. Error %d", res);
    return RES_ERROR;
  }

  return flash_bgo_cbl.p_api->open(&flash_bgo_ctrl,&flash_bgo_cfg);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_driver_bgo_deinit(void)
{
  flash_bgo_cbl.p_api->close(&flash_bgo_ctrl);
  return tx_event_flags_delete(&flash_bgo_flags);
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызываема после окончания оперции с DataFlash

  \param p_args
-----------------------------------------------------------------------------------------------------*/
static void  Flash_bgo_callback(flash_callback_args_t *p_args)
{
  switch (p_args->event)
  {
  case FLASH_EVENT_ERASE_COMPLETE :
    flash_bgo_status |= BIT_FLASH_EVENT_ERASE_COMPLETE;
    break;
  case FLASH_EVENT_WRITE_COMPLETE :
    flash_bgo_status |= BIT_FLASH_EVENT_WRITE_COMPLETE;
    break;
  case FLASH_EVENT_BLANK          :
    flash_bgo_status |= BIT_FLASH_EVENT_BLANK;
    break;
  case FLASH_EVENT_NOT_BLANK      :
    flash_bgo_status |= BIT_FLASH_EVENT_NOT_BLANK;
    break;
  case FLASH_EVENT_ERR_DF_ACCESS  :
    flash_bgo_status |= BIT_FLASH_EVENT_ERR_DF_ACCESS;
    break;
  case FLASH_EVENT_ERR_CF_ACCESS  :
    flash_bgo_status |= BIT_FLASH_EVENT_ERR_CF_ACCESS;
    break;
  case FLASH_EVENT_ERR_CMD_LOCKED :
    flash_bgo_status |= BIT_FLASH_EVENT_ERR_CMD_LOCKED;
    break;
  case FLASH_EVENT_ERR_FAILURE    :
    flash_bgo_status |= BIT_FLASH_EVENT_ERR_FAILURE;
    break;
  case FLASH_EVENT_ERR_ONE_BIT    :
    flash_bgo_status |= BIT_FLASH_EVENT_ERR_ONE_BIT;
    break;
  }
  tx_event_flags_set(&flash_bgo_flags,FLAG_FLASH_OP_DONE,TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param flags
  \param actual_flags
  \param wait_option

  \return uint32_t - TX_SUCCESS  если флаг получен
-----------------------------------------------------------------------------------------------------*/
uint32_t Wait_bgo_end(ULONG wait_option)
{
  ULONG  actual_flags = 0;
  return tx_event_flags_get(&flash_bgo_flags, FLAG_FLASH_OP_DONE, TX_OR_CLEAR,&actual_flags,  wait_option);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_bgo_status(void)
{
  return flash_bgo_status;
}

/*-----------------------------------------------------------------------------------------------------


  \param stat
-----------------------------------------------------------------------------------------------------*/
void Set_bgo_status(uint32_t stat)
{
  flash_bgo_status = stat;
}


/*-----------------------------------------------------------------------------------------------------
  Стираем область в DataFlash с асинхронно выполняемыми операциями Background Operation

  \param start_addr
  \param area_size

  \return int возвращает RES_OK если стирание произведено успешно
-----------------------------------------------------------------------------------------------------*/
uint32_t DataFlash_bgo_EraseArea(uint32_t start_addr, uint32_t area_size)
{
  ssp_err_t res;
  UINT      os_res;
  uint32_t  addr;
  uint32_t  num_blocks;
  addr = start_addr & 0xFFFFFFC0; // Корректирруем адрес чтобы он был выровнен по границе 64-х байт
  num_blocks = (((start_addr + area_size - 1) & 0xFFFFFFC0) + 0x40  - addr) / 0x40;

  Set_bgo_status(0);
  res = flash_bgo_cbl.p_api->erase(flash_bgo_cbl.p_ctrl, addr , num_blocks);
  if (res != SSP_SUCCESS)   return RES_ERROR;
  os_res = Wait_bgo_end(ms_to_ticks(100));
  if (os_res != TX_SUCCESS) return RES_ERROR;
  if ((Get_bgo_status() & BIT_FLASH_EVENT_ERASE_COMPLETE) == 0) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Записиываем в DataFlash с асинхронно выполняемыми операциями Background Operation

  \param start_addr
  \param buf
  \param buf_size  - размер буфера должен быть кратен 4, поскольку запись возможна только по 4-е байта с выравниванием по границе 4

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t DataFlash_bgo_WriteArea(uint32_t start_addr, uint8_t *buf, uint32_t buf_size)
{
  ssp_err_t res;
  UINT      os_res;

  Set_bgo_status(0);
  res = flash_bgo_cbl.p_api->write(flash_bgo_cbl.p_ctrl, (uint32_t) buf  , start_addr, buf_size);
  os_res = Wait_bgo_end(ms_to_ticks(100));
  if (os_res != TX_SUCCESS) return RES_ERROR;
  if (res != SSP_SUCCESS)   return RES_ERROR;
  if ((Get_bgo_status() & BIT_FLASH_EVENT_WRITE_COMPLETE) == 0) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param start_addr
  \param buf
  \param buf_size

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t DataFlash_bgo_ReadArea(uint32_t start_addr, uint8_t *buf, uint32_t buf_size)
{
  return  flash_bgo_cbl.p_api->read(flash_bgo_cbl.p_ctrl, buf  , start_addr , buf_size);
}


/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param buf
-----------------------------------------------------------------------------------------------------*/
void Read_OSIS_to_buf(uint8_t *buf, uint32_t sz)
{
  for (uint32_t i = 0; i < sz; i++) buf[i] = *(uint8_t *)(OSIS_ADDRESS + i);
}


/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param sz

  \return uint8_t - 0 если все байты ID равны 0xFF
-----------------------------------------------------------------------------------------------------*/
uint8_t Is_flash_protection_disabled(void)
{
  for (uint32_t i = 0; i < 16; i++)
  {
    if (*(uint8_t *)(OSIS_ADDRESS + i) != 0xFF)
    {
      return 0;
    }
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Становлена ли защита 3 уровня


  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Is_flash_protection_type_3(void)
{
  if (((*(uint8_t *)(OSIS_ADDRESS + 15)) & 0xC0)== 0x00)
  {
    return 1;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param pb

  \return const char*
-----------------------------------------------------------------------------------------------------*/
flash_id_code_mode_t Get_flash_protection_command(uint8_t protection_byte, uint8_t ID_clear_flag)
{
  if (ID_clear_flag) return FLASH_ID_CODE_MODE_UNLOCKED;

  switch (protection_byte & 0xC0)
  {
  case 0xC0:
    return FLASH_ID_CODE_MODE_LOCKED_WITH_ALL_ERASE_SUPPORT;
  case 0x80:
    return FLASH_ID_CODE_MODE_LOCKED;
  case 0x00:
    return FLASH_ID_CODE_MODE_FULL_LOCKED;
  }
  return FLASH_ID_CODE_MODE_UNLOCKED;
}


/*-----------------------------------------------------------------------------------------------------


  \param p_id_bytes
  \param mode

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t Set_Flash_protection(uint8_t const *const p_id_bytes, flash_id_code_mode_t mode)
{
  return flash_bgo_cbl.p_api->idCodeSet(flash_bgo_cbl.p_ctrl, p_id_bytes, mode);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Switch_Flash_driver_to_no_bgo(void)
{
  uint32_t res = RES_OK;
  if (Flash_driver_bgo_deinit() != SSP_SUCCESS)
  {
    res = RES_ERROR;
  }
  if (Flash_driver_init() != SSP_SUCCESS)
  {
    res = RES_ERROR;
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Switch_Flash_driver_to_bgo(void)
{
  uint32_t res = RES_OK;
  if (Flash_driver_deinit() != SSP_SUCCESS)
  {
    res = RES_ERROR;
  }
  if (Flash_driver_bgo_init() != SSP_SUCCESS)
  {
    res = RES_ERROR;
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param address
  \param num_bytes
  \param p_blank_check_result

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_blank_check(uint32_t const address, uint32_t const num_bytes, flash_result_t *const p_blank_check_result)
{
  return flash_cbl.p_api->blankCheck(flash_cbl.p_ctrl, address, num_bytes , p_blank_check_result);
}


/*-----------------------------------------------------------------------------------------------------


  \param address
  \param num_blocks

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_erase_block(uint32_t const address, uint32_t const num_blocks)
{
  return flash_cbl.p_api->erase(flash_cbl.p_ctrl, address, num_blocks);
}

/*-----------------------------------------------------------------------------------------------------


  \param src_address
  \param flash_address
  \param num_bytes

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_write_block(uint32_t const src_address, uint32_t const flash_address, uint32_t const num_bytes)
{
  return flash_cbl.p_api->write(flash_cbl.p_ctrl, (uint32_t)src_address, flash_address, num_bytes);
}

