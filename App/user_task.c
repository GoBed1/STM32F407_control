#include "user_task.h"
#include "cmsis_os.h"
#include "com_debug.h"
#include "user_logic.h"
#include "semphr.h"
#define CMD_LED_SWITCH 0
#define STATUS_LED_SWITCH    100

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern ModbusRtuClient encoder_client;
extern EventGroupHandle_t eg;
extern ModbusRtu_Resend_t Resend;
extern uint8_t test_tx_cmd[8];
extern SemaphoreHandle_t uart2_rx_semaphore;
// ✅ 定义 Modbus 从机全局变量
nmbs_t modbus_slave;

nmbs_server_t slave_data = {
    .id = 0x02, // 从机地址（LoRa转485模块的地址）
    .coils = {0},
    .regs = {0}};

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

// 创建接收控制任务
osThreadId_t modbusRecvTaskHandle;
const osThreadAttr_t modbusRecvTask_attributes = {
    .name = "modbusRecvTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
// 接收工控机数据任务
osThreadId_t RecvMasterTaskHandle;
const osThreadAttr_t RecvMasterTask_attributes = {
    .name = "RecvMasterTask",
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

    // ✅ 创建二值信号量（初始状态为可用）
    // uart2_rx_semaphore = xSemaphoreCreateBinary();
    // if (uart2_rx_semaphore == NULL) {
    //     debug_println("❌ Failed to create UART2 semaphore!");
    //     return;
    // }
    // xSemaphoreGive(uart2_rx_semaphore);  // 初始释放，允许第一次访问
  // encoder_client.Rx_lora_task_handle = NULL;//用于测试lora-485接收的任务句柄
  // ✅ 创建事件组（用来通知RX任务）
  // EventGroupHandle_t eg = xEventGroupCreate();
  // 创建事件组（用来通知RX任务）
  EventGroupCreate_Init();
  if (eg == NULL)
  {
    // debug_println("Failed to create event group!");
    return;
  }
  // debug_println("Event group created");
  HAL_UARTEx_ReceiveToIdle_DMA(encoder_client.huart, encoder_client.rx_buf, (uint16_t)sizeof(encoder_client.rx_buf));

  // 初始化Modbus客户端
  // ✅ 初始化寄存器默认值
  // slave_data.regs[10] = 1;  // 默认颜色：红色
  // slave_data.regs[11] = 50; // 默认亮度：50
  nmbs_error err = nmbs_server_init(&modbus_slave, &slave_data);
  if (err != NMBS_ERROR_NONE)
  {
    // debug_println("❌ Modbus server init failed: %d", err);

    return;
  }
  else
  {
    // debug_println("✅ Modbus server initialized successfully");
  }

  user_task_handle = osThreadNew(start_user_task, NULL, &user_task_attributes); // 创建用户任务线程
                                                                                // 爆闪灯任务
  lightTaskHandle = osThreadNew(YX95R_LED_task, NULL, &lightTask_attributes);   // 灯控任务线程

  modbusRecvTaskHandle = osThreadNew(ModbusRecv_task, NULL, &modbusRecvTask_attributes); // modbus接收任务线程

  RecvMasterTaskHandle = osThreadNew(RecvMaster_task, NULL, &RecvMasterTask_attributes); // 接收工控机数据任务线程
  // if (RecvMasterTaskHandle == NULL) {
  //       debug_println("❌ Failed to create RecvMasterTask!");
  //   } else {
  //       debug_println("✅ RecvMasterTask created");
  //   }
}

void start_user_task(void *argument)
{
  for (;;)
  {
    // debug_println("Hello from User Task!");
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    osDelay(1000);
  }
}
// 爆闪灯
void YX95R_LED_task(void *argument)
{

  for (;;)
  {
    taskENTER_CRITICAL();
    uint16_t cmd_led_switch = slave_data.coils[CMD_LED_SWITCH];
    taskEXIT_CRITICAL();
    // YX95R_RGB_Control_Light(2, 6,1); // 红色慢闪
    //    	  YX95R_RGB_Light_Off(2);
    //  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    // YX95R_RGB_Control_Light(2, 6, 1); // 红色慢闪
    // debug_println("YX95R_LED_task111111111111111111111111");
    // debug_println("线圈状态：%d", slave_data.coils[STATUS_LED_SWITCH]);
    // debug_println("线圈指令：%d", slave_data.coils[CMD_LED_SWITCH]);
    // debug_println("线圈other：%d", slave_data.coils[1]);
    // debug_println("保持reg状态：%d", slave_data.regs[0]);

    if (cmd_led_switch == 1 && slave_data.coils[STATUS_LED_SWITCH] == 0)
    {
      // 灯打开
      // debug_println(" LED ON ");
      YX95R_RGB_Control_Light(1, 6, 1); // 红色慢闪
      // 更新状态寄存器coils[100]且清除命令寄存器coils[0]
      taskENTER_CRITICAL();
      slave_data.coils[STATUS_LED_SWITCH] = 1;
      taskEXIT_CRITICAL();
      // 通知RX任务：我发送了命令，你可以等待响应了！
      xEventGroupSetBits(eg, EVENT_CMD_SENT);
    }
     if (cmd_led_switch == 0 && slave_data.coils[STATUS_LED_SWITCH] == 1)
    {
      // 灯关闭
      // debug_println(" LED off ");
      YX95R_RGB_Light_Off(1); // 红色慢闪
      // 更新状态寄存器coils[100]且清除命令寄存器coils[0]
      taskENTER_CRITICAL();
      slave_data.coils[STATUS_LED_SWITCH] = 0;
      taskEXIT_CRITICAL();
      // 通知RX任务：我发送了命令，你可以等待响应了！
      xEventGroupSetBits(eg, EVENT_CMD_SENT);
    }
    // 是否在线查询

    osDelay(500); // 闪 5 秒

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
  // HAL_UARTEx_ReceiveToIdle_DMA(client->huart, client->rx_buf, (uint16_t)sizeof(client->rx_buf));

  for (;;)
  {
    // ✅ 等待TX任务发送命令的事件
    EventBits_t uxBits = xEventGroupWaitBits(
        eg,             // 事件组
        EVENT_CMD_SENT, // 等待这个事件
        pdTRUE,         // 自动清除标志
        pdFALSE,        // 不需要等待所有位
        portMAX_DELAY   // 无限等待
    );
    // 发送命令
    if ((uxBits & EVENT_CMD_SENT) != 0)
    {
      taskENTER_CRITICAL();
      // debug_println("[RX] TX sent command, now waiting for UART response (3 seconds)...");
      taskEXIT_CRITICAL();
      // 等待中断唤醒（或 3 秒超时）
      uint32_t notif = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3000));

      if (notif != 0)
      {
        // ✅ 有数据！解析处理
        // timeout count 清零
        Resend.timeout_resend_count = 0;
        taskENTER_CRITICAL();
        // debug_println("Recevice1: ");
        // for (int i = 0; i < encoder_client.rx_frame_len; i++)
        // {
        //   debug_println("%02X ", encoder_client.parse_buf[i]);
        // }
        taskEXIT_CRITICAL();
        modbus_RxData_logic(encoder_client.parse_buf, encoder_client.rx_frame_len);
      }
      else
      {
        // 处理超时重发
        Resend.timeout_resend_count++;
        // ❌ 超时
        // debug_println("Timeout--%d\n", Resend.timeout_resend_count);

        if (Resend.timeout_resend_count > Resend.timeout_max_resend)
        {
          // 重发次数过多，重置计数器并退出任务
          Resend.timeout_resend_count = 0;
          // debug_println("Max timeout resend reached. Exiting task.");
        }
        else
        {
          // HAL_UARTEx_ReceiveToIdle_DMA(client->huart, client->rx_buf, (uint16_t)sizeof(client->rx_buf));
          // 测试重新发指令
          // YX95R_RGB_Send_Command(test_tx_cmd, 8);
          // debug_println("Resend command due to timeout...%d", Resend.timeout_resend_count);
          // 发送标志位置为0
          xEventGroupSetBits(eg, EVENT_CMD_SENT);
        }
      }
    }
    // 下一轮轮询（100ms）
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(3000));
  }
}

