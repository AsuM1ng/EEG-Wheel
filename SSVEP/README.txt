这是博瑞康脑电在线采集＋分类系统  API很重要不能改
打开neuracle.EEGRecorder，只打Oz O1 O2 POz PO3 PO4 ref gnd成功采到信号后，点击dataserver，选ssvep-8，start，然后关闭此弹窗（很重要）开始输出数据
环境是ssvep 复制到 C/用户/.conda/env 也可以自己配python 3.10
运行main即可
其中采集recevier-EEG.py，分类fbcca.py
分类
8   前进
10 喝水
12 后退
15 展臂
None 静止
连续判别三次结果相同输出指令