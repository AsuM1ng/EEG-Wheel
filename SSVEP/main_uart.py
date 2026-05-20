import time
import numpy as np
import sys
import serial
import serial.tools.list_ports
from pynput.keyboard import Key, Listener

from receiver_EEG import NeuracleEEG
from fbcca import FBCCAClassifier


# =========================================================
# 串口命令
# =========================================================

CMD_STOP       = bytes([0x00])   # 停止所有动作
CMD_DRINKING   = bytes([0x01])   # 1：喝水
CMD_ELBOW      = bytes([0x02])   # 2：屈臂
CMD_FORWARD    = bytes([0x03])   # 3：前进
CMD_BACKWARD   = bytes([0x04])   # 4：后退


# =========================================================
# 自动找串口（COM7 优先）
# =========================================================

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


# =========================================================
# EEG Device Setup
# =========================================================

eeg_device = NeuracleEEG(
    host='127.0.0.1',
    port=8712,
    device='Neuracle',
    channels=9,      # 8 EEG + 1 TRG
    fs=1000,
    buffer_seconds=5
)

eeg_device.start()


# =========================================================
# 打开串口
# =========================================================

port = find_port()
if not port:
    print("[ERROR] Cannot find serial port. Connect device and try again.")
    sys.exit(1)
print(f"[SERIAL] Detected port on {port}")

ser = serial.Serial(port, baudrate=115200, timeout=1)
print(f"[SERIAL] Opened {port} @ 115200 bps")


# =========================================================
# SSVEP Classifier
# =========================================================

classifier = FBCCAClassifier(
    freqs=[8, 10, 12, 15],
    fs=1000,
    window_size=1.5,
    n_harmonics=3,
    low=6,
    high=40,
    notch=50
)


# =========================================================
# 连续计数确认机制
# =========================================================

CONSECUTIVE_THRESHOLD = 6    # 连续6次同一类别才发送
consecutive_count = 0         # 当前连续计数
consecutive_freq = None       # 当前连续的频率类别


# =========================================================
# Channel Selection
#
# SSVEP-8:
# 0~7 = EEG
# 8   = TRG
# =========================================================

occ_channels = [
    0, 1, 2, 3,
    4, 5, 6, 7
]


# =========================================================
# 频率到串口命令的映射
# =========================================================

freq_to_cmd = {
    8:  CMD_FORWARD,    # 8Hz  -> 前进 (命令3)
    10: CMD_ELBOW,      # 10Hz -> 屈臂 (命令2)
    12: CMD_DRINKING,   # 12Hz -> 喝水 (命令1)
    15: CMD_BACKWARD,   # 15Hz -> 后退 (命令4)
}

freq_to_name = {
    8:  "前进 (8Hz)",
    10: "屈臂 (10Hz)",
    12: "喝水 (12Hz)",
    15: "后退 (15Hz)",
}


# =========================================================
# EEG Quality Check
# =========================================================

def eeg_quality_check(eeg):

    if eeg is None:
        return False

    if eeg.size == 0:
        return False

    if not np.isfinite(eeg).all():
        return False

    std = np.std(eeg, axis=1)

    # 防止平线
    if np.any(std < 0.05):
        return False

    # 防止数值爆炸
    if np.any(std > 5000):
        return False

    return True


# =========================================================
# 键盘监听
# =========================================================

pressed_keys = set()

def on_press(key):
    global pressed_keys
    
    key_char = getattr(key, 'char', None)
    key_id = id(key)

    # 过滤重复按压
    if key_id in pressed_keys:
        return
    pressed_keys.add(key_id)

    # 1 = 喝水
    if key_char == '1':
        ser.write(CMD_DRINKING)
        print("[KEYBOARD] Sent: 喝水 (0x01)")

    # 2 = 屈臂
    elif key_char == '2':
        ser.write(CMD_ELBOW)
        print("[KEYBOARD] Sent: 屈臂 (0x02)")

    # 3 = 前进
    elif key_char == '3':
        ser.write(CMD_FORWARD)
        print("[KEYBOARD] Sent: 前进 (0x03)")

    # 4 = 后退
    elif key_char == '4':
        ser.write(CMD_BACKWARD)
        print("[KEYBOARD] Sent: 后退 (0x04)")

    # 0 = 停止
    elif key_char == '0':
        ser.write(CMD_STOP)
        print("[KEYBOARD] Sent: 停止 (0x00)")

    # q = 退出
    elif key_char == 'q':
        print("\nExiting...")
        ser.close()
        eeg_device.stop()
        sys.exit(0)


def on_release(key):
    global pressed_keys
    key_id = id(key)
    pressed_keys.discard(key_id)


# 启动键盘监听
listener = Listener(on_press=on_press, on_release=on_release)
listener.start()
print("\n[KEYBOARD] Listening: 1/2/3/4/0 to send commands, Q to quit.")


# =========================================================
# Main Online Loop
# =========================================================

print("\n========================================")
print("Starting Online SSVEP Classification")
print("========================================")

print("Using channels:", occ_channels)
print("SSVEP freq to command mapping:")
for freq, cmd in freq_to_cmd.items():
    print(f"  {freq}Hz -> {freq_to_name[freq]} ({cmd.hex()})")
print(f"Consecutive threshold: {CONSECUTIVE_THRESHOLD} times")

try:

    while True:

        # 每500ms更新一次
        n_update = eeg_device.get_update_count()

        if n_update >= 500:

            # 获取最近1.5秒数据
            data = eeg_device.get_data(
                window=1.5
            )

            eeg_device.reset_update_count()

            # 只保留EEG通道
            eeg = data[
                occ_channels,
                :
            ]

            # EEG质量检查
            ok = eeg_quality_check(eeg)
            if not ok:
                print("EEG quality check failed")
                # 质量不合格，计数归零
                consecutive_count = 0
                consecutive_freq = None
                continue

            # FBCCA预测
            result, score, scores = classifier.predict(
                eeg
            )

            if result is not None:

                score_text = " | ".join(
                    [
                        f"{f}Hz:{scores[f]:.3f}"
                        for f in scores
                    ]
                )

                print(
                    f"RAW -> "
                    f"{result}Hz "
                    f"| Score={score:.3f} "
                    f"| {score_text}"
                )

                # ---- 连续计数确认逻辑 ----
                if result == consecutive_freq:
                    # 同一类别，计数+1
                    consecutive_count += 1
                else:
                    # 类别变了，重新开始计数
                    consecutive_count = 1
                    consecutive_freq = result

                name = freq_to_name.get(result, f"{result}Hz")
                print(
                    f">>> 累计: {name} "
                    f"| 连续: {consecutive_count}/{CONSECUTIVE_THRESHOLD}"
                )

                # 达到阈值，发送串口命令
                if consecutive_count >= CONSECUTIVE_THRESHOLD:
                    
                    cmd = freq_to_cmd.get(consecutive_freq)
                    
                    if cmd:
                        ser.write(cmd)
                        print(
                            f">>> ✅ 执行: "
                            f"{name} "
                            f"| Sent: {cmd.hex()} "
                            f"| 连续确认: {consecutive_count}次"
                        )
                    
                    # 发送后重置计数
                    consecutive_count = 0
                    consecutive_freq = None

        time.sleep(0.02)

except KeyboardInterrupt:

    ser.close()
    eeg_device.stop()

    print("\nLoop terminated.")
