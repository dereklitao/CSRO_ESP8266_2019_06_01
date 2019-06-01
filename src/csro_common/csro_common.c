#include "csro_common.h"

#define MACSTR_FORMAT "%02x%02x%02x%02x%02x%02x"
#define PASSSTR_FORMAT "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"

csro_system sysinfo;
csro_mqtt mqttinfo;
esp_mqtt_client_handle_t mqtt_client;

void csro_system_get_info(void)
{
    size_t len = 0;
    nvs_handle handle;
    nvs_open("system", NVS_READWRITE, &handle);
    nvs_get_str(handle, "router_ssid", NULL, &len);
    nvs_get_str(handle, "router_ssid", sysinfo.router_ssid, &len);
    nvs_get_str(handle, "router_pass", NULL, &len);
    nvs_get_str(handle, "router_pass", sysinfo.router_pass, &len);
    nvs_get_u16(handle, "interval", &mqttinfo.interval);
    if (mqttinfo.interval < 2 || mqttinfo.interval > 30)
    {
        mqttinfo.interval = 5;
        nvs_set_u16(handle, "interval", mqttinfo.interval);
    }
    nvs_commit(handle);
    nvs_close(handle);

    esp_wifi_get_mac(WIFI_MODE_STA, sysinfo.mac);
    sprintf(sysinfo.mac_str, MACSTR_FORMAT, sysinfo.mac[0], sysinfo.mac[1], sysinfo.mac[2], sysinfo.mac[3], sysinfo.mac[4], sysinfo.mac[5]);
    sprintf(sysinfo.host_name, "CSRO_%s", sysinfo.mac_str);
    sprintf(mqttinfo.id, "csro/%s", sysinfo.mac_str);

#ifdef NLIGHT
    sprintf(sysinfo.dev_type, "nlight%d", NLIGHT);
#elif defined DLIGHT
    sprintf(sysinfo.dev_type, "dlight");
#elif defined RGBLIGHT
    sprintf(sysinfo.dev_type, "rgblight");
#elif defined MOTOR
    sprintf(sysinfo.dev_type, "motor%d", MOTOR);
#elif defined AIR_MONITOR
    sprintf(sysinfo.dev_type, "airmon");
#endif

    sprintf(mqttinfo.name, "csro/%s/%s", sysinfo.mac_str, sysinfo.dev_type);
    sprintf(mqttinfo.pass, PASSSTR_FORMAT, sysinfo.mac[1], sysinfo.mac[3], sysinfo.mac[5], sysinfo.mac[0], sysinfo.mac[2], sysinfo.mac[4], sysinfo.mac[5], sysinfo.mac[3], sysinfo.mac[1], sysinfo.mac[4], sysinfo.mac[2], sysinfo.mac[0]);
    printf("id = %s.\nname = %s.\npass = %s.\n", mqttinfo.id, mqttinfo.name, mqttinfo.pass);
}