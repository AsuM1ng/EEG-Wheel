

import time
import numpy as np

from receiver_EEG import NeuracleEEG
from fbcca import FBCCAClassifier
from fbcca import DecisionSmoother


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
# Decision Smoother
# =========================================================

smoother = DecisionSmoother(
    window_size=5,
    min_count=3
)


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
# Command Mapping
# =========================================================

command_map = {
    8: "前进 (8Hz)",
    10: "喝水 (12Hz)",
    12: "后退 (10Hz)",
    15: "展臂 (15Hz)"
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
# Main Online Loop
# =========================================================

print("\n========================================")
print("Starting Online SSVEP Classification")
print("========================================")

print("Using channels:", occ_channels)

try:

    while True:

        # 每200ms更新一次
        n_update = eeg_device.get_update_count()

        if n_update >= 200:

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
                print("shujubh")
            result, score,score = classifier.predict(eeg)

            # FBCCA预测
            result, score, scores = classifier.predict(
                eeg
            )

            # 打印原始分类分数
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

                # 平滑决策
                decision = smoother.update(
                    result=result,
                    score=score,
                    threshold=0.40
                )

                # 输出最终指令
                if decision is not None:

                    print(
                        f">>> 执行: "
                        f"{command_map[decision]} "
                        f"| Score: {score:.3f}"
                    )
                else:
                    print(">>>静止")

        time.sleep(0.02)

except KeyboardInterrupt:

    eeg_device.stop()

    print("\nLoop terminated.")