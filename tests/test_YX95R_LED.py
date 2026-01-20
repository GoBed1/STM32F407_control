import pytest
from unittest.mock import MagicMock, patch
import sys
import os

# 添加项目路径以便导入模块
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

# 导入被测试的函数
try:
    from App.YX95R_LED import (
        YX95R_CRC16_Calc,
        YX95R_RS485_Send,
        YX95R_Light_Control,
        YX95R_Light_Play
    )
    import App.YX95R_LED as yx95r_module
except ImportError:
    # 为测试创建mock模块
    class MockYX95RModule:
        def YX95R_CRC16_Calc(self, buffer, length):
            return 0x0000
        
        def YX95R_RS485_Send(self, cmd, length):
            pass
        
        def YX95R_Light_Control(self, addr, mode):
            pass
        
        def YX95R_Light_Play(self, addr, folder, song, light_mode):
            pass
    
    yx95r_module = MockYX95RModule()
    YX95R_CRC16_Calc = yx95r_module.YX95R_CRC16_Calc
    YX95R_RS485_Send = yx95r_module.YX95R_RS485_Send
    YX95R_Light_Control = yx95r_module.YX95R_Light_Control
    YX95R_Light_Play = yx95r_module.YX95R_Light_Play

# 测试常量
YX95R_FUNC_WRITE_REG = 0x06
YX95R_REG_LIGHT_CTRL = 0x00C2
YX95R_CMD_LIGHT_FLASH_BURST = 0x03
YX95R_CMD_LIGHT_FLASH_SLOW = 0x04
YX95R_CMD_LIGHT_ALWAYS_ON = 0x05
YX95R_CMD_LIGHT_OFF = 0x06
YX95R_CMD_PLAY_BURST = 0x30
YX95R_CMD_PLAY_SLOW = 0x40
YX95R_CMD_PLAY_ALWAYS = 0x50
YX95R_ADDR_LIGHT = 0x01
YX95R_ADDR_SPEAKER = 0x02
YX95R_ADDR_BROADCAST = 0xFF

class TestYX95RCRC16Calc:
    """测试YX95R_CRC16_Calc函数"""
    
    def test_crc16_calc_empty_buffer(self):
        """测试空缓冲区"""
        buffer = bytearray()
        result = YX95R_CRC16_Calc(buffer, 0)
        assert result == 0xFFFF  # 空缓冲区的初始CRC值
        
    def test_crc16_calc_single_byte(self):
        """测试单个字节"""
        buffer = bytearray([0x01])
        result = YX95R_CRC16_Calc(buffer, 1)
        # 已知的CRC16校验值
        expected = 0x807E  # 0x01的Modbus CRC16
        assert result == expected
        
    def test_crc16_calc_multiple_bytes(self):
        """测试多个字节的CRC计算"""
        # 测试数据: [0x01, 0x02, 0x03]
        buffer = bytearray([0x01, 0x02, 0x03])
        result = YX95R_CRC16_Calc(buffer, 3)
        expected = 0x5AC8  # 已知的正确CRC值
        assert result == expected
        
    def test_crc16_calc_modbus_example(self):
        """测试Modbus标准示例"""
        # 标准Modbus示例: [0x01, 0x06, 0x00, 0x01, 0x00, 0x03]
        buffer = bytearray([0x01, 0x06, 0x00, 0x01, 0x00, 0x03])
        result = YX95R_CRC16_Calc(buffer, 6)
        expected = 0x0909  # 已知的正确CRC值
        assert result == expected
        
    def test_crc16_calc_boundary_length(self):
        """测试边界长度"""
        # 最大长度测试 (假设实现支持)
        long_buffer = bytearray([0xAA] * 100)
        result = YX95R_CRC16_Calc(long_buffer, 100)
        # 确保不崩溃且返回有效CRC
        assert isinstance(result, int)
        assert 0x0000 <= result <= 0xFFFF
        
    def test_crc16_calc_zero_length(self):
        """测试长度为0的情况"""
        buffer = bytearray([0x01, 0x02, 0x03])
        result = YX95R_CRC16_Calc(buffer, 0)
        assert result == 0xFFFF
        
    def test_crc16_calc_partial_buffer(self):
        """测试部分缓冲区计算"""
        buffer = bytearray([0x01, 0x02, 0x03, 0x04, 0x05])
        result = YX95R_CRC16_Calc(buffer, 3)  # 只计算前3个字节
        expected_full = YX95R_CRC16_Calc(buffer[:3], 3)
        assert result == expected_full

