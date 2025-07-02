/**
 ***********************************************************************************************************************
 * @file           : gpio.c
 * @brief          : 通用输入输出配置文件
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

#include "gpio.h"




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/

#define NULL                   0x00

#define IDR_OFFSET             0x08
#define ODR_OFFSET             0x0C




/* ------- macro -----------------------------------------------------------------------------------------------------*/

#define BITBAND(addr, bitnum)  ((addr & 0xF0000000) + 0x02000000 + ((addr & 0x000FFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr)         *((volatile unsigned long*)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))




/* ------- function prototypes ---------------------------------------------------------------------------------------*/

static void gpioModeInit(GPIO_TypeDef* port, uint16_t pin, GPIOMode_TypeDef mode);

static void gpioModeInitWithEXTI(GPIO_TypeDef* port, uint16_t pin, GPIOMode_TypeDef mode,
                                 EXTITrigger_TypeDef extiTrigger);

GPIOErrCode pinInit(GPIOPortEnum port, GPIOPinEnum pin, GPIOModeEnum mode);
GPIOErrCode pinInitWithEXTI(GPIOPortEnum port, GPIOPinEnum pin, GPIOModeEnum mode, EdgeModeEnum extiTrigger);
GPIOErrCode pinSet(GPIOPortEnum port, GPIOPinEnum pin);
GPIOErrCode pinReset(GPIOPortEnum port, GPIOPinEnum pin);
GPIOErrCode pinToggle(GPIOPortEnum port, GPIOPinEnum pin);
GPIOErrCode writePin(GPIOPortEnum port, GPIOPinEnum pin, GPIOPinStateEnum state);
uint8_t readPin(GPIOPortEnum port, GPIOPinEnum pin);
GPIO_TypeDef* getPort(GPIOPortEnum port);
static GPIOErrCode enablePort(GPIOPortEnum port);

/* ------- variables -------------------------------------------------------------------------------------------------*/

GPIOIntfTypeDef gpioIntf = {
    .pinInit         = pinInit,
    .pinInitWithEXTI = pinInitWithEXTI,
    .pinSet          = pinSet,
    .pinReset        = pinReset,
    .pinToggle       = pinToggle,
    .pinWrite        = writePin,
    .pinRead         = readPin,
};




/* ------- function implement ----------------------------------------------------------------------------------------*/


static GPIOErrCode enablePort(GPIOPortEnum port) {
    // 使能GPIO时钟
    switch (port) {
        case PORT_A:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            break;
        case PORT_B:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
            break;
        case PORT_C:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
            break;
        case PORT_D:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
            break;
        case PORT_E:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
            break;
        case PORT_F:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
            break;
        case PORT_G:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
            break;
        default:
            return PORT_NOT_EXIST;
    }
    return GPIO_SUCCESS;
}

/**
 * @brief GPIO引脚模式初始化
 *
 * @param port
 * @param pin
 * @param mode
 */
static void gpioModeInit(GPIO_TypeDef* port, uint16_t pin, GPIOMode_TypeDef mode) {

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin   = pin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = mode;
    GPIO_Init(port, &GPIO_InitStruct);
}

/**
 *@brief GPIO引脚与EXIT初始化
 *
 * @param port
 * @param pin
 * @param mode
 * @param extiMode
 * @param extiTrigger
 */
