#include "sys.h"
#include "main.h"
#include "led.h"
#include "motions.h"
#include "epos_api.h"
#include "nmtMaster.h"
#include "rs485.h"
#include "wheel.h"

extern u8 get_serial_cmd(void);
extern volatile u8 emergency_stop_flag;

int main(void) {
    u8 cmd;

    // 系统初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init(168);
    LED_Init();
    CAN1_Init(&Master_Data, 1000000);
    TIM2_Init();
    USART1_Init(115200);
    // RS485初始化由 wheel_init() 处理

    // CANopen初始化
    setNodeId(&Master_Data, 0x01);
    setState(&Master_Data, Initialisation);
    setState(&Master_Data, Operational);

    // 上电自检
    LED_StartupBlink();

    printf("\r\n========== STM32 Ready ==========\r\n");
    printf("Commands: 0x01=Drinking, 0x02=Elbow, 0x03=FWD, 0x04=BWD, 0x00=Stop\r\n");
    printf("==================================\r\n");

    // CANopen NMT复位
    printf("[NMT] Wait 1000ms before reset...\r\n");
    delay_ms(1000);
    masterSendNMTstateChange(&Master_Data, 0x00, NMT_Reset_Comunication);
    printf("[NMT] Reset Communication sent\r\n");
    delay_ms(500);
    setState(&Master_Data, Operational);
    printf("[NMT] Back to Operational\r\n");

    // 轮子初始化
    printf("[WHEEL] RS485 wheel init...\r\n");
    wheel_init();
    printf("[WHEEL] Ready\r\n");

    current_motion = 0;

    while (1) {
        // 轮子状态机
        wheel_step();

        // 急停标志
        if (emergency_stop_flag) {
            emergency_stop_flag = 0;
            LED_AllBlink();
            motion_stop_all();
            wheel_emergency_stop();
            continue;
        }

        // 串口命令
        cmd = get_serial_cmd();
        if (cmd != 0) {
            printf("[CMD] Received: 0x%02X\r\n", cmd);
        switch (cmd) {
                case CMD_STOP:  // 0x00: 中断所有动作
                    current_motion = 0;
                    motion_stop_all();
                    wheel_emergency_stop();
                    printf("[CMD] Emergency stop\r\n");
                    break;
                case CMD_DRINKING:
                    if (current_motion == 0) {
                        LED0_Blink();
                        current_motion = CMD_DRINKING;
                        printf("[MOTION] Start drinking\r\n");
                    }
                    break;
                case CMD_ELBOW:
                    if (current_motion == 0) {
                        LED1_Blink();
                        current_motion = CMD_ELBOW;
                        printf("[MOTION] Start elbow\r\n");
                    }
                    break;
                default:
                    // 交给轮子处理
                    {
                        u8 ret = wheel_cmd_handler(cmd);
                        if (ret == 1) {
                            printf("Unknown cmd: 0x%02X\r\n", cmd);
                        }
                    }
                    break;
            }
        }

        // 机械臂状态机
        if (current_motion != 0) {
            motions_step();
        }

        delay_ms(10);
    }
}
