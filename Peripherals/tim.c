/**
 ***********************************************************************************************************************
 * @file           : tim.c
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




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "tim.h"
#include <math.h>
#include <stdlib.h>




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/

#define GET_TIM_CLOCK(TIMX, CLK)                                                                                       \
    do {                                                                                                               \
        if (TIMX == TIM1 || TIMX == TIM8) {                                                                            \
            CLK = SYSCLK;                                                                                              \
        } else {                                                                                                       \
            CLK = SYSCLK / 2;                                                                                          \
        }                                                                                                              \
    } while (0);





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

static TIMErrCode timCalcARRPSC(uint32_t clkFreq, uint32_t targetFreq, uint32_t* arr, uint16_t* psc);
TIMErrCode timInit(TIMObjTypeDef* timObj, TIM_TypeDef* tim);
TIMErrCode timSetFrequency(TIMObjTypeDef* timObj, uint32_t targetFreq);
TIMErrCode timStart(TIMObjTypeDef* timObj);
TIMErrCode timStop(TIMObjTypeDef* timObj);
TIMErrCode timSetCallback(TIMObjTypeDef* TIMObjTypeDef, void (*callback)(void));
TIMErrCode timEnableISR(TIMObjTypeDef* timObj);
TIMErrCode timConfigCount(TIMObjTypeDef* timObj, uint32_t freq, uint32_t maxCount);
TIMErrCode timEncoderConfig(TIMObjTypeDef* timObj);




/* ------- variables -------------------------------------------------------------------------------------------------*/

TIMIntfTypeDef timIntf = {
    .init             = timInit,
    .setFrequency     = timSetFrequency,
    .enableISR        = timEnableISR,
    .start            = timStart,
    .stop             = timStop,
    .countConfig      = timConfigCount,
    .timEncoderConfig = timEncoderConfig,
};


/* ------- function implement ----------------------------------------------------------------------------------------*/

TIMErrCode timInit(TIMObjTypeDef* timObj, TIM_TypeDef* tim) {
    uint32_t clk = 0;
    GET_TIM_CLOCK(tim, clk);    // 获取定时器时钟频率
    timObj->tim     = tim;      // 设置定时器外设
    timObj->clkFreq = 72000000; // 设置定时器时钟频率

    // 启用时钟
    if (tim == TIM1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    } else if (tim == TIM2) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    } else if (tim == TIM3) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    } else if (tim == TIM4) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    } else if (tim == TIM5) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    } else if (tim == TIM6) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    } else if (tim == TIM7) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    } else if (tim == TIM8) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    } else {
        return TIM_ERR_INVALID; // 无效的定时器
    }

    return TIM_SUCCESS;
}

static TIMErrCode timCalcARRPSC(uint32_t clkFreq, uint32_t targetFreq, uint32_t* arr, uint16_t* psc) {
    if (targetFreq == 0 || clkFreq == 0) {
        return TIM_ERR_PARAM; // 无效的频率
    }

    uint32_t totalDiv = clkFreq / targetFreq; // 计算总分频系数


    *psc              = (uint16_t)(sqrt(totalDiv));
    *arr              = (uint16_t)(totalDiv / (*psc) - 1);

    if (*arr > 65535) {
        return TIM_ERR_PARAM; // ARR超出范围
    }

    if (*psc > 65535) {
        return TIM_ERR_PARAM; // PSC超出范围
    }

    if (*psc == 0) {
        *psc = 1; // 确保PSC不为0
    }
    if (*arr == 0) {
        *arr = 1; // 确保ARR不为0
    }

    return TIM_SUCCESS;
}

TIMErrCode timSetFrequency(TIMObjTypeDef* timObj, uint32_t targetFreq) {
    if (timObj == NULL || timObj->tim == NULL) {
        return TIM_ERR_PARAM; // 无效的定时器对象
    }

    uint32_t arr   = 0;
    uint16_t psc   = 0;

    TIMErrCode err = timCalcARRPSC(timObj->clkFreq, targetFreq, &arr, &psc);
    if (err != TIM_SUCCESS) {
        return err; // 计算ARR和PSC失败
    }

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period        = arr - 1;
    TIM_TimeBaseStructure.TIM_Prescaler     = psc - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;

    TIM_TimeBaseInit(timObj->tim, &TIM_TimeBaseStructure);

    timObj->targetFreq = targetFreq; // 更新目标频率
    timObj->init       = 1;          // 标记定时器已初始化

    return TIM_SUCCESS;
}

