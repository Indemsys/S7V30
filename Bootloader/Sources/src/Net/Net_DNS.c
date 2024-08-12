// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-03-07
// 18:32:36
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net.h"


static NX_DNS                  dns_client;
static NX_IP                  *dns_ip_ptr;

#define DNS_LOCAL_CACHE_SIZE   512
UCHAR                          dns_local_cache[DNS_LOCAL_CACHE_SIZE];

/*-----------------------------------------------------------------------------------------------------


  \param ip_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_DNS_client_create(NX_IP  *ip_ptr)
{
  UINT status;

  if (dns_ip_ptr != 0) return RES_ERROR;

  #ifndef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    #error NX_DNS_CLIENT_USER_CREATE_PACKET_POOL must be defined
  #endif

  status = nx_dns_create(&dns_client,ip_ptr,(UCHAR *)"DNS Client");
  if (status != NX_SUCCESS)
  {
    NETLOG("DNS client creating error %d", status);
    return RES_ERROR;
  }

  #ifndef NX_DNS_CACHE_ENABLE
    #error NX_DNS_CACHE_ENABLE must be defined
  #endif

  status = nx_dns_cache_initialize(&dns_client, dns_local_cache, DNS_LOCAL_CACHE_SIZE);
  if (status != NX_SUCCESS)
  {
    nx_dns_delete(&dns_client);
    NETLOG("DNS cache creating error %d", status);
    return RES_ERROR;
  }

  status = nx_dns_packet_pool_set(&dns_client,&net_packet_pool);
  if (status != NX_SUCCESS)
  {
    nx_dns_delete(&dns_client);
    NETLOG("DNS pool setting error %d", status);
    return RES_ERROR;
  }


  NETLOG("DNS client created successfully");
  dns_ip_ptr = ip_ptr;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_DNS_client_delete(void)
{
  if (dns_ip_ptr == 0) return RES_ERROR;

  nx_dns_delete(&dns_client);

  dns_ip_ptr = 0;

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param host_name_ptr
  \param dns_address
  \param wait_option

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT DNS_get_host_address(UCHAR *host_name_ptr, ULONG *host_address, ULONG wait_option)
{
  if (dns_ip_ptr == 0) return NX_NOT_CREATED;
  return nx_dns_host_by_name_get(&dns_client, host_name_ptr, host_address, wait_option);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Is_DNS_created(void)
{
  if (dns_ip_ptr == 0) return 0;
  return 1;
}


