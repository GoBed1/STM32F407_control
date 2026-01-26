#include "user_logic.h"
extern ModbusRtu_Resend_t Resend;
extern ModbusRtuClient encoder_client;

// 处理接收到的数据包，并返回处理结果
mb_err_op_t modbus_RxData_logic(uint8_t *Rx_data, uint16_t RxLen)
{

    // 提取地址和功能码
    uint8_t slave_id = Rx_data[0];
    uint8_t func_code = Rx_data[1];

    // 根据从机地址分类处理
    switch (slave_id)
    {
    case 0x01: // 灯
        switch (func_code)
        {
        case 0x03: // 读取寄存器
                   // 做crc校验
            // debug_println("CRC OK！");
            return OP_OK_QUERY;

            break;

        case 0x06: // 写寄存器
            // uint16_t crc;
            // debug_println("CRC 0x06 OK！");

            uint16_t crc = YX95R_CRC16_Calc(Rx_data, RxLen - 2);
            uint16_t received_crc = ((uint16_t)Rx_data[RxLen - 1] << 8) | Rx_data[RxLen - 2];
            if (crc == received_crc)
            {
                // 错误次数清0
                Resend.crcError_resend_count = 0;
                taskENTER_CRITICAL();
                // debug_println("CRC OK！");
                taskEXIT_CRITICAL();
                return OP_OK_QUERY;
            }
            else
            {
                // 重新发送命令并计算发送次数，大于3次，如果还是错误则返回错误
                if (Resend.crcError_resend_count < Resend.crcError_max_resend)
                {
                    Resend.crcError_resend_count++;
                    // 重新发送命令×
                }
                else
                {
                    Resend.crcError_resend_count = 0;
                    // debug_println("CRC ERROR！");
                    return ERR_BAD_CRC;
                }
            }

            break;

        default:
            // status.success = 0;
            return 1;
        }
        break;

    case 0x02: // lora-485模块
        switch (func_code)
        {
        case 0x03: // 读取寄存器
            // status.device = DEVICE_LIGHT;
            // status.func = FUNC_READ;
            // status.data_len = Rx_data[2];
            // status.reg_value = (Rx_data[3] << 8) | Rx_data[4];
            // status.success = 1;
            break;

        case 0x06: // 写寄存器
            // status.device = DEVICE_LIGHT;
            // status.func = FUNC_WRITE;
            // status.reg_addr = (Rx_data[2] << 8) | Rx_data[3];
            // status.reg_value = (Rx_data[4] << 8) | Rx_data[5];
            // status.success = 1;
            break;

        default:
            // status.success = 0;
            return 1;
        }
        break;

    case 0x03: // 喇叭

        switch (func_code)
        {
        case 0x03: // 读取寄存器
            // status.device = 0x03;
            // status.func = FUNC_READ;
            // status.data_len = Rx_data[2];
            // status.reg_value = (Rx_data[3] << 8) | Rx_data[4];
            // status.success = 1;
            break;

        case 0x06: // 写寄存器
            // status.device = 0x03;
            // status.func = FUNC_WRITE;
            // status.reg_addr = (Rx_data[2] << 8) | Rx_data[3];
            // status.reg_value = (Rx_data[4] << 8) | Rx_data[5];
            // status.success = 1;
            break;

        default:
            // status.success = 0;
            return 1;
        }
        break;

    default:
        // 未知的从机地址
        // status.success = 0;
        return 1;
    }

    return 1;
}