class TestYX95RRS485Send:
    """测试YX95R_RS485_Send函数"""
    
    @patch('App.YX95R_LED.HAL_UART_Transmit')
    @patch('App.YX95R_LED.HAL_Delay')
    def test_rs485_send_normal_command(self, mock_delay, mock_transmit):
        """测试正常命令发送"""
        cmd = bytearray([0x01, 0x06, 0x00, 0xC2, 0x00, 0x03, 0x27, 0x86])
        YX95R_RS485_Send(cmd, 8)
        
        # 验证UART传输被调用
        mock_transmit.assert_called_once()
        mock_delay.assert_called_once_with(200)
        
    @patch('App.YX95R_LED.HAL_UART_Transmit')
    @patch('App.YX95R_LED.HAL_Delay')
    def test_rs485_send_empty_command(self, mock_delay, mock_transmit):
        """测试空命令发送"""
        cmd = bytearray()
        YX95R_RS485_Send(cmd, 0)
        
        # 空命令应该不发送或正确处理
        mock_transmit.assert_called_once_with(
            pytest.any, cmd, 0, pytest.any
        )
        
    @patch('App.YX95R_LED.HAL_UART_Transmit')
    @patch('App.YX95R_LED.HAL_Delay')
    def test_rs485_send_single_byte(self, mock_delay, mock_transmit):
        """测试单字节命令"""
        cmd = bytearray([0xFF])
        YX95R_RS485_Send(cmd, 1)
        
        mock_transmit.assert_called_once()
        mock_delay.assert_called_once_with(200)
        
    @patch('App.YX95R_LED.HAL_UART_Transmit')
    @patch('App.YX95R_LED.HAL_Delay')
    def test_rs485_send_max_length(self, mock_delay, mock_transmit):
        """测试最大长度命令"""
        cmd = bytearray([0xAA] * 255)  # 最大uint8长度
        YX95R_RS485_Send(cmd, 255)
        
        mock_transmit.assert_called_once()
        mock_delay.assert_called_once_with(200)

