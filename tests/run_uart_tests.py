#!/usr/bin/env python3
"""
UART应用模块单元测试运行脚本
"""

import subprocess
import sys
import os

def run_uart_tests():
    """运行UART应用模块单元测试"""
    print("🚀 开始运行UART应用模块单元测试...")
    print("=" * 60)
    
    # 添加项目路径
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    sys.path.insert(0, project_root)
    
    # UART相关的测试文件
    test_files = [
        "test_uart_app.py"
    ]
    
    results = []
    
    for test_file in test_files:
        print(f"\n📋 运行测试文件: {test_file}")
        print("-" * 40)
        
        try:
            # 运行pytest测试
            result = subprocess.run([
                sys.executable, "-m", "pytest", 
                os.path.join("tests", test_file),
                "-v",
                "--tb=short"
            ], cwd=project_root, capture_output=True, text=True, timeout=60)
            
            # 打印输出
            print(result.stdout)
            if result.stderr:
                print("❌ 错误输出:")
                print(result.stderr)
            
            results.append((test_file, result.returncode == 0))
            
        except subprocess.TimeoutExpired:
            print(f"⏰ 测试超时: {test_file}")
            results.append((test_file, False))
        except Exception as e:
            print(f"❌ 运行测试时出错: {e}")
            results.append((test_file, False))
    
    print("\n" + "=" * 60)
    print("📊 UART测试结果汇总:")
    print("-" * 40)
    
    all_passed = True
    for test_file, passed in results:
        status = "✅ 通过" if passed else "❌ 失败"
        print(f"{status}: {test_file}")
        if not passed:
            all_passed = False
    
    print("\n" + "=" * 60)
    if all_passed:
        print("🎉 所有UART测试通过!")
        return 0
    else:
        print("⚠️  部分UART测试失败，请检查代码")
        return 1

def check_dependencies():
    """检查必要的依赖"""
    print("🔍 检查依赖...")
    
    try:
        import pytest
        print("✅ pytest 已安装")
    except ImportError:
        print("❌ pytest 未安装，请运行: pip install pytest")
        return False
    
    try:
        from unittest.mock import MagicMock, patch
        print("✅ unittest.mock 可用")
    except ImportError:
        print("❌ unittest.mock 不可用")
        return False
    
    return True

if __name__ == "__main__":
    if check_dependencies():
        exit_code = run_uart_tests()
        sys.exit(exit_code)
    else:
        print("❌ 依赖检查失败")
        sys.exit(1)