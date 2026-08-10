/**
 * @file    command_pack_queue.h
 * @brief   命令协议包结构与包队列类型定义。
 *          本文件提供结构体类型，学生无需修改。
 *          队列实现与封包/解包函数在 command_pack_queue.c 中完成。
 */
#ifndef COMMAND_PACK_QUEUE_H
#define COMMAND_PACK_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 协议包结构体（4 字节，已提供，无需修改）
 * ================================================================
 * 字节布局：
 *   [0] header[0] — 帧头高字节，固定 0x0A
 *   [1] header[1] — 帧头低字节，固定 0x5A（拼成 16 位 0x0A5A）
 *   [2] cmd       — 命令字：高 4 位为闪烁次数，低 4 位为 LED 掩码
 *   [3] checksum  — 校验和 = (header[0] + header[1] + cmd) 的低 8 位
 *
 * 例如：{0x0A, 0x5A, 0x31, 0xXX} 表示"LED1 闪烁 3 次"
 */
#define HEADER_HIGH_BYTE 0x0AU
#define HEADER_LOW_BYTE  0x5AU
#define HEADER_WORD      0x0A5AU

/* 命令字位段：bit7~bit4 闪烁次数，bit3~bit0 LED 掩码 */
#define CMD_BLINK_SHIFT  4U
#define CMD_LED_MASK     0x0FU

typedef struct
{
    uint8_t header[2];   /* 帧头，固定 {0x0A, 0x5A} */
    uint8_t cmd;         /* 命令字：高 4 位闪烁次数，低 4 位 LED 掩码 */
    uint8_t checksum;    /* 校验和，header[0]+header[1]+cmd 的低 8 位 */
} command_packet;

/* ================================================================
 * 校验和宏（已提供）
 * ================================================================
 * 用法：checksum = PACKET_CHECKSUM(pkt->header[0], pkt->header[1], pkt->cmd);
 */
#define PACKET_CHECKSUM(h0, h1, cmd) ((uint8_t)((h0) + (h1) + (cmd)))

/* ================================================================
 * 包队列结构体（已提供，无需修改）
 * ================================================================
 * 用环形队列缓存多个 command_packet。
 * head/tail/count 的用法 ring_buffer 一致。
 */
#define PACKET_QUEUE_SIZE 16

typedef struct
{
    command_packet buf[PACKET_QUEUE_SIZE];
    uint8_t        head;
    uint8_t        tail;
    uint8_t        count;
} packet_queue;

/* ================================================================
 * 需要实现的函数声明
 * ================================================================ */

/* ---- 题目 1：队列操作 ---- */
void packet_queue_init(packet_queue *q);
bool packet_queue_push(packet_queue *q, const command_packet *pkt);
bool packet_queue_pop(packet_queue *q, command_packet *pkt);
bool packet_queue_is_empty(const packet_queue *q);
bool packet_queue_is_full(const packet_queue *q);

/* ---- 题目 2：位运算封包 ---- */
void command_pack_create(command_packet *pkt, uint8_t blink_count, uint8_t led_mask);

/* ---- 题目 3：位运算解包 ---- */
bool command_pack_unpack(const command_packet *pkt, uint8_t *blink_count, uint8_t *led_mask);

/* ---- 已封装的 LED 命令执行（直接调用即可） ---- */
void command_led_execute(uint8_t blink_count, uint8_t led_mask);

#ifdef __cplusplus
}
#endif

#endif /* COMMAND_PACK_QUEUE_H */
