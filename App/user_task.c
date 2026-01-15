#include "user_task.h"
#include "cmsis_os.h"

extern UART_HandleTypeDef huart1;

osThreadId_t user_task_handle;
const osThreadAttr_t user_task_attributes = {
  .name = "userTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

void init_user_task(void)
{
  user_task_handle = osThreadNew(start_user_task, NULL, &user_task_attributes);  // 创建用户任务线程
}

void start_user_task(void *argument)
{
  for (;;)
  {
//	  LOG_INFO("SYS", "Hello From Board: %s\n",BOARD_NAME);
//    HAL_GPIO_TogglePin(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
	  HAL_UART_Transmit(&huart1, (uint8_t *)"Hello\r\n", 7, HAL_MAX_DELAY);
	 	  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
   HAL_UART_Transmit(&huart1, (uint8_t *)"Hello from User Task!\r\n", 23, HAL_MAX_DELAY);
//	 check_stack_watermark("userTask");
   osDelay(1000);
  }
}
