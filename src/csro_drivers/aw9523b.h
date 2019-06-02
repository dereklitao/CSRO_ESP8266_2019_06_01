#ifndef AW9523B_H_
#define AW9523B_H_
#include "csro_common/csro_common.h"

void csro_aw9523b_init(void);
void csro_set_led(uint8_t led_num, uint8_t bright);
void csro_start_vibrator(uint8_t period_ms);
void csro_set_relay(uint8_t relay_num, bool state);

#endif