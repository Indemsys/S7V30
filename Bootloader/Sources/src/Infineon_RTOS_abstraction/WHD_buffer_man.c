// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-05-12
// 15:22:08
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "whd.h"
#include   "whd_types_int.h"
#include   "whd_network_types.h"
#include   "WHD_buffer_man.h"

extern NX_PACKET_POOL          net_packet_pool;

#define HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_BUS_HEADER_SIZE    (sizeof(whd_buffer_header_t) + BDC_HEADER_WITH_PAD + SDPCM_HEADER)
#define HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE          (HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_BUS_HEADER_SIZE + WHD_ETHERNET_SIZE)

//--------------------------------------------------------------------------------------------------
// cy_buffer_allocate_dynamic_packet
//--------------------------------------------------------------------------------------------------
#define BLOCK_SIZE_ALIGNMENT                    (64)
static NX_PACKET* cy_buffer_allocate_dynamic_packet(uint16_t payload_size)
{
  NX_PACKET *packet;
  ULONG header_size;
  UINT total_size;

  //
  // Allocate a dynamic packet to satisfy a request for a payload size that is larger than  the size in the packet pool.
  //
  // NOTE: This API is only used for WHD communications to support IOVARS with payloads larger than WHD_LINK_MTU.
  // The nx_packet_pool_owner pointer in the packet is left as NULL.
  // When the packet is released in WHD_buffer_release, the NULL nx_packet_pool_owner  pointer will trigger a direct free rather than trying to release back to the packet pool.
  //
  // Packets allocated via this API should never be passed to the IP stack as they
  // can not be released properly by the stack since they do not belong to a packet pool.
  //

  total_size =(payload_size + BLOCK_SIZE_ALIGNMENT + sizeof(NX_PACKET)+ 1);

  packet = App_malloc(total_size);
  if (packet)
  {
    // Initialize the packet structure elements that are needed for use.
    header_size                   = (ULONG)(((sizeof(NX_PACKET)+ NX_PACKET_ALIGNMENT - 1) / NX_PACKET_ALIGNMENT) * NX_PACKET_ALIGNMENT);
    packet->nx_packet_data_start  = (UCHAR *)((uint32_t)packet + header_size);
    packet->nx_packet_data_end    = (UCHAR *)((uint32_t)packet + header_size + payload_size);
    packet->nx_packet_prepend_ptr = packet->nx_packet_data_start;
    packet->nx_packet_append_ptr  = (UCHAR *)((uint32_t)packet->nx_packet_prepend_ptr + payload_size);
    packet->nx_packet_length      = payload_size;
  }

  return packet;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer
  \param direction
  \param size
  \param timeout_ms

  \return whd_result_t
-----------------------------------------------------------------------------------------------------*/
whd_result_t WHD_host_buffer_get(whd_buffer_t *buffer, whd_buffer_dir_t direction, uint16_t size, uint32_t timeout_ms)
{
  UINT             status = NX_NO_PACKET;
  NX_PACKET      **nx_buffer = (NX_PACKET **)buffer;

  if (buffer == NULL) return WHD_BADARG;

  if (size > WHD_LINK_MTU)
  {
    // Request for a packet with a payload larger than MTU. Try to create a dynamically allocated packet to satisfy the request.
    *nx_buffer = cy_buffer_allocate_dynamic_packet(size);
    if (*nx_buffer == NULL) return WHD_BUFFER_UNAVAILABLE_PERMANENT;
    return WHD_SUCCESS;
  }

  // WHD_RTT_LOGs("\r\n");
  status = nx_packet_allocate(&net_packet_pool, nx_buffer, 0, ms_to_ticks(timeout_ms));


  //  WHD_RTT_LOGs("- Get packet ");
  if (status != NX_SUCCESS)
  {
    //    WHD_RTT_LOGs("  Error\r\n");
    return WHD_BUFFER_ALLOC_FAIL;
  }
  //  WHD_RTT_LOGs(" %08X sz=%d\r\n", (uint32_t)*nx_buffer, size);

  (*nx_buffer)->nx_packet_length = size;
  (*nx_buffer)->nx_packet_append_ptr =(*nx_buffer)->nx_packet_prepend_ptr + size;

  return WHD_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer
  \param direction
-----------------------------------------------------------------------------------------------------*/
void         WHD_buffer_release(whd_buffer_t buffer, whd_buffer_dir_t direction)
{
  NX_PACKET *nx_buffer = (NX_PACKET *) buffer;

  if (nx_buffer->nx_packet_pool_owner == NULL)
  {
    App_free(nx_buffer); // This was a dynamically allocated packet since it is not part of a packet pool.  Free it directly.
    return;
  }

  if (direction == WHD_NETWORK_TX)
  {
    //  TCP transmit packet isn't returned immediately to the pool.
    //  The stack holds the packet temporarily until ACK is received. Otherwise, the same packet is used for
    //  re-transmission. Return prepend pointer to the original location which the stack expects (the start of IP header). For other packets, resetting prepend pointer isn't required.
    //
    if (nx_buffer->nx_packet_length > HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE)
    {
      WHD_buffer_add_remove_at_front(&buffer, HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE);
    }
    nx_packet_transmit_release(nx_buffer);
  }
  else
  {
    nx_packet_release(nx_buffer);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer

  \return uint8_t*
-----------------------------------------------------------------------------------------------------*/
uint8_t* WHD_buffer_get_current_piece_data_pointer(whd_buffer_t buffer)
{
  NX_PACKET *nx_buffer = (NX_PACKET *) buffer;
  return nx_buffer->nx_packet_prepend_ptr;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer

  \return uint16_t
-----------------------------------------------------------------------------------------------------*/
uint16_t     WHD_buffer_get_current_piece_size(whd_buffer_t buffer)
{
  NX_PACKET *nx_buffer = (NX_PACKET *) buffer;
  return (unsigned short) nx_buffer->nx_packet_length;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer
  \param size

  \return whd_result_t
-----------------------------------------------------------------------------------------------------*/
whd_result_t WHD_buffer_set_size(whd_buffer_t buffer, uint16_t size)
{
  NX_PACKET *nx_buffer = (NX_PACKET *) buffer;

  if ((nx_buffer->nx_packet_prepend_ptr + size) > nx_buffer->nx_packet_data_end)
  {
    return WHD_BUFFER_SIZE_SET_ERROR;
  }

  nx_buffer->nx_packet_length = size;
  nx_buffer->nx_packet_append_ptr = nx_buffer->nx_packet_prepend_ptr + size;
  return WHD_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param buffer
  \param add_remove_amount

  \return whd_result_t
-----------------------------------------------------------------------------------------------------*/
whd_result_t WHD_buffer_add_remove_at_front(whd_buffer_t *buffer, int32_t add_remove_amount)
{
  NX_PACKET **nx_buffer = (NX_PACKET **) buffer;
  UCHAR *new_start =(*nx_buffer)->nx_packet_prepend_ptr + add_remove_amount;

  if (new_start < (*nx_buffer)->nx_packet_data_start)
  {
    return WHD_BUFFER_POINTER_MOVE_ERROR;  // Trying to move to a location before start - not supported without buffer chaining
  }
  else if (new_start > (*nx_buffer)->nx_packet_data_end)
  {
    return WHD_BUFFER_POINTER_MOVE_ERROR;  // Trying to move to a location after end of buffer - not supported without buffer chaining
  }
  else
  {
    (*nx_buffer)->nx_packet_prepend_ptr = new_start;
    if ((*nx_buffer)->nx_packet_append_ptr < (*nx_buffer)->nx_packet_prepend_ptr)
    {
      (*nx_buffer)->nx_packet_append_ptr =(*nx_buffer)->nx_packet_prepend_ptr;
    }
    (*nx_buffer)->nx_packet_length = (ULONG)((*nx_buffer)->nx_packet_length - (ULONG) add_remove_amount);
  }
  return WHD_SUCCESS;
}

