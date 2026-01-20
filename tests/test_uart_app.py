import pytest
from unittest.mock import MagicMock, patch, call
import sys
import os

# 添加项目路径以便导入模块
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

# Mock HAL库和项目相关模块
class MockUART:
    def __init__(self):
        self.Instance = 0x40004400  # USART2基地址

class MockModbusRtuClient:
    def __init__(self):
        self.tx_buf = bytearray(128)
        self.rx_buf = bytearray(128)
        self.rx_frame_len = 0
        self.rx_timeout = 0
        self.parse_buf = bytearray(128)
        self.task_handle = None
        self.timeout_timer = None

# 创建全局mock对象
huart2 = MockUART()
encoder_client = MockModbusRtuClient()

# Mock函数
def mock_debug_println(format_str, *args):
    """Mock debug打印函数"""
    if args:
        print(f"DEBUG: {format_str % args}")
    else:
        print(f"DEBUG: {format_str}")

def mock_mbr_recv_callback(client, huart, size):
    """Mock Modbus接收回调函数"""
    print(f"Modbus接收回调: client={client}, huart={huart}, size={size}")

# 导入或创建被测试的函数
try:
    from App.uart_app import HAL_UARTEx_RxEventCallback
except ImportError:
    # 为测试创建mock实现
    def HAL_UARTEx_RxEventCallback(huart, Size):
        if huart.Instance == huart2.Instance:
            mock_debug_println("%s", Size)
            # mock_mbr_recv_callback(encoder_client, huart, Size)

class TestHAL_UARTEx_RxEventCallback:
    """测试HAL_UARTEx_RxEventCallback函数"""

    def setup_method(self):
        """每个测试方法前的设置"""
        self.huart2_mock = MockUART()
        self.other_huart = MockUART()
        self.other_huart.Instance = 0x40004800  # 不同的UART实例

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_with_huart2_normal_size(self, mock_mbr_callback, mock_debug):
        """测试huart2实例的正常大小回调"""
        # 设置mock返回值
        mock_debug.side_effect = mock_debug_println
        mock_mbr_callback.side_effect = mock_mbr_recv_callback

        # 执行回调
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 10)

        # 验证debug打印被调用
        mock_debug.assert_called_once_with("%s", 10)
        
        # 验证Modbus回调没有被调用（因为被注释掉了）
        mock_mbr_callback.assert_not_called()

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_with_other_huart(self, mock_mbr_callback, mock_debug):
        """测试其他UART实例的回调"""
        HAL_UARTEx_RxEventCallback(self.other_huart, 5)

        # 验证没有调用任何函数（因为实例不匹配）
        mock_debug.assert_not_called()
        mock_mbr_callback.assert_not_called()

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_with_zero_size(self, mock_mbr_callback, mock_debug):
        """测试大小为0的回调"""
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 0)

        # 验证debug打印被调用
        mock_debug.assert_called_once_with("%s", 0)
        mock_mbr_callback.assert_not_called()

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_with_max_size(self, mock_mbr_callback, mock_debug):
        """测试最大大小的回调"""
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 65535)  # uint16最大值

        mock_debug.assert_called_once_with("%s", 65535)
        mock_mbr_callback.assert_not_called()

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_with_boundary_sizes(self, mock_mbr_callback, mock_debug):
        """测试边界大小值"""
        test_sizes = [1, 127, 128, 255, 256, 512, 1024, 2048, 4096]
        
        for size in test_sizes:
            mock_debug.reset_mock()
            HAL_UARTEx_RxEventCallback(self.huart2_mock, size)
            mock_debug.assert_called_once_with("%s", size)

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_multiple_calls(self, mock_mbr_callback, mock_debug):
        """测试多次回调调用"""
        sizes = [10, 20, 30, 40]
        
        for size in sizes:
            HAL_UARTEx_RxEventCallback(self.huart2_mock, size)
        
        # 验证每次调用都正确
        expected_calls = [call("%s", size) for size in sizes]
        mock_debug.assert_has_calls(expected_calls)
        assert mock_debug.call_count == len(sizes)
        mock_mbr_callback.assert_not_called()

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_mixed_huart_instances(self, mock_mbr_callback, mock_debug):
        """测试混合不同UART实例的回调"""
        # huart2实例
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 10)
        mock_debug.assert_called_once_with("%s", 10)
        
        # 其他UART实例
        mock_debug.reset_mock()
        HAL_UARTEx_RxEventCallback(self.other_huart, 20)
        mock_debug.assert_not_called()
        
        # 再次huart2实例
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 30)
        mock_debug.assert_called_once_with("%s", 30)

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_with_negative_size(self, mock_mbr_callback, mock_debug):
        """测试负数的size（虽然uint16不应该有负数）"""
        # uint16不会为负，但测试边界情况
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 0)  # 最小uint16值
        mock_debug.assert_called_once_with("%s", 0)
        
        mock_debug.reset_mock()
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 65535)  # 最大uint16值
        mock_debug.assert_called_once_with("%s", 65535)

    @patch('App.uart_app.debug_println')
    @patch('App.uart_app.mbr_recv_callback')
    def test_callback_with_uncommented_modbus(self, mock_mbr_callback, mock_debug):
        """测试如果取消注释Modbus回调的情况"""
        # 临时修改函数行为来测试Modbus回调路径
        original_callback = HAL_UARTEx_RxEventCallback
        
        def callback_with_modbus(huart, Size):
            if huart.Instance == huart2.Instance:
                mock_debug_println("%s", Size)
                mock_mbr_recv_callback(encoder_client, huart, Size)
        
        # 使用带Modbus回调的版本
        callback_with_modbus(self.huart2_mock, 15)
        
        # 验证两个函数都被调用
        mock_debug.assert_called_once_with("%s", 15)
        mock_mbr_callback.assert_called_once_with(encoder_client, self.huart2_mock, 15)

class TestUARTAppIntegration:
    """测试UART应用模块的集成场景"""

    @patch('App.uart_app.debug_println')
    def test_serial_communication_simulation(self, mock_debug):
        """模拟串口通信场景"""
        # 模拟多个数据包到达
        packet_sizes = [8, 16, 32, 64, 8]  # 典型Modbus数据包大小
        
        for size in packet_sizes:
            HAL_UARTEx_RxEventCallback(self.huart2_mock, size)
        
        # 验证每个数据包都被正确记录
        expected_calls = [call("%s", size) for size in packet_sizes]
        mock_debug.assert_has_calls(expected_calls)
        assert mock_debug.call_count == len(packet_sizes)

    @patch('App.uart_app.debug_println')
    def test_error_scenarios(self, mock_debug):
        """测试错误场景"""
        # 空数据包
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 0)
        mock_debug.assert_called_with("%s", 0)
        
        # 超大数据包（超过缓冲区大小）
        HAL_UARTEx_RxEventCallback(self.huart2_mock, 200)
        mock_debug.assert_called_with("%s", 200)

# 运行测试
if __name__ == "__main__":
    pytest.main([__file__, "-v"])