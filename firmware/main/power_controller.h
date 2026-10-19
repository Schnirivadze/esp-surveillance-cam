#ifndef POWER_CONTROLLER
#define POWER_CONTROLLER

#include "esp_adc/adc_oneshot.h"
static adc_oneshot_unit_handle_t adc1_handle;

extern void setup_power_controllers(void);
extern void check_usb_power_connected(void);
extern void check_battery_power_level(void);

#endif