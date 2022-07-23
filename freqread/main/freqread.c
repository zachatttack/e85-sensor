#include "driver/ledc.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "stdio.h"
#include "webserver.h"
#include "wifi.h"

static const char *INFO = "INFO";

#define EXAMPLE_PCNT_HIGH_LIMIT 300
#define EXAMPLE_PCNT_LOW_LIMIT -1

// #define EXAMPLE_EC11_GPIO_A 0 //button
#define EXAMPLE_EC11_GPIO_A 23
// #define EXAMPLE_EC11_GPIO_A 1 //pin13 gpi01 tx0

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO (5) // Define the output GPIO
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY (4095)     // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY (136) // Frequency in Hertz. Set frequency at 5 kHz

static void example_ledc_init(void) {
  // Prepare and then apply the LEDC PWM timer configuration
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_MODE,
      .timer_num = LEDC_TIMER,
      .duty_resolution = LEDC_DUTY_RES,
      .freq_hz = LEDC_FREQUENCY, // Set output frequency at 5 kHz
      .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {.speed_mode = LEDC_MODE,
                                        .channel = LEDC_CHANNEL,
                                        .timer_sel = LEDC_TIMER,
                                        .intr_type = LEDC_INTR_DISABLE,
                                        .gpio_num = LEDC_OUTPUT_IO,
                                        .duty = 0, // Set duty to 0%
                                        .hpoint = 0};
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

static void counter_setup(pcnt_unit_handle_t *pcnt_unit) {

  ESP_LOGI(INFO, "install pcnt unit");
  pcnt_unit_config_t unit_config = {
      .high_limit = EXAMPLE_PCNT_HIGH_LIMIT,
      .low_limit = EXAMPLE_PCNT_LOW_LIMIT,
  };
  ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, pcnt_unit));

  ESP_LOGI(INFO, "set glitch filter");
  pcnt_glitch_filter_config_t filter_config = {
      .max_glitch_ns = 1000,
  };
  ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(*pcnt_unit, &filter_config));

  ESP_LOGI(INFO, "install pcnt channels");
  pcnt_chan_config_t chan_a_config = {.edge_gpio_num = EXAMPLE_EC11_GPIO_A,
                                      .level_gpio_num = -1};
  pcnt_channel_handle_t pcnt_chan_a = NULL;
  ESP_ERROR_CHECK(pcnt_new_channel(*pcnt_unit, &chan_a_config, &pcnt_chan_a));

  ESP_LOGI(INFO, "set edge and level actions for pcnt channels");
  ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
      pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
      PCNT_CHANNEL_EDGE_ACTION_HOLD));
  ESP_ERROR_CHECK(
      pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                    PCNT_CHANNEL_LEVEL_ACTION_KEEP));

  ESP_LOGI(INFO, "enable pcnt unit");
  // ESP_ERROR_CHECK(pcnt_unit_enable(*pcnt_unit));
  ESP_LOGI(INFO, "clear pcnt unit");
  ESP_ERROR_CHECK(pcnt_unit_clear_count(*pcnt_unit));
  ESP_LOGI(INFO, "start pcnt unit");
  ESP_ERROR_CHECK(pcnt_unit_start(*pcnt_unit));
}

char buffer[20];

void app_main(void) {
  pcnt_unit_handle_t pcnt_unit = NULL;
  counter_setup(&pcnt_unit);

  // Report counter value
  int pulse_count = 0;

  wifi_init();

  // Set the LEDC peripheral configuration
  example_ledc_init();
  // Set duty to 50%
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
  // Update duty to apply the new value
  // ESP_LOGI(INFO, "updating duty");
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

  start_webserver();

  while (1) {
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
    ESP_LOGI(INFO, "Frequency: %d Hz", pulse_count);
    sprintf(buffer,"pulses: %d" , pulse_count);
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
