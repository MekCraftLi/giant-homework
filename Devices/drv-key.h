/**
 ***********************************************************************************************************************
 * @file           : drv-key.h
 * @brief          : 按键对象的定义
 * @author         : 李嘉豪
 * @date           : 2025-07-06
 ***********************************************************************************************************************
 * @attention
 *
 * 按键对象的定义和方法声明
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __DRV_KEY_H__
#define __DRV_KEY_H__




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "../Peripherals/gpio.h"
#include "../Services/time-service.h"
#include <stdint.h>





/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef struct {
    GPIOPortEnum port;          // 按键所在GPIO端口
    GPIOPinEnum pin;            // 按键所在GPIO引脚
    uint8_t pressed;            // 按键是否按下
    SoftTimerHandle pressTimer; // 按键按下定时器句柄
} KeyObjTypeDef;                // 按键对象类型

typedef struct {
    void (*init)(KeyObjTypeDef* keyObj, GPIOPortEnum port, GPIOPinEnum pin); // 初始化按键对象
    uint8_t (*isPressed)(KeyObjTypeDef* keyObj);                             // 检查按键是否按下
} KeyIntfTypeDef;                                                            // 按键接口类型定义



/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/





/*-------- function prototypes ---------------------------------------------------------------------------------------*/





#endif /* __DRV_KEY_H__ */
