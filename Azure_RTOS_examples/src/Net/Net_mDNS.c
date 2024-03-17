// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-04-10
// 14:57:00
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "nx_web_http_common.h"

NX_MDNS mdns;

#define MDNS_PRIORITY                   3
#define MDNS_LOCAL_SERVICE_CACHE_SIZE   2048
#define MDNS_PEER_SERVICE_CACHE_SIZE    2048

static ULONG mdns_thread_stack[512]  BSP_PLACE_IN_SECTION_V2(".stack.mdns_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);


static ULONG local_service_cache[MDNS_LOCAL_SERVICE_CACHE_SIZE / sizeof(ULONG)];
static ULONG peer_service_cache[MDNS_PEER_SERVICE_CACHE_SIZE / sizeof(ULONG)];

/*-----------------------------------------------------------------------------------------------------


  \param mdns_ptr
  \param name
  \param state
-----------------------------------------------------------------------------------------------------*/
static VOID  _mDNS_probing_notify(struct NX_MDNS_STRUCT *mdns_ptr, UCHAR *name, UINT state)
{
  switch (state)
  {
  case NX_MDNS_LOCAL_SERVICE_REGISTERED_SUCCESS:
    {
      APPLOG("mDNS Service: %s Registered success.",name);
      break;
    }
  case NX_MDNS_LOCAL_SERVICE_REGISTERED_FAILURE:
    {
      APPLOG("mDNS Service: %s Registered failure.",name);
      break;
    }
  case NX_MDNS_LOCAL_HOST_REGISTERED_SUCCESS:
    {
      APPLOG("mDNS Host: %s Registered success.",name);
      break;
    }
  case NX_MDNS_LOCAL_HOST_REGISTERED_FAILURE:
    {
      APPLOG("mDNS Host: %s Registered failure!",name);
      break;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param instance
  \param type
  \param subtype
  \param txt
  \param ttl
  \param priority
  \param weight
  \param port
  \param is_unique
-----------------------------------------------------------------------------------------------------*/
static uint32_t register_local_service(UCHAR *instance, UCHAR *type, UCHAR *subtype, UCHAR *txt, UINT ttl, UINT priority, UINT weight, UINT port, UINT is_unique)
{
  UINT status;
  status = nx_mdns_service_add(&mdns, instance, type, subtype, txt, ttl, (USHORT)priority, (USHORT)weight, (USHORT)port, (UCHAR)is_unique, 0);
  if (status != NX_MDNS_SUCCESS)
  {
    APPLOG("mDNS Local Service Added: %s %s failed", instance, type);
  }
  else
  {
    APPLOG("mDNS Local Service Added: %s %s successfully", instance, type);
  }
  return status;
}


/*-----------------------------------------------------------------------------------------------------


  \param ip_ptr
  \param mdns_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t mDNS_start(NX_IP *ip_ptr, NX_MDNS *mdns_ptr)
{
  UINT status;
  status =  nx_mdns_create(mdns_ptr,ip_ptr,&net_packet_pool, MDNS_PRIORITY, mdns_thread_stack,
                           sizeof(mdns_thread_stack), (UCHAR *)ivar.this_host_name,
                           (VOID *)local_service_cache, sizeof(local_service_cache),
                           (VOID *)peer_service_cache, sizeof(peer_service_cache), _mDNS_probing_notify);

  if (status != NX_MDNS_SUCCESS)
  {
    APPLOG("mDNS creating with host name %s error = 0x%04X",ivar.this_host_name, status);
    return status;
  }
  else
  {
    APPLOG("mDNS created successfully. Host name: %s", ivar.this_host_name);
  }

  static      UINT     service1_ttl      = 120;
  static      UINT     service1_priority = 0;
  static      UINT     service1_weights  = 0;

  status = register_local_service("HTTP server", "_http._tcp", NX_NULL, NX_NULL, service1_ttl, service1_priority, service1_weights, NX_WEB_HTTP_SERVER_PORT, NX_TRUE);
  if (status != NX_MDNS_SUCCESS)
  {
    APPLOG("mDNS registering local HTTP service error = 0x%04X",status);
  }
  status = register_local_service("HTTPS server", "_https._tcp", NX_NULL, NX_NULL, service1_ttl, service1_priority, service1_weights, NX_WEB_HTTPS_SERVER_PORT, NX_TRUE);
  if (status != NX_MDNS_SUCCESS)
  {
    APPLOG("mDNS registering local HTTPS service error = 0x%04X",status);
  }
  status = register_local_service("FTP server", "_ftp._tcp", NX_NULL, NX_NULL, service1_ttl, service1_priority, service1_weights, NX_FTP_SERVER_CONTROL_PORT, NX_TRUE);
  if (status != NX_MDNS_SUCCESS)
  {
    APPLOG("mDNS registering local FTP service error = 0x%04X",status);
  }
  status = register_local_service("Telnet server", "_telnet._tcp", NX_NULL, NX_NULL, service1_ttl, service1_priority, service1_weights, NX_TELNET_SERVER_PORT, NX_TRUE);
  if (status != NX_MDNS_SUCCESS)
  {
    APPLOG("mDNS registering local Telnet service error = 0x%04X",status);
  }

  status = nx_mdns_enable(mdns_ptr , ip_ptr->nx_ip_interface->nx_interface_index);
  if (status != NX_MDNS_SUCCESS)
  {
    APPLOG("mDNS enabling error = 0x%04X",status);
    return status;
  }

  APPLOG("mDNS enabled successfilly.");

  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param mdns_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t mDNS_delete(NX_MDNS *mdns_ptr)
{
  return nx_mdns_delete(mdns_ptr);
}