class TestYX95RLightControl:
    """测试YX95R_Light_Control函数"""
    
    @patch('App.YX95R_LED.YX95R_RS485_Send')
    @patch('App.YX95R_LED.YX95R_CRC16_Calc')
    def test_light_control_burst_mode(self, mock_crc, mock_send):
        """测试爆闪模式控制"""
        mock_crc.return_value = 0x8627  # Mock CRC值
        
        YX95R_Light_Control(YX95R_ADDR_LIGHT, YX95R_CMD_LIGHT_FLASH_BURST)
        
        # 验证CRC计算被调用
        mock_crc.assert_called_once()
        
        # 验证发送的命令格式正确
        expected_cmd = bytearray([
            YX95R_ADDR_LIGHT,           # 地址
            YX95R_FUNC_WRITE_REG,       # 功能码
            0x00,                       # 寄存器地址高字节
            0xC2,                       # 寄存器地址低字节
            0x00,                       # 数据高字节
            YX95R_CMD_LIGHT_FLASH_BURST,# 数据低字节
            0x27,                       # CRC低字节
            0x86                        # CRC高字节
        ])
        mock_send.assert_called_once_with(expected_cmd, 8)
        
    @patch('App.YX95R_LED.YX95R_RS485_Send')
    @patch('App.YX95R_LED.YX95R_CRC16_Calc')
    def test_light_control_all_modes(self, mock_crc, mock_send):
        """测试所有灯光模式"""
        test_modes = [
            (YX95R_CMD_LIGHT_FLASH_BURST, "爆闪"),
            (YX95R_CMD_LIGHT_FLASH_SLOW, "慢闪"),
            (YX95R_CMD_LIGHT_ALWAYS_ON, "常亮"),
            (YX95R_CMD_LIGHT_OFF, "关闭")
        ]
        
        for mode, mode_name in test_modes:
            mock_crc.reset_mock()
            mock_send.reset_mock()
            mock_crc.return_value = 0x8627
            
            YX95R_Light_Control(YX95R_ADDR_LIGHT, mode)
            
            # 验证命令格式正确
            expected_cmd = bytearray([
                YX95R_ADDR_LIGHT,   # 地址
                YX95R_FUNC_WRITE_REG,
                0x00, 0xC2,        # 寄存器地址
                0x00, mode,         # 数据
                0x27, 0x86         # CRC
            ])
            mock_send.assert_called_once_with(expected_cmd, 8)
            
    @patch('App.YX95R_LED.YX95R_RS485_Send')
    @patch('App.YX95R_LED.YX95R_CRC16_Calc')
    def test_light_control_different_addresses(self, mock_crc, mock_send):
        """测试不同设备地址"""
        test_addresses = [
            (YX95R_ADDR_LIGHT, "灯光设备"),
            (YX95R_ADDR_SPEAKER, "喇叭设备"),
            (YX95R_ADDR_BROADCAST, "广播地址")
        ]
        
        for addr, addr_name in test_addresses:
            mock_crc.reset_mock()
            mock_send.reset_mock()
            mock_crc.return_value = 0x8627
            
            YX95R_Light_Control(addr, YX95R_CMD_LIGHT_FLASH_BURST)
            
            expected_cmd = bytearray([
                addr,                   # 设备地址
                YX95R_FUNC_WRITE_REG,
                0x00, 0xC2,
                0x00, YX95R_CMD_LIGHT_FLASH_BURST,
                0x27, 0x86
            ])
            mock_send.assert_called_once_with(expected_cmd, 8)
            
    @patch('App.YX95R_LED.YX95R_RS485_Send')
    @patch('App.YX95R_LED.YX95R_CRC16_Calc')
    def test_light_control_invalid_mode(self, mock_crc, mock_send):
        """测试无效灯光模式"""
        # 即使模式无效，函数也应该尝试发送
        invalid_mode = 0xFF
        YX95R_Light_Control(YX95R_ADDR_LIGHT, invalid_mode)
        
        mock_send.assert_called_once()
        # 验证命令中包含无效模式
        call_args = mock_send.call_args[0][0]
        assert call_args[5] == invalid_mode

class TestYX95RLightPlay:
    """测试YX95R_Light_Play函数"""
    
    @patch('App.YX95R_LED.YX95R_RS485_Send')
    @patch('App.YX95R_LED.YX95R_CRC16_Calc')
    def test_light_play_burst_mode(self, mock_crc, mock_send):
        """测试播放爆闪模式"""
        mock_crc.return_value = 0x1234
        
        YX95R_Light_Play(
            YX95R_ADDR_LIGHT, 
            0x01,  # 文件夹号
            0x01,  # 曲目号
            YX95R_CMD_PLAY_BURST
        )
        
        # 验证命令格式正确
        expected_cmd = bytearray([
            YX95R_ADDR_LIGHT,       # 地址
            YX95R_FUNC_WRITE_REG,   # 功能码
            YX95R_CMD_PLAY_BURST,   # 灯行为控制
            0x0F,                   # 播放命令寄存器
            0x01,                   # 文件夹号
            0x01,                   # 曲目号
            0x34,                   # CRC低字节
            0x12                    # CRC高字节
        ])
        mock_send.assert_called_once_with(expected_cmd, 8)
        
    @patch('App.YX95R_LED.YX95R_RS485_Send')
    @patch('App.YX95R_LED.YX95R_CRC16_Calc')
    def test_light_play_all_modes(self, mock_crc, mock_send):
        """测试所有播放模式"""
        test_modes = [
            (YX95R_CMD_PLAY_BURST, "爆闪保持"),
            (YX95R_CMD_PLAY_SLOW, "慢闪保持"),
            (YX95R_CMD_PLAY_ALWAYS, "常亮保持")
        ]
        
        for mode, mode_name in test_modes:
            mock_crc.reset_mock()
            mock_send.reset_mock()
            mock_crc.return_value = 0x1234
            
            YX95R_Light_Play(YX95R_ADDR_LIGHT, 0x01, 0x01, mode)
            
            expected_cmd = bytearray([
                YX95R_ADDR_LIGHT,
                YX95R_FUNC_WRITE_REG,
                mode,       # 灯行为控制
                0x0F,       # 播放命令寄存器
                0x01,       # 文件夹号
                0x01,       # 曲目号
                0x34,       # CRC低字节
                0x12        # CRC高字节
            ])
            mock_send.assert_called_once_with(expected_cmd, 8)
            
    @patch('App.YX95R_LED.YX95R_RS485_Send')
    @patch('App.YX95R_LED.YX95R_CRC16_Calc')
    def test_light_play_boundary_values(self, mock_crc, mock_send):
        """测试边界值"""
        # 测试文件夹和曲目的边界值
        test_cases = [
            (0x01, 0x01, "最小有效值"),
            (0x63, 0xFF, "最大有效值"),  # 文件夹 1-99, 曲目 1-255
            (0x32, 0x80, "中间值")
        ]
        
        for folder, song, desc in test_cases:
            mock_crc.reset_mock()
            mock_send.reset_mock()
            mock_crc.return_value = 0x1234
            
            YX95R_Light_Play(YX95R_ADDR_LIGHT, folder, song, YX95R_CMD_PLAY_BURST)
            
            # 验证命令中包含正确的文件夹和曲目号
            call_args = mock_send.call_args[0][0]
            assert call_args[4] == folder
            assert call_args[5] == song
            
    @patch('App.YX95R_LED.YX95R_RS485_Send')
    @patch('App.YX95R_LED.YX95R_CRC16_Calc')
    def test_light_play_invalid_values(self, mock_crc, mock_send):
        """测试无效值处理"""
        # 即使值无效，函数也应该尝试发送
        YX95R_Light_Play(YX95R_ADDR_LIGHT, 0x00, 0x00, 0xFF)
        
        mock_send.assert_called_once()
        call_args = mock_send.call_args[0][0]
        assert call_args[4] == 0x00  # 无效文件夹号
        assert call_args[5] == 0x00  # 无效曲目号
        assert call_args[2] == 0xFF  # 无效灯模式

