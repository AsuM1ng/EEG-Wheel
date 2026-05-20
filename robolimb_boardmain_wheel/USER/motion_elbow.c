#include "motion_elbow.h"
#include "epos_api.h"
#include "timer2.h"

// 动作状态
static struct {
    u8 state;           // 0=未启动, 1=弯曲, 2=伸展, 3=完成
    u32 step_start;     // 步骤开始时间
    u8 running;         // 1=正在执行
    u8 wait_count;      // 状态字查询计数
} ctx = {0};

// 急停函数
void motion_elbow_stop(void) {
    epos_disable(4);
    ctx.running = 0;
    ctx.state = 0;
    printf("[ELBOW] Stopped\r\n");
}

// 动作结束时禁止电机，确保下次启动干净
static void motion_elbow_end(void) {
    epos_disable(4);
    ctx.running = 0;
    ctx.state = 0;
    printf("[ELBOW] COMPLETE - motor disabled\r\n");
}

// 周期性请求状态字
static void poll_statusword(void)
{
    ctx.wait_count++;
    if (ctx.wait_count >= 5) {
        ctx.wait_count = 0;
        epos_request_statusword(4);
    }
}

// 等待到位检查（时间优先 + bit10 确认）
static u8 check_reached(u32 timeout_ms)
{
    u32 elapsed = get_tim2_tick() - ctx.step_start;
    
    // 优先检查 bit10：到位即提前跳出
    if (epos_is_reached(4)) {
        return 1;
    }
    
    // 超时兜底
    if (elapsed > timeout_ms) {
        printf("[ELBOW] Timeout after %lums (bit10 may be sticky)\r\n", elapsed);
        return 2;
    }
    
    return 0;
}

// 非阻塞步进执行
int motion_elbow_step(void) {
    u32 now = get_tim2_tick();
    u8 ret;
    
    if (ctx.state == 0) {
        ctx.state = 1;
        ctx.running = 1;
        ctx.step_start = now;
        ctx.wait_count = 0;
        
        // 配置关节4（肘）的运动参数：速度/加速度/减速度
        epos_config_motion(4, 1000, 10000, 10000);
        // 第一步：弯曲
        epos_enable(4);
        epos_set_position(4, -40000);
        delay_ms(100);  // 等待EPOS处理完成
        epos_go(4);
        
        printf("[ELBOW] Step1: Bend (node 4)\r\n");
        return 1;
    }
    
    if (ctx.state == 1) {
        poll_statusword();
        ret = check_reached(10000);
        if (ret == 1) {
            printf("[ELBOW] Step1 reached\r\n");
            ctx.state = 2;
            ctx.step_start = now;
            ctx.wait_count = 0;
            
            // 第二步：伸展回零位
            epos_enable(4);
            epos_set_position(4, 0);  // 回到初始零位
            delay_ms(100);
            epos_go(4);
            
            printf("[ELBOW] Step2: Extend to zero (node 4)\r\n");
        } else if (ret == 2) {
            ctx.state = 2;
            ctx.step_start = now;
        }
        return 1;
    }
    
    if (ctx.state == 2) {
        poll_statusword();
        ret = check_reached(10000);
        if (ret >= 1) {
            motion_elbow_end();  // 禁止电机，下次启动干净
            return 0;
        }
        return 1;
    }
    
    return 1;
}
