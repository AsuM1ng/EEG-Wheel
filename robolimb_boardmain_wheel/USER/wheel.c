#include "wheel.h"
#include "motions.h"   // CMD_FORWARD, CMD_BACKWARD
#include "rs485.h"     // MODBUS_WriteRegister
#include "delay.h"      // delay_ms
#include <stdio.h>

// ========== 静态变量 ==========
static u8  wheel_state = WHEEL_STATE_IDLE;
static u32 wheel_tick_ms = 0;
static u8  wheel_link_ok = 0;
static u8  wheel_left_id = 1;
static u8  wheel_right_id = 2;
static u32 wheel_baud = 9600;
static u8  wheel_even_parity = 1;

static u8 wheel_probe_left(u8 slave_id) {
    u16 val = 0;
    return (MODBUS_ReadRegister(slave_id, 0x0080, &val) == 0) ? 1 : 0;
}

static u8 wheel_probe_right(u8 slave_id) {
    u16 val = 0;
    return (MODBUS2_ReadRegister(slave_id, 0x0080, &val) == 0) ? 1 : 0;
}

static u8 wheel_probe_bus_ids(void)
{
    u8 left_ok = 0;
    u8 right_ok = 0;

    if (wheel_probe_left(1)) { wheel_left_id = 1; left_ok = 1; }
    else if (wheel_probe_left(2)) { wheel_left_id = 2; left_ok = 1; }

    if (wheel_probe_right(2)) { wheel_right_id = 2; right_ok = 1; }
    else if (wheel_probe_right(1)) { wheel_right_id = 1; right_ok = 1; }

    printf("[WHEEL] Probe result: left=%s right=%s\r\n", left_ok ? "OK" : "FAIL", right_ok ? "OK" : "FAIL");
    return (left_ok && right_ok) ? 1 : 0;
}

static u8 wheel_write_speed_both(s16 speed) {
    u8 ret_l;
    u8 ret_r;

    if (!wheel_link_ok) {
        printf("[WHEEL] Link not ready, ignore speed=%d\r\n", speed);
        return 1;
    }

    ret_l = MODBUS_WriteRegister(wheel_left_id, 0x0040, (u16)speed);
    ret_r = MODBUS2_WriteRegister(wheel_right_id, 0x0040, (u16)speed);
    if (ret_l != 0 || ret_r != 0) {
        printf("[WHEEL] Speed write failed: L(id=%u)=%d R(id=%u)=%d speed=%d\r\n",
               wheel_left_id, ret_l, wheel_right_id, ret_r, speed);
        return 1;
    }
    return 0;
}

