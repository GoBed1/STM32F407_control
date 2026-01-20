#ifndef READ_ENCODER_TASK_H
#define READ_ENCODER_TASK_H
// #include "stm32h7xx_hal.h"
#include "stm32f4xx_hal.h"

#include "cmsis_os.h"
#include "nex_modbus_rtu_client.h"
// extern ModbusRtuClient encoder_client;
// void read_encoder_task(void* args);
// void init_read_encoder_task(UART_HandleTypeDef *huart);
void mbr_recv_callback(ModbusRtuClient *client, UART_HandleTypeDef *huart, uint16_t size);
// void mbr_timeout_timer_cb(TimerHandle_t xTimer);
#endif /* READ_ENCODER_TASK_H */