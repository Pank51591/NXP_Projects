#ifndef _DRIVER_DC_FAN_H_
#define _DRIVER_DC_FAN_H_
#include "stdint.h"

typedef enum
{
    FAN_LEVEL_0=0,
    FAN_LEVEL_1,//VL
    FAN_LEVEL_2,//LO
    FAN_LEVEL_3,//ME
    FAN_LEVEL_4,//HI
    FAN_LEVEL_5,//TURBO
}dc_fan_level_t;

void driver_dc_fan_init(void);
void driver_dc_fan_set_duty(uint16_t d);
void driver_dc_fan_set_level(dc_fan_level_t level);
void driver_dc_fan_pid_100ms(void);
#endif
