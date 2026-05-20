#include "epos_api.h"
#include "can_driver.h"
#include "delay.h"
#include <string.h>

// ========== 全局状态字缓存 ==========
volatile uint16_t g_statusword[5] = {0};

// ========== 每节点运动参数（epos_config_motion 设置后复用） ==========
// 0 = 尚未配置，使用默认值
static u32 g_vel[5]  = {0, 0, 0, 0, 0};
static u32 g_acc[5]  = {0, 0, 0, 0, 0};
static u32 g_dec[5]  = {0, 0, 0, 0, 0};

// ========== 默认值（0=未配置时使用）==========
#define DEFAULT_VELOCITY     1000    // RPM
#define DEFAULT_ACCELERATION  50000   // UU/s²
#define DEFAULT_DECELERATION  50000   // UU/s²

// ========== 获取当前生效参数（用户设置值 > 0，否则默认）==========
static u32 cur_vel(u8 node_id) { return g_vel[node_id] ? g_vel[node_id] : DEFAULT_VELOCITY; }
static u32 cur_acc(u8 node_id) { return g_acc[node_id] ? g_acc[node_id] : DEFAULT_ACCELERATION; }
static u32 cur_dec(u8 node_id) { return g_dec[node_id] ? g_dec[node_id] : DEFAULT_DECELERATION; }

// ========== SDO底层函数 ==========

// 发送SDO写请求（32位）
static void sdo_write_u32(u8 node_id, u16 index, u8 subindex, u32 value)
{
    Message tx = {0};
    tx.cob_id = 0x600 + node_id;
    tx.len = 8;
    tx.rtr = 0;
    
    tx.data[0] = 0x23;  // SDO Download Request (4字节)
    tx.data[1] = index & 0xFF;
    tx.data[2] = (index >> 8) & 0xFF;
    tx.data[3] = subindex;
    tx.data[4] = value & 0xFF;
    tx.data[5] = (value >> 8) & 0xFF;
    tx.data[6] = (value >> 16) & 0xFF;
    tx.data[7] = (value >> 24) & 0xFF;
    
    canSend(0, &tx);
    printf("[CAN] SDO WRITE node=%d idx=0x%04X val=%lu\r\n", node_id, index, value);
}

// 发送SDO写请求（16位）
static void sdo_write_u16(u8 node_id, u16 index, u8 subindex, u16 value)
{
    Message tx = {0};
    tx.cob_id = 0x600 + node_id;
    tx.len = 6;
    tx.rtr = 0;
    
    tx.data[0] = 0x2B;  // SDO Download Request (2字节)
    tx.data[1] = index & 0xFF;
    tx.data[2] = (index >> 8) & 0xFF;
    tx.data[3] = subindex;
    tx.data[4] = value & 0xFF;
    tx.data[5] = (value >> 8) & 0xFF;
    
    canSend(0, &tx);
    printf("[CAN] SDO WRITE node=%d idx=0x%04X val=0x%04X\r\n", node_id, index, value);
}

// 发送SDO写请求（8位）
static void sdo_write_u8(u8 node_id, u16 index, u8 subindex, u8 value)
{
    Message tx = {0};
    tx.cob_id = 0x600 + node_id;
    tx.len = 5;
    tx.rtr = 0;
    
    tx.data[0] = 0x2F;  // SDO Download Request (1字节)
    tx.data[1] = index & 0xFF;
    tx.data[2] = (index >> 8) & 0xFF;
    tx.data[3] = subindex;
    tx.data[4] = value;
    
    canSend(0, &tx);
    printf("[CAN] SDO WRITE node=%d idx=0x%04X val=0x%02X\r\n", node_id, index, value);
}

// 发送SDO读请求
static void sdo_read_request(u8 node_id, u16 index, u8 subindex)
{
    Message tx = {0};
    tx.cob_id = 0x600 + node_id;
    tx.len = 4;
    tx.rtr = 0;
    
    tx.data[0] = 0x40;  // SDO Upload Request
    tx.data[1] = index & 0xFF;
    tx.data[2] = (index >> 8) & 0xFF;
    tx.data[3] = subindex;
    
    canSend(0, &tx);
}

// ========== 内部：写入所有运动参数 ==========
static void write_motion_params(u8 node_id, u32 velocity, u32 acceleration, u32 deceleration)
{
    sdo_write_u32(node_id, OD_PROFILE_VEL, 0, velocity);      // 目标速度
    sdo_write_u32(node_id, OD_PROFILE_ACC, 0, acceleration);   // 加速度
    sdo_write_u32(node_id, OD_PROFILE_DEC, 0, deceleration);   // 减速度
}

// ========== 电机使能（使用默认运动参数）============
void epos_enable(u8 node_id)
{
    // 清零状态字缓存，防止旧值（bit10=1）导致 check_reached() 误判
    g_statusword[node_id] = 0;
    
    write_motion_params(node_id, cur_vel(node_id), cur_acc(node_id), cur_dec(node_id));
    
    // 设置位置模式 (0x6060 = 1)
    sdo_write_u8(node_id, OD_OP_MODE, 0, MODE_PPM);
    
    // DS402 使能序列：逐级推进，不能跳步
    // Switch-on-disabled →(0x06)→ Ready-to-switch-on →(0x07)→ Switched-on →(0x0F)→ Operation-enabled
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, 0x06);  // Shutdown
    delay_ms(3);
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, 0x07);  // Switch on
    delay_ms(3);
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, 0x0F);  // Enable operation
    printf("[EPOS] Enable node=%d vel=%lu acc=%lu dec=%lu\r\n",
           node_id, cur_vel(node_id), cur_acc(node_id), cur_dec(node_id));
}

