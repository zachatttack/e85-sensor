/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"

static const char *INFO = "INFO";

#define EXAMPLE_PCNT_HIGH_LIMIT 300
#define EXAMPLE_PCNT_LOW_LIMIT -1

#define EXAMPLE_EC11_GPIO_A 0
#define EXAMPLE_EC11_GPIO_B 2

void counter_setup(pcnt_unit_handle_t * pcnt_unit) {

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
  ESP_ERROR_CHECK(pcnt_unit_enable(*pcnt_unit));
  ESP_LOGI(INFO, "clear pcnt unit");
  ESP_ERROR_CHECK(pcnt_unit_clear_count(*pcnt_unit));
  ESP_LOGI(INFO, "start pcnt unit");
  ESP_ERROR_CHECK(pcnt_unit_start(*pcnt_unit));
}

void app_main(void){
    pcnt_unit_handle_t pcnt_unit = NULL;
    counter_setup(&pcnt_unit);

    // Report counter value
    int pulse_count = 0;

    while (1) {
      ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
      ESP_LOGI(INFO, "Frequency: %d Hz", pulse_count);
      // ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
      vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

