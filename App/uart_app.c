#include "main.h"
#include "uart_app.h"
#include "com_debug.h"
#include "read_encoder_task.h"
#include "semphr.h"
#include <stdio.h>
#include <stdarg.h>
// extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern ModbusRtuClient encoder_client;
extern uint8_t a;
SemaphoreHandle_t uart2_rx_semaphore;
// uint16_t g_received_len = 0;
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // if (a ==4)
    // {
     // ✅ 尝试获取信号量（中断版本，不阻塞）
        
        if (huart->Instance == huart2.Instance)
        {
        //     if (xSemaphoreTakeFromISR(uart2_rx_semaphore, &xHigherPriorityTaskWoken) == pdTRUE)
        // {
            // 放通知到ModbusRtuClient模块处理数据包
            debug_println("%d", Size);

            mbr_recv_callback(&encoder_client, huart, Size);
              // ✅ 释放信号量
            // xSemaphoreGiveFromISR(uart2_rx_semaphore, &xHigherPriorityTaskWoken);
        } 
        // else
        // {
        //     // ⚠️ 信号量被占用，说明有冲突，丢弃此次数据
        //     debug_println("⚠️ UART2 RX conflict, data dropped冲突");
            
        //     // 重启DMA接收
        //     HAL_UARTEx_ReceiveToIdle_DMA(huart, encoder_client.rx_buf, 
        //                                  sizeof(encoder_client.rx_buf));
        // }
        //  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // }
    }
// }


