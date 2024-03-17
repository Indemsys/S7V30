#include  "S7V30.h"
#include "Modbus.h"

/*-----------------------------------------------------------------------------------------------------


  \param value
  \param pbuf

  \return uint8_t*
-----------------------------------------------------------------------------------------------------*/
uint8_t* MB_ASCII_BinToHex(uint8_t  value,uint8_t *pbuf)
{
  uint8_t  nibble;


  nibble =(value >> 4) & 0x0F;
  if (nibble <= 9)
  {
    *pbuf++= (uint8_t)(nibble + '0');
  }
  else
  {
    *pbuf++= (uint8_t)(nibble - 10 + 'A');
  }

  nibble = value & 0x0F;
  if (nibble <= 9)
  {
    *pbuf++= (uint8_t)(nibble + '0');
  }
  else
  {
    *pbuf++= (uint8_t)(nibble - 10 + 'A');
  }
  return (pbuf);
}


/*-----------------------------------------------------------------------------------------------------


  \param phex

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  MB_ASCII_HexToBin(uint8_t  *phex)
{
  uint8_t  value;
  uint8_t  high;
  uint8_t  low;


  high =*phex;
  phex++;
  low  =*phex;
  if (high <= '9')
  {
    value  = (uint8_t)(high - '0');
  }
  else if (high <= 'F')
  {
    value  = (uint8_t)(high - 'A' + 10);
  }
  else
  {
    value  = (uint8_t)(high - 'a' + 10);
  }
  value <<= 4;

  if (low <= '9')
  {
    value += (uint8_t)(low - '0');
  }
  else if (low <= 'F')
  {
    value += (uint8_t)(low - 'A' + 10);
  }
  else
  {
    value += (uint8_t)(low - 'a' + 10);
  }
  return (value);
}


/*-----------------------------------------------------------------------------------------------------


  \param pch

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  MB_ASCII_RxCalcLRC(T_MODBUS_ch  *pch)
{
  uint8_t   lrc;
  uint16_t   len;
  uint8_t  *pblock;


  len    =(pch->rx_packet_size - 5) / 2;
  pblock = (uint8_t *)&pch->rx_buf[1];
  lrc    = 0;
  while (len-- > 0)
  {
    lrc    += MB_ASCII_HexToBin(pblock);
    pblock += 2;
  }

  lrc = ~lrc + 1;
  return (lrc);
}


/*-----------------------------------------------------------------------------------------------------


  \param pch
  \param tx_bytes

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  MB_ASCII_TxCalcLRC(T_MODBUS_ch  *pch, uint16_t tx_bytes)
{
  uint8_t     lrc;
  uint16_t     len;
  uint8_t    *pblock;


  len    =(tx_bytes - 1) / 2;
  pblock = (uint8_t *)&pch->tx_buf[1];
  lrc    = 0;
  while (len-- > 0)
  {
    lrc    += MB_ASCII_HexToBin(pblock);
    pblock += 2;
  }
  lrc = ~lrc + 1;
  return (lrc);
}


/*-----------------------------------------------------------------------------------------------------


  \param pch

  \return uint16_t
-----------------------------------------------------------------------------------------------------*/
uint16_t  MB_RTU_RxCalcCRC(T_MODBUS_ch  *pch)
{
  uint16_t      crc;
  uint8_t      shiftctr;
  uint8_t     flag;
  uint16_t      length;
  uint8_t     *pblock;


  pblock = (uint8_t *)&pch->rx_frame_buf[0];
  length = pch->rx_data_cnt + 2;
  crc    = 0xFFFF;
  while (length > 0)
  {
    length--;
    crc      ^= (uint16_t)*pblock++;
    shiftctr  = 8;
    do
    {
      flag   =(crc & 0x0001)? 1 : 0;
      crc  >>= 1;
      if (flag == 1)
      {
        crc ^= MODBUS_CRC16_POLY;
      }
      shiftctr--;
    } while (shiftctr > 0);
  }
  pch->rx_packet_crc = crc;
  return (crc);
}

/*-----------------------------------------------------------------------------------------------------


  \param pch

  \return uint16_t
-----------------------------------------------------------------------------------------------------*/
uint16_t  MB_RTU_TxCalcCRC(T_MODBUS_ch *pch)
{
  uint16_t      crc;
  uint8_t      shiftctr;
  uint8_t     flag;
  uint16_t      length;
  uint8_t     *pblock;


  pblock = (uint8_t *)&pch->tx_frame_buf[0];
  length = pch->tx_data_cnt + 2;
  crc    = 0xFFFF;
  while (length > 0)
  {
    length--;
    crc      ^= (uint16_t)*pblock++;
    shiftctr  = 8;
    do
    {
      flag   =(crc & 0x0001)? 1 : 0;
      crc  >>= 1;
      if (flag == 1)
      {
        crc ^= MODBUS_CRC16_POLY;
      }
      shiftctr--;
    } while (shiftctr > 0);
  }
  return (crc);
}

