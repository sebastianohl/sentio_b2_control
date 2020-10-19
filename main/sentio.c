#include "sentio.h"
#include "esp_log.h"

static const char *TAG = "sentio-b2-control";

sentio_state_t sentio_state = {};
uart_handle_t uart = {.configRx = {.baud_rate = 57600,
                                   .data_bits = UART_DATA_8_BITS,
                                   .parity = UART_PARITY_DISABLE,
                                   .stop_bits = UART_STOP_BITS_1,
                                   .flow_ctrl = UART_HW_FLOWCTRL_DISABLE},
                      .configTx = {.baud_rate = 57600,
                                   .data_bits = UART_DATA_8_BITS,
                                   .parity = UART_PARITY_DISABLE,
                                   .stop_bits = UART_STOP_BITS_1,
                                   .flow_ctrl = UART_HW_FLOWCTRL_DISABLE},
                      .wait_ticks = 30000 / portTICK_PERIOD_MS,
                      .tx_pin = CONFIG_TX_GPIO,
                      .rx_pin = CONFIG_RX_GPIO};


int sentio_get_status(const char *command, char value[100])
{
    char cmd[100] = {0};
    snprintf(cmd, 99, "GET %s\r\n", command);
    uart_write(&uart, cmd, strlen(cmd));

    uart_cycle(&uart);
    char buf_value[100] = {0};
    size_t buf_len = 99;
    uart_get_buffer(&uart, buf_value, &buf_len);

    ESP_LOGD(TAG, "buffer value %d %s", buf_len, buf_value);
    char out_cmd[100] = {0};

    if (sscanf(buf_value, "%s %s", out_cmd, value) == 2)
    {
        ESP_LOGI(TAG, "cmd %s value %s", out_cmd, value);

        if (strcmp(out_cmd, command) == 0)
        {
        }
        else if (strcmp(out_cmd, "UNKNOWN") == 0)
        {
            ESP_LOGE(TAG, "unknown command %s", command);
            return -1;
        }
    }
    else
    {
        ESP_LOGE(TAG, "sscanf failed: %s", buf_value);
        return -1;
    }
    return 0;
}

int sentio_set_status(const char *command, const char *value)
{
    char cmd[100] = {0};
    snprintf(cmd, 99, "SET %s %s\r\n", command, value);
    uart_write(&uart, cmd, strlen(cmd));

    uart_cycle(&uart);
    char buf_value[100] = {0};
    size_t buf_len = 99;
    uart_get_buffer(&uart, buf_value, &buf_len);

    ESP_LOGD(TAG, "buffer value %d %s", buf_len, buf_value);

    char out_cmd[100] = {0};
    char out_val[100] = {0};
    if (sscanf(buf_value, "%s %s", out_cmd, out_val) == 2)
    {
        ESP_LOGI(TAG, "cmd %s value %s", out_cmd, out_val);

        if (strcmp(out_cmd, command) == 0)
        {
        }
        else if (strcmp(out_cmd, "UNKNOWN") == 0)
        {
            ESP_LOGE(TAG, "unknown command %s", command);
            return -1;
        }
    }
    else
    {
        ESP_LOGE(TAG, "sscanf failed: %s", buf_value);
        return -1;
    }
    return 0;
}

int sentio_get_value(const char *command, char value[100])
{
    char cmd[100] = {0};
    snprintf(cmd, 99, "GET %s VAL\r\n", command);
    uart_write(&uart, cmd, strlen(cmd));

    uart_cycle(&uart);
    char buf_value[100] = {0};
    size_t buf_len = 99;
    uart_get_buffer(&uart, buf_value, &buf_len);

    ESP_LOGD(TAG, "buffer value %d %s", buf_len, buf_value);
    char out_cmd[100] = {0};

    if (sscanf(buf_value, "%s %s", out_cmd, value) == 2)
    {
        ESP_LOGI(TAG, "cmd %s value %s", out_cmd, value);

        if (strcmp(out_cmd, command) == 0)
        {
        }
        else if (strcmp(out_cmd, "UNKNOWN") == 0)
        {
            ESP_LOGE(TAG, "unknown command %s", command);
            return -1;
        }
    }
    else
    {
        ESP_LOGE(TAG, "sscanf failed: %s", buf_value);
        return -1;
    }
    return 0;
}

int sentio_set_value(const char *command, const char *value, const size_t len)
{
    char cmd[100] = {0};
    snprintf(cmd, 99, "SET %s VAL %.*s\r\n", command, len, value);
    uart_write(&uart, cmd, strlen(cmd));

    uart_cycle(&uart);
    char buf_value[100] = {0};
    size_t buf_len = 99;
    uart_get_buffer(&uart, buf_value, &buf_len);

    ESP_LOGD(TAG, "buffer value %d %s", buf_len, buf_value);

    char out_cmd[100] = {0};
    char out_val[100] = {0};

    if (sscanf(buf_value, "%s %s", out_cmd, out_val) == 2)
    {
        ESP_LOGI(TAG, "cmd %s value %s", out_cmd, out_val);

        if (strcmp(out_cmd, command) == 0)
        {
        }
        else if (strcmp(out_cmd, "UNKNOWN") == 0)
        {
            ESP_LOGE(TAG, "unknown command %s", command);
            return -1;
        }
    }
    else
    {
        ESP_LOGE(TAG, "sscanf failed: %s", buf_value);
        return -1;
    }

    return 0;
}

