#ifndef WIFI_DEBUG_H
  #define WIFI_DEBUG_H

//#define ENABLE_WIFI_DRV_DEBUG_PRINT
//#define ENABLE_WIFI_MAC_DEBUG_PRINT


#ifdef ENABLE_WIFI_DRV_DEBUG_PRINT
  #define WIFI_DRV_PRINT(...)      WHD_RTT_LOGs(##__VA_ARGS__);
#else
  #define WIFI_DRV_PRINT(...)
#endif

#ifdef ENABLE_WIFI_MAC_DEBUG_PRINT
  #define WIFI_MAC_PRINT(...)      WHD_RTT_LOGs(##__VA_ARGS__);
#else
  #define WIFI_MAC_PRINT(...)
#endif




void               WHD_RTT_LOGs(const char *fmt_ptr, ...);
extern const char* WHD_event_to_string(uint32_t  val);


#endif // WIFI_DEBUG_H



