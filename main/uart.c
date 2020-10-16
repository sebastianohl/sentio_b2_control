/*
 * uart.c
 *
 *  Created on: Jan 24, 2020
 *      Author: ohli
 */

#include "uart.h"
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <strings.h>
#include <string.h>

static const char *TAG = "sentio-b2-control";

void uart_cycle(uart_handle_t *handle) {
	uart_event_t event;

	gpio_set_level(handle->rx_pin, 0);
	gpio_set_level(handle->tx_pin, 0);

	// Waiting for UART event.
	if (xQueueReceive(handle->queue, (void*) &event, handle->wait_ticks)) {
		ESP_LOGI(TAG, "event %d", event.type);
		switch (event.type) {
		// Event of UART receving data
		// We'd better handler data event fast, there would be much more data events than
		// other types of events. If we take too much time on data event, the queue might be full.
		case UART_DATA:
			if (handle->buffer_fill + event.size > UART_BUF_SIZE) {
				bzero(handle->buffer, UART_RD_BUF_SIZE);
				handle->buffer_fill = 0;
			}
			uart_read_bytes(RX_UART_NUM, handle->buffer + handle->buffer_fill,
					event.size, handle->wait_ticks);
			handle->buffer_fill += event.size;
			break;

			// Event of HW FIFO overflow detected
		case UART_FIFO_OVF:
			// If fifo overflow happened, you should consider adding flow control for your application.
			// The ISR has already reset the rx FIFO,
			// As an example, we directly flush the rx buffer here in order to read more data.
			uart_flush_input(RX_UART_NUM);
			xQueueReset(handle->queue);
			break;

			// Event of UART ring buffer full
		case UART_BUFFER_FULL:
			// If buffer full happened, you should consider encreasing your buffer size
			// As an example, we directly flush the rx buffer here in order to read more data.
			uart_flush_input(RX_UART_NUM);
			xQueueReset(handle->queue);
			break;

		case UART_PARITY_ERR:
			break;

			// Event of UART frame error
		case UART_FRAME_ERR:
			break;

			// Others
		default:
			break;
		}
	}
}

void uart_write(uart_handle_t *handle, const char *buffer, size_t length) {
	gpio_set_level(handle->rx_pin, 1);
	gpio_set_level(handle->tx_pin, 1);
	vTaskDelay(1 / portTICK_PERIOD_MS);
	uart_write_bytes(TX_UART_NUM, buffer, length);
	uart_wait_tx_done(TX_UART_NUM, 500 / portTICK_PERIOD_MS);
	gpio_set_level(handle->tx_pin, 0);
	gpio_set_level(handle->rx_pin, 0);
}

void uart_get_buffer(uart_handle_t *handle, char *buffer, size_t *length) {
	memset(buffer, 0, *length);
	memcpy(buffer, handle->buffer,
			(*length > handle->buffer_fill) ? handle->buffer_fill : *length);
	*length = (*length > handle->buffer_fill) ? handle->buffer_fill : *length;
	handle->buffer_fill = 0;
}

void uart_init(uart_handle_t *handle) {
	// Configure parameters of an UART driver,
	// communication pins and install the driver
	uart_param_config(RX_UART_NUM, &handle->configRx);
	uart_param_config(TX_UART_NUM, &handle->configTx);

	// Install UART driver, and get the queue.
	uart_driver_install(RX_UART_NUM, UART_BUF_SIZE, 0, 100, &handle->queue, 0);
	uart_driver_install(TX_UART_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0);

	gpio_pad_select_gpio(handle->tx_pin);
	gpio_set_direction(handle->tx_pin, GPIO_MODE_OUTPUT);
	gpio_set_level(handle->tx_pin, 0);
	gpio_pad_select_gpio(handle->rx_pin);
	gpio_set_direction(handle->rx_pin, GPIO_MODE_OUTPUT);
	gpio_set_level(handle->rx_pin, 0);

}
