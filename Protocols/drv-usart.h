/**
 ***********************************************************************************************************************
 * @file           : drv-usart.h
 * @brief          : 串口驱动类
 * @author         : 李嘉豪
 * @date           : 2025-04-24
 ***********************************************************************************************************************
 * @attention
 *
 * 串口驱动类，实现串口的初始化、发送、接收等功能，头文件中定义串口驱动类的属性和方法
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __DRV_USART_H
#define __DRV_USART_H




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include <stdint.h>
#include "stm32f10x_usart.h"





/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef enum {
    USART_SUCCESS = 0x00, // 成功
    USART_ERROR   = 0x01, // 错误
} USARTErrCode;

typedef struct {
    USART_TypeDef* handler; // 串口句柄
    uint8_t* txBuffer;      // 发送缓冲区
    uint8_t* rxBuffer;      // 接收缓冲区
    uint16_t txBufferSize;  // 发送缓冲区大小
    uint16_t rxBufferSize;  // 接收缓冲区大小
    uint16_t txIndex;       // 发送索引
    uint16_t rxIndex;       // 接收索引
    uint16_t recvLen;       // 接收长度

} USARTObjTypeDef;

typedef struct {
    USARTErrCode (*init)(USARTObjTypeDef* usartObj, USART_TypeDef* handler, uint32_t baudRate,
                         uint16_t txBufferSize, uint16_t rxBufferSize, uint8_t IDLE_IT);
    USARTErrCode (*send)(USARTObjTypeDef* usartObj, uint8_t* data, uint16_t size);
} USARTIntfTypeDef;




/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/

extern USARTIntfTypeDef usartIntf; // 串口驱动对外接口




/*-------- function prototypes ---------------------------------------------------------------------------------------*/




#endif /* __DRV_USART_H */
