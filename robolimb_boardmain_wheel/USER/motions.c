#include "motions.h"
#include "motion_drinking.h"
#include "motion_elbow.h"
#include "epos_api.h"

// 当前运动中的动作（0=无，1=drinking，2=elbow）
volatile u8 current_motion = 0;

// 动作步进（非阻塞，每10ms在主循环调用）
void motions_step(void) {
    if (current_motion == CMD_DRINKING) {
        int ret = motion_drinking_step();
        if (ret == 0 || ret == -1) {
            current_motion = 0;
        }
    } else if (current_motion == CMD_ELBOW) {
        int ret = motion_elbow_step();
        if (ret == 0 || ret == -1) {
            current_motion = 0;
        }
    }
}

// 立即停止所有动作
void motion_stop_all(void) {
    // 禁用所有节点
    epos_disable(1);
    epos_disable(2);
    epos_disable(3);
    epos_disable(4);

    // 重置各动作状态机
    motion_drinking_stop();
    motion_elbow_stop();

    // 清除当前运动标识
    current_motion = 0;

    // 轮子也急停
    wheel_emergency_stop();

    printf("[STOP] All motions aborted\r\n");
}
