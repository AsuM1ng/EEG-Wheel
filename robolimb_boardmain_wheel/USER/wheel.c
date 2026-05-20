#include "wheel.h"
#include "motions.h"   // CMD_FORWARD, CMD_BACKWARD
#include "rs485.h"     // MODBUS_WriteRegister
#include "delay.h"      // delay_ms
#include <stdio.h>

// ========== 静态变量 ==========
static u8  wheel_state = WHEEL_STATE_IDLE;
static u32 wheel_tick_ms = 0;

static u8 wheel_probe_link(void) {
    u16 val = 0;
    if (MODBUS_ReadRegister(1, 0x0080, &val) == 0) return 1;
    if (MODBUS2_ReadRegister(2, 0x0080, &val) == 0) return 1;
    return 0;
}

static u8 wheel_write_speed_both(s16 speed) {
    u8 ret_l = MODBUS_WriteRegister(1, 0x0040, (u16)speed);
    u8 ret_r = MODBUS2_WriteRegister(2, 0x0040, (u16)speed);
    if (ret_l != 0 || ret_r != 0) {
        printf("[WHEEL] Speed write failed: L=%d R=%d speed=%d\r\n", ret_l, ret_r, speed);
        return 1;
    }
    return 0;
}

static void wheel_dump_driver_status(void) {
    u16 val = 0;
    if (MODBUS_ReadRegister(1, 0x0080, &val) == 0) {
        printf("[WHEEL] L reg0x0080=%u\r\n", val);
    }
    if (MODBUS2_ReadRegister(2, 0x0080, &val) == 0) {
        printf("[WHEEL] R reg0x0080=%u\r\n", val);
    }

    if (MODBUS_ReadRegister(1, 0x0044, &val) == 0) {
        printf("[WHEEL] L reg0x0044=%u\r\n", val);
    }
    if (MODBUS2_ReadRegister(2, 0x0044, &val) == 0) {
        printf("[WHEEL] R reg0x0044=%u\r\n", val);
    }
}

// ========== 内部函数 ==========
static void wheel_start(u8 dir) {
    s16 speed = (dir == 1) ? WHEEL_SPEED : -WHEEL_SPEED;
    if (wheel_write_speed_both(speed) != 0) {
        wheel_state = WHEEL_STATE_IDLE;
        current_motion = 0;
        return;
    }
    wheel_state = WHEEL_STATE_RUN;
    printf("[WHEEL] Start: dir=%s speed=%d\r\n",
           (dir == 1) ? "FWD" : "BWD", speed);
}

static void wheel_stop(void) {
    wheel_write_speed_both(0);
    printf("[WHEEL] Stop\r\n");
}

// ========== 公共函数 ==========
void wheel_init(void) {
    u8 link_ok = 0;

    RS485_Init(9600, 1);    // USART3: 左轮 (PB10/PB11), 8E1
    RS4852_Init(9600, 1);   // USART2: 右轮 (PA2/PA3), 8E1
    link_ok = wheel_probe_link();
    if (!link_ok) {
        printf("[WHEEL] Probe failed with 8E1, retry 8N1...\r\n");
        RS485_Init(9600, 0);
        RS4852_Init(9600, 0);
        link_ok = wheel_probe_link();
    }
    printf("[WHEEL] RS485 probe: %s\r\n", link_ok ? "OK" : "FAILED");

    // 设RS485通讯控制模式=占空比调速 (0x0080=0)
    MODBUS_WriteRegister(1, 0x0080, 0);   // 左轮
    MODBUS2_WriteRegister(2, 0x0080, 0);  // 右轮
    delay_ms(50);

    // 释放电机 (0x0044=1)
    MODBUS_WriteRegister(1, 0x0044, 1);   // 左轮
    MODBUS2_WriteRegister(2, 0x0044, 1);  // 右轮
    delay_ms(50);

    wheel_dump_driver_status();

    printf("[WHEEL] Init done: mode=duty_cycle, released\r\n");
}

void wheel_step(void) {
    if (wheel_state == WHEEL_STATE_IDLE)
        return;

    wheel_tick_ms += 10;   // 主循环每 10ms 调用一次
    if (wheel_tick_ms >= WHEEL_RUN_MS) {
        wheel_stop();
        wheel_state = WHEEL_STATE_IDLE;
        current_motion = 0;  // 清除全局状态，解除阻塞
        wheel_tick_ms = 0;
        printf("[WHEEL] Auto stop done\r\n");
    }
}

void wheel_emergency_stop(void) {
    wheel_stop();
    wheel_state = WHEEL_STATE_IDLE;
    current_motion = 0;  // 清除全局状态
    wheel_tick_ms = 0;
}

// ========== 命令处理（在 main.c switch 中调用）==========
// 返回: 0=已处理, 1=轮子忙/不是轮子命令
u8 wheel_cmd_handler(u8 cmd) {
    // 检查全局动作状态，任何动作进行中都阻塞
    if (current_motion != 0) {
        printf("[WHEEL] Blocked: current_motion=0x%02X\r\n", current_motion);
        return 1;  // 有动作正在执行，阻塞
    }
    
    switch (cmd) {
        case CMD_FORWARD:
            current_motion = CMD_FORWARD;  // 设置全局状态，阻塞其他动作
            wheel_start(1);
            return 0;
        case CMD_BACKWARD:
            current_motion = CMD_BACKWARD; // 设置全局状态，阻塞其他动作
            wheel_start(2);
            return 0;
    }
    return 1;   // 不是轮子命令
}
