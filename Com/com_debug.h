#ifndef _COM_DEBUG_H
#define _COM_DEBUG_H
#include "stdio.h"
#include "main.h"
#include "string.h"

#include "event_groups.h"

#define EVENT_CMD_SENT (1 << 0)  // ✅ TX发送了命令的事件

extern uint8_t a;
#define DEBUG0

#ifdef DEBUG0

#define FILE_NAME ( strrchr(__FILE__,'/') ? strrchr(__FILE__,'/') +1 : __FILE__ )

#define FILE_NAME2 ( strrchr(FILE_NAME,'\\') ? strrchr(FILE_NAME,'\\') +1 : FILE_NAME )

// main.c:10  xxxxx
#define debug_println(format, ...)  printf("[%20s:%d]  " format "\r\n", FILE_NAME2,__LINE__,##__VA_ARGS__)
#else
#define debug_println(format, ...)
#endif



#endif 


