#!/usr/bin/env python3
"""
keyboard_test.py
通过键盘控制机械臂动作。

按键映射：
  1 -> 动作1：抬手喝水（D0 灯闪烁）
  2 -> 动作2：肘关节弯曲（D1 灯闪烁）
  0 -> 停止所有动作（双灯闪烁）
  q -> 退出程序

接线：USB-TTL CH340 -> STM32F405 USART1 (COM6, 115200)
"""
import sys
import serial
import serial.tools.list_ports
from pynput.keyboard import Key, Listener

# ---------- 串口命令 ----------
CMD_STOP        = bytes([0x00])   # 停止所有动作
CMD_DRINKING    = bytes([0x01])   # 动作1：抬手喝水
CMD_ELBOW       = bytes([0x02])   # 动作2：肘关节弯曲
CMD_FORWARD     = bytes([0x03])   # 动作3：前进
CMD_BACKWARD    = bytes([0x04])   # 动作4：后退


# ---------- 自动找串口（COM7 优先） ----------
def find_port():
    # 1. 先尝试 COM7
    try:
        ports = [p.device for p in serial.tools.list_ports.comports()]
        if "COM7" in ports:
            return "COM7"
    except:
        pass
    
    # 2. 自动检测 CH340
    for p in serial.tools.list_ports.comports():
        desc = (p.description or "").lower()
        hwid = (p.hwid or "").lower()
        if "usb-serial" in desc or "ch340" in desc or "ch341" in hwid or "7523" in hwid:
            return p.device
    return None

# ---------- 主程序 ----------
def main():
    # 找串口（COM7 优先）
    port = find_port()
    if not port:
        print("[ERROR] Cannot find serial port. Connect device and try again.")
        sys.exit(1)
    print(f"[SERIAL] Detected port on {port}")

    # 打开串口
    ser = serial.Serial(port, baudrate=115200, timeout=1)
    print(f"[SERIAL] Opened {port} @ 115200 bps")
    print("-" * 50)
    print("  1 -> Motion1: Drinking (D0 blink)")
    print("  2 -> Motion2: Elbow bend (D1 blink)")
    print("  3 -> Motion3: Forward     (D0+D1 blink)")
    print("  4 -> Motion4: Backward    (D0+D1 blink)")
    print("  0 -> Stop all   (Both blink)")
    print("  q -> Quit")
    print("-" * 50)

    # 跟踪当前按住的键，防止 pynput 自动重复（按住不放会反复触发 press）
    pressed_keys = set()

    def on_press(key):
        # 获取键的标识（忽略重复按压）
        key_char = getattr(key, 'char', None)
        key_id = id(key)  # 即使 char 为 None 也唯一标识 Key 对象

        # 过滤掉按住不放产生的重复 press 事件
        if key_id in pressed_keys:
            return
        pressed_keys.add(key_id)

        # 数字键 1
        if key_char == '1':
            ser.write(CMD_DRINKING)
            print("  [SEND] CMD_DRINKING (0x01)")

        # 数字键 2
        elif key_char == '2':
            ser.write(CMD_ELBOW)
            print("  [SEND] CMD_ELBOW (0x02)")

        # 数字键 0
        elif key_char == '0':
            ser.write(CMD_STOP)
            print("  [SEND] CMD_STOP (0x00)")

        # 数字键 3
        elif key_char == '3':
            ser.write(CMD_FORWARD)
            print("  [SEND] CMD_FORWARD (0x03)")

        # 数字键 4
        elif key_char == '4':
            ser.write(CMD_BACKWARD)
            print("  [SEND] CMD_BACKWARD (0x04)")

        # q 键退出
        elif key_char == 'q':
            print("\nExiting...")
            ser.close()
            sys.exit(0)

    def on_release(key):
        # 松开键时从集合中移除，允许下次重新触发
        key_id = id(key)
        pressed_keys.discard(key_id)

    # 监听键盘（同时监听 press 和 release）
    listener = Listener(on_press=on_press, on_release=on_release)
    listener.start()
    print("[READY] Press 1/2/3/4/0 to control, Q to quit.")
    listener.join()

if __name__ == "__main__":
    main()
