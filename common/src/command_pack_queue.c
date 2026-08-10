/**
 * @file    command_pack_queue.c
 * @brief   命令协议包队列实现与封包/解包函数。
 *          学生在此文件中完成 TODO 标注的函数。
 */
#include "command_pack_queue.h"
#include "led.h"

/* ================================================================
 * 题目 1：队列实现
 * ================================================================
 * 环形队列的 head 指向下一个读位置，tail 指向下一个写位置，
 * 用 count 区分"空"和"满"。
 */

/* TODO: 初始化队列，将 head/tail/count 归零 */
void packet_queue_init(packet_queue *q)
{
    
    /* 在此实现 */
}

/* TODO: 入队，成功返回 true，队列满返回 false */
bool packet_queue_push(packet_queue *q, const command_packet *pkt)
{
    
    /* 在此实现 */
}

/* TODO: 出队，将数据写入 *pkt，成功返回 true，队列空返回 false */
bool packet_queue_pop(packet_queue *q, command_packet *pkt)
{
    
    /* 在此实现 */
}

/* TODO: 判空 */
bool packet_queue_is_empty(const packet_queue *q)
{
    
    /* 在此实现 */
}

/* TODO: 判满 */
bool packet_queue_is_full(const packet_queue *q)
{
   
    /* 在此实现 */
}

/* ================================================================
 * 题目 2：位运算封包（含校验和）
 * ================================================================
 * 用移位和按位或填入 command_packet，并计算校验和。
 * 命令字中：高 4 位为闪烁次数，低 4 位为 LED 掩码。
 *
 * 例如 command_pack_create(&pkt, 3U, LED_MASK_LED1)：
 *   pkt.header   = {0x0A, 0x5A}
 *   pkt.cmd      = 0x31
 *   pkt.checksum = (0x0A + 0x5A + 0x31) 的低 8 位
 *
 * 要求：使用位运算完成，不得逐字节赋值成员。
 */

/* TODO: 用位运算完成封包，并填入校验和 */
void command_pack_create(command_packet *pkt, uint8_t blink_count, uint8_t led_mask)
{
   
    /* 在此实现 */
}

/* ================================================================
 * 题目 3：位运算解包（含校验）
 * ================================================================
 * 从 command_packet 中解出命令并校验。
 *   1. 用位运算将两个 header 字节拼成 uint16_t，与 HEADER_WORD 比较；
 *   2. 用 PACKET_CHECKSUM 宏重新计算校验和并与 pkt->checksum 比较；
 *   3. 校验通过则解出闪烁次数与 LED 掩码写入输出参数，返回 true；
 *      任意一项校验失败返回 false。
 */

/* TODO: 用位运算校验包头和校验和，解出闪烁次数与 LED 掩码 */
bool command_pack_unpack(const command_packet *pkt, uint8_t *blink_count, uint8_t *led_mask)
{ 
    
   

    /* 在此实现 */
}

/* ================================================================
 * LED 命令执行（已封装，直接调用即可）
 * ================================================================
 * blink_count：闪烁次数
 * led_mask：LED 掩码（低 4 位，bit0~bit3 对应 LED1~LED4）
 */
void command_led_execute(uint8_t blink_count, uint8_t led_mask)
{
    /* LED1~LED4 对应 PB3~PB6，4 位掩码整体左移 3 位即得引脚掩码 */
    // 可以查阅LED_PIN的引脚定义,思考为什么这样实现
    uint8_t led_pins = (uint8_t)((led_mask & CMD_LED_MASK) << 3);

    for (uint8_t i = 0U; i < blink_count; i++)
    {
        led_on(led_pins);
        HAL_Delay(200U);
        led_off(led_pins);
        HAL_Delay(200U);
    }
}
