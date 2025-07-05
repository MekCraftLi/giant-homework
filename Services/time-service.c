/**
 ***********************************************************************************************************************
 * @file           : timeSerive.c
 * @brief          : 时间服务类
 * @author         : 李嘉豪
 * @date           : 2025-04-29
 ***********************************************************************************************************************
 * @attention
 *
 * 用于提供全局可访问的时间服务
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "time-service.h"
#include <stdlib.h>
#include <string.h>



/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

void servInit(void);
void delayUWithSyst(uint32_t u);
void delayMWithSyst(uint32_t m);
void delaySWithSyst(uint32_t s);
float getGlobalTime(void);

SoftTimerHandle softTimerRegister(void);
void softTimerUnregister(SoftTimerHandle handle);
float getElapsedTime(SoftTimerHandle handle);




/* ------- variables -------------------------------------------------------------------------------------------------*/

TimeServIntfTypeDef timeServIntf = {
    .servInit            = servInit,
    .delayUs             = delayUWithSyst,
    .delayMs             = delayMWithSyst,
    .delaySec            = delaySWithSyst,
    .getGlobalTime       = getGlobalTime,
    .softTimerRegister   = softTimerRegister,
    .softTimerUnregister = softTimerUnregister,
    .getElapsedTime      = getElapsedTime,
};

static TIMObjTypeDef systTimObjSub;
static TIMObjTypeDef systTimObj;

static float globalTime = 0.0f; // 全局时间变量



/* ------- function implement ----------------------------------------------------------------------------------------*/

void servInit(void) {
    timIntf.init(&systTimObjSub, TIM2);
    timIntf.init(&systTimObj, TIM5);
    timIntf.countConfig(&systTimObjSub, 1000000, 0xFFFF); // 配置定时器2为1us计数
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
    timIntf.countConfig(&systTimObj, systTimObj.clkFreq, 0xFFFF);
    TIM_SelectInputTrigger(systTimObj.tim, TIM_TS_ITR0);
    TIM_SelectSlaveMode(systTimObj.tim, TIM_SlaveMode_External1);
    timIntf.start(&systTimObj);
    timIntf.start(&systTimObjSub);
}

void delayUWithSyst(uint32_t u) { systIntf.systDelayClkCycles(72 * u); }

void delayMWithSyst(uint32_t m) {
    while (m--) {
        delayUWithSyst(1000);
    }
}

void delaySWithSyst(uint32_t s) {
    while (s--) {
        delayMWithSyst(1000);
    }
}

float getGlobalTime(void) {
    uint16_t systTimSub = TIM_GetCounter(systTimObjSub.tim); // 获取定时器计数值
    uint16_t systTim    = TIM_GetCounter(systTimObj.tim);
    globalTime          = (float)systTimSub / 1000000.0f; // 将计数值转换为秒
    globalTime += (float)systTim * 0xFFFF / 1000000.0f;
    return globalTime; // 返回全局时间
}


/**
 * @brief 注册软定时器
 *
 * @return SoftTimerHandle
 */
SoftTimerHandle softTimerRegister(void) {
    SoftTimerDataTypeDef* timerData = (SoftTimerDataTypeDef*)malloc(sizeof(SoftTimerDataTypeDef));

    return (SoftTimerHandle)timerData; // 返回新注册的定时器句柄
}

/**
 * @brief 注销软定时器
 *
 * @param handle
 */
void softTimerUnregister(SoftTimerHandle handle) {
    if (handle != NULL) {
        free((SoftTimerDataTypeDef*)handle); // 释放定时器数据
    }
}

/**
 * @brief 获取软定时器经过的时间
 *
 * @param handle
 */
float getElapsedTime(SoftTimerHandle handle) {
    if (handle == NULL) {
        return 0.0f; // 无效的句柄
    }
    float eplasedTime = getGlobalTime() - ((SoftTimerDataTypeDef*)handle)->startTime; // 返回经过的时间
    ((SoftTimerDataTypeDef*)handle)->startTime = getGlobalTime();                     // 更新开始时间
    return eplasedTime;                                                               // 返回经过的时间
}
