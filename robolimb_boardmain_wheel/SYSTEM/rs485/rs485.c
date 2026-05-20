#include "rs485.h"
#include "delay.h"
#include <stdio.h>

// ========== 硬件定义 ==========
// USART3: TX=PB10, RX=PB11
#define RS485_USART        USART3
#define RS485_USART_CLK    RCC_APB1Periph_USART3
#define RS485_USART_APB    RCC_APB1PeriphClockCmd

// (自动方向模块, 无需方向控制引脚)

// 收发缓冲
#define RS485_RX_BUF_SIZE  32
static u8 rs485_rx_buf[RS485_RX_BUF_SIZE];
static volatile u8 rs485_rx_len = 0;

// ========== CRC16 (MODBUS) ==========
u16 CRC16(u8 *data, u16 len)
{
    u16 crc = 0xFFFF;
    u8 i, j;
    for (j = 0; j < len; j++) {
        crc ^= data[j];
        for (i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// ========== USART3 初始化 (9600, 8E1) ==========
void RS485_Init(u32 baudrate)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    // 1. GPIO 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    // 2. USART3 时钟
    RS485_USART_APB(RS485_USART_CLK, ENABLE);

    // 3. GPIO 配置: PB10=TX, PB11=RX (复用功能)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

    // TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // (PA8 方向控制已移除 -- 自动切换模块)

    // 5. USART3 配置: 9600, 8E1
    USART_InitStructure.USART_BaudRate            = baudrate;
    USART_InitStructure.USART_WordLength        = USART_WordLength_9b;  // 9位=8数据+1校验
    USART_InitStructure.USART_StopBits          = USART_StopBits_1;
    USART_InitStructure.USART_Parity            = USART_Parity_Even;    // 偶校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(RS485_USART, &USART_InitStructure);

    USART_Cmd(RS485_USART, ENABLE);

    // 6. USART3 RX 中断
    USART_ITConfig(RS485_USART, USART_IT_RXNE, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel                   = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    printf("[RS485] Init done, baud=%lu, 8E1, auto-dir\r\n", baudrate);
}

// ========== USART3 中断: 接收 ==========
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        u8 res = USART_ReceiveData(USART3);
        if (rs485_rx_len < RS485_RX_BUF_SIZE) {
            rs485_rx_buf[rs485_rx_len++] = res;
        }
    }
}

// ========== 发送n字节 ==========
void RS485_SendBytes(u8 *data, u16 len)
{
    u16 i;
    for (i = 0; i < len; i++) {
        while (USART_GetFlagStatus(RS485_USART, USART_FLAG_TC) == RESET);
        USART_SendData(RS485_USART, data[i]);
    }
    while (USART_GetFlagStatus(RS485_USART, USART_FLAG_TC) == RESET);
}

// ========== 清接收缓冲 ==========
static void RS485_FlushRx(void)
{
    rs485_rx_len = 0;
}

// ========== 等待响应(Polling) ==========
// 返回: 读到字节数
static u8 RS485_WaitResponse(u8 *buf, u8 expected_len, u32 timeout_ms)
{
    u32 start = 0;
    // 简单延时等待: 每字节约1.04ms @ 9600
    delay_us(200);  // 等待响应开始

    for (start = 0; start < timeout_ms * 1000; start += 500) {
        if (rs485_rx_len >= expected_len) {
            u8 i;
            for (i = 0; i < rs485_rx_len && i < expected_len; i++) {
                buf[i] = rs485_rx_buf[i];
            }
            RS485_FlushRx();
            return rs485_rx_len;  // 返回实际收到的字节数
        }
        delay_us(500);
    }
    RS485_FlushRx();
    return 0;  // 超时
}

// ========== 写单个寄存器 (功能码0x06) ==========
// 从站地址=1或2, 寄存器地址=0x0040(速度), 数据=speed(-990~990)
// 返回: 0=成功, 1=超时, 2=异常响应
u8 MODBUS_WriteRegister(u8 slave_addr, u16 reg_addr, u16 value)
{
    u8 tx_frame[8];
    u8 rx_buf[8];
    u16 crc;

    // 组帧
    tx_frame[0] = slave_addr;        // 从站地址
    tx_frame[1] = 0x06;              // 功能码: 写单个寄存器
    tx_frame[2] = (u8)(reg_addr >> 8);  // 寄存器地址高字节
    tx_frame[3] = (u8)(reg_addr & 0xFF); // 寄存器地址低字节
    tx_frame[4] = (u8)(value >> 8);   // 数据高字节
    tx_frame[5] = (u8)(value & 0xFF);  // 数据低字节

    // CRC16
    crc = CRC16(tx_frame, 6);
    tx_frame[6] = (u8)(crc & 0xFF);       // CRC低字节
    tx_frame[7] = (u8)((crc >> 8) & 0xFF); // CRC高字节

    RS485_FlushRx();
    RS485_SendBytes(tx_frame, 8);

    // 等待从站回送 (应返回8字节, 与发送相同)
    u8 rx_len = RS485_WaitResponse(rx_buf, 8, 200);  // 200ms超时

    if (rx_len == 0) {
        printf("[RS485] Write timeout! slave=%d reg=0x%04X val=%d\r\n",
               slave_addr, reg_addr, (s16)value);
        return 1;  // 超时
    }

    // 检查: 从站回送应与发送帧完全一致(功能码0x06正常响应)
    if (rx_buf[0] != slave_addr || rx_buf[1] != 0x06) {
        // 异常响应 (功能码0x86)
        if (rx_buf[1] == 0x86) {
            printf("[RS485] Exception! slave=%d error_code=0x%02X\r\n",
                   slave_addr, rx_buf[2]);
        }
        printf("[RS485] Write response error! slave=%d\r\n", slave_addr);
        return 2;
    }

    printf("[RS485] Write OK: slave=%d reg=0x%04X val=%d\r\n",
           slave_addr, reg_addr, (s16)value);
    return 0;
}

// ========== 读单个寄存器 (功能码0x03) ==========
// 返回: 0=成功, 1=超时, 2=异常响应
u8 MODBUS_ReadRegister(u8 slave_addr, u16 reg_addr, u16 *value)
{
    u8 tx_frame[8];
    u8 rx_buf[8];
    u16 crc;

    tx_frame[0] = slave_addr;
    tx_frame[1] = 0x03;              // 功能码: 读保持寄存器
    tx_frame[2] = (u8)(reg_addr >> 8);
    tx_frame[3] = (u8)(reg_addr & 0xFF);
    tx_frame[4] = 0x00;              // 读1个寄存器
    tx_frame[5] = 0x01;

    crc = CRC16(tx_frame, 6);
    tx_frame[6] = (u8)(crc & 0xFF);
    tx_frame[7] = (u8)((crc >> 8) & 0xFF);

    RS485_FlushRx();
    RS485_SendBytes(tx_frame, 8);

    // 读响应: [ADR][0x03][字节数][数据H][数据L][CRCL][CRCH] = 6字节
    u8 rx_len = RS485_WaitResponse(rx_buf, 6, 200);
    if (rx_len == 0) {
        printf("[RS485] Read timeout! slave=%d reg=0x%04X\r\n", slave_addr, reg_addr);
        return 1;
    }

    if (rx_buf[1] == 0x83) {  // 读异常
        printf("[RS485] Read exception! slave=%d error=0x%02X\r\n", slave_addr, rx_buf[2]);
        return 2;
    }

    *value = ((u16)rx_buf[3] << 8) | rx_buf[4];
    printf("[RS485] Read OK: slave=%d reg=0x%04X val=%d\r\n",
           slave_addr, reg_addr, *value);
    return 0;
}

// ================================================================
// 第二路 RS485 (USART2: PA2=TX, PA3=RX) --> 右轮驱动器
// ================================================================
static u8 rs4852_rx_buf[RS485_RX_BUF_SIZE];
static volatile u8 rs4852_rx_len = 0;

void RS4852_Init(u32 baudrate)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    // 1. GPIO 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    // 2. USART2 时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 3. GPIO 配置: PA2=TX, PA3=RX (复用功能)
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

    // TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 4. USART2 配置: 9600, 8E1
    USART_InitStructure.USART_BaudRate            = baudrate;
    USART_InitStructure.USART_WordLength        = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits          = USART_StopBits_1;
    USART_InitStructure.USART_Parity            = USART_Parity_Even;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART2, ENABLE);

    // 5. USART2 RX 中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    printf("[RS485-2] Init done, baud=%lu, 8E1, auto-dir\r\n", baudrate);
}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        u8 res = USART_ReceiveData(USART2);
        if (rs4852_rx_len < RS485_RX_BUF_SIZE) {
            rs4852_rx_buf[rs4852_rx_len++] = res;
        }
    }
}

