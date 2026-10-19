#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "power_controller.h"
#include "global_values.h"

void app_main(void)
{
	setup_power_controllers();
	while (true)
	{
		check_usb_power_connected();
		ESP_LOGI("main", "Usb is %sconnected",usb_power_connected? "" : "dis");
		if (!usb_power_connected)
		{
			check_battery_power_level();
			ESP_LOGI("main", "Battery is at %i%%",battery_percentage);
		}
		
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	
}
