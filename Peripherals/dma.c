/**
 ***********************************************************************************************************************
 * @file           : dma.c
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




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "dma.h"
#include <stdlib.h>




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/

#define IS_PERIPH_ADDR(addr) ((addr) >= 0x40000000 && (addr) <= 0x500FFFFF)




/* ------- function prototypes ---------------------------------------------------------------------------------------*/

DMAErrCode init(DMAObjTypeDef* obj, DMA_Channel_TypeDef* channel, uint32_t priority);
DMAErrCode setSorce(DMAObjTypeDef* obj, uint32_t srcAddr, DMASizeEnum srcSize, uint8_t srcLen);
DMAErrCode setSorceCycle(DMAObjTypeDef* obj, uint32_t srcAddr, DMASizeEnum srcSize, uint8_t srcLen);
DMAErrCode setDest(DMAObjTypeDef* obj, uint32_t dstAddr, DMASizeEnum dstSize, uint8_t dstLen);
DMAErrCode start(DMAObjTypeDef* obj);
DMAErrCode stop(DMAObjTypeDef* obj);
DMAErrCode reset(DMAObjTypeDef* obj);




/* ------- variables -------------------------------------------------------------------------------------------------*/

DMAIntfTypeDef dmaIntf = {
    .init          = init,
    .setSorce      = setSorce,
    .setSorceCycle = setSorceCycle,
    .setDest       = setDest,
    .start         = start,
    .stop          = stop,
    .reset         = reset,
};




/* ------- function implement ----------------------------------------------------------------------------------------*/

/**
 * @brief 初始化DMA对象
 *
 * @param obj
 * @param channel
 * @param priority
 * @return DMAErrCode
 */
DMAErrCode init(DMAObjTypeDef* obj, DMA_Channel_TypeDef* channel, uint32_t priority) {
    // 检查参数
    if (channel == NULL) {
        return DMA_ERR_PARAM;
    }
    if (priority != DMA_Priority_VeryHigh && priority != DMA_Priority_High && priority != DMA_Priority_Medium &&
        priority != DMA_Priority_Low) {
        return DMA_ERR_PARAM;
    }
    uint32_t DMAPort = 0;
    if ((uint32_t)channel - AHBPERIPH_BASE <= 0x80) {
        DMAPort = RCC_AHBPeriph_DMA1; // DMA1通道
    } else if ((uint32_t)channel - AHBPERIPH_BASE < 0x0458) {
        DMAPort = RCC_AHBPeriph_DMA2; // DMA2通道
    } else {
        return DMA_ERR_PARAM; // 无效的DMA通道
    }

    RCC_AHBPeriphClockCmd(DMAPort, ENABLE); // 使能DMA时钟
    obj->channel  = channel;
    obj->priority = priority;
    return DMA_SUCCESS;
}

/**
 * @brief 设置DMA源地址和数据大小
 *
 * @param obj
 * @param srcAddr
 * @param srcSize
 * @param srcLen
 * @return DMAErrCode
 */
DMAErrCode setSorce(DMAObjTypeDef* obj, uint32_t srcAddr, DMASizeEnum srcSize, uint8_t srcLen) {

    // 检查参数
    if (srcSize < DMA_SIZE_BYTE || srcSize > DMA_SIZE_WORD || srcLen == 0) {
        return DMA_ERR_PARAM;
    }
    // 检查地址是否有效
    if (srcAddr == NULL) {
        return DMA_ERR_ADDR_INVALID;
    }


    obj->srcAddr     = srcAddr;
    obj->srcSize     = srcSize;
    obj->srcLen      = srcLen;
    obj->srcAddrType = IS_PERIPH_ADDR(srcAddr) ? DMA_ADDR_TYPE_PERIPHERAL : DMA_ADDR_TYPE_MEMORY;

    return DMA_SUCCESS;
}


/**
 * @brief 设置DMA源地址和数据大小，并启用循环模式
 *
 * @param obj
 * @param srcAddr
 * @param srcSize
 * @param srcLen
 * @return DMAErrCode
 */
