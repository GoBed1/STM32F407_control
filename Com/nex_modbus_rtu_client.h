#pragma once
#ifndef NEX_MODBUS_RTU_CLIENT_H
#define NEX_MODBUS_RTU_CLIENT_H

#include "main.h"
// #include "stm32h7xx_hal.h"
#include "stm32f4xx_hal.h"
// #include "event_groups.h"

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
// #include <stdint.h>
#define mbr_printf(...)  // LOGGER_INFO("MB-RTU", __VA_ARGS__)
#define mbr_printf_line(...)  // LOGGER_INFO("MB-RTU", __VA_ARGS__)

typedef enum ERR_OP_LIST
{
  // Errors
  ERR_NOT_MASTER    = 10,// 无主设备错误
  ERR_POLLING       = 11,//轮询错误
  ERR_BUFF_OVERFLOW = 12,//缓冲区溢出错误
  ERR_BAD_CRC       = 13,// CRC校验错误
  ERR_EXCEPTION     = 14,// 从机异常响应错误
  ERR_BAD_SIZE      = 15,// 错误的帧大小错误
  ERR_BAD_ADDRESS   = 16,// 错误的从机地址错误
  ERR_TIME_OUT      = 17,// 超时错误
  ERR_BAD_SLAVE_ID  = 18,// 错误的从机ID错误
  ERR_BAD_TCP_ID    = 19,// 错误的TCP ID错误
  ERR_FUC_CODE       = 20,// 功能码错误
  // Operations
  OP_OK_QUERY       = 21  // this value is not an error, it is a number different than zero to acknowledge a correct operation,
                                        // which is needed because FreeRTOS notifications return zero on timeout.
	                                    // Therefore we define our own Error and Operation codes and keep zero exclusively for FreeRTOS primitives
}mb_err_op_t;

typedef struct ModbusRtuClient {
  
  UART_HandleTypeDef *huart;
  uint8_t tx_buf[128]; // 记录发送数据的缓冲区
  uint8_t rx_buf[256];
  uint8_t rx_frame_len;
  uint8_t rx_timeout;
  uint8_t parse_buf[256];

  // EventGroupHandle_t event_group; // ✅ 新增：事件组句柄

  TaskHandle_t task_handle;
  TaskHandle_t Rx_lora_task_handle;
  TimerHandle_t timeout_timer;
} ModbusRtuClient;

// extern ModbusRtuClient encoder_client;
// extern EventGroupHandle_t eg;
// extern uint16_t a=0;
typedef struct {
   // ✅ 重试相关
    uint8_t crcError_resend_count;        // 校验码错误导致重发的次数
    uint8_t crcError_max_resend;        // 校验码最大重试次数（建议 3）
    uint8_t   timeout_resend_count;                           //超时重发次数
    uint8_t   timeout_max_resend;                           //超时最大重发次数（建议 3）
    uint8_t  Resend_buf[128];                      //重发数据缓存
    uint8_t  Resend_len;                      //重发数据长度

}ModbusRtu_Resend_t;



void NexModbusClient_Init(void);

uint16_t mbr_calc_crc(uint8_t *Buffer, uint8_t u8length);
void mbr_put_crc(uint8_t* frame, uint8_t len_wo_crc);
int mbr_parse_2regs(const uint8_t* frame, uint8_t len,
                  uint16_t regs_out[2], uint32_t* value_out);
#endif // NEX_MODBUS_RTU_CLIENT_H
