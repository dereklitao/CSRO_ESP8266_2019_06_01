#ifndef AW9523B_H_
#define AW9523B_H_
#include "csro_common/csro_common.h"

#ifdef NLIGHT
#if NLIGHT == 3
#define LED1_REG_ADDR 0x22
#define LED2_REG_ADDR 0x20
#define LED3_REG_ADDR 0x2C
#define LED4_REG_ADDR 0x21
#define LED5_REG_ADDR 0x23
#define LED6_REG_ADDR 0x2D
#endif
#endif

void csro_aw9523b_init(void);
void csro_set_led(uint8_t led_num, uint8_t bright);
void csro_start_vibrator(uint8_t period_ms);
void csro_set_relay(uint8_t relay_num, bool state);

#endif