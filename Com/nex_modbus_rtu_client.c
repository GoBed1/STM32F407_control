
#include "nex_modbus_rtu_client.h"
// #include <stddef.h>
#include "event_groups.h"
ModbusRtuClient encoder_client;
// uint16_t a=0;
// ✅ 创建事件组（用来通知RX任务）
  // EventGroupHandle_t eg  = xEventGroupCreate();
  EventGroupHandle_t eg = NULL; // 初始化事件组为NULL
  void NexModbusClient_Init(void) {
    if (eg == NULL) {
        eg = xEventGroupCreate();
    }
}
uint16_t mbr_calc_crc(uint8_t *Buffer, uint8_t u8length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < u8length; i++)
    {
        temp = temp ^ Buffer[i];
        for (unsigned char j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>=1;
            if (flag)
                temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    // the returned value is already swapped
    // crcLo byte is first & crcHi byte is last
    return temp;

}

void mbr_put_crc(uint8_t* frame, uint8_t len_wo_crc)
{
  if (!frame) {
    return;
  }
  // NOTE: this project's mbr_calc_crc() returns swapped order.
  // Keep compatibility with existing on-wire expectation.
  uint16_t crc = mbr_calc_crc(frame, len_wo_crc);
  frame[len_wo_crc]     = (uint8_t)((crc >> 8) & 0xFF);
  frame[len_wo_crc + 1] = (uint8_t)(crc & 0xFF);
}

  // Parse a Modbus RTU frame that returns two 16-bit registers (byte count = 4).
  // On success, regs_out[0] is the high 16 bits, regs_out[1] is the low 16 bits,
  // and *value_out is the combined 32-bit value. Returns 0 on success, -1 on error.
int mbr_parse_2regs(const uint8_t* frame, uint8_t len,
                    uint16_t regs_out[2], uint32_t* value_out)
{
  if (!frame || len < 9 || !regs_out || !value_out) {
    return -1;
  }

  uint8_t fc = frame[1];
  uint8_t bc = frame[2];
  if (fc != 0x03 || bc != 0x04) {
    return -1;
  }

  uint16_t crc_calc = mbr_calc_crc((uint8_t*)frame, (uint8_t)(len - 2));
  uint8_t crc_hi = (uint8_t)((crc_calc >> 8) & 0xFF);
  uint8_t crc_lo = (uint8_t)(crc_calc & 0xFF);
  if (frame[len - 2] != crc_hi || frame[len - 1] != crc_lo) {
    return -1;
  }

  uint16_t r0 = ((uint16_t)frame[3] << 8) | (uint16_t)frame[4];
  uint16_t r1 = ((uint16_t)frame[5] << 8) | (uint16_t)frame[6];
  regs_out[0] = r0;
  regs_out[1] = r1;
  *value_out = ((uint32_t)r0 << 16) | (uint32_t)r1;
  return 0;
}
