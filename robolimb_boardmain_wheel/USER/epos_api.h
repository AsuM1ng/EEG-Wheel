#ifndef __EPOS_API_H
#define __EPOS_API_H

#include "sys.h"

// ========== 对象字典索引定义 ==========
#define OD_CONTROLWORD      0x6040
#define OD_STATUSWORD       0x6041
#define OD_OP_MODE          0x6060
#define OD_TARGET_POS       0x607A
#define OD_ACTUAL_POS       0x6064
#define OD_PROFILE_VEL      0x6081
#define OD_PROFILE_ACC      0x6083  // Profile Acceleration (uint32, User Units/s²)
#define OD_PROFILE_DEC      0x6084  // Profile Deceleration (uint32, User Units/s²)
#define OD_QUICK_STOP_DEC   0x6085  // Quick Stop Deceleration (uint32, User Units/s²)

// ========== 操作模式 ==========
#define MODE_PPM            1   // 位置模式
#define MODE_PVM            3   // 速度模式
#define MODE_HM             6   // 回零模式

// ========== 控制字命令 ==========
#define CMD_SHUTDOWN        0x06
#define CMD_SWITCH_ON       0x0F
#define CMD_ENABLE_OP       0x0F
#define CMD_NEW_SETPOINT    0x20
#define CMD_START_OP        0x3F  // Enable + New Set-Point
#define CMD_HALT            0x5F  // Enable + Halt

// ========== 状态字各位 ==========
#define SW_READY_SWITCH_ON      (1 << 0)
#define SW_SWITCHED_ON          (1 << 1)
#define SW_OPERATION_ENABLED    (1 << 2)
#define SW_FAULT                (1 << 3)
#define SW_VOLTAGE_ENABLED      (1 << 4)
#define SW_QUICK_STOP           (1 << 5)
#define SW_SWITCH_ON_DISABLED   (1 << 6)
#define SW_WARNING              (1 << 7)
#define SW_TARGET_REACHED       (1 << 10)
#define SW_SETPOINT_ACK         (1 << 12)

// ========== 全局状态字缓存 ==========
extern volatile uint16_t g_statusword[5];  // [0]不用，[1-4]对应节点

// ========== 电机使能（使用默认速度/加速度/减速度） ==========
void epos_enable(u8 node_id);

// ========== 电机使能（指定运动参数）============
void epos_enable_ex(u8 node_id, u32 velocity, u32 acceleration, u32 deceleration);

// ========== 电机控制 ==========
void epos_disable(u8 node_id);
void epos_go(u8 node_id);
void epos_halt(u8 node_id);

// ========== 运动参数（单个设置）============
// 单位：EPOS4 User Units，默认1 RPM, 1 UU/s²
void epos_set_velocity(u8 node_id, u32 velocity);          // 0x6081
void epos_set_acceleration(u8 node_id, u32 acceleration); // 0x6083
void epos_set_deceleration(u8 node_id, u32 deceleration); // 0x6084

// ========== 一次性设置所有运动参数（推荐）============
// 调用后存到对应节点的静态变量，后续 epos_enable() 自动复用
// 0 = 使用默认值
void epos_config_motion(u8 node_id, u32 velocity, u32 acceleration, u32 deceleration);

// ========== 参数设置 ==========
void epos_set_position(u8 node_id, int32_t position);
void epos_set_mode(u8 node_id, u8 mode);

// ========== 状态查询 ==========
void epos_request_statusword(u8 node_id);
u16 epos_get_statusword(u8 node_id);
u8 epos_is_reached(u8 node_id);
u8 epos_is_enabled(u8 node_id);
u8 epos_is_fault(u8 node_id);

// ========== 批量操作 ==========
void epos_enable_all(u8 node_mask);
void epos_disable_all(u8 node_mask);
u8 epos_all_reached(u8 node_mask);

#endif
