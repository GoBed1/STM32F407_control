#ifndef __USER_LOGIC_H
#define __USER_LOGIC_H
#include "main.h"
#include "nex_modbus_rtu_client.h"
#include "com_debug.h"

mb_err_op_t modbus_RxData_logic(uint8_t *Rx_data, uint16_t RxLen);
#endif // __USER_LOGIC_H