import pytest
from unittest.mock import MagicMock, patch
import sys
import os

# 添加项目路径以便导入模块
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

# 导入用户任务函数
try:
    from App.user_task import YX95R_LED_task
    import App.user_task as user_task_module
except ImportError:
    # 为测试创建mock模块
    class MockUserTaskModule:
        def YX95R_LED_task(self, argument):
            pass
    
    user_task_module = MockUserTaskModule()
    YX95R_LED_task = user_task_module.YX95R_LED_task

# 测试常量
LED1_GPIO_Port = MagicMock()
LED1_Pin = 0

class TestYX95RLEDTask:
    """测试YX95R_LED_task函数"""
    
    @patch('App.user_task.YX95R_RS485_Send')
    @patch('App.user_task.YX95R_CRC16_Calc')
    @patch('App.user_task.HAL_GPIO_TogglePin')
    @patch('App.user_task.osDelay')
    def test_led_task_basic_operation(self, mock_delay, mock_toggle, mock_crc, mock_send):
        """测试LED任务基本操作"""
        mock_crc.return_value = 0x8627
        mock_argument = MagicMock()
        
        # 运行一次任务迭代
        try:
            YX95R_LED_task(mock_argument)
        except Exception as e:
            # 允许函数运行但捕获可能的异常（由于无限循环）
            pass
        
        # 验证GPIO切换被调用
        mock_toggle.assert_called_once_with(LED1_GPIO_Port, LED1_Pin)
        
        # 验证CRC计算被调用
        mock_crc.assert_called_once()
        called_args = mock_crc.call_args[0]
        assert len(called_args[0]) == 6  # 6字节的缓冲区
        assert called_args[1] == 6       # 长度为6
        
        # 验证RS485发送被调用
        mock_send.assert_called_once()
        send_args = mock_send.call_args[0]
        assert len(send_args[0]) == 8    # 8字节的命令
        assert send_args[1] == 8         # 长度为8
        
        # 验证延迟被调用
        mock_delay.assert_called_once_with(1000)
    
    @patch('App.user_task.YX95R_RS485_Send')
    @patch('App.user_task.YX95R_CRC16_Calc')
    @patch('App.user_task.HAL_GPIO_TogglePin')
    @patch('App.user_task.osDelay')
    def test_led_task_command_content(self, mock_delay, mock_toggle, mock_crc, mock_send):
        """测试LED任务生成的命令内容"""
        mock_crc.return_value = 0x1234
        mock_argument = MagicMock()
        
        try:
            YX95R_LED_task(mock_argument)
        except:
            pass
        
        # 验证发送的命令内容正确
        send_args = mock_send.call_args[0]
        cmd = send_args[0]
        
        # 验证命令格式
        assert cmd[0] == 0xFF  # 广播地址
        assert cmd[1] == 0x06  # 功能码
        assert cmd[2] == 0x00  # 寄存器地址高字节
        assert cmd[3] == 0xC2  # 寄存器地址低字节
        assert cmd[4] == 0x00  # 数据高字节
        assert cmd[5] == 0x41  # 数据低字节
        
        # 验证CRC部分
        assert cmd[6] == 0x34  # CRC低字节
        assert cmd[7] == 0x12  # CRC高字节
    
    @patch('App.user_task.YX95R_RS485_Send')
    @patch('App.user_task.YX95R_CRC16_Calc')
    @patch('App.user_task.HAL_GPIO_TogglePin')
    @patch('App.user_task.osDelay')
    def test_led_task_crc_calculation(self, mock_delay, mock_toggle, mock_crc, mock_send):
        """测试CRC计算的输入数据"""
        def mock_crc_side_effect(buffer, length):
            # 验证CRC计算的输入数据正确
            expected_buffer = bytearray([0xFF, 0x06, 0x00, 0xC2, 0x00, 0x41])
            assert list(buffer[:6]) == list(expected_buffer)
            assert length == 6
            return 0xABCD  # 返回固定的CRC值
        
        mock_crc.side_effect = mock_crc_side_effect
        mock_argument = MagicMock()
        
        try:
            YX95R_LED_task(mock_argument)
        except:
            pass
        
        # 验证发送的命令包含正确的CRC
        send_args = mock_send.call_args[0]
        cmd = send_args[0]
        assert cmd[6] == 0xCD  # CRC低字节
        assert cmd[7] == 0xAB  # CRC高字节
    
    @patch('App.user_task.YX95R_RS485_Send')
    @patch('App.user_task.YX95R_CRC16_Calc')
    @patch('App.user_task.HAL_GPIO_TogglePin')
    @patch('App.user_task.osDelay')
    def test_led_task_multiple_iterations(self, mock_delay, mock_toggle, mock_crc, mock_send):
        """测试多次迭代（模拟有限次循环）"""
        mock_crc.return_value = 0x8627
        mock_argument = MagicMock()
        
        # 模拟运行3次迭代
        iteration_count = 0
        def mock_delay_side_effect(delay):
            nonlocal iteration_count
            iteration_count += 1
            if iteration_count >= 3:
                raise StopIteration("模拟中断无限循环")
        
        mock_delay.side_effect = mock_delay_side_effect
        
        try:
            YX95R_LED_task(mock_argument)
        except StopIteration:
            pass  # 预期的中断
        
        # 验证调用了3次
        assert mock_toggle.call_count == 3
        assert mock_crc.call_count == 3
        assert mock_send.call_count == 3
        assert mock_delay.call_count == 3
    
    @patch('App.user_task.YX95R_RS485_Send')
    @patch('App.user_task.YX95R_CRC16_Calc')
    @patch('App.user_task.HAL_GPIO_TogglePin')
    @patch('App.user_task.osDelay')
    def test_led_task_edge_cases(self, mock_delay, mock_toggle, mock_crc, mock_send):
        """测试边界情况"""
        # 测试CRC计算返回边界值
        test_crc_values = [
            (0x0000, "最小CRC值"),
            (0xFFFF, "最大CRC值"),
            (0x1234, "正常CRC值")
        ]
        
        for crc_value, desc in test_crc_values:
            mock_crc.reset_mock()
            mock_send.reset_mock()
            mock_toggle.reset_mock()
            mock_delay.reset_mock()
            
            mock_crc.return_value = crc_value
            mock_argument = MagicMock()
            
            try:
                YX95R_LED_task(mock_argument)
            except:
                pass
            
            # 验证CRC值正确使用
            send_args = mock_send.call_args[0]
            cmd = send_args[0]
            assert cmd[6] == (crc_value & 0xFF)        # CRC低字节
            assert cmd[7] == ((crc_value >> 8) & 0xFF)  # CRC高字节

# 运行测试
if __name__ == "__main__":
    pytest.main([__file__, "-v"])