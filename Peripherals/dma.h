/**
 ***********************************************************************************************************************
 * @file           : dma.h
 * @brief          : DMA配置文件
 * @author         : 李嘉豪
 * @date           : 2025-07-01
 ***********************************************************************************************************************
 * @attention
 *
 * DMA对象的定义与属性和方法的实现
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __DMA_H
#define __DMA_H




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "stm32f10x.h"
#include "stm32f10x_dma.h"




/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef enum {
    DMA_SUCCESS,          // 成功
    DMA_ERR_TIMEOUT,      // 超时
    DMA_ERR_BUSY,         // 忙
    DMA_ERR_PARAM,        // 参数错误
    DMA_ERR_ADDR_INVALID, // 地址无效
} DMAErrCode;

typedef enum {
    DMA_SIZE_BYTE,      // 字节
    DMA_SIZE_HALF_WORD, // 半字
    DMA_SIZE_WORD,      // 字
} DMASizeEnum;


typedef enum {
    DMA_ADDR_TYPE_PERIPHERAL = 0x22, // 外设地址
    DMA_ADDR_TYPE_MEMORY,            // 内存地址
} DMAAddrTypeEnum;

typedef struct {
    DMA_Channel_TypeDef* channel; // DMA通道
    uint32_t srcAddr;             // 源地址
    DMASizeEnum srcSize;          // 源数据大小
    uint16_t srcLen;              // 源数据长度
    DMAAddrTypeEnum srcAddrType;  // 源地址类型

    uint32_t dstAddr;            // 目的地址
    DMASizeEnum dstSize;         // 目的数据大小
    uint16_t dstLen;             // 目的数据长度
    DMAAddrTypeEnum dstAddrType; // 目的地址类型

    uint32_t CPAR;  // 外设地址
    uint32_t CMAR;  // 内存地址
    uint32_t CNDTR; // 传输数据长度

    uint8_t cycleMode; // 循环模式

    uint32_t priority; // 优先级

    void (*callback)(void*); // 回调函数指针
} DMAObjTypeDef;





/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- function prototypes ---------------------------------------------------------------------------------------*/

typedef struct {
    DMAErrCode (*init)(DMAObjTypeDef* dmaObj, DMA_Channel_TypeDef* channel, uint32_t priority);
    DMAErrCode (*setSorce)(DMAObjTypeDef*, uint32_t srcAddr, DMASizeEnum srcSize, uint16_t srcLen);
    DMAErrCode (*setSorceCycle)(DMAObjTypeDef*, uint32_t srcAddr, DMASizeEnum srcSize, uint16_t srcLen);
    DMAErrCode (*setDest)(DMAObjTypeDef*, uint32_t dstAddr, DMASizeEnum, uint8_t dstLen);
    DMAErrCode (*start)(DMAObjTypeDef* dmaObj);
    DMAErrCode (*stop)(DMAObjTypeDef* dmaObj);
    DMAErrCode (*reset)(DMAObjTypeDef* dmaObj);
    DMAErrCode (*configISR)(DMAObjTypeDef* dmaObj);
} DMAIntfTypeDef;




/*-------- variables -------------------------------------------------------------------------------------------------*/

extern DMAIntfTypeDef dmaIntf;




#endif /* __DMA_H */
