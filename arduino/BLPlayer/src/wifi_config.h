#ifndef __wifi_config_h__
#define __wifi_config_h__
/**
 * @brief 自定义WIFI连接的配置
 * 
 */
#define home 0

#if home == 1
    #define WIFI_SSID_DEFAULT "XSC-2.4"        // WIFI的网络名称
    #define WIFI_PWD_DEFAULT  "84940782"    // WIFI的密码
#else
    #define WIFI_SSID_DEFAULT "RelianceTech"        // WIFI的网络名称
    #define WIFI_PWD_DEFAULT  "RelianceTech2019"    // WIFI的密码
#endif


#endif // __wifi_config_h__