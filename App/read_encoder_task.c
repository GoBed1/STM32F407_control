#include "read_encoder_task.h"
#include "main.h"
#include "nex_modbus_rtu_client.h"
#include <string.h>
// #include "stm32h7xx_hal.h"
#include "stm32f4xx_hal.h"
#include "com_debug.h"
#include "timers.h"
// #include "lwip.h"

// ModbusRtuClient encoder_client;

void mbr_recv_callback(ModbusRtuClient *client, UART_HandleTypeDef *huart, uint16_t size)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    debug_println("111111111111111111111111111111111111111");
    // 将数据保存到mb_parse_buf中以供主任务处理
    if (size > 0)
    {
        if (size > sizeof(client->parse_buf))
        {
            size = (uint16_t)sizeof(client->parse_buf);
        }
        memcpy(client->parse_buf, client->rx_buf, size);
        client->rx_frame_len = (uint8_t)size;
    }
    // 重新启动 DMA 接收以接收下一个数据包
    HAL_UARTEx_ReceiveToIdle_DMA(huart, client->rx_buf, (uint16_t)sizeof(client->rx_buf));

    // 判断哪个从机发来的数据，然后通知对应的任务
    if (client->parse_buf[0] == 0x01 || client->parse_buf[0] == 0x02)
    { // 爆闪灯从机
        if (client->task_handle)
        {
            vTaskNotifyGiveFromISR(client->task_handle,
                                   &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else if (client->parse_buf[0] == 0x03)
    { // lora转485从机
        debug_println("2222222222222222222222222222222222222222222222");
        for (int i = 0; i < client->rx_frame_len; i++)
        {
            debug_println("%02X ", client->parse_buf[i]);
        }

        if (client->Rx_lora_task_handle)
        {
            debug_println("111111111111111111111111111111111111111");

            vTaskNotifyGiveFromISR(client->Rx_lora_task_handle,
                                   &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
