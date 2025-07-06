/**
 ***********************************************************************************************************************
 * @file           : app-input.c
 * @brief          : 输入数据处理应用
 * @author         : 李嘉豪
 * @date           : 2025-07-06
 ***********************************************************************************************************************
 * @attention
 *
 * 输入信号处理, 负责事件产生
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "../Peripherals/gpio.h"
#include "../Peripherals/tim.h"
#include "app-input.h"




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

void appDataUpdate(void* argument); // 应用数据更新函数




/* ------- variables -------------------------------------------------------------------------------------------------*/

static TIMObjTypeDef encoderTim;





/* ------- function implement ----------------------------------------------------------------------------------------*/

void inputAppInit(void* argument) {
    InputAppParamTypeDef* pParam = (InputAppParamTypeDef*)argument;

    // 编码器配置
    gpioIntf.pinInit(PORT_A, PIN_6, INPUT_FLOATING);
    gpioIntf.pinInit(PORT_A, PIN_7, INPUT_FLOATING);
    timIntf.init(&encoderTim, TIM3);
    timIntf.timEncoderConfig(&encoderTim);

    // 按键配置
    gpioIntf.pinInitWithEXTI(PORT_C, PIN_5, INPUT_PULL_UP, FALLING_EDGE);  // 按键1
    gpioIntf.pinInitWithEXTI(PORT_A, PIN_15, INPUT_PULL_UP, FALLING_EDGE); // 按键2

    pParam->key0Timer = timeServIntf.softTimerRegister();
    pParam->key1Timer = timeServIntf.softTimerRegister();
}

void inputAppLoop(void* argument) {

    InputAppParamTypeDef* pParam = (InputAppParamTypeDef*)argument;

    // 更新
    appDataUpdate(pParam);

    static uint16_t lastEncoderValue = 0x7FFF;
    static uint8_t lastKey0State;
    static uint8_t lastKey1State;

    if (pParam->key0Pressed - lastKey0State == 1) {
        pParam->event = INPUT_EVENT_KEY_0_CLICK; // 按键1点击事件
        lastKey1State = pParam->key0Pressed;     // 更新按键1状态
    } else if (pParam->key1Pressed - lastKey1State == 1) {
        pParam->event = INPUT_EVENT_KEY_1_CLICK; // 按键2点击事件
        lastKey1State = pParam->key1Pressed;     // 更新按键2状态
    } else if (pParam->encoderValue - lastEncoderValue >= 4) {
        pParam->event    = INPUT_EVENT_ENCODER_MOVE_POSITIVE;
        lastEncoderValue = pParam->encoderValue; // 更新编码器值
    } else if (pParam->encoderValue - lastEncoderValue <= -4) {
        pParam->event    = INPUT_EVENT_ENCODER_MOVE_NEGATIVE;
        lastEncoderValue = pParam->encoderValue; // 更新编码器值
    } else {
		pParam->key0Pressed = lastKey0State = 0;
		pParam->key1Pressed = lastKey1State = 0;
        pParam->event = INPUT_EVENT_NONE; // 无事件
    }
}


void appDataUpdate(void* argument) {
    InputAppParamTypeDef* pParam = (InputAppParamTypeDef*)argument;

    pParam->event                = INPUT_EVENT_NONE;

    pParam->encoderValue         = TIM_GetCounter(encoderTim.tim);
}
