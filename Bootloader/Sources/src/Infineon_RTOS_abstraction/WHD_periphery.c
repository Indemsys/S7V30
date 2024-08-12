// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-05-12
// 15:31:04
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "whd.h"
#include   "whd_bus_sdio_protocol.h"
#include   "cyhal_sdio.h"
#include   "WHD_periphery.h"

#ifdef ENABLE_WIFI_DRV_DEBUG_PRINT
  #define WIFI_DRV_PRINT(...)      WHD_RTT_LOGs(##__VA_ARGS__);
#else
  #define WIFI_DRV_PRINT(...)
#endif

/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param event
  \param enable
-----------------------------------------------------------------------------------------------------*/
void cyhal_sdio_irq_enable(cyhal_sdio_t *obj, cyhal_sdio_irq_event_t irq_event, bool enable)
{
  if (irq_event == CYHAL_SDIO_CARD_INTERRUPT)
  {
    if (enable == WHD_TRUE)
    {
      SDIO1_irq_enable();
    }
    else
    {
      SDIO1_irq_disable();
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
   Регистрируем функцию вызываемую из прерывания обслуживающего прием данных из SDIO

  \param obj
  \param handler
  \param handler_arg
-----------------------------------------------------------------------------------------------------*/
void cyhal_sdio_register_irq(cyhal_sdio_t *obj, cyhal_sdio_irq_handler_t handler, void *handler_arg)
{

}

/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param direction
  \param argument
  \param data
  \param length
  \param response

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cyhal_sdio_bulk_transfer(cyhal_sdio_t *obj, cyhal_transfer_t direction, uint32_t argument, const uint32_t *data, uint16_t length, uint32_t *response)
{
  static ssp_err_t          status;
  sdmmc_io_transfer_mode_t  tmode;
  const sdmmc_instance_t   *sdio_inst_ptr = (sdmmc_instance_t *)(*obj);

  whd_bus_sdio_cmd53_argument_t *arg = (whd_bus_sdio_cmd53_argument_t *)&argument;

  if (arg->block_mode == 0)
  {
    tmode = SDMMC_IO_MODE_TRANSFER_BYTE;
  }
  else
  {
    tmode = SDMMC_IO_MODE_TRANSFER_BLOCK;
  }

  if (direction == CYHAL_WRITE)
  {
    if (tmode == SDMMC_IO_MODE_TRANSFER_BLOCK)
    {
      WIFI_DRV_PRINT("WB> A:%08X F:%d Sz=%02X", arg->register_address, arg->function_number, arg->count * 64);
      status = sdio_inst_ptr->p_api->writeIoExt(sdio_inst_ptr->p_ctrl, (uint8_t *)data, arg->function_number, arg->register_address , arg->count,  SDMMC_IO_MODE_TRANSFER_BLOCK, SDMMC_IO_ADDRESS_MODE_INCREMENT);
      if (status == SSP_SUCCESS)
      {
        if (SDIO1_wait_transfer_complete(10) != RES_OK)
        {
          WIFI_DRV_PRINT(" Transfer error\r\n");
          status = SSP_ERR_ABORTED;
        }
        else
        {
          WIFI_DRV_PRINT("  Ok\r\n");
        }
      }
    }
    else
    {
      WIFI_DRV_PRINT("WB> A:%08X F:%d Sz=%02X", arg->register_address, arg->function_number, arg->count);
      status = sdio_inst_ptr->p_api->writeIoExt(sdio_inst_ptr->p_ctrl,(uint8_t const*)data, arg->function_number, arg->register_address , arg->count,  tmode, SDMMC_IO_ADDRESS_MODE_INCREMENT);
      if (status == SSP_SUCCESS)
      {
        if (SDIO1_wait_transfer_complete(10) != RES_OK)
        {
          WIFI_DRV_PRINT(" Transfer error\r\n");
          status = SSP_ERR_ABORTED;
        }
        else
        {
          #ifdef DEBUG_RTT_PRINT
          uint32_t n = arg->count;
          if (n <= 64)
          {
            for (uint32_t i = 0; i < n; i++)
            {
              WIFI_DRV_PRINT(" %02X", ((uint8_t const *)data)[i]);
            }
          }
          WIFI_DRV_PRINT("  Ok\r\n");
          #endif
        }
      }
      else
      {
        WIFI_DRV_PRINT("  Cmd Error= %08X\r\n", status);
      }
    }
  }
  else
  {
    uint32_t cnt = arg->count;
    WIFI_DRV_PRINT("RB< A:%08X F:%d Sz=%02X", arg->register_address, arg->function_number, cnt);
    status = sdio_inst_ptr->p_api->readIoExt(sdio_inst_ptr->p_ctrl, (uint8_t *)data, arg->function_number, arg->register_address ,&cnt,  tmode, SDMMC_IO_ADDRESS_MODE_INCREMENT);
    if (status == SSP_SUCCESS)
    {
      if (SDIO1_wait_transfer_complete(10) != RES_OK)
      {
        WIFI_DRV_PRINT(" Transfer error\r\n");
        status = SSP_ERR_ABORTED;
      }
      else
      {
        #ifdef DEBUG_RTT_PRINT
        uint32_t n = arg->count;
        if (n <= 64)
        {
          for (uint32_t i = 0; i < n; i++)
          {
            WIFI_DRV_PRINT(" %02X", ((uint8_t const *)data)[i]);
          }
        }
        WIFI_DRV_PRINT("  Ok\r\n");
        #endif
      }
    }
    else
    {
      WIFI_DRV_PRINT("  Error= %08X\r\n", status);
    }
  }

  if (status != SSP_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}


/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param direction
  \param command
  \param argument
  \param response

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cyhal_sdio_send_cmd(const cyhal_sdio_t *obj, cyhal_transfer_t direction, cyhal_sdio_command_t command, uint32_t argument, uint32_t *response)
{
  ssp_err_t                       status;
  const sdmmc_instance_t         *sdio_inst_ptr = (sdmmc_instance_t *)(*obj);
  whd_bus_sdio_cmd52_argument_t  *arg = (whd_bus_sdio_cmd52_argument_t *)&argument;

  if (command != CYHAL_SDIO_CMD_IO_RW_DIRECT)
  {
    return CY_RTOS_GENERAL_ERROR;
  }

  if (direction == CYHAL_WRITE)
  {
    WIFI_DRV_PRINT("W > A:%08X F:%d = %02X", arg->register_address, arg->function_number, arg->write_data);
    // CMD52 запись одного байта в регистр
    status = sdio_inst_ptr->p_api->writeIo(sdio_inst_ptr->p_ctrl,&arg->write_data, arg->function_number, arg->register_address , (sdmmc_io_write_mode_t)arg->raw_flag);
    if (status == SSP_SUCCESS)
    {
      WIFI_DRV_PRINT("  Ok\r\n");
    }
    else
    {
      WIFI_DRV_PRINT("  Error= %08X\r\n", status);
    }
  }
  else
  {
    WIFI_DRV_PRINT("R < A:%08X F:%d = ", arg->register_address, arg->function_number);
    // CMD52 чтение одного байта из регистра
    status = sdio_inst_ptr->p_api->readIo(sdio_inst_ptr->p_ctrl,&arg->write_data, arg->function_number, arg->register_address);
    if (status == SSP_SUCCESS)
    {
      WIFI_DRV_PRINT("%02X  Ok\r\n",arg->write_data);
    }
    else
    {
      WIFI_DRV_PRINT("?   Error= %08X\r\n", status);
    }
  }

  if (response) *response = arg->write_data;


  if (status != SSP_SUCCESS)
  {
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param pin
  \param event
  \param enable
-----------------------------------------------------------------------------------------------------*/
void cyhal_gpio_irq_enable(cyhal_gpio_t pin, cyhal_gpio_irq_event_t event, bool enable)
{


}


/*-----------------------------------------------------------------------------------------------------


  \param pin
  \param intrPriority
  \param handler
  \param handler_arg
-----------------------------------------------------------------------------------------------------*/
void cyhal_gpio_register_irq(cyhal_gpio_t pin, uint8_t intrPriority, cyhal_gpio_irq_handler_t handler, void *handler_arg)
{


}

/*-----------------------------------------------------------------------------------------------------


  \param pin
  \param direction
  \param drvMode
  \param initVal

  \return cy_rslt_t
-----------------------------------------------------------------------------------------------------*/
cy_rslt_t cyhal_gpio_init(cyhal_gpio_t pin, cyhal_gpio_direction_t direction, cyhal_gpio_drive_mode_t drvMode, bool initVal)
{
  uint32_t   reg;
  T_reg_PFS  *pfs;

  if (pin == 0) return CY_RSLT_SUCCESS;

  reg = *pin;
  pfs = (T_reg_PFS *)reg;

  pfs->PSEL = PSEL_00;
  pfs->PMR  = PORT__IO;

  switch (direction)
  {
  case  CYHAL_GPIO_DIR_INPUT         :
    pfs->PDR = GP_INP;
    break;
  case  CYHAL_GPIO_DIR_OUTPUT        :
    pfs->PDR = GP_OUT;
    break;
  case  CYHAL_GPIO_DIR_BIDIRECTIONAL :
    pfs->PDR = GP_INP;
    break;
  }

  pfs->DSCR = LO_DRV;

  switch (drvMode)
  {
  case CYHAL_GPIO_DRIVE_NONE                :
    pfs->PDR = GP_INP;
    pfs->PCR = PULLUP_DIS;
    break;
  case CYHAL_GPIO_DRIVE_ANALOG              :
    pfs->PCR = PULLUP_DIS;
    break;
  case CYHAL_GPIO_DRIVE_PULLUP              :
    pfs->PCR = PULLUP__EN;
    break;
  case CYHAL_GPIO_DRIVE_PULLDOWN            :
    pfs->PCR = PULLUP_DIS;
    break;
  case CYHAL_GPIO_DRIVE_OPENDRAINDRIVESLOW  :
    pfs->PCR = PULLUP_DIS;
    break;
  case CYHAL_GPIO_DRIVE_OPENDRAINDRIVESHIGH :
    pfs->PCR = PULLUP_DIS;
    break;
  case CYHAL_GPIO_DRIVE_STRONG              :
    pfs->PCR  = PULLUP_DIS;
    pfs->DSCR = HI_DRV;
    break;
  case CYHAL_GPIO_DRIVE_PULLUPDOWN          :
    pfs->PCR = PULLUP_DIS;
    break;
  }

  pfs->PODR = initVal;

  return CY_RSLT_SUCCESS;
}



