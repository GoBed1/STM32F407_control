#include "main.h"
#include "uart_app.h"
#include "com_debug.h"
#include "read_encoder_task.h"

#include <stdio.h>
#include <stdarg.h>
// extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern ModbusRtuClient encoder_client;
extern uint8_t a;
// uint16_t g_received_len = 0;
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (a ==4)
    {
        if (huart->Instance == huart2.Instance)
        {
            // 放通知到ModbusRtuClient模块处理数据包
            debug_println("%d", Size);
            mbr_recv_callback(&encoder_client, huart, Size);
        }
    }
}
