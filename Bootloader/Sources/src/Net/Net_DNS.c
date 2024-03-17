// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-03-07
// 18:32:36
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "S7V30.h"
#include   "nxd_dns.h"


static NX_DNS                  dns_client;
static NX_IP                  *dns_ip_ptr;

#define DNS_LOCAL_CACHE_SIZE   512
UCHAR                          dns_local_cache[DNS_LOCAL_CACHE_SIZE];

/*-----------------------------------------------------------------------------------------------------


  \param ip_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Net_DNS_client_create(NX_IP  *ip_ptr)
{
  UINT status;

  if (dns_ip_ptr != 0) return RES_ERROR;

  #ifndef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    #error NX_DNS_CLIENT_USER_CREATE_PACKET_POOL must be defined
  #endif

  status = nx_dns_create(&dns_client,ip_ptr,(UCHAR *)"DNS Client");
  if (status != NX_SUCCESS)
  {
    APPLOG("DNS client creating error %d", status);
    return RES_ERROR;
  }

  #ifndef NX_DNS_CACHE_ENABLE
    #error NX_DNS_CACHE_ENABLE must be defined
  #endif

  status = nx_dns_cache_initialize(&dns_client, dns_local_cache, DNS_LOCAL_CACHE_SIZE);
  if (status != NX_SUCCESS)
  {
    nx_dns_delete(&dns_client);
    APPLOG("DNS cache creating error %d", status);
    return RES_ERROR;
  }

  status = nx_dns_packet_pool_set(&dns_client,&net_packet_pool);
  if (status != NX_SUCCESS)
  {
    nx_dns_delete(&dns_client);
    APPLOG("DNS pool setting error %d", status);
    return RES_ERROR;
  }

// Вызов здесь это функции может вызвать ошибку  NX_DNS_DUPLICATE_ENTRY
// Поскольку gateway_address по умолчаню является и адресом DNS сервера
//  if ((g_network_type == NET_BY_WIFI_AP) &&(ivar.wifi_ap_addr_assign_method == IP_ADDRESS_ASSIGNMENT_METHOD_WINDOWS_HOME_NETWORK))
//  {
//    status = nx_dns_server_add(&dns_client, wifi_ap_net_props.gateway_address);
//    if (status != NX_SUCCESS)
//    {
//      nx_dns_delete(&dns_client);
//      APPLOG("DNS server addition error %d", status);
//      return RES_ERROR;
//    }
//  }


  APPLOG("DNS client created successfully");
  dns_ip_ptr = ip_ptr;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Net_DNS_client_delete(void)
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

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void DNS_client_controller(void)
{
  NX_IP   *ip_ptr = NULL;

  switch (g_network_type)
  {
  case NET_BY_WIFI_STA:
    if (WIFI_STA_network_active_flag()) ip_ptr = wifi_sta_ip_ptr;
    break;
  case NET_BY_WIFI_AP:
    if (WIFI_AP_network_active_flag())  ip_ptr = wifi_ap_ip_ptr;
    break;
  case NET_BY_RNDIS:
    if (RNDIS_network_active_flag())    ip_ptr = rndis_ip_ptr;
    break;
  case NET_BY_ECM:
    if (ECM_Host_network_active_flag()) ip_ptr = ecm_host_ip_ptr;
    break;
  }

  if (ip_ptr != NULL)
  {
    Net_DNS_client_create(ip_ptr);
  }
  else
  {
    Net_DNS_client_delete();
  }

}