static void wheel_dump_driver_status(void) {
    u16 val = 0;
    if (MODBUS_ReadRegister(wheel_left_id, 0x0080, &val) == 0) {
        printf("[WHEEL] L(id=%u) reg0x0080=%u\r\n", wheel_left_id, val);
    }
    if (MODBUS2_ReadRegister(wheel_right_id, 0x0080, &val) == 0) {
        printf("[WHEEL] R(id=%u) reg0x0080=%u\r\n", wheel_right_id, val);
    }

    if (MODBUS_ReadRegister(wheel_left_id, 0x0044, &val) == 0) {
        printf("[WHEEL] L(id=%u) reg0x0044=%u\r\n", wheel_left_id, val);
    }
    if (MODBUS2_ReadRegister(wheel_right_id, 0x0044, &val) == 0) {
        printf("[WHEEL] R(id=%u) reg0x0044=%u\r\n", wheel_right_id, val);
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
    wheel_tick_ms = 0;
    wheel_state = WHEEL_STATE_RUN;
    printf("[WHEEL] Start: dir=%s speed=%d\r\n",
           (dir == 1) ? "FWD" : "BWD", speed);
}

static void wheel_stop(void) {
    wheel_write_speed_both(0);
    printf("[WHEEL] Stop\r\n");
}


static u8 wheel_try_link(u32 baud, u8 even_parity)
{
    u8 ok;
    RS485_Init(baud, even_parity);
    RS4852_Init(baud, even_parity);
    delay_ms(20);
    ok = wheel_probe_bus_ids();
    if (ok) {
        wheel_baud = baud;
        wheel_even_parity = even_parity;
    }
    return ok;
}

static void wheel_print_comm_checklist(void)
{
    printf("[WHEEL][CHECK] Driver LED not 2Hz => no RS485 session established\r\n");
    printf("[WHEEL][CHECK] 1) SW8 must be ON (485/CAN control mode)\r\n");
    printf("[WHEEL][CHECK] 2) If 0x0120 changed, power-cycle driver to apply\r\n");
    printf("[WHEEL][CHECK] 3) Try default comm mode (DIP 1~8 all ON)\r\n");
    printf("[WHEEL][CHECK] 4) Verify A/B polarity (also test A/B swapped)\r\n");
    printf("[WHEEL][CHECK] 5) Keep COM<->GND common reference\r\n");
    printf("[WHEEL][CHECK] 6) If possible, verify same line with PC 485 tool\r\n");
}

// ========== 公共函数 ==========
void wheel_init(void) {
    u8 link_ok = 0;

    wheel_link_ok = 0;
    printf("[WHEEL] Try link: 9600 8E1...\r\n");
    link_ok = wheel_try_link(9600, 1);
    if (!link_ok) {
        printf("[WHEEL] Try link: 9600 8N2...\r\n");
        link_ok = wheel_try_link(9600, 0);
    }
    if (!link_ok) {
        printf("[WHEEL] Try link: 19200 8E1...\r\n");
        link_ok = wheel_try_link(19200, 1);
    }
    if (!link_ok) {
        printf("[WHEEL] Try link: 19200 8N2...\r\n");
        link_ok = wheel_try_link(19200, 0);
    }
    if (!link_ok) {
        printf("[WHEEL] Try link: 115200 8E1...\r\n");
        link_ok = wheel_try_link(115200, 1);
    }
    if (!link_ok) {
        printf("[WHEEL] Try link: 115200 8N2...\r\n");
        link_ok = wheel_try_link(115200, 0);
    }
    printf("[WHEEL] RS485 dual-bus probe: %s\r\n", link_ok ? "OK" : "FAILED");
    printf("[WHEEL] Probe order: 9600/19200/115200 x 8E1/8N2, slave 1/2\r\n");
    if (!link_ok) {
        printf("[WHEEL] Init aborted: no driver response on both ports\r\n");
        wheel_print_comm_checklist();
        wheel_state = WHEEL_STATE_IDLE;
        return;
    }

    wheel_link_ok = 1;
    printf("[WHEEL] Link cfg: baud=%lu, %s\r\n", wheel_baud, wheel_even_parity ? "8E1" : "8N2");
    printf("[WHEEL] Detected slave IDs: left=%u right=%u\r\n", wheel_left_id, wheel_right_id);

    // 设RS485通讯控制模式=占空比调速 (0x0080=0)
    MODBUS_WriteRegister(wheel_left_id, 0x0080, 0);   // 左轮
    MODBUS2_WriteRegister(wheel_right_id, 0x0080, 0);  // 右轮
    delay_ms(50);

    // 释放电机 (0x0044=1)
    MODBUS_WriteRegister(wheel_left_id, 0x0044, 1);   // 左轮
    MODBUS2_WriteRegister(wheel_right_id, 0x0044, 1);  // 右轮
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
// 返回: 0=已处理, 1=不是轮子命令, 2=轮子暂不可执行
u8 wheel_cmd_handler(u8 cmd) {
    if (!wheel_link_ok) {
        printf("[WHEEL] Command blocked: driver link not ready\r\n");
        return 2;
    }

    // 检查全局动作状态，任何动作进行中都阻塞
    if (current_motion != 0) {
        printf("[WHEEL] Blocked: current_motion=0x%02X\r\n", current_motion);
        return 2;  // 有动作正在执行，阻塞
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
