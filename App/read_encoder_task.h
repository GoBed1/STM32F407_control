#ifndef READ_ENCODER_TASK_H
#define READ_ENCODER_TASK_H
// #include "stm32h7xx_hal.h"
#include "stm32f4xx_hal.h"

#include "cmsis_os.h"
#include "nex_modbus_rtu_client.h"
void mbr_recv_callback(ModbusRtuClient *client, UART_HandleTypeDef *huart, uint16_t size);


#endif /* READ_ENCODER_TASK_H */