DMAErrCode setSorceCycle(DMAObjTypeDef* obj, uint32_t srcAddr, DMASizeEnum srcSize, uint8_t srcLen) {
    // 检查参数
    if (srcSize < DMA_SIZE_BYTE || srcSize > DMA_SIZE_WORD || srcLen == 0) {
        return DMA_ERR_PARAM;
    }
    // 检查地址是否有效
    if (srcAddr == NULL) {
        return DMA_ERR_ADDR_INVALID;
    }

    obj->srcAddr     = srcAddr;
    obj->srcSize     = srcSize;
    obj->srcLen      = srcLen;
    obj->srcAddrType = IS_PERIPH_ADDR(srcAddr) ? DMA_ADDR_TYPE_PERIPHERAL : DMA_ADDR_TYPE_MEMORY;

    obj->cycleMode   = 1; // 设置为循环模式

    return DMA_SUCCESS;
}


/**
 * @brief 设置DMA目的地址和数据大小
 *
 * @param obj
 * @param dstAddr
 * @param dstSize
 * @param dstLen
 * @return DMAErrCode
 */
DMAErrCode setDest(DMAObjTypeDef* obj, uint32_t dstAddr, DMASizeEnum dstSize, uint8_t dstLen) {

    // 检查参数
    if (dstSize < DMA_SIZE_BYTE || dstSize > DMA_SIZE_WORD) {
        return DMA_ERR_PARAM;
    }
    if (dstLen == 0) {
        return DMA_ERR_PARAM; // 目的数据长度不能为0
    }
    // 检查地址是否有效
    if (dstAddr == NULL) {
        return DMA_ERR_ADDR_INVALID;
    } else if (obj->srcAddrType == DMA_ADDR_TYPE_PERIPHERAL && IS_PERIPH_ADDR(dstAddr)) {
        return DMA_ERR_ADDR_INVALID; // 源地址和目的地址不能都是外设地址
    }

    obj->dstAddr     = dstAddr;
    obj->dstSize     = dstSize;
    obj->dstAddrType = IS_PERIPH_ADDR(dstAddr) ? DMA_ADDR_TYPE_PERIPHERAL : DMA_ADDR_TYPE_MEMORY;
    obj->dstLen      = dstLen;

    return DMA_SUCCESS;
}


/**
 * @brief * 启动DMA传输
 *
 * @param obj
 * @return DMAErrCode
 */
