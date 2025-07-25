/**
 ***********************************************************************************************************************
 * @file           : systick.h
 * @brief          : 系统滴答定时器对外接口
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

#ifndef __SYSTICK_H__
#define __SYSTICK_H__




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include <stdint.h>




/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef struct {
    void (*systInit)(void);
    void (*systDelayClkCycles)(uint32_t clkCycles);
} SystIntfTypeDef;




/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/

extern SystIntfTypeDef systIntf; // SysTick对外接口




/*-------- function prototypes ---------------------------------------------------------------------------------------*/





#endif /* __SYSTICK_H__ */
