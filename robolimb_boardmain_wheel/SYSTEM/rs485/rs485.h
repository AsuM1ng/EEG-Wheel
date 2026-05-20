#ifndef __RS485_H
#define __RS485_H

#include "sys.h"

// ========== 初始化 ==========
void RS485_Init(u32 baudrate);

// ========== MODBUS-RTU 功能函数 ==========
// 写单个寄存器 (功能码0x06)
// 返回: 0=成功, 1=超时, 2=异常响应
u8 MODBUS_WriteRegister(u8 slave_addr, u16 reg_addr, u16 value);

// 读单个寄存器 (功能码0x03)
// 返回: 0=成功, 1=超时, 2=异常响应
u8 MODBUS_ReadRegister(u8 slave_addr, u16 reg_addr, u16 *value);

// ========== 底层函数 ==========
void RS485_SendBytes(u8 *data, u16 len);
u16 CRC16(u8 *data, u16 len);

// ========== 第二路 RS485 (USART2: PA2=TX, PA3=RX) ==========
void RS4852_Init(u32 baudrate);
void RS4852_SendBytes(u8 *data, u16 len);
u8 MODBUS2_WriteRegister(u8 slave_addr, u16 reg_addr, u16 value);
u8 MODBUS2_ReadRegister(u8 slave_addr, u16 reg_addr, u16 *value);

#endif