DMAErrCode start(DMAObjTypeDef* obj) {
    // 1.检查DMA通道是否存在
    if (obj->channel == NULL) {
        return DMA_ERR_PARAM;
    }
    // 2.检查地址是否有效
    if (obj->srcAddr == NULL || obj->dstAddr == NULL) {
        return DMA_ERR_ADDR_INVALID;
    } else if (obj->srcAddrType == obj->dstAddrType == DMA_ADDR_TYPE_PERIPHERAL) {
        return DMA_ERR_ADDR_INVALID;
    }

    // 3.配置DMA通道
    DMA_InitTypeDef DMA_InitStruct;
    if (obj->srcAddrType == DMA_ADDR_TYPE_PERIPHERAL) {
        if (obj->srcLen == 1 || obj->dstLen == 1) {
            obj->CNDTR = DMA_InitStruct.DMA_BufferSize =
                obj->srcLen > obj->dstLen ? obj->srcLen : obj->dstLen; // 传输数据长度取源和目的地址的最大长度
        } else {
            obj->CNDTR = DMA_InitStruct.DMA_BufferSize =
                obj->srcLen > obj->dstLen ? obj->dstLen : obj->srcLen; // 传输数据长度取源和目的地址的最小长度
        }

        obj->CPAR                             = obj->srcAddr; // 外设地址
        obj->CMAR                             = obj->dstAddr; // 内存地址
        DMA_InitStruct.DMA_PeripheralBaseAddr = obj->srcAddr;
        DMA_InitStruct.DMA_MemoryBaseAddr     = obj->dstAddr; // 目的地址
        DMA_InitStruct.DMA_PeripheralInc      = obj->srcLen > 0 ? DMA_PeripheralInc_Enable : DMA_PeripheralInc_Disable;
        DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralSRC;                      // 外设为源地址
        DMA_InitStruct.DMA_M2M                = DMA_M2M_Disable;                            // 非内存到内存传输
        DMA_InitStruct.DMA_MemoryInc          = obj->dstLen > 0;                            // 内存地址递增
        DMA_InitStruct.DMA_MemoryDataSize     = (obj->dstSize * 2) << 2;                    // 目的数据大小
        DMA_InitStruct.DMA_PeripheralDataSize = obj->srcSize << 2;                          // 源数据大小
        DMA_InitStruct.DMA_Mode     = obj->cycleMode ? DMA_Mode_Circular : DMA_Mode_Normal; // 循环模式或正常模式
        DMA_InitStruct.DMA_Priority = obj->priority;                                        // 优先级

    } else {
        obj->CPAR = obj->dstAddr; // 外设地址
        obj->CMAR = obj->srcAddr; // 内存地址
        if (obj->srcLen == 1 || obj->dstLen == 1) {
            obj->CNDTR = DMA_InitStruct.DMA_BufferSize =
                obj->srcLen > obj->dstLen ? obj->srcLen : obj->dstLen; // 传输数据长度取源和目的地址的最大长度
        } else {
            obj->CNDTR = DMA_InitStruct.DMA_BufferSize =
                obj->srcLen > obj->dstLen ? obj->dstLen : obj->srcLen; // 传输数据长度取源和目的地址的最小长度
        }

        if (obj->dstAddrType == DMA_ADDR_TYPE_MEMORY) {
            DMA_InitStruct.DMA_M2M = DMA_M2M_Enable; // 内存到内存传输
        } else {
            DMA_InitStruct.DMA_M2M = DMA_M2M_Disable; // 非内存到内存传输
        }
        DMA_InitStruct.DMA_PeripheralBaseAddr = obj->dstAddr; // 源地址
        DMA_InitStruct.DMA_MemoryBaseAddr     = obj->srcAddr; // 目的地址
        DMA_InitStruct.DMA_PeripheralInc      = obj->dstLen > 0 ? DMA_PeripheralInc_Enable : DMA_PeripheralInc_Disable;
        DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralDST;                                 // 外设为目的地址
        DMA_InitStruct.DMA_MemoryInc = obj->srcLen > 0 ? DMA_MemoryInc_Enable : DMA_MemoryInc_Disable; // 内存地址递增
        DMA_InitStruct.DMA_MemoryDataSize     = obj->srcSize;                                          // 源数据大小
        DMA_InitStruct.DMA_PeripheralDataSize = obj->dstSize;                                          // 目的数据大小
        DMA_InitStruct.DMA_Mode     = obj->cycleMode ? DMA_Mode_Circular : DMA_Mode_Normal; // 循环模式或正常模式
        DMA_InitStruct.DMA_Priority = obj->priority;                                        // 优先级
    }
    // 4.初始化DMA通道
    DMA_Init(obj->channel, &DMA_InitStruct);

    // 5.使能DMA通道
    DMA_Cmd(obj->channel, ENABLE);
}

/**
 *@brief * 停止DMA传输
 *
 * @param obj
 * @return DMAErrCode
 */
DMAErrCode stop(DMAObjTypeDef* obj) {
    // 检查参数
    if (obj->channel == NULL) {
        return DMA_ERR_PARAM;
    }

    // 禁用DMA通道
    DMA_Cmd(obj->channel, DISABLE);

    // 清除DMA通道的所有标志位
    DMA_ClearFlag(obj->channel->CCR);

    return DMA_SUCCESS;
}

/**
 * @brief * 重置DMA对象
 *
 * @param obj
 * @return DMAErrCode
 */
DMAErrCode reset(DMAObjTypeDef* obj) {
    // 检查参数
    if (obj->channel == NULL) {
        return DMA_ERR_PARAM;
    }

    // 禁用DMA通道
    DMA_Cmd(obj->channel, DISABLE);

    obj->channel->CPAR  = obj->CPAR;
    obj->channel->CMAR  = obj->CMAR;
    obj->channel->CNDTR = obj->CNDTR;

    return DMA_SUCCESS;
}