void RS4852_SendBytes(u8 *data, u16 len)
{
    u16 i;
    for (i = 0; i < len; i++) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        USART_SendData(USART2, data[i]);
    }
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}

static void RS4852_FlushRx(void)
{
    rs4852_rx_len = 0;
}

static u8 RS4852_WaitResponse(u8 *buf, u8 expected_len, u32 timeout_ms)
{
    u32 start = 0;
    delay_us(200);
    for (start = 0; start < timeout_ms * 1000; start += 500) {
        if (rs4852_rx_len >= expected_len) {
            u8 i;
            for (i = 0; i < rs4852_rx_len && i < expected_len; i++) {
                buf[i] = rs4852_rx_buf[i];
            }
            RS4852_FlushRx();
            return rs4852_rx_len;
        }
        delay_us(500);
    }
    RS4852_FlushRx();
    return 0;
}

u8 MODBUS2_WriteRegister(u8 slave_addr, u16 reg_addr, u16 value)
{
    u8 tx_frame[8];
    u8 rx_buf[8];
    u16 crc;

    tx_frame[0] = slave_addr;
    tx_frame[1] = 0x06;
    tx_frame[2] = (u8)(reg_addr >> 8);
    tx_frame[3] = (u8)(reg_addr & 0xFF);
    tx_frame[4] = (u8)(value >> 8);
    tx_frame[5] = (u8)(value & 0xFF);

    crc = CRC16(tx_frame, 6);
    tx_frame[6] = (u8)(crc & 0xFF);
    tx_frame[7] = (u8)((crc >> 8) & 0xFF);

    RS4852_FlushRx();
    RS4852_SendBytes(tx_frame, 8);

    u8 rx_len = RS4852_WaitResponse(rx_buf, 8, 200);
    if (rx_len == 0) {
        printf("[RS485-2] Write timeout! slave=%d reg=0x%04X val=%d\r\n",
               slave_addr, reg_addr, (s16)value);
        return 1;
    }
    if (rx_buf[0] != slave_addr || rx_buf[1] != 0x06) {
        if (rx_buf[1] == 0x86) {
            printf("[RS485-2] Exception! slave=%d error=0x%02X\r\n",
                   slave_addr, rx_buf[2]);
        }
        printf("[RS485-2] Write response error! slave=%d\r\n", slave_addr);
        return 2;
    }
    printf("[RS485-2] Write OK: slave=%d reg=0x%04X val=%d\r\n",
           slave_addr, reg_addr, (s16)value);
    return 0;
}

