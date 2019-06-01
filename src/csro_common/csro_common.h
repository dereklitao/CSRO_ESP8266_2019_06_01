#ifndef CSRO_COMMON_H_
#define CSRO_COMMON_H_

#include "esp_system.h"
#include "nvs_flash.h"
#include "FreeRTOS.h"

#include "time.h"

typedef enum
{
    idle = 0,
    smart_config = 1,
    smart_config_timeout = 2,
    mqtt_router_connecting = 3,
    mqtt_server_connecting = 4,
    mqtt_server_connected = 5,
} wifi_status;

typedef struct
{
    wifi_status status;
    uint8_t restore_flag;
    uint8_t router_flag;
    char router_ssid[50];
    char router_pass[50];

    uint8_t mac[6];
    char mac_str[20];
    uint8_t ip[4];
    char ip_str[20];

    char host_name[20];
    char dev_type[20];

    uint32_t power_cnt;
    uint32_t router_cnt;
    uint32_t server_cnt;

    bool time_sync;
    time_t time_start;
    time_t time_now;
    struct tm time_info;
    char time_str[64];
} csro_system;

extern csro_system sysinfo;

#endif