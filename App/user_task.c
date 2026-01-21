#include "user_task.h"
#include "cmsis_os.h"
#include "com_debug.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern ModbusRtuClient encoder_client;
extern EventGroupHandle_t eg;
osThreadId_t user_task_handle;
const osThreadAttr_t user_task_attributes = {
    .name = "userTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

// 创建灯控制任务
osThreadId_t lightTaskHandle;
const osThreadAttr_t lightTask_attributes = {
    .name = "LightTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

// 创建灯控制任务
osThreadId_t modbusRecvTaskHandle;
const osThreadAttr_t modbusRecvTask_attributes = {
    .name = "modbusRecvTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

void init_user_task(void)
{
  memset(&encoder_client, 0, sizeof(encoder_client));
  encoder_client.huart = &huart2;
  encoder_client.task_handle = NULL;
  encoder_client.rx_frame_len = 0;
  encoder_client.rx_timeout = 0;

  // ✅ 创建事件组（用来通知RX任务）
  // EventGroupHandle_t eg = xEventGroupCreate();
   NexModbusClient_Init();
    
  if (eg == NULL)
  {
    debug_println("❌ Failed to create event group!");
    return;
  }
  debug_println("✅ Event group created");

  user_task_handle = osThreadNew(start_user_task, NULL, &user_task_attributes); // 创建用户任务线程
                                                                                // 爆闪灯任务
  lightTaskHandle = osThreadNew(YX95R_LED_task, NULL, &lightTask_attributes);   // 灯控任务线程

  modbusRecvTaskHandle = osThreadNew(ModbusRecv_task, NULL, &modbusRecvTask_attributes); // modbus接收任务线程
}

void start_user_task(void *argument)
{
  for (;;)
  {
    debug_println("Hello from User Task!");
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    osDelay(1000);
  }
}
// 爆闪灯
void YX95R_LED_task(void *argument)
{

  for (;;)
  {
    // YX95R_RGB_Control_Light(2, 6,1); // 红色慢闪
    //    	  YX95R_RGB_Light_Off(2);
    //  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    // YX95R_RGB_Control_Light(2, 6, 1); // 红色慢闪
    if(a>0&&a<=2){
      a--;
      YX95R_RGB_Is_Online(0xff);
    // ✅ 通知RX任务：我发送了命令，你可以等待响应了！
    xEventGroupSetBits(eg, EVENT_CMD_SENT);
    }
    // 是否在线查询
    
    osDelay(1000); // 闪 5 秒

    // 灯关闭
    // YX95R_Light_Control(YX95R_ADDR_LIGHT, YX95R_CMD_LIGHT_OFF);
    // osDelay(2000);  // 停 2 秒
  }
}
// modbus接收任务
void ModbusRecv_task(void *argument)
{

  // extern ModbusRtuClient encoder_client;
  ModbusRtuClient *client = &encoder_client;
  client->task_handle = xTaskGetCurrentTaskHandle();

  TickType_t last_wake = xTaskGetTickCount();
  HAL_UARTEx_ReceiveToIdle_DMA(client->huart, client->rx_buf, (uint16_t)sizeof(client->rx_buf));

  for (;;)
  {
    // ✅ 等待TX任务发送命令的事件
    EventBits_t uxBits = xEventGroupWaitBits(
        eg,  // 事件组
        EVENT_CMD_SENT,               // 等待这个事件
        pdTRUE,                       // 自动清除标志
        pdFALSE,                      // 不需要等待所有位
        portMAX_DELAY                 // 无限等待
    );
    // 发送命令

    // YX95R_RGB_Control_Light(0xFF, 6,1); // 红色慢闪
    // HAL_UART_Transmit_DMA(&huart2, encoder_client.tx_buf, 8);
    //        YX95R_RGB_Control_Light(2, 6,1); // 红色慢闪
     if ((uxBits & EVENT_CMD_SENT) != 0)
    {
      taskENTER_CRITICAL();
      debug_println("[RX] 📨 TX sent command, now waiting for UART response (3 seconds)...");
      taskEXIT_CRITICAL();
    // 等待中断唤醒（或 3 秒超时）
    uint32_t notif = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(4000));

    if (notif != 0)
    {
      // ✅ 有数据！解析处理
      // uint16_t regs[2] = {0};
      // uint32_t value = 0;
      // mbr_parse_2regs(encoder_client.parse_buf,
      //                 encoder_client.rx_frame_len,
      //                 regs, &value);
      // debug_println("OK: 0x%08lX\n", (unsigned long)value);
      // debug_println("Recevice1: %.*s\n", client->rx_frame_len , client->parse_buf);
      taskENTER_CRITICAL();
      debug_println("Recevice1: ");
      for (int i = 0; i < encoder_client.rx_frame_len; i++)
      {
        debug_println("%02X ", encoder_client.parse_buf[i]);
      }
      taskEXIT_CRITICAL();
    }
    else
    {
      // ❌ 超时
      debug_println("Timeout\n");
    }
  }
    // 下一轮轮询（100ms）
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));
  }
}