static void gpioModeInitWithEXTI(GPIO_TypeDef* port, uint16_t pin, GPIOMode_TypeDef mode,
                                 EXTITrigger_TypeDef extiTrigger) {
    // 1.配置GPIO引脚
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin   = pin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = mode;
    GPIO_Init(port, &GPIO_InitStruct);
    uint8_t extiPinSource = 0;
    uint8_t portSource    = 0;

    for (extiPinSource = 0; extiPinSource < 20; extiPinSource++) {
        if (pin == (1 << extiPinSource)) {
            break;
        }
    }

    if (extiPinSource == 20) {
        return;
    }

    // 2.配置EXTI线路
    portSource = ((((uint32_t)port - APB2PERIPH_BASE) >> 8) - 0x08) / 4;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    if (portSource > 7) {
        return;
    }

    GPIO_EXTILineConfig(portSource, extiPinSource);
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line    = pin;
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = extiTrigger;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 3.配置NVIC
    NVIC_InitTypeDef NVIC_InitStruct;

    if (pin < 0x0010) {
        NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn + extiPinSource;
    } else if (pin >= 0x0010 && pin < 0x0200) {
        NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    } else {
        NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    }

    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 *@brief 获取GPIO端口
 *
 * @param port
 * @return GPIO_TypeDef*
 */
inline GPIO_TypeDef* getPort(GPIOPortEnum port) {

    if (port > PORT_G) {
        return (GPIO_TypeDef*)NULL;
    }

    return (GPIO_TypeDef*)(0x40010800 + 0x400 * (uint32_t)port);
}

/**
 *@brief GPIO引脚初始化对外接口
 *
 * @param port
 * @param pin
 * @param mode
 * @return GPIOErrCode
 */
GPIOErrCode pinInit(GPIOPortEnum port, GPIOPinEnum pin, GPIOModeEnum mode) {
    // 1.检查参数

    if (pin > PIN_15) {
        // 引脚不存在
        return PIN_NOT_EXIST;
    }

    GPIO_TypeDef* GPIOx = getPort(port);
    if (GPIOx == NULL) {
        // 端口不存在
        return PORT_NOT_EXIST;
    }

    enablePort(port);

    uint16_t pinBit = 1 << (uint8_t)pin;

    // 之前写好的GPIO初始化函数
    gpioModeInit(GPIOx, pinBit, (GPIOMode_TypeDef)mode);

    return GPIO_SUCCESS;
}


/**
 * @brief GPIO引脚和EXTI初始化对外接口
 *
 * @param port
 * @param pin
 * @param mode
 * @return GPIOErrCode
 */
GPIOErrCode pinInitWithEXTI(GPIOPortEnum port, GPIOPinEnum pin, GPIOModeEnum mode, EdgeModeEnum extiTrigger) {
    // 1.检查参数

    if (pin > PIN_15) {
        // 引脚不存在
        return PIN_NOT_EXIST;
    }

    GPIO_TypeDef* GPIOx = getPort(port);
    if (GPIOx == NULL) {
        // 端口不存在
        return PORT_NOT_EXIST;
    }
    enablePort(port);

    uint16_t pinBit = pin << (uint8_t)pin;

    // 之前写好的GPIO初始化函数
    gpioModeInitWithEXTI(GPIOx, pinBit, (GPIOMode_TypeDef)mode, (EXTITrigger_TypeDef)extiTrigger);

    return GPIO_SUCCESS;
}
/**
 * @brief GPIO引脚设置高电平
 *
 * @param port
 * @param pin
 * @return GPIOErrCode
 */
GPIOErrCode pinSet(GPIOPortEnum port, GPIOPinEnum pin) {
    if (pin > PIN_15) {
        // 引脚不存在
        return PIN_NOT_EXIST;
    }
    uint32_t GPIOx = (uint32_t)getPort(port);
    if (GPIOx == NULL) {
        // 端口不存在
        return PORT_NOT_EXIST;
    }

    BIT_ADDR(GPIOx + ODR_OFFSET, pin) = 1;

    return GPIO_SUCCESS;
}


/**
 * @brief GPIO引脚设置低电平
 *
 * @param port
 * @param pin
 * @return GPIOErrCode
 */
GPIOErrCode pinReset(GPIOPortEnum port, GPIOPinEnum pin) {
    if (pin > PIN_15) {
        // 引脚不存在
        return PIN_NOT_EXIST;
    }
    uint32_t GPIOx = (uint32_t)getPort(port);
    if (GPIOx == NULL) {
        // 端口不存在
        return PORT_NOT_EXIST;
    }

    BIT_ADDR(GPIOx + ODR_OFFSET, pin) = 0;

    return GPIO_SUCCESS;
}

uint32_t debug_addr;

/**
 * @brief GPIO引脚切换电平
 *
 * @param port
 * @param pin
 * @return GPIOErrCode
 */
GPIOErrCode pinToggle(GPIOPortEnum port, GPIOPinEnum pin) {
    if (pin > PIN_15) {
        // 引脚不存在
        return PIN_NOT_EXIST;
    }
    uint32_t GPIOx = (uint32_t)getPort(port);
    if (GPIOx == NULL) {
        // 端口不存在
        return PORT_NOT_EXIST;
    }

    debug_addr = !BIT_ADDR(GPIOx + IDR_OFFSET, pin);


    BIT_ADDR(GPIOx + ODR_OFFSET, pin) = !BIT_ADDR(GPIOx + IDR_OFFSET, pin);

    return GPIO_SUCCESS;
}

/**
 * @brief GPIO引脚设置电平
 *
 * @param port
 * @param pin
 * @param state
 * @return GPIOErrCode
 */

GPIOErrCode writePin(GPIOPortEnum port, GPIOPinEnum pin, GPIOPinStateEnum state) {
    if (pin > PIN_15) {
        // 引脚不存在
        return PIN_NOT_EXIST;
    }
    uint32_t GPIOx = (uint32_t)getPort(port);
    if (GPIOx == NULL) {
        // 端口不存在
        return PORT_NOT_EXIST;
    }

    BIT_ADDR(GPIOx + ODR_OFFSET, pin) = state;

    return GPIO_SUCCESS;
}

/**
 * @brief GPIO引脚读取电平
 *
 * @param port
 * @param pin
 * @return GPIOPinStateEnum
 */
uint8_t readPin(GPIOPortEnum port, GPIOPinEnum pin) {
    uint32_t GPIOx = (uint32_t)getPort(port);
    return BIT_ADDR(GPIOx + IDR_OFFSET, pin);
}
