# ssvep_stim.py
# -*- coding: utf-8 -*-

from psychopy import visual, core, event
import numpy as np


# =========================
# Window
# =========================

win = visual.Window(
    size=[1080, 720],
    #size=[1920, 1080],
    fullscr=False,
    color=[-1, -1, -1],
    units='pix',
    monitor='testMonitor',
    waitBlanking=True
)

# =========================
# Refresh Rate
# =========================

fps = win.getActualFrameRate()

if fps is None:
    fps = 60

print("Refresh rate:", fps)


# =========================
# Parameters
# =========================

freqs = [8, 10, 12, 15]

labels = [
    "前进",
    "喝水",
    "后退",
    "展臂"
]

positions = [
    (-300, 200),
    (300, 200),
    (-300, -200),
    (300, -200)
]

stim_size = 300


# =========================
# Stimuli
# =========================

stimuli = []
texts = []

for i in range(4):

    rect = visual.Rect(
        win=win,
        width=stim_size,
        height=stim_size,
        pos=positions[i],
        fillColor=[1, 1, 1],
        lineColor=[1, 1, 1],
        colorSpace='rgb'
    )

    text = visual.TextStim(
        win=win,
        text=labels[i],
        pos=positions[i],
        color=[1, -1, -1],  # 红色
        colorSpace='rgb',
        height=60,
        bold=True
    )

    stimuli.append(rect)
    texts.append(text)

# 中央注视十字
fixation = visual.TextStim(
    win=win,
    text='+',
    color=[1, 1, 1],
    colorSpace='rgb',
    height=60
)

# =========================
# Main Loop
# =========================

frameN = 0

while True:

    t = frameN / fps

    for i, f in enumerate(freqs):

        # 方波闪烁
        lum = 1 if np.sin(
            2 * np.pi * f * t
        ) >= 0 else -1

        stimuli[i].fillColor = [
            lum, lum, lum
        ]

        stimuli[i].lineColor = [
            lum, lum, lum
        ]

        stimuli[i].draw()
        texts[i].draw()

    fixation.draw()

    win.flip()

    frameN += 1

    keys = event.getKeys()

    if 'escape' in keys:
        break

win.close()
core.quit()