// 接收工控机数据任务(从机任务slave task)
void RecvMaster_task(void *argument)
{
  // taskENTER_CRITICAL();
  // debug_println("modbusSlave_task started...........");
  // taskEXIT_CRITICAL();
  // encoder_client.Rx_lora_task_handle = xTaskGetCurrentTaskHandle();//用于测试lora-485接收的任务句柄
  for (;;)
  {
    // ✅ 等待中断通知（无限等待）
    // uint32_t notif1 = ulTaskNotifyTake(
    //     pdTRUE,       // 收到通知后清除计数
    //     portMAX_DELAY // 无限等待
    // );
    // // 在这里添加接收工控机数据的代码
    // if (notif1 != 0)
    // {
    // 接收数据逻辑处理代码...
    // taskENTER_CRITICAL();
    // debug_println("Recevice master.......: ");
    // for (int i = 0; i < encoder_client.rx_frame_len; i++)
    // {
    //   debug_println("%02X ", encoder_client.parse_buf[i]);
    // }
    // taskEXIT_CRITICAL();
    // }
    // ✅ 轮询处理 Modbus 请求
    nmbs_error a= nmbs_server_poll(&modbus_slave);
    debug_println("%d",a);
    // send()

    osDelay(500); // 根据需要调整延迟时间
  }
}
