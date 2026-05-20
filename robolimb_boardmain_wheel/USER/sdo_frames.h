#ifndef __SDO_FRAMES_H
#define __SDO_FRAMES_H

#include "sys.h"
#include "can.h"

#define SDO_MAX_DATA_LEN 8

typedef struct {
    UNS8 len;
    UNS8 data[SDO_MAX_DATA_LEN];
} SDO_Frame;



extern const SDO_Frame SDO_ACTIVATE_INIT;
extern const SDO_Frame SDO_ACTIVATE_PPM;
extern const SDO_Frame SDO_ACTIVATE_SETV2000;
extern const SDO_Frame SDO_ACTIVATE_SETV1000;
extern const SDO_Frame SDO_ACTIVATE_SETV100;
extern const SDO_Frame SDO_DISABLE;
extern const SDO_Frame SDO_ENABLE;
extern const SDO_Frame SDO_GO;
extern const SDO_Frame SDO_TARGET_POS_NODE10000;
extern const SDO_Frame SDO_TARGET_POS_NODE1000;
extern const SDO_Frame SDO_TARGET_POS_NODE2000;
extern const SDO_Frame SDO_TARGET_POS_NODE5000;
extern const SDO_Frame SDO_TARGET_POS_NODEUN10000;
extern const SDO_Frame SDO_TARGET_POS_NODEUN1000;
extern const SDO_Frame SDO_TARGET_POS_NODEUN2000;
extern const SDO_Frame SDO_TARGET_POS_NODEUN5000;
extern const SDO_Frame SDO_TARGET_POS_NODE100;
extern const SDO_Frame SDO_TARGET_POS_NODE200;
extern const SDO_Frame SDO_TARGET_POS_NODE500;
extern const SDO_Frame SDO_TARGET_POS_NODEUN100;
extern const SDO_Frame SDO_TARGET_POS_NODEUN200;
extern const SDO_Frame SDO_TARGET_POS_NODEUN500;

UNS8 send_sdo_to_node(UNS8 node_id, const SDO_Frame *frame);

// ========== 状态字读取 ==========
// 全局状态字缓存（节点1-4）
extern volatile uint16_t g_statusword[5];  // [0]不用，[1-4]对应节点

// 发送读状态字请求
void request_statusword(UNS8 node_id);

// 检查节点是否到位（bit 12: set-point acknowledged）
u8 is_node_reached(UNS8 node_id);

// 检查多个节点是否全部到位
u8 are_all_nodes_reached(u8 node_mask);  // node_mask: bit0=节点1, bit1=节点2...

#endif
