#include "csro_devices.h"
#include "csro_drivers/aw9523b.h"

#ifdef NLIGHT

#define KEY_01_NUM GPIO_NUM_16
#define KEY_02_NUM GPIO_NUM_4
#define KEY_03_NUM GPIO_NUM_10
#define KEY_04_NUM GPIO_NUM_0
#define KEY_05_NUM GPIO_NUM_13
#define KEY_06_NUM GPIO_NUM_12

#define GPIO_MASK ((1ULL << KEY_01_NUM) | (1ULL << KEY_02_NUM) | (1ULL << KEY_03_NUM) | (1ULL << KEY_04_NUM) | (1ULL << KEY_05_NUM) | (1ULL << KEY_06_NUM))

#define CFG_NLIGHT_TOPIC "csro/light/%s_%s_%d/config"
#define CFG_NLIGHT_PAYLOAD "{\"~\":\"csro/%s/%s\",\"name\":\"%s_%d_%s\",\"avty_t\":\"~/available\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\",\"stat_t\":\"~/state\",\"stat_val_tpl\":\"{{value_json.state[%d]}}\",\"cmd_t\":\"~/set/%d\",\"pl_on\":1,\"pl_off\":0, \"opt\":\"false\",\"qos\":1}"

void csro_update_nlight_state(void);

int light_state[3];
uint32_t key_hold_time[3];

static void key_task(void *pvParameters)
{
    static uint32_t holdtime[3];
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_MASK;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    while (true)
    {
        bool key_update = false;
        if (gpio_get_level(KEY_01_NUM) == 0 || gpio_get_level(KEY_02_NUM) == 0)
        {
            holdtime[0]++;
            if (holdtime[0] == 5)
            {
                if (light_state[0] == 0)
                {
                    light_state[0] = 1;
                }
                else if (light_state[0] == 1)
                {
                    light_state[0] = 0;
                }
                key_update = true;
            }
        }
        else
        {
            holdtime[0] = 0;
        }
        if (gpio_get_level(KEY_03_NUM) == 0 || gpio_get_level(KEY_04_NUM) == 0)
        {
            holdtime[1]++;
            if (holdtime[1] == 5)
            {
                if (light_state[1] == 0)
                {
                    light_state[1] = 1;
                }
                else if (light_state[1] == 1)
                {
                    light_state[1] = 0;
                }
                key_update = true;
            }
        }
        else
        {
            holdtime[1] = 0;
        }
        if (gpio_get_level(KEY_05_NUM) == 0 || gpio_get_level(KEY_06_NUM) == 0)
        {
            holdtime[2]++;
            if (holdtime[2] == 5)
            {
                if (light_state[2] == 0)
                {
                    light_state[2] = 1;
                }
                else if (light_state[2] == 1)
                {
                    light_state[2] = 0;
                }
                key_update = true;
            }
        }
        else
        {
            holdtime[2] = 0;
        }
        if (key_update)
        {
            csro_update_nlight_state();
        }
        vTaskDelay(20 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

static void nlight_task(void *pvParameters)
{
    while (true)
    {
        vTaskDelay(100 / portTICK_RATE_MS);
        for (size_t i = 0; i < 3; i++)
        {
            if (light_state[i] == 1)
            {
                csro_set_led(2 * i + 1, 128);
                csro_set_led(2 * i + 2, 128);
                csro_set_relay(1 + i, true);
            }
            else
            {
                csro_set_led(2 * i + 1, 0);
                csro_set_led(2 * i + 2, 0);
                csro_set_relay(1 + i, false);
            }
        }
    }
    vTaskDelete(NULL);
}

void csro_nlight_init(void)
{
    csro_aw9523b_init();
    xTaskCreate(nlight_task, "nlight_task", 2048, NULL, configMAX_PRIORITIES - 8, NULL);
    xTaskCreate(key_task, "key_task", 2048, NULL, configMAX_PRIORITIES - 8, NULL);
}

void csro_update_nlight_state(void)
{
    cJSON *state_json = cJSON_CreateObject();
    cJSON_AddItemToObject(state_json, "state", cJSON_CreateIntArray(light_state, 3));
    cJSON_AddStringToObject(state_json, "time", sysinfo.time_str);
    cJSON_AddNumberToObject(state_json, "run", (int)(sysinfo.time_now - sysinfo.time_start));
    char *out = cJSON_PrintUnformatted(state_json);
    strcpy(mqttinfo.content, out);
    free(out);
    cJSON_Delete(state_json);
    sprintf(mqttinfo.pub_topic, "csro/%s/%s/state", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_publish(mqtt_client, mqttinfo.pub_topic, mqttinfo.content, 0, 0, 1);
}

void csro_nlight_on_connect(esp_mqtt_client_handle_t client)
{
    sprintf(mqttinfo.sub_topic, "csro/%s/%s/set/#", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_subscribe(client, mqttinfo.sub_topic, 1);

    for (size_t i = 1; i < NLIGHT + 1; i++)
    {
        sprintf(mqttinfo.pub_topic, CFG_NLIGHT_TOPIC, sysinfo.mac_str, sysinfo.dev_type, i);
        sprintf(mqttinfo.content, CFG_NLIGHT_PAYLOAD, sysinfo.mac_str, sysinfo.dev_type, sysinfo.dev_type, i, sysinfo.mac_str, i - 1, i);
        esp_mqtt_client_publish(client, mqttinfo.pub_topic, mqttinfo.content, 0, 1, 1);
    }
    sprintf(mqttinfo.pub_topic, "csro/%s/%s/available", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_publish(mqtt_client, mqttinfo.pub_topic, "online", 0, 1, 1);
    csro_update_nlight_state();
}

void csro_nlight_on_message(esp_mqtt_event_handle_t event)
{
    bool update = false;
    char topic[50];
    for (size_t i = 1; i < NLIGHT + 1; i++)
    {
        sprintf(topic, "csro/%s/%s/set/%d", sysinfo.mac_str, sysinfo.dev_type, i);
        if (strncmp(topic, event->topic, event->topic_len) == 0)
        {
            if (strncmp("0", event->data, event->data_len) == 0)
            {
                light_state[i - 1] = 0;
                update = true;
            }
            else if (strncmp("1", event->data, event->data_len) == 0)
            {
                light_state[i - 1] = 1;
                update = true;
            }
        }
    }
    if (update)
    {
        csro_update_nlight_state();
    }
}

#endif