TIMErrCode timStart(TIMObjTypeDef* timObj) {
    if (timObj == NULL || timObj->tim == NULL) {
        return TIM_ERR_PARAM; // 无效的定时器对象
    }
    if (!timObj->init) {
        return TIM_ERR_UNINIT; // 定时器未初始化
    }

    TIM_Cmd(timObj->tim, ENABLE); // 启动定时器

    return TIM_SUCCESS;
}

TIMErrCode timStop(TIMObjTypeDef* timObj) {
    if (timObj == NULL || timObj->tim == NULL) {
        return TIM_ERR_PARAM; // 无效的定时器对象
    }
    if (!timObj->init) {
        return TIM_ERR_UNINIT; // 定时器未初始化
    }

    TIM_Cmd(timObj->tim, DISABLE); // 停止定时器

    return TIM_SUCCESS;
}

TIMErrCode timSetCallback(TIMObjTypeDef* timObj, void (*callback)(void)) {
    if (timObj == NULL || timObj->tim == NULL) {
        return TIM_ERR_PARAM; // 无效的定时器对象
    }
    if (!timObj->init) {
        return TIM_ERR_UNINIT; // 定时器未初始化
    }

    timObj->Callback = callback; // 设置回调函数

    return TIM_SUCCESS;
}

TIMErrCode timEnableISR(TIMObjTypeDef* timObj) {
    if (timObj == NULL || timObj->tim == NULL) {
        return TIM_ERR_PARAM; // 无效的定时器对象
    }
    if (!timObj->init) {
        return TIM_ERR_UNINIT; // 定时器未初始化
    }

    uint8_t IRQChannel = 0;

    if (timObj->tim == TIM1) {
        IRQChannel = TIM1_UP_IRQn; // 定时器1更新中断
    } else if (timObj->tim == TIM2) {
        IRQChannel = TIM2_IRQn; // 定时器2更新中断
    } else if (timObj->tim == TIM3) {
        IRQChannel = TIM3_IRQn; // 定时器3更新中断
    } else if (timObj->tim == TIM4) {
        IRQChannel = TIM4_IRQn; // 定时器4更新中断
    } else if (timObj->tim == TIM5) {
        IRQChannel = TIM5_IRQn; // 定时器5更新中断
    } else if (timObj->tim == TIM6) {
        IRQChannel = TIM6_IRQn; // 定时器6更新中断
    } else if (timObj->tim == TIM7) {
        IRQChannel = TIM7_IRQn; // 定时器7更新中断
    } else if (timObj->tim == TIM8) {
        IRQChannel = TIM8_UP_IRQn; // 定时器8更新中断
    } else {
        return TIM_ERR_INVALID; // 无效的定时器
    }

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = IRQChannel; // 根据具体定时器选择中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(timObj->tim, TIM_IT_Update, ENABLE); // 使能更新中断

    timObj->ISREnabled = 1; // 标记中断已使能

    return TIM_SUCCESS;
}

TIMErrCode timConfigCount(TIMObjTypeDef* timObj, uint32_t freq, uint32_t maxCount) {
    if (timObj == NULL || timObj->tim == NULL) {
        return TIM_ERR_PARAM; // 无效的定时器对象
    }

    if (freq == 0 || maxCount == 0) {
        return TIM_ERR_PARAM; // 无效的频率或计数值
    }

    uint16_t psc = timObj->clkFreq / freq - 1; // 计算预分频值

    if (psc > 65535) {
        return TIM_ERR_PARAM; // 预分频值超出范围
    }
    if (timObj->tim == TIM2) {
        if (maxCount > 0xFFFFFFFF) {
            return TIM_ERR_PARAM; // 最大计数值超出范围
        }
    } else {
        if (maxCount > 0xFFFF) {
            return TIM_ERR_PARAM; // 最大计数值超出范围
        }
    }

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period        = maxCount - 1;
    TIM_TimeBaseStructure.TIM_Prescaler     = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(timObj->tim, &TIM_TimeBaseStructure);
    timObj->init = 1;

    return TIM_SUCCESS;
}

TIMErrCode timEncoderConfig(TIMObjTypeDef* timObj) {
    if (timObj == NULL || timObj->tim == NULL) {
        return TIM_ERR_PARAM; // 无效的定时器对象
    }

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Prescaler     = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period        = 0xFFFF;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(timObj->tim, &TIM_TimeBaseStructure);

    TIM_EncoderInterfaceConfig(timObj->tim, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Falling);
    TIM_SetCounter(timObj->tim, 32767);
    TIM_Cmd(timObj->tim, ENABLE);

    return TIM_SUCCESS;
}
