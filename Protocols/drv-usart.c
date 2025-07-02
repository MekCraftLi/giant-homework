/**
 ***********************************************************************************************************************
 * @file           : drv-usart.c
 * @brief          : 串口驱动类
 * @author         : 李嘉豪
 * @date           : 2025-04-24
 ***********************************************************************************************************************
 * @attention
 *
 * 串口驱动类，实现串口的初始化、发送、接收等功能
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "../Peripherals/gpio.h"
#include "drv-usart.h"
#include "stm32f10x_usart.h"
#include <stdlib.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

USARTErrCode usartInit(USARTObjTypeDef* usartObj, USART_TypeDef* handler, uint32_t baudRate, uint16_t txBufferSize,
                       uint16_t rxBufferSize, uint8_t IDLE_IT);





/* ------- variables -------------------------------------------------------------------------------------------------*/

static USARTObjTypeDef* irqObj[3] = {0}; // 串口中断回调函数注册
static uint8_t irqObjIndex        = 0;   // 串口对象索引

USARTIntfTypeDef usartIntf        = {
           .init = usartInit,
};



/* ------- function implement ----------------------------------------------------------------------------------------*/


/**
 * @brief 串口初始化
 *
 * @param usartObj
 * @param baudRate
 * @param txBuffer
 * @param txBufferSize
 * @param rxBuffer
 * @param rxBufferSize
 * @param IDLE_IT
 * @return USARTErrCode
 */

USARTErrCode usartInit(USARTObjTypeDef* usartObj, USART_TypeDef* handler, uint32_t baudRate, uint16_t txBufferSize,
                       uint16_t rxBufferSize, uint8_t IDLE_IT) {

    /* 1. 参数检查 */


    if (usartObj == NULL) {
        return USART_ERROR;
    }
    if (handler == NULL) {
        return USART_ERROR;
    }
    if (txBufferSize == 0 || rxBufferSize == 0) {
        return USART_ERROR;
    }

    /* 2. 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE); // 使能GPIOA和USART时钟

    /* 3. 配置GPIO */
    gpioIntf.pinInit(PORT_A, PIN_9, ALT_OUTPUT_PUSH_PULL); // PA9设置成为复用推挽输出

    gpioIntf.pinInit(PORT_A, PIN_10, INPUT_FLOATING); // PA10设置成为浮空输入

    /* 4. 配置USART对象 */

    usartObj->handler      = handler;
    usartObj->txBuffer     = (uint8_t*)malloc(txBufferSize);
    usartObj->rxBuffer     = (uint8_t*)malloc(rxBufferSize);
    usartObj->txBufferSize = txBufferSize;
    usartObj->rxBufferSize = rxBufferSize;
    usartObj->txIndex      = 0;
    usartObj->rxIndex      = 0;
    usartObj->recvLen      = 0;

    /* 5. 注册串口对象到中断回调会调用的数组中去 */
    irqObj[irqObjIndex++]  = usartObj; // 将串口对象注册到中断处理函数数组


    /* 6. 配置USART的寄存器 */
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate            = baudRate;
    USART_InitStruct.USART_WordLength          = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits            = USART_StopBits_1;
    USART_InitStruct.USART_Parity              = USART_Parity_No;
    USART_InitStruct.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控制
    USART_Init(USART1, &USART_InitStruct);

    /* 7. 使能USART */
    USART_Cmd(USART1, ENABLE);

    /* 8. 使能USART的中断 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    if (IDLE_IT) {
        USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    }


    /* 9. 配置NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;

    switch ((uint32_t)handler) {
        case (uint32_t)USART1:
            NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
            break;
        case (uint32_t)USART2:
            NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
            break;
        case (uint32_t)USART3:
            NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
            break;
        default:
            return USART_ERROR;
    }

    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    return USART_SUCCESS;
}

/**
 * @brief 空闲中断处理
 *
 * @param usartObj
 */
static void usartReceiveFromIDLE(USARTObjTypeDef* usartObj) {
    uint8_t temp;
    if (USART_GetITStatus(usartObj->handler, USART_IT_IDLE) != RESET) {
        /* 空闲中断 */
        temp = usartObj->handler->SR;
        temp = usartObj->handler->DR;
        if (usartObj->rxIndex > 0) {
            usartObj->recvLen = usartObj->rxIndex;
            usartObj->rxIndex = 0;
        }
    } else if (USART_GetITStatus(usartObj->handler, USART_IT_RXNE)) {
        temp = USART_ReceiveData(usartObj->handler);
        if (usartObj->rxIndex < usartObj->rxBufferSize) {
            usartObj->rxBuffer[usartObj->rxIndex++] = temp;
        } else {
            usartObj->rxIndex = 0;
        }
    }
}

/**
 * @brief USART 中断处理
 *
 */
void USART1_IRQHandler(void) {
    for (uint8_t i = 0; i < 3; i++) {
        if (irqObj[i] != NULL) {
            usartReceiveFromIDLE(irqObj[i]);
        }
    }
}
