/**
 ***********************************************************************************************************************
 * @file           : tim.h
 * @brief          : 定时器配置文件
 * @author         : 李嘉豪
 * @date           : 2025-07-02
 ***********************************************************************************************************************
 * @attention
 *
 * 实现定时器的初始化和其他功能封装
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __TIM_H
#define __TIM_H




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "stm32f10x.h"
#include "stm32f10x_tim.h"




/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef enum {
    TIM_SUCCESS,
    TIM_ERR_PARAM,
    TIM_ERR_INVALID,
    TIM_ERR_UNINIT,
} TIMErrCode;

typedef enum {
    TIM_MODE_BASIC,
    TIM_MODE_DELAY,
    TIM_MODE_PWM,
    TIM_MODE_ENCODER,
    TIM_MODE_TRIGGER,
} TIMModeEnum;

typedef enum {
    TIMER_CHANNEL_1,
    TIMER_CHANNEL_2,
    TIMER_CHANNEL_3,
    TIMER_CHANNEL_4,
} TIMChannelEnum;

typedef struct {
    TIM_TypeDef* tim;       // 定时器外设
    uint8_t init;           // 是否已初始化
    uint32_t clkFreq;       // 定时器时钟频率
    uint32_t targetFreq;    // 目标频率
    TIMModeEnum mode;       // 定时器模式
    TIMChannelEnum channel; // 定时器通道
    uint8_t ISREnabled;     // 是否使能中断
    void (*Callback)(void); // 定时器回调函数
} TIMObjTypeDef;

typedef struct {
    TIMErrCode (*init)(TIMObjTypeDef* timObj, TIM_TypeDef* tim);
    TIMErrCode (*start)(TIMObjTypeDef* timObj);
    TIMErrCode (*stop)(TIMObjTypeDef* timObj);
    TIMErrCode (*setFrequency)(TIMObjTypeDef* timObj, uint32_t freq);
    TIMErrCode (*getFrequency)(TIMObjTypeDef* timObj, uint32_t* freq);
    TIMErrCode (*disableISR)(TIMObjTypeDef* timObj);
    TIMErrCode (*enableISR)(TIMObjTypeDef* timObj);
    TIMErrCode (*setCallback)(TIMObjTypeDef* timObj, void (*Callback)(void));
    TIMErrCode (*countConfig)(TIMObjTypeDef* timObj, uint32_t freq, uint32_t maxCount);
    TIMErrCode (*timEncoderConfig)(TIMObjTypeDef* timObj);
} TIMIntfTypeDef;




/*-------- define ----------------------------------------------------------------------------------------------------*/

#define SYSCLK 72000000 // 系统时钟频率，单位Hz




/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- function prototypes ---------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/

extern TIMIntfTypeDef timIntf;




#endif /* __TIM_H */
