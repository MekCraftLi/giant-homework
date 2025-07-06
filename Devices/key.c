/**
 ***********************************************************************************************************************
 * @file           : Bsp-Key.c
 * @brief          : 按键驱动
 * @author         : 李嘉豪
 * @date           : 2025-04-05
 ***********************************************************************************************************************
 * @attention
 *
 * None
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "Bsp-Key.h"
#include "misc.h"
#include <stdlib.h>




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- variables -------------------------------------------------------------------------------------------------*/

extern uint32_t sysTick; // 系统滴答计数器




/* ------- function prototypes ---------------------------------------------------------------------------------------*/





/* ------- function implement ----------------------------------------------------------------------------------------*/

/**
 * @brief  创建按键对象
 *
 * @param pin
 * @param callback
 * @return KeyTypeDef*
 */
KeyTypeDef* keyCreate(uint16_t pin, void (*callback)(void*)) {
    KeyTypeDef* pNewKey;
    pNewKey = (KeyTypeDef*)malloc(sizeof(KeyTypeDef));
    if (pNewKey == NULL) {
        return NULL;
    }
    pNewKey->keyCallback      = callback;
    pNewKey->keyPin           = pin;
    pNewKey->keylastPressTime = 0;
    return pNewKey;
}

/**
 * @brief  按键下降沿触发回调函数
 *
 * @param pKey
 */
void keyFallingTriggerCallback(KeyTypeDef* pKey, void* argument) {
    // 和上一次触发事件相差大于5000即认为稳定
    if (sysTick - pKey->keylastPressTime > 500) {

        if (pKey->keyCallback != NULL) {
            pKey->keyCallback(argument);
        }
    }

    pKey->keylastPressTime = sysTick;

    EXTI_ClearITPendingBit(pKey->keyPin); // 清除中断标志位
}