void update_power(struct homie_handle_s *handle, int node, int property)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        {
            char out_value[100] = {0};
            if (sentio_get_status("SAUNA", out_value) == 0)
            {
                char value[100] = {0};
                sprintf(value, "%s",
                        (strcmp(out_value, "off;") == 0) ? "false" : "true");
                ESP_LOGI(TAG, "power status %s", value);

                homie_publish_property_value(handle, node, property, value);
            }
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void write_power(struct homie_handle_s *handle, int node, int property,
                 const char *data, int data_len)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        if (strncmp(data, "true", data_len) == 0)
        {
            sentio_set_status("SAUNA", "ON");
        }
        else
        {
            sentio_set_status("SAUNA", "OFF");
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void update_timer(struct homie_handle_s *handle, int node, int property)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        {
            char out_value[100] = {0};
            if (sentio_get_status("TIMER", out_value) == 0)
            {
                char value[100] = {0};
                sprintf(value, "%s",
                        (strcmp(out_value, "off;") == 0) ? "false" : "true");
                ESP_LOGI(TAG, "timer status %s", value);

                homie_publish_property_value(handle, node, property, value);
            }
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void write_timer(struct homie_handle_s *handle, int node, int property,
                 const char *data, int data_len)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        if (strncmp(data, "true", data_len) == 0)
        {
            sentio_set_status("TIMER", "ON");
        }
        else
        {
            sentio_set_status("TIMER", "OFF");
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void update_heattimer(struct homie_handle_s *handle, int node, int property)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        {
            char out_value[100] = {0};
            if (sentio_get_status("HEATTIMER", out_value) == 0)
            {
                char value[100] = {0};
                sprintf(value, "%s",
                        (strcmp(out_value, "off;") == 0) ? "false" : "true");
                ESP_LOGI(TAG, "heatingtimer status %s", value);

                homie_publish_property_value(handle, node, property, value);
            }
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void write_heattimer(struct homie_handle_s *handle, int node, int property,
                     const char *data, int data_len)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        if (strncmp(data, "true", data_len) == 0)
        {
            sentio_set_status("HEATTIMER", "ON");
        }
        else
        {
            sentio_set_status("HEATTIMER", "OFF");
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void update_target_temp(struct homie_handle_s *handle, int node, int property)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        {
            char out_value[100] = {0};
            if (sentio_get_value("SAUNA", out_value) == 0)
            {
                uint32_t temp = 0;
                if (sscanf(out_value, "%d", &temp))
                {
                    char value[100] = {0};
                    sprintf(value, "%d", temp);

                    ESP_LOGI(TAG, "target temperature %s", value);

                    homie_publish_property_value(handle, node, property, value);
                }
                else
                {
                    ESP_LOGE(TAG, "bad value for target temperature %s",
                             out_value);
                }
            }
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void write_target_temp(struct homie_handle_s *handle, int node, int property,
                       const char *data, int data_len)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {

        sentio_set_value("SAUNA", data, data_len);

        xSemaphoreGive(sentio_state.mutex);
    }
}

void update_timer_value(struct homie_handle_s *handle, int node, int property)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        {
            char out_value[100] = {0};
            if (sentio_get_value("TIMER", out_value) == 0)
            {
                uint32_t temp = 0;
                if (sscanf(out_value, "%d", &temp))
                {
                    char value[100] = {0};
                    sprintf(value, "%d", temp);

                    ESP_LOGI(TAG, "timer value %s", value);

                    homie_publish_property_value(handle, node, property, value);
                }
                else
                {
                    ESP_LOGE(TAG, "bad value for timer %s", out_value);
                }
            }
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void write_timer_value(struct homie_handle_s *handle, int node, int property,
                       const char *data, int data_len)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {

        sentio_set_value("TIMER", data, data_len);

        xSemaphoreGive(sentio_state.mutex);
    }
}

void update_heater_temp(struct homie_handle_s *handle, int node, int property)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        {
            char out_value[100] = {0};
            if (sentio_get_value("TEMP-HEATER", out_value) == 0)
            {
                uint32_t temp = 0;
                if (sscanf(out_value, "%d", &temp))
                {
                    char value[100] = {0};
                    sprintf(value, "%d", temp);

                    ESP_LOGI(TAG, "heater temperature %s", value);

                    homie_publish_property_value(handle, node, property, value);
                }
                else
                {
                    ESP_LOGE(TAG, "bad value for heater temperature %s",
                             out_value);
                }
            }
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void update_bench_temp(struct homie_handle_s *handle, int node, int property)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        {
            char out_value[100] = {0};
            if (sentio_get_value("TEMP-BENCH", out_value) == 0)
            {
                uint32_t temp = 0;
                if (sscanf(out_value, "%d", &temp))
                {
                    char value[100] = {0};
                    sprintf(value, "%d", temp);

                    ESP_LOGI(TAG, "bench temperature %s", value);

                    homie_publish_property_value(handle, node, property, value);
                }
                else
                {
                    ESP_LOGE(TAG, "bad value for bench temperature %s",
                             out_value);
                }
            }
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void update_heattimer_value(struct homie_handle_s *handle, int node,
                            int property)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {
        {
            char out_value[100] = {0};
            if (sentio_get_value("HEATTIMER", out_value) == 0)
            {
                uint32_t temp = 0;
                if (sscanf(out_value, "%d", &temp))
                {
                    char value[100] = {0};
                    sprintf(value, "%d", temp);

                    ESP_LOGI(TAG, "heater timer value %s", value);

                    homie_publish_property_value(handle, node, property, value);
                }
                else
                {
                    ESP_LOGE(TAG, "bad value for heater timer %s", out_value);
                }
            }
        }
        xSemaphoreGive(sentio_state.mutex);
    }
}

void write_heattimer_value(struct homie_handle_s *handle, int node,
                           int property, const char *data, int data_len)
{
    if (xSemaphoreTake(sentio_state.mutex, (portTickType)portMAX_DELAY) ==
        pdTRUE)
    {

        sentio_set_value("HEATTIMER", data, data_len);

        xSemaphoreGive(sentio_state.mutex);
    }
}
