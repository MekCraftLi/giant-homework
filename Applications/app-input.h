/**
 ***********************************************************************************************************************
 * @file           : app-input.h
 * @brief          : 输入数据处理应用
 * @author         : 李嘉豪
 * @date           : 2025-07-06
 ***********************************************************************************************************************
 * @attention
 *
 * 输入信号处理, 负责事件产生和信号信息的更新
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __APP_INPUT_H__
#define __APP_INPUT_H__




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "../Services/time-service.h"
#include <stdint.h>





/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef enum {
    INPUT_EVENT_NONE,
    INPUT_EVENT_ENCODER_MOVE_POSITIVE, // 编码器正向移动事件
    INPUT_EVENT_ENCODER_MOVE_NEGATIVE, // 编码器反向移动事件
    INPUT_EVENT_KEY_0_CLICK,           // 按键点击事件
    INPUT_EVENT_KEY_1_CLICK,           // 按键点击事件

} InputEventEnum; // 输入事件枚举类型定义


typedef struct {
    InputEventEnum event;  // 输入事件
    uint16_t encoderValue; // 编码器值
    uint8_t key0Pressed;   // 按键1是否按下
    uint8_t key1Pressed;   // 按键2是否按下

    SoftTimerHandle key0Timer; // 按键1定时器句柄
    SoftTimerHandle key1Timer; // 按键2定时器句柄

} InputAppParamTypeDef; // 输入应用参数类型定义




/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/





/*-------- function prototypes ---------------------------------------------------------------------------------------*/

void inputAppInit(void* argument); // 输入应用初始化函数
void inputAppLoop(void* argument); // 输入应用循环函数




#endif /* __APP_INPUT_H__ */