// ========== 电机使能（指定运动参数）============
void epos_enable_ex(u8 node_id, u32 velocity, u32 acceleration, u32 deceleration)
{
    // 用传入值，0 替换为默认值
    u32 vel = velocity    ? velocity    : DEFAULT_VELOCITY;
    u32 acc = acceleration? acceleration: DEFAULT_ACCELERATION;
    u32 dec = deceleration? deceleration: DEFAULT_DECELERATION;
    
    write_motion_params(node_id, vel, acc, dec);
    
    // 设置位置模式
    sdo_write_u8(node_id, OD_OP_MODE, 0, MODE_PPM);
    
    // DS402 使能序列
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, 0x06);
    delay_ms(3);
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, 0x07);
    delay_ms(3);
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, 0x0F);
    printf("[EPOS] EnableEx node=%d vel=%lu acc=%lu dec=%lu\r\n", node_id, vel, acc, dec);
}

// ========== 电机控制 ==========

void epos_disable(u8 node_id)
{
    // 禁止后状态字无意义，清零防止干扰下次启动
    g_statusword[node_id] = 0;
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, CMD_SHUTDOWN);
    printf("[EPOS] Disable node=%d\r\n", node_id);
}

void epos_go(u8 node_id)
{
    // 发新目标后，EPOS4 会把 bit10 清零，但 g_statusword 缓存里还是旧值
    // 先清掉 bit10，等 SDO 响应刷新后再做判断
    g_statusword[node_id] &= ~SW_TARGET_REACHED;
    
    // 先清 bit4 (New Setpoint)，确保产生有效的 0→1 跳变
    // 这会让 EPOS4 识别到新指令，正确清除 bit10 (Target Reached)
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, 0x0F);         // bit4=0, Enable only
    delay_ms(1);                                               // 等待 EPOS 处理
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, CMD_START_OP); // bit4=1, New Setpoint
    printf("[EPOS] GO node=%d\r\n", node_id);
}

void epos_halt(u8 node_id)
{
    sdo_write_u16(node_id, OD_CONTROLWORD, 0, CMD_HALT);
}

// ========== 运动参数（单个设置）============

void epos_set_velocity(u8 node_id, u32 velocity)
{
    sdo_write_u32(node_id, OD_PROFILE_VEL, 0, velocity);
}

void epos_set_acceleration(u8 node_id, u32 acceleration)
{
    sdo_write_u32(node_id, OD_PROFILE_ACC, 0, acceleration);
}

void epos_set_deceleration(u8 node_id, u32 deceleration)
{
    sdo_write_u32(node_id, OD_PROFILE_DEC, 0, deceleration);
}

// ========== 一次性设置所有运动参数 ==========
// 设置后自动存入节点静态变量，后续 epos_enable() 自动复用
// 传入 0 表示使用默认值（DEFAULT_VELOCITY / DEFAULT_ACCELERATION / DEFAULT_DECELERATION）
void epos_config_motion(u8 node_id, u32 velocity, u32 acceleration, u32 deceleration)
{
    g_vel[node_id] = velocity;
    g_acc[node_id] = acceleration;
    g_dec[node_id] = deceleration;
    printf("[EPOS] ConfigMotion node=%d vel=%lu acc=%lu dec=%lu\r\n",
           node_id,
           velocity    ? velocity    : DEFAULT_VELOCITY,
           acceleration? acceleration: DEFAULT_ACCELERATION,
           deceleration? deceleration: DEFAULT_DECELERATION);
}

// ========== 参数设置 ==========

void epos_set_position(u8 node_id, int32_t position)
{
    sdo_write_u32(node_id, OD_TARGET_POS, 0, (u32)position);
}

void epos_set_mode(u8 node_id, u8 mode)
{
    sdo_write_u8(node_id, OD_OP_MODE, 0, mode);
}

// ========== 状态查询 ==========

void epos_request_statusword(u8 node_id)
{
    sdo_read_request(node_id, OD_STATUSWORD, 0);
}

u16 epos_get_statusword(u8 node_id)
{
    if (node_id >= 1 && node_id <= 4) {
        return g_statusword[node_id];
    }
    return 0;
}

u8 epos_is_reached(u8 node_id)
{
    u16 sw = epos_get_statusword(node_id);
    // Bit 10: Target reached（持续状态，稳定可靠）
    // Bit 12: Set-point acknowledged（瞬时脉冲，SDO轮询容易漏采，已弃用）
    return (sw & SW_TARGET_REACHED) ? 1 : 0;
}

u8 epos_is_enabled(u8 node_id)
{
    u16 sw = epos_get_statusword(node_id);
    return (sw & SW_OPERATION_ENABLED) ? 1 : 0;
}

u8 epos_is_fault(u8 node_id)
{
    u16 sw = epos_get_statusword(node_id);
    return (sw & SW_FAULT) ? 1 : 0;
}

// ========== 批量操作 ==========

void epos_enable_all(u8 node_mask)
{
    u8 i;
    for (i = 1; i <= 4; i++) {
        if (node_mask & (1 << (i-1))) {
            epos_enable(i);
        }
    }
}

void epos_disable_all(u8 node_mask)
{
    u8 i;
    for (i = 1; i <= 4; i++) {
        if (node_mask & (1 << (i-1))) {
            epos_disable(i);
        }
    }
}

u8 epos_all_reached(u8 node_mask)
{
    u8 i;
    for (i = 1; i <= 4; i++) {
        if ((node_mask & (1 << (i-1))) && !epos_is_reached(i)) {
            return 0;
        }
    }
    return 1;
}
