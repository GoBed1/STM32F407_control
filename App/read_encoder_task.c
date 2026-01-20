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
    // DMA 接收完成，先使 D-Cache 无效化，确保 CPU 从物理内存读取最新数据
  // uint32_t addr_aligned = (uint32_t)client->rx_buf & ~31U;
  // uint32_t len_aligned = ((uint32_t)client->rx_buf + size - addr_aligned + 31) & ~31U;
    // SCB_InvalidateDCache_by_Addr((uint32_t *)addr_aligned, len_aligned);
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
    // HAL_DMA_DISABLE_IT(hdma_uart4_rx, DMA_IT_HT);
    // if (size > 0) {
        // vTaskNotifyGiveFromISR(client->task_handle, &xHigherPriorityTaskWoken);
    // }
    // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // debug_println("Recevice1: %.*s\n", client->rx_frame_len , client->parse_buf);
     if (client->task_handle) {
        vTaskNotifyGiveFromISR(client->task_handle,
                              &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// void mbr_timeout_timer_cb(TimerHandle_t xTimer)
// {
//   ModbusRtuClient* client = (ModbusRtuClient*) pvTimerGetTimerID(xTimer);
//   if (!client) {
//     return;
//   }
//   client->rx_timeout = 1;
//   xTaskNotifyGive(client->task_handle);
// }



// void read_encoder_task(void* args) {
//   (void)args;
//   // while (!(system_ready & NET_READY)) {
//   //   osDelay(100);
//   // }
//   ModbusRtuClient* client = &encoder_client;

//   client->task_handle = xTaskGetCurrentTaskHandle();
//   client->rx_timeout = 0;
//   client->rx_frame_len = 0;

//   // client->timeout_timer = xTimerCreate("encMbTmo",
//   //                                     pdMS_TO_TICKS(3000),
//   //                                     pdFALSE,
//   //                                     (void*)client,
//   //                                     mbr_timeout_timer_cb);

//   osDelay(1000);
//   HAL_UARTEx_ReceiveToIdle_DMA(client->huart, client->rx_buf, (uint16_t)sizeof(client->rx_buf));
//   //  HAL_DMA_DISABLE_IT(hdma_uart4_rx, DMA_IT_HT);
//   TickType_t last_wake = xTaskGetTickCount();
//   for (;;) {
//     // wait frame silence（绝对延时）
//     vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(9));

//     // client->tx_buf[0] = 0x01; // Slave ID
//     // client->tx_buf[1] = 0x03; // Function code: Read Holding Registers
//     // client->tx_buf[2] = 0x00; // Starting address high byte
//     // client->tx_buf[3] = 0x00; // Starting address low byte
//     // client->tx_buf[4] = 0x00; // Quantity of registers high byte
//     // client->tx_buf[5] = 0x02; // Quantity of registers low byte
//     // mbr_put_crc(client->tx_buf, 6);
//     // mbr_printf_line("[send]");
//     // for(int i=0;i<8;i++){
//     //   mbr_printf_line("%02X ",client->tx_buf[i]);
//     // }
//     // mbr_printf_line("\r\n");
//     // HAL_UART_Transmit_DMA(client->huart, client->tx_buf, 8);

//     client->rx_timeout = 0;
//     client->rx_frame_len = 0;
//     // xTimerReset(client->timeout_timer, 0);
//     (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//     xTimerStop(client->timeout_timer, 0);

//     if(client->rx_timeout){
//       mbr_printf_line("[err] timeout\r\n");
//       client->rx_timeout = 0;
//       continue;
//     }
//     mbr_printf_line("[recv]b:%d raw:",(int)client->rx_frame_len);
//     for(int i=0;i<client->rx_frame_len;i++){
//       mbr_printf_line("%02X ",client->parse_buf[i]);
//     }
//     mbr_printf_line("\r\n");
//     if (client->rx_frame_len >= 9) {
//       uint16_t regs[2];
//       uint32_t value32;
//       if (mbr_parse_2regs(client->parse_buf, client->rx_frame_len, regs, &value32) == 0) {
//         mbr_printf_line("[parse] regs: %04X %04X, u32: %lu\r\n",
//                           (unsigned)regs[0], (unsigned)regs[1], (unsigned long)value32);
//         send_encoder_data(value32, HAL_GetTick() / 1000, (HAL_GetTick() % 1000) * 1000000);
//         static uint32_t send_count = 0;
//         static uint32_t last_report_tick = 0;
//         send_count++;
//         uint32_t now = HAL_GetTick();
//         if (now - last_report_tick >= 1000) {
//             LOGGER_DEBUG("UDP","UDP sent in last 1s: %lu\r\n", (unsigned long)send_count);
//             send_count = 0;
//             last_report_tick = now;
//         }
//       } 
//       else {
//         mbr_printf_line("[parse] invalid frame or CRC\r\n");
//         continue;
//       }
//     }
//   }
// }

// void init_read_encoder_task(UART_HandleTypeDef *huart) {
//   memset(&encoder_client, 0, sizeof(encoder_client));
//   encoder_client.huart = huart;
//   encoder_client.task_handle = NULL;
//   encoder_client.rx_frame_len = 0;
//   encoder_client.rx_timeout = 0;

//   encoder_client.task_handle = osThreadNew(read_encoder_task, NULL, &read_encoder_task_attributes);
//   if( encoder_client.task_handle == NULL ) {
//     LOG_PRINTF(LOG_LEVEL_ERROR, "MB-RTU", "Failed to create Modbus RTU client task\r\n");
//   }
// }
