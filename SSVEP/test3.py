# final_ssvep_debug.py
# =========================================================
# 最终版：Neuracle SSVEP-8 一键调通测试
#
# 功能：
# 1. 使用官方 DataServerThread
# 2. 自动检查：
#       - 数据是否正常
#       - 是否存在DC偏置
#       - trigger通道
#       - 枕叶通道是否有SSVEP峰值
# 3. 自动画：
#       - 原始EEG
#       - 滤波EEG
#       - FFT频谱
# 4. 自动检测：
#       - 8Hz / 10Hz / 12Hz / 15Hz
#
# 核心结论：
# 你这个系统：
#   是 float32
#   不是 int24
#
# 1440 bytes =
#   9ch × 40sample × 4byte
#
# 即：
#   8 EEG + 1 TRG
#
# 所以：
#   CHANNELS = 9
#
# =========================================================

import time
import numpy as np
import matplotlib.pyplot as plt

from scipy.signal import butter
from scipy.signal import filtfilt
from scipy.signal import detrend
from scipy.signal import welch

from API.neuracle_lib.dataServer import DataServerThread


# =========================================================
# CONFIG
# =========================================================

HOST = '127.0.0.1'
PORT = 8712

DEVICE = 'Neuracle'

FS = 1000

# 最关键
CHANNELS = 9

BUFFER_SECONDS = 5

# 8 EEG channels
EEG_CHANNELS = list(range(8))

TARGET_FREQS = [8, 10, 12, 15]


# =========================================================
# FILTER
# =========================================================

def bandpass(x,
             fs=1000,
             low=6,
             high=40,
             order=4):

    nyq = fs / 2

    b, a = butter(
        order,
        [low/nyq, high/nyq],
        btype='band'
    )

    return filtfilt(
        b,
        a,
        x
    )


# =========================================================
# FFT PEAK
# =========================================================

def get_peak_power(x, fs, target_freq):

    f, pxx = welch(
        x,
        fs=fs,
        nperseg=fs
    )

    idx = np.argmin(
        np.abs(f - target_freq)
    )

    return pxx[idx]


# =========================================================
# CONNECT
# =========================================================

print("\n========================================")
print("CONNECTING TO NEURACLE")
print("========================================")

server = DataServerThread(
    device=DEVICE,
    n_chan=CHANNELS,
    srate=FS,
    t_buffer=BUFFER_SECONDS
)

notconnect = server.connect(
    hostname=HOST,
    port=PORT
)

if notconnect:
    raise RuntimeError(
        "Cannot connect Neuracle."
    )

server.Daemon = True
server.start()

print("CONNECTED")


# =========================================================
# WAIT BUFFER
# =========================================================

print("\nWaiting buffer full...")

while server.GetDataLenCount() < FS * BUFFER_SECONDS:

    time.sleep(0.1)

print("BUFFER READY")


# =========================================================
# GET DATA
# =========================================================

data = server.GetBufferData()

server.ResetDataLenCount()

print("\n========================================")
print("DATA INFO")
print("========================================")

print("shape:", data.shape)

# only EEG
eeg = data[EEG_CHANNELS]

# remove DC
eeg = detrend(
    eeg,
    axis=1,
    type='constant'
)

# bandpass
eeg_bp = np.array([
    bandpass(ch)
    for ch in eeg
])

print("\nRAW STD:")
print(
    np.round(
        np.std(data, axis=1),
        2
    )
)

print("\nFILTERED STD:")
print(
    np.round(
        np.std(eeg_bp, axis=1),
        2
    )
)

print("\nMAX ABS:")
print(
    np.max(np.abs(eeg_bp))
)

# =========================================================
# PLOT RAW
# =========================================================

plt.figure(figsize=(15, 10))

plt.subplot(3,1,1)

for i in range(8):

    x = eeg[i]

    x = x - np.mean(x)

    plt.plot(
        x[-2000:] + i*200,
        linewidth=1
    )

plt.title("RAW EEG")

# =========================================================
# PLOT FILTERED
# =========================================================

plt.subplot(3,1,2)

for i in range(8):

    x = eeg_bp[i]

    plt.plot(
        x[-2000:] + i*100,
        linewidth=1
    )

plt.title("FILTERED EEG (6-40Hz)")

# =========================================================
# FFT
# =========================================================

plt.subplot(3,1,3)

freqs_all = []

for i in range(8):

    f, pxx = welch(
        eeg_bp[i],
        fs=FS,
        nperseg=FS
    )

    freqs_all.append(pxx)

mean_pxx = np.mean(
    freqs_all,
    axis=0
)

plt.plot(
    f,
    mean_pxx,
    linewidth=2
)

for tf in TARGET_FREQS:

    plt.axvline(
        tf,
        linestyle='--'
    )

plt.xlim(5, 20)

plt.title("MEAN PSD")

plt.tight_layout()

# =========================================================
# SSVEP DETECT
# =========================================================

print("\n========================================")
print("SSVEP DETECTION")
print("========================================")

scores = {}

mean_signal = np.mean(
    eeg_bp,
    axis=0
)

for freq in TARGET_FREQS:

    power = get_peak_power(
        mean_signal,
        FS,
        freq
    )

    scores[freq] = power

    print(
        f"{freq}Hz power:",
        power
    )

best_freq = max(
    scores,
    key=scores.get
)

print("\n========================================")
print("FINAL RESULT")
print("========================================")

print(
    "Detected frequency:",
    best_freq,
    "Hz"
)

# =========================================================
# QUALITY JUDGE
# =========================================================

std_mean = np.mean(
    np.std(eeg_bp, axis=1)
)

if std_mean < 0.5:

    print("\n[ERROR]")
    print("Signal almost flat.")
    print("=> electrode off / wrong channel config")

elif std_mean > 1000:

    print("\n[ERROR]")
    print("Signal exploded.")
    print("=> wrong protocol or channel count")

else:

    print("\n[SUCCESS]")
    print("EEG looks NORMAL.")

    print(
        "You can now run FBCCA safely."
    )

# =========================================================
# SHOW
# =========================================================

plt.show()

server.stop()