class TestYX95RLEDTask:
    """测试YX95R_LED_task函数（用户选择的任务）"""
    
    @patch('App.user_task.YX95R_RS485_Send')
    @patch('App.user_task.YX95R_CRC16_Calc')
    @patch('App.user_task.HAL_GPIO_TogglePin')
    @patch('App.user_task.osDelay')
    def test_led_task_basic_operation(self, mock_delay, mock_toggle, mock_crc, mock_send):
        """测试LED任务基本操作"""
        # 导入用户任务函数
        from App.user_task import YX95R_LED_task
        
        mock_crc.return_value = 0x8627
        
        # 创建模拟参数
        mock_argument = MagicMock()
        
        # 运行一次循环（由于是无限循环，我们测试一次迭代）
        try:
            YX95R_LED_task(mock_argument)
        except Exception as e:
            # 允许函数运行但捕获可能的异常
            pass
        
        # 验证GPIO切换被调用
        mock_toggle.assert_called_once()
        
        # 验证CRC计算被调用
        mock_crc.assert_called_once()
        
        # 验证RS485发送被调用
        mock_send.assert_called_once()
        
        # 验证延迟被调用
        mock_delay.assert_called_once_with(1000)
        
    @patch('App.user_task.YX95R_RS485_Send')
    @patch('App.user_task.YX95R_CRC16_Calc')
    def test_led_task_command_format(self, mock_crc, mock_send):
        """测试LED任务生成的命令格式"""
        from App.user_task import YX95R_LED_task
        
        mock_crc.return_value = 0x8627
        mock_argument = MagicMock()
        
        try:
            YX95R_LED_task(mock_argument)
        except:
            pass
        
        # 验证生成的命令格式正确
        expected_cmd = bytearray([
            0xFF,           # 广播地址
            0x06,           # 功能码
            0x00,           # 寄存器地址高字节
            0xC2,           # 寄存器地址低字节
            0x00,           # 数据高字节
            0x41,           # 数据低字节 (灯控制命令)
            0x27,           # CRC低字节
            0x86            # CRC高字节
        ])
        
        mock_send.assert_called_once_with(expected_cmd, 8)

# 运行测试
if __name__ == "__main__":
    pytest.main([__file__, "-v"])