u8 MODBUS2_ReadRegister(u8 slave_addr, u16 reg_addr, u16 *value)
{
    u8 tx_frame[8];
    u8 rx_buf[8];
    u16 crc;

    tx_frame[0] = slave_addr;
    tx_frame[1] = 0x03;
    tx_frame[2] = (u8)(reg_addr >> 8);
    tx_frame[3] = (u8)(reg_addr & 0xFF);
    tx_frame[4] = 0x00;
    tx_frame[5] = 0x01;

    crc = CRC16(tx_frame, 6);
    tx_frame[6] = (u8)(crc & 0xFF);
    tx_frame[7] = (u8)((crc >> 8) & 0xFF);

    RS4852_FlushRx();
    RS4852_SendBytes(tx_frame, 8);

    u8 rx_len = RS4852_WaitResponse(rx_buf, 6, 200);
    if (rx_len == 0) {
        printf("[RS485-2] Read timeout! slave=%d reg=0x%04X\r\n", slave_addr, reg_addr);
        return 1;
    }
    if (rx_buf[1] == 0x83) {
        printf("[RS485-2] Read exception! slave=%d error=0x%02X\r\n", slave_addr, rx_buf[2]);
        return 2;
    }
    *value = ((u16)rx_buf[3] << 8) | rx_buf[4];
    printf("[RS485-2] Read OK: slave=%d reg=0x%04X val=%d\r\n",
           slave_addr, reg_addr, *value);
    return 0;
}
