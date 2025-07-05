/**
 ***********************************************************************************************************************
 * @file           : timeService.h
 * @brief          : 时间服务
 * @author         : 李嘉豪
 * @date           : 2025-04-29
 ***********************************************************************************************************************
 * @attention
 *
 * None
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __TIME_SERVICE_H__
#define __TIME_SERVICE_H__




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "../Peripherals/dma.h"
#include "../Peripherals/systick.h"
#include "../Peripherals/tim.h"





/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef uint32_t SoftTimerHandle; // 时间句柄类型定义，用于标识时间服务的实例或任务

/* 时间服务对外接口 */
typedef struct {
    void (*servInit)(void);
    void (*delayUs)(uint32_t us);
    void (*delayMs)(uint32_t ms);
    void (*delaySec)(uint32_t sec);
    float (*getGlobalTime)(void);
    SoftTimerHandle (*softTimerRegister)(void);
    void (*softTimerUnregister)(SoftTimerHandle handle);
    float (*getElapsedTime)(SoftTimerHandle handle);
} TimeServIntfTypeDef;

/* 软件定时器计时数据 */
typedef struct {
    float startTime; // 计时开始时间
} SoftTimerDataTypeDef;




/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/

extern TimeServIntfTypeDef timeServIntf;




/*-------- function prototypes ---------------------------------------------------------------------------------------*/





#endif /* __TIME_SERVICE_H__ */
