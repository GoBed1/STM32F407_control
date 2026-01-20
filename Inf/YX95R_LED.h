#ifndef YX95R_LED_H
#define YX95R_LED_H
#include "main.h"
#include "cmsis_os.h"
#include <stdint.h>
#include <string.h>
#include "nex_modbus_rtu_client.h"
/* ============================= 灯控制命令定义 ============================= */

// Modbus 功能码
#define YX95R_FUNC_WRITE_REG          0x06  // 写单个寄存器
#define YX95R_FUNC_READ_REG          0x03  // 读单个寄存器

#define YX95R_CMD_VOLUME_UP           0x0004  // 音量增大
#define YX95R_CMD_VOLUME_DOWN         0x0005  // 音量减小
#define YX95R_CMD_SET_VOLUME          0x0006  // 设置音量

#define YX95R_CMD_SET_ADDRESS         0x00C0  // 设置设备地址

#define YX95R_CMD_CHIP_RESET          0x000C  // 芯片复位

#define YX95R_CMD_STOPMUSIC          0x0016  // 停止播放
#define YX95R_CMD_LOOP_SONG            0x0008  // 循环指定歌曲

#define YX95R_CMD_LIGHT_CONTROL      0x00C2  // 灯控制命令寄存器

/* ============================= 函数声明 ============================= */



/**
 * @brief 计算 Modbus CRC16 校验码
 * @param buffer: 数据缓冲区指针
 * @param len: 数据长度
 * @return uint16_t: CRC16 值
 */
uint16_t YX95R_CRC16_Calc(uint8_t *buffer, uint16_t len);


/**
 * @brief 通过 RS485 发送 Modbus 命令包
 * @param cmd: 命令数据指针
 * @param len: 命令长度
 * @return void
 * 
 * 注意：
 *  1. 自动控制 RE/DE 脚切换收发模式
 *  2. 等待发送完成才会返回
 *  3. 发送前自动算 CRC 并填充
 */
void YX95R_RGB_Send_Command(uint8_t * cmd, uint8_t len);

//通用 Modbus 寄存器写入函数
static void YX95R_RGB_Write_Register(uint8_t addr, uint16_t reg, uint16_t data);

void YX95R_RGB_Volume_Up(uint8_t addr);

void YX95R_RGB_Volume_Down(uint8_t addr);

void YX95R_RGB_Set_Volume(uint8_t addr, uint16_t volume);

void YX95R_RGB_Set_Address(uint8_t current_addr, uint16_t new_addr);

void YX95R_RGB_Chip_Reset(uint8_t addr);

void YX95R_RGB_StopMusic(uint8_t addr);

void YX95R_RGB_Loop_Song(uint8_t addr, uint16_t song_index);

void YX95R_RGB_Control_Light(uint8_t addr, uint8_t X, uint8_t Y);

void YX95R_RGB_Light_Off(uint8_t addr);

uint8_t YX95R_RGB_Is_Online(uint8_t addr);

#endif // YX95R_LED_H
