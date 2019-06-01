#include "csro_common/csro_common.h"
#include "csro_devices/csro_devices.h"

void app_main(void)
{
    nvs_handle handle;
    nvs_flash_init();
    nvs_open("system", NVS_READWRITE, &handle);
    nvs_get_u32(handle, "power_cnt", &sysinfo.power_cnt);
    nvs_set_u32(handle, "power_cnt", (sysinfo.power_cnt + 1));
    nvs_get_u8(handle, "router_flag", &sysinfo.router_flag);
    nvs_commit(handle);
    nvs_close(handle);

    csro_device_init();

    if (sysinfo.router_flag == 1)
    {
        csro_start_mqtt();
    }
    else
    {
        csro_start_smart_config();
    }
    printf("\r\n=====%d Start OK!=====\r\n", sysinfo.power_cnt + 1);
}