#include "motion_drinking.h"
#include "epos_api.h"
#include "timer2.h"

// 动作状态
static struct {
    u8 state;           // 0=未启动, 1=伸手, 2=抬臂, 3=放臂, 4=缩手, 5=完成
    u32 step_start;     // 步骤开始时间
    u8 running;         // 1=正在执行
    u8 wait_count;      // 状态字查询计数
} ctx = {0};

// 各步骤参与节点掩码
#define NODES_STEP1 0x0E  // 节点2,3,4
#define NODES_STEP2 0x0B  // 节点1,2,4
#define NODES_STEP3 0x0B  // 节点1,2,4
#define NODES_STEP4 0x0E  // 节点2,3,4

// 急停函数（供 motions.c 调用，立即禁用所有节点）
void motion_drinking_stop(void) {
    epos_disable(1);
    epos_disable(2);
    epos_disable(3);
    epos_disable(4);
    ctx.running = 0;
    ctx.state = 0;
    printf("[DRINKING] Stopped\r\n");
}

// 动作结束时统一禁止所有电机，确保下次启动干净
static void motion_drinking_end(void) {
    // 所有关节归零准备位（可选，让机器人停在确定位置）
    // 然后禁止电机
    epos_disable(1);
    epos_disable(2);
    epos_disable(3);
    epos_disable(4);
    ctx.running = 0;
    ctx.state = 0;
    printf("[DRINKING] COMPLETE - all motors disabled\r\n");
}

// 周期性请求状态字（非阻塞）
static void poll_statusword(u8 node_mask)
{
    u8 i;
    ctx.wait_count++;
    if (ctx.wait_count >= 5) {  // 每50ms请求一次
        ctx.wait_count = 0;
        for (i = 1; i <= 4; i++) {
            if (node_mask & (1 << (i-1))) {
                epos_request_statusword(i);
            }
        }
    }
}

// 非阻塞等待到位检查
// 策略：时间为主（超时安全兜底）+ bit10 确认（到位时加速判断）
// 如果 bit10 正常：电机到位即提前跳出
// 如果 bit10 粘滞：超时后强制跳出
static u8 check_reached(u8 node_mask, u32 timeout_ms)
{
    u32 elapsed = get_tim2_tick() - ctx.step_start;
    
    // 优先检查 bit10：到位即提前跳出
    if (epos_all_reached(node_mask)) {
        return 1;
    }
    
    // 超时兜底：防止 bit10 粘滞导致永远等待
    if (elapsed > timeout_ms) {
        printf("[DRINKING] Timeout after %lums (bit10 may be sticky)\r\n", elapsed);
        return 2;
    }
    
    return 0;
}

// 非阻塞步进执行（每10ms调用一次）
int motion_drinking_step(void) {
    u32 now = get_tim2_tick();
    u8 ret;
    
    // 首次启动
    if (ctx.state == 0) {
        ctx.state = 1;
        ctx.running = 1;
        ctx.step_start = now;
        ctx.wait_count = 0;
        
        // ===== 第一步：伸手 =====
        // 每个关节单独配置速度/加速度/减速度（只需配置一次，存到节点静态变量）
        // 单位：EPOS4 User Units（默认1 RPM, 1 UU/s²）
        // 关节1-3: 肩/肘，惯量较大，用较小速度
        epos_config_motion(1, 100, 10000, 10000);
        epos_config_motion(2, 100, 10000, 10000);
        epos_config_motion(3, 10000, 10000, 10000);
        epos_config_motion(4, 200, 10000, 10000);  // 肘关节，可稍快
			
        epos_enable(2); epos_enable(3); epos_enable(4);
        epos_set_position(2, -100);
        epos_set_position(3, -200);
        epos_set_position(4, 60000);
        delay_ms(100);  // 等待上一个指令被EPOS处理完成
        epos_go(2); epos_go(3); epos_go(4);
        delay_ms(5000);
        printf("[DRINKING] Step1: Reach out (nodes 2,3,4)\r\n");
        return 1;
    }
    
    // 步骤1：等待到位
    if (ctx.state == 1) {
        poll_statusword(NODES_STEP1);
        ret = check_reached(NODES_STEP1, 10000);
        if (ret == 1) {
            printf("[DRINKING] Step1 reached\r\n");
            ctx.state = 2;
            ctx.step_start = now;
            ctx.wait_count = 0;
            
            // ===== 第二步：抬臂 =====
            epos_enable(1); epos_enable(2); epos_enable(4);
            epos_set_position(1, -100);
            epos_set_position(2, -200);
						epos_set_position(3, -400);
            epos_set_position(4, -40000);
            delay_ms(100);
            epos_go(1); epos_go(2); epos_go(3);epos_go(4);
						delay_ms(10000);
            
            printf("[DRINKING] Step2: Lift arm (nodes 1,2,4)\r\n");
        } else if (ret == 2) {
            ctx.state = 2;
            ctx.step_start = now;
        }
        return 1;
    }
    
    // 步骤2：等待到位
    if (ctx.state == 2) {
        poll_statusword(NODES_STEP2);
        ret = check_reached(NODES_STEP2, 10000);
        if (ret == 1) {
            printf("[DRINKING] Step2 reached\r\n");
            ctx.state = 3;
            ctx.step_start = now;
            ctx.wait_count = 0;
            
            // ===== 第三步：放臂 =====
            epos_enable(1); epos_enable(2); epos_enable(4);
            epos_set_position(1, 0);  // 归零（elbow动作用完node1需重置）
            epos_set_position(2, -100);
						epos_set_position(3, -200);
            epos_set_position(4, 40000);
            delay_ms(100);
            epos_go(1); epos_go(2); epos_go(3);epos_go(4);
            delay_ms(5000);
            printf("[DRINKING] Step3: Lower arm (nodes 1,2,4)\r\n");
        } else if (ret == 2) {
            ctx.state = 3;
            ctx.step_start = now;
        }
        return 1;
    }
    
    // 步骤3：等待到位
    if (ctx.state == 3) {
        poll_statusword(NODES_STEP3);
        ret = check_reached(NODES_STEP3, 10000);
        if (ret == 1) {
            printf("[DRINKING] Step3 reached\r\n");
            ctx.state = 4;
            ctx.step_start = now;
            ctx.wait_count = 0;
            
            // ===== 第四步：缩手 =====
            epos_enable(2); epos_enable(3); epos_enable(4);
            epos_set_position(2, 0);
            epos_set_position(3, 0);
            epos_set_position(4, 0);
            delay_ms(100);
            epos_go(2); epos_go(3); epos_go(4);
            delay_ms(5000);
            printf("[DRINKING] Step4: Retract (nodes 2,3,4)\r\n");
        } else if (ret == 2) {
            ctx.state = 4;
            ctx.step_start = now;
        }
        return 1;
    }
    
    // 步骤4：等待到位
    if (ctx.state == 4) {
        poll_statusword(NODES_STEP4);
        ret = check_reached(NODES_STEP4, 10000);
        if (ret >= 1) {
            motion_drinking_end();  // 禁止所有电机，下次启动干净
            return 0;  // 完成
        }
        return 1;
    }
    
    return 1;
}
