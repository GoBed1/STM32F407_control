#include "com_debug.h"

extern UART_HandleTypeDef huart1;
uint8_t a=2;
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 1000);
    return len;
}

