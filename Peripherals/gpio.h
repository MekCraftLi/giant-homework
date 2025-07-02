/**
 ***********************************************************************************************************************
 * @file           : gpio.h
 * @brief          : 通用输入输出接口
 * @author         : 李嘉豪
 * @date           : 2025-04-05
 ***********************************************************************************************************************
 * @attention
 *
 * None
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __GPIO_H
#define __GPIO_H




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"




/*-------- typedef ---------------------------------------------------------------------------------------------------*/

// GPIO端口封装
typedef enum {
    PORT_A,
    PORT_B,
    PORT_C,
    PORT_D,
    PORT_E,
    PORT_F,
    PORT_G,
} GPIOPortEnum;

// GPIO引脚封装
typedef enum {
    PIN_0,
    PIN_1,
    PIN_2,
    PIN_3,
    PIN_4,
    PIN_5,
    PIN_6,
    PIN_7,
    PIN_8,
    PIN_9,
    PIN_10,
    PIN_11,
    PIN_12,
    PIN_13,
    PIN_14,
    PIN_15,
} GPIOPinEnum;

// GPIO模式封装
typedef enum {
    INPUT_ANALOG          = 0x0,  // 模拟输入
    INPUT_FLOATING        = 0x04, // 浮空输入
    INPUT_PULL_DOWN       = 0x28, // 下拉输入
    INPUT_PULL_UP         = 0x48, // 上拉输入
    OUTPUT_OPEN_DRAIN     = 0x14, // 开漏输出
    OUTPUT_PUSH_PULL      = 0x10, // 推挽输出
    ALT_OUTPUT_OPEN_DRAIN = 0x1C, // 复用开漏输出
    ALT_OUTPUT_PUSH_PULL  = 0x18  // 复用推挽输出
} GPIOModeEnum;

// GPIO引脚状态封装
typedef enum {
    PIN_SET   = 0x1, // 高电平
    PIN_RESET = 0x0, // 低电平
} GPIOPinStateEnum;

typedef enum {
    RAISING_EDGE = 0x08, // 上升沿
    FALLING_EDGE = 0x0C, // 下降沿
    BOTH_EDGE    = 0x10, // 上升沿和下降沿
} EdgeModeEnum;

typedef enum {
    PORT_NOT_EXIST      = 0x00, // 不存在该端口
    PIN_NOT_EXIST       = 0x01, // 不存在该引脚
    PIN_STATE_NOT_EXIST = 0x02, // 不存在该电平
    PIN_MODE_NOT_EXIST  = 0x03, // 不存在该引脚模式
    PIN_EXTI_NOT_EXIST  = 0x04, // 不存在该EXTI中断
    GPIO_SUCCESS        = 0x05, // 成功
} GPIOErrCode;


// GPIO接口封装

typedef struct {
    // GPIO初始化
    GPIOErrCode (*pinInit)(GPIOPortEnum, GPIOPinEnum, GPIOModeEnum);
    // GPIO初始化并配置EXTI中断
    GPIOErrCode (*pinInitWithEXTI)(GPIOPortEnum, GPIOPinEnum, GPIOModeEnum, EdgeModeEnum);
    // 设置引脚高电平
    GPIOErrCode (*pinSet)(GPIOPortEnum, GPIOPinEnum);
    // 设置引脚低电平
    GPIOErrCode (*pinReset)(GPIOPortEnum, GPIOPinEnum);
    // 切换引脚电平
    GPIOErrCode (*pinToggle)(GPIOPortEnum, GPIOPinEnum);
    // 设置引脚电平
    GPIOErrCode (*pinWrite)(GPIOPortEnum, GPIOPinEnum, GPIOPinStateEnum);
    // 读取引脚电平
    uint8_t (*pinRead)(GPIOPortEnum, GPIOPinEnum);
} GPIOIntfTypeDef;




/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/

extern GPIOIntfTypeDef gpioIntf; // GPIO接口实例




/*-------- function prototypes ---------------------------------------------------------------------------------------*/





#endif /* __GPIO_H */
