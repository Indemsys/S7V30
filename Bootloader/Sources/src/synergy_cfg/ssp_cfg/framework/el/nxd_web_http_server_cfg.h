/* generated configuration header file - do not edit */
#ifndef NXD_WEB_HTTP_SERVER_CFG_H_
#define NXD_WEB_HTTP_SERVER_CFG_H_
/* Defined, this option removes the basic Web HTTP server error checking */
#if (BSP_CFG_PARAM_CHECKING_ENABLE)
            #define NX_DISABLE_ERROR_CHECKING
            #endif

/* Enabling NX Secure Component within Web HTTP Server Instance */
#if (1)
#ifndef NX_WEB_HTTPS_ENABLE
#define NX_WEB_HTTPS_ENABLE
#endif
#endif

#if (!1)
            #define NX_WEB_HTTP_NO_FILEX
            #endif
#if (0)
            #define NX_WEB_HTTP_MULTIPART_ENABLE
            #endif
#define NX_WEB_HTTP_SERVER_WINDOW_SIZE                  49152 // При таком размере окна скорость загрузси с TLS 1.3 достигала 2.2 Мбайт в сек
                                                              // При  размере окна 65536 скорость загрузси с TLS 1.3 достигала 3 Мбайт в сек
                                                              // Размер окна должен быть согласован с размером пула сетевых пакетов net_packet_pool_buffer
                                                              // При размере окна 65536 и количестве пакетов в пуле = 70 может присходит сбой загрузки файлов с использованием TLS 1.3
#define NX_WEB_HTTP_SERVER_TIMEOUT                      (10 * NX_IP_PERIODIC_RATE)
#define NX_WEB_HTTP_SERVER_TIMEOUT_ACCEPT               (10 * NX_IP_PERIODIC_RATE)
#define NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT           (10 * NX_IP_PERIODIC_RATE)
#define NX_WEB_HTTP_SERVER_TIMEOUT_RECEIVE              (10 * NX_IP_PERIODIC_RATE)
#define NX_WEB_HTTP_SERVER_TIMEOUT_SEND                 (10 * NX_IP_PERIODIC_RATE)
#define NX_WEB_HTTP_MAX_HEADER_FIELD                    256
#define NX_WEB_HTTP_SERVER_MAX_PENDING                  10
#define NX_WEB_HTTP_MAX_RESOURCE                        256
#define NX_WEB_HTTP_SERVER_SESSION_MAX                  10
#define NX_WEB_HTTP_SERVER_MIN_PACKET_SIZE              600
#define NX_WEB_HTTP_SERVER_THREAD_TIME_SLICE            2
#define NX_WEB_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH         20
#define NX_WEB_HTTP_SERVER_RETRY_SECONDS                2
#define NX_WEB_HTTP_SERVER_RETRY_MAX                    10
#define NX_WEB_HTTP_SERVER_RETRY_SHIFT                  1
#endif /* NXD_WEB_HTTP_SERVER_CFG_H_ */
