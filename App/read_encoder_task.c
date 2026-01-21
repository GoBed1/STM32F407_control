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
const osThreadAttr_t read_encoder_task_attributes = {
    .name = "read_encoder_task",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityLow,
};

void mbr_recv_callback(ModbusRtuClient *client, UART_HandleTypeDef *huart, uint16_t size){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // 将数据保存到mb_parse_buf中以供主任务处理
    if (size > 0) {
    if (size > sizeof(client->parse_buf)) {
      size = (uint16_t)sizeof(client->parse_buf);
    }
    memcpy(client->parse_buf, client->rx_buf, size);
    client->rx_frame_len = (uint8_t)size;
    
    }
    // 重新启动 DMA 接收以接收下一个数据包
  HAL_UARTEx_ReceiveToIdle_DMA(huart, client->rx_buf, (uint16_t)sizeof(client->rx_buf));
    
     if (client->task_handle) {
        vTaskNotifyGiveFromISR(client->task_handle,
                              &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
