#include "csro_common.h"

static EventGroupHandle_t wifi_event_group;
static TaskHandle_t mqtt_task_handle;

static void timer_task(void *pvParameters)
{
    static int count = 0;
    while (true)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        time(&sysinfo.time_now);
        localtime_r(&sysinfo.time_now, &sysinfo.time_info);
        strftime(sysinfo.time_str, sizeof(sysinfo.time_str), "%Y-%m-%d %H:%M:%S", &sysinfo.time_info);
        if (sysinfo.time_info.tm_year < (2016 - 1900))
        {
            count++;
        }
        else
        {
            if (sysinfo.time_sync == false)
            {
                sysinfo.time_start = sysinfo.time_now - count;
                sysinfo.time_sync = true;
            }
        }
    }
    vTaskDelete(NULL);
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    if (event->event_id == MQTT_EVENT_CONNECTED)
    {
        csro_device_on_connect(event->client);
    }
    else if (event->event_id == MQTT_EVENT_DATA)
    {
        csro_device_on_message(event);
    }
    return ESP_OK;
}

static void mqtt_task(void *pvParameters)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
    setenv("TZ", "CST-8", 1);
    tzset();

    sprintf(mqttinfo.broker, MQTT_BROKER);
    sprintf(mqttinfo.lwt_topic, "csro/%s/%s/available", sysinfo.mac_str, sysinfo.dev_type);

    esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
        .client_id = mqttinfo.id,
        .username = mqttinfo.name,
        .password = mqttinfo.pass,
        .uri = mqttinfo.broker,
        .keepalive = 60,
        .lwt_topic = mqttinfo.lwt_topic,
        .lwt_msg = "offline",
        .lwt_retain = 1,
        .lwt_qos = 1,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(mqtt_client);

    while (true)
    {
        if (sysinfo.time_info.tm_year >= (2016 - 1900))
        {
            printf("Free heap: %d, time is: %s, run %d seconds\n", esp_get_free_heap_size(), sysinfo.time_str, (int)(sysinfo.time_now - sysinfo.time_start));
        }
        else
        {
            printf("Free heap size: %d\n", esp_get_free_heap_size());
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    if (event->event_id == SYSTEM_EVENT_STA_START)
    {
        tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, sysinfo.host_name);
        esp_wifi_connect();
    }
    else if (event->event_id == SYSTEM_EVENT_STA_GOT_IP)
    {
        if (mqtt_task_handle == NULL)
        {
            xTaskCreate(mqtt_task, "mqtt_task", 4096, NULL, configMAX_PRIORITIES - 7, &mqtt_task_handle);
        }
    }
    else if (event->event_id == SYSTEM_EVENT_STA_DISCONNECTED)
    {
        if (mqtt_task_handle != NULL)
        {
            vTaskDelete(mqtt_task_handle);
            mqtt_task_handle = NULL;
        }
        esp_wifi_connect();
    }
    return ESP_OK;
}

void csro_start_mqtt(void)
{
    csro_system_get_info();
    xTaskCreate(timer_task, "timer_task", 2048, NULL, 6, NULL);
    wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    esp_event_loop_init(wifi_event_handler, NULL);

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);

    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.sta.ssid, (char *)sysinfo.router_ssid);
    strcpy((char *)wifi_config.sta.password, (char *)sysinfo.router_pass);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}