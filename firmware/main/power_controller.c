#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "global_values.h"
#include "power_controller.h"

static adc_oneshot_unit_handle_t adc1_handle;
static const uint8_t ocv_curve[11] = {
	0,
	10,
	20,
	30,
	42,
	55,
	68,
	78,
	86,
	93,
	100};

void setup_power_controllers(void)
{
	const char *TAG = "setup_power_controllers";
	ESP_LOGI(TAG, "Setting up...");

	adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT_1};
	adc_oneshot_new_unit(&init_config, &adc1_handle);

	adc_oneshot_chan_cfg_t config = {
		.bitwidth = ADC_BITWIDTH_DEFAULT,
		.atten = ADC_ATTEN_DB_12,
	};
	adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);
	adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config);

	ESP_LOGI(TAG, "Set up");
}

void check_usb_power_connected(void)
{
	int val = 0;
	adc_oneshot_read(adc1_handle, ADC_CHANNEL_1, &val);

	usb_power_connected = val > 750;
}

void check_battery_power_level(void)
{
    int raw_reading = 0;
    int32_t adc_sum = 0;
    const int SAMPLES = 64; 

    for (int i = 0; i < SAMPLES; i++) {
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw_reading);
        adc_sum += raw_reading;
    }
    
    uint16_t adc_reading = adc_sum / SAMPLES; 

    if (adc_reading <= 2000) {
        battery_percentage = 0;
        return;
    }
    if (adc_reading >= 3600) {
        battery_percentage = 100;
        return;
    }

    uint16_t normalized = adc_reading - 2000;
    uint16_t index = normalized / 160;

    uint8_t low_pct = ocv_curve[index];
    uint8_t high_pct = ocv_curve[index + 1];
    uint16_t remainder = normalized % 160;

    battery_percentage = low_pct + (uint8_t)(((high_pct - low_pct) * remainder) / 160);
}