#include "csro_common/csro_common.h"

void app_main(void)
{
    nvs_handle handle;
    nvs_flash_init();
    nvs_open("system", NVS_READWRITE, &handle);
    nvs_get_u32(handle, "power_count", &sysinfo.power_cnt);
    nvs_set_u32(handle, "power_count", (sysinfo.power_cnt + 1));
    nvs_get_u8(handle, "router_flag", &sysinfo.router_flag);
    nvs_commit(handle);
    nvs_close(handle);
    printf("Start OK!\r\n");
}