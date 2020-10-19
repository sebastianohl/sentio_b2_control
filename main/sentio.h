#ifndef SENTIO_H_
#define SENTIO_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "homie.h"
#include "uart.h"

struct sentio_state_s
{
    SemaphoreHandle_t mutex;
};
typedef struct sentio_state_s sentio_state_t;

extern sentio_state_t sentio_state;
extern uart_handle_t uart;

void update_power(struct homie_handle_s *handle, int node, int property);
void write_power(struct homie_handle_s *handle, int node, int property,
                 const char *data, int data_len);
void update_timer(struct homie_handle_s *handle, int node, int property);
void write_timer(struct homie_handle_s *handle, int node, int property,
                 const char *data, int data_len);
void update_heattimer(struct homie_handle_s *handle, int node, int property);
void write_heattimer(struct homie_handle_s *handle, int node, int property,
                     const char *data, int data_len);
void update_target_temp(struct homie_handle_s *handle, int node, int property);
void write_target_temp(struct homie_handle_s *handle, int node, int property,
                       const char *data, int data_len);
void update_timer_value(struct homie_handle_s *handle, int node, int property);
void write_timer_value(struct homie_handle_s *handle, int node, int property,
                       const char *data, int data_len);
void update_heater_temp(struct homie_handle_s *handle, int node, int property);
void update_bench_temp(struct homie_handle_s *handle, int node, int property);
void update_heattimer_value(struct homie_handle_s *handle, int node,
                            int property);
void write_heattimer_value(struct homie_handle_s *handle, int node,
                           int property, const char *data, int data_len);

#endif