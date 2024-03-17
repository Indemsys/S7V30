/* generated configuration header file - do not edit */
#ifndef NXD_DHCPV6_SERVER_CFG_H_
#define NXD_DHCPV6_SERVER_CFG_H_
#define NX_DHCPV6_SERVER_THREAD_PRIORITY               (1)
            #define NX_DHCPV6_IP_LEASE_TIMER_INTERVAL              (60)
            #define NX_DHCPV6_PACKET_WAIT_OPTION                   (1)
            #define NX_DHCPV6_PREFERENCE_VALUE                     (0)
            #define NX_DHCPV6_MAX_OPTION_REQUEST_OPTIONS           (6)
            #define NX_DHCPV6_DEFAULT_T1_TIME                      (2000)
            #define NX_DHCPV6_DEFAULT_T2_TIME                      (3000)
            #define NX_DHCPV6_STATUS_MESSAGE_MAX                   (100)
            #define NX_DHCPV6_MAX_LEASES                           (100)
            #define NX_DHCPV6_MAX_CLIENTS                          (120)
            #define NX_DHCPV6_FRAGMENT_OPTION                      (NX_DONT_FRAGMENT)
            #define NX_DHCPV6_DEFAULT_PREFERRED_TIME               (2 * NX_DHCPV6_DEFAULT_T1_TIME)
            #define NX_DHCPV6_DEFAULT_VALID_TIME                   (2 * NX_DHCPV6_DEFAULT_PREFERRED_TIME)
            #define NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_ID       "abcdeffghijklmnopqrstuvwxyz"
            #define NX_DHCPV6_SERVER_DUID_VENDOR_PRIVATE_ID        (0x12345678)
            #define NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH   (48)
            #define NX_DHCPV6_STATUS_MESSAGE_SUCCESS               "IA OPTION GRANTED"
            #define NX_DHCPV6_STATUS_MESSAGE_UNSPECIFIED           "IA OPTION NOT GRANTED-FAILURE UNSPECIFIED"
            #define NX_DHCPV6_STATUS_MESSAGE_NO_ADDRS_AVAILABLE    "IA OPTION NOT GRANTED-NO ADDRESSES AVAILABLE"
            #define NX_DHCPV6_STATUS_MESSAGE_NO_BINDING            "IA OPTION NOT GRANTED-INVALID CLIENT REQUEST"
            #define NX_DHCPV6_STATUS_MESSAGE_NOT_ON_LINK           "IA OPTION NOT GRANTED-CLIENT NOT ON LINK"
            #define NX_DHCPV6_STATUS_MESSAGE_USE_MULTICAST         "IA OPTION NOT GRANTED-CLIENT MUST USE MULTICAST"
            #define NX_DHCPV6_SESSION_TIMEOUT                      (20)
#endif /* NXD_DHCPV6_SERVER_CFG_H_ */
