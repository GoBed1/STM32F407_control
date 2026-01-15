#ifndef _USER_TASK_H
#define _USER_TASK_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HEARTBEAT_LED_PORT  LED0_GPIO_Port
#define HEARTBEAT_LED_PIN   LED0_Pin

void init_user_task(void);
void start_user_task(void *argument);

#ifdef __cplusplus
}
#endif

#endif
