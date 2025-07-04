/**
 ***********************************************************************************************************************
 * @file           : drv-iic.c
 * @brief          : IIC驱动类
 * @author         : 李嘉豪
 * @date           : 2025-04-24
 ***********************************************************************************************************************
 * @attention
 *
 * None
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "../Services/time-service.h"
#include "drv-iic.h"
#include <stdlib.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/

#define TIMEOUT 0




/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

IICErrCode iicInit(IICObjTypeDef* iicObj, IICImplTypeEnum type, GPIOPortEnum SDA, GPIOPinEnum SDA_Pin, GPIOPortEnum SCL,
                   GPIOPinEnum SCL_Pin, uint16_t txBufferSize, uint16_t rxBufferSize, uint16_t timeoutMs,
                   uint32_t speed);
IICErrCode iicSend(IICObjTypeDef* iicObj);
IICErrCode iicTxEquipWithDMA(IICObjTypeDef* iicObj);
IICErrCode iicSendWithDMA(IICObjTypeDef* iicObj);





/* ------- variables -------------------------------------------------------------------------------------------------*/

IICIntfTypeDef iicIntf = {
    .init            = iicInit,
    .transmit        = iicSend,
    .equippedWithDMA = iicTxEquipWithDMA,
    .sendWithDMA     = iicSendWithDMA,
};

extern uint16_t debug_errCnt;





/* ------- function implement ----------------------------------------------------------------------------------------*/

/**
 * @brief IIC初始化
 *
 * @param iicObj
 * @param SDA
 * @param SDA_Pin
 * @param SCL
 * @param SCL_Pin
 * @param txBufferSize
 * @param rxBufferSize
 * @return IICErrCode
 */
IICErrCode iicInit(IICObjTypeDef* iicObj, IICImplTypeEnum type, GPIOPortEnum SDA, GPIOPinEnum SDA_Pin, GPIOPortEnum SCL,
                   GPIOPinEnum SCL_Pin, uint16_t txBufferSize, uint16_t rxBufferSize, uint16_t timeoutUs,
                   uint32_t speed) {
    /* 1. 参数检查 */
    if (iicObj == NULL) {
        return IIC_ERR_PARAM;
    }
    if (txBufferSize == 0 || rxBufferSize == 0) {
        return IIC_ERR_PARAM;
    }
    if (speed <= 0 || speed > 400000) {
        return IIC_ERR_PARAM;
    }

    if (!(iicObj->txBuffer = (uint8_t*)malloc(txBufferSize))) {
        return IIC_ERR_MEM_ALLOC_FAIL;
    }
    if (!(iicObj->rxBuffer = (uint8_t*)malloc(rxBufferSize))) {
        return IIC_ERR_MEM_ALLOC_FAIL;
    }

    iicObj->timeoutUs    = timeoutUs;
    iicObj->speed        = speed;
    iicObj->txBufferSize = txBufferSize;
    iicObj->rxBufferSize = rxBufferSize;
    iicObj->txIndex      = 0;
    iicObj->rxIndex      = 0;



    if (type == IIC_SOFTWARE) {
        iicObj->type = IIC_SOFTWARE; // 设置IIC类型为软件IIC

        //    /* 2. 使能时钟 */
        //    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); // 使能GPIOA和GPIOB时钟

        /* 3. 配置GPIO */
        gpioIntf.pinInit(SDA, SDA_Pin, OUTPUT_OPEN_DRAIN); // SDA设置成为开漏输出
        gpioIntf.pinInit(SCL, SCL_Pin, OUTPUT_OPEN_DRAIN); // SCL设置成为开漏输出

        /* 4. 配置IIC对象 */
        iicObj->SDAPort = SDA;
        iicObj->SDAPin  = SDA_Pin;
        iicObj->SCLPort = SCL;
        iicObj->SCLPin  = SCL_Pin;


        /* 5. 初始状态释放总线 */
        gpioIntf.pinSet(iicObj->SCLPort, iicObj->SCLPin); // SCL拉高
        gpioIntf.pinSet(iicObj->SDAPort, iicObj->SDAPin); // SDA拉高
        return IIC_SUCCESS;
    } else {
        /* 2. 使能时钟 */
        iicObj->type = type; // 设置IIC类型为硬件IIC

        gpioIntf.pinInit(SCL, SCL_Pin, ALT_OUTPUT_OPEN_DRAIN);
        gpioIntf.pinInit(SDA, SDA_Pin, ALT_OUTPUT_OPEN_DRAIN);



        if (type == IIC_HARDWARE_1) {
            iicObj->i2c = I2C1;
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
        } else if (type == IIC_HARDWARE_2) {
            iicObj->i2c = I2C2;
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
        } else {
            return IIC_ERR_PARAM; // 无效的IIC类型
        }

        if (speed > 100000) {
            RCC_PCLK1Config(RCC_HCLK_Div2);
        }

        I2C_SoftwareResetCmd(iicObj->i2c, ENABLE);
        I2C_SoftwareResetCmd(iicObj->i2c, DISABLE);

        I2C_InitTypeDef i2cstruct;
        i2cstruct.I2C_Mode                = I2C_Mode_I2C;
        i2cstruct.I2C_DutyCycle           = I2C_DutyCycle_2;
        i2cstruct.I2C_OwnAddress1         = 0x00;
        i2cstruct.I2C_Ack                 = I2C_Ack_Enable;
        i2cstruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
        i2cstruct.I2C_ClockSpeed          = speed;
        I2C_Init(I2C1, &i2cstruct);

        I2C_Cmd(I2C1, ENABLE);

        return IIC_SUCCESS;
    }
}


static __inline void iicDelay(IICObjTypeDef* iicObj) { timeServIntf.delayUs(1); }

/**
 *@brief 发送START信号
 *
 * @param iicObj
 */
static IICErrCode iicStart(IICObjTypeDef* iicObj) {

    GPIOPortEnum DPort = iicObj->SDAPort;
    GPIOPortEnum CPort = iicObj->SCLPort;
    GPIOPinEnum DPin   = iicObj->SDAPin;
    GPIOPinEnum CPin   = iicObj->SCLPin;

    gpioIntf.pinSet(DPort, DPin); // 数据线↑

    iicDelay(iicObj);

    gpioIntf.pinSet(CPort, CPin); // 时钟线↑

    uint32_t timeout = iicObj->timeoutUs;

    /* 检查总线是否空闲 */
    while (!gpioIntf.pinRead(DPort, DPin)) {
        if (timeout-- == 0) {

            return IIC_ERR_BUSY;
        }

        iicDelay(iicObj);
    }

    /* 发送START信号 */
    gpioIntf.pinReset(DPort, DPin); // 数据线↓
    iicDelay(iicObj);
    gpioIntf.pinReset(CPort, CPin); // 时钟线↓
    iicDelay(iicObj);

    return IIC_SUCCESS;
}

/**
 *@brief IIC发送STOP信号
 *
 * @param iicObj
 */
static IICErrCode iicStop(IICObjTypeDef* iicObj) {
    GPIOPortEnum DPort = iicObj->SDAPort;
    GPIOPortEnum CPort = iicObj->SCLPort;
    GPIOPinEnum DPin   = iicObj->SDAPin;
    GPIOPinEnum CPin   = iicObj->SCLPin;

    gpioIntf.pinReset(DPort, DPin); // 数据线↓
    iicDelay(iicObj);
    gpioIntf.pinSet(CPort, CPin); // 时钟线↑
    iicDelay(iicObj);
    gpioIntf.pinSet(DPort, DPin); // 数据线↑
    iicDelay(iicObj);

    return IIC_SUCCESS;
}

/**
 *@brief IIC等待ACK
 *
 * @param iicObj
 * @return IICErrCode
 */
static IICErrCode iicWaitAck(IICObjTypeDef* iicObj) {
    GPIOPortEnum DPort = iicObj->SDAPort;
    GPIOPortEnum CPort = iicObj->SCLPort;
    GPIOPinEnum DPin   = iicObj->SDAPin;
    GPIOPinEnum CPin   = iicObj->SCLPin;

    gpioIntf.pinSet(DPort, DPin); // 数据线↑
    iicDelay(iicObj);
    gpioIntf.pinSet(CPort, CPin); // 时钟线↑
    iicDelay(iicObj);
    uint32_t timeOut = iicObj->timeoutUs;
    while (gpioIntf.pinRead(DPort, DPin)) {
        if (timeOut-- == 0) {
            iicStop(iicObj);
            return IIC_ERR_NACK;
        }
        iicDelay(iicObj);
    }

    gpioIntf.pinReset(CPort, CPin); // 时钟线↓

    return IIC_SUCCESS;
}

/**
 *@brief IIC发送单字节
 *
 * @param iicObj
 * @return IICErrCode
 */
static IICErrCode iicSendByte(IICObjTypeDef* iicObj, uint8_t byte) {
    GPIOPortEnum DPort = iicObj->SDAPort;
    GPIOPortEnum CPort = iicObj->SCLPort;
    GPIOPinEnum DPin   = iicObj->SDAPin;
    GPIOPinEnum CPin   = iicObj->SCLPin;

    for (uint8_t i = 0; i < 8; i++) {
        gpioIntf.pinWrite(DPort, DPin, (GPIOPinStateEnum)((byte & 0x80) != 0));
        byte <<= 1;

        iicDelay(iicObj);

        gpioIntf.pinSet(CPort, CPin);

        iicDelay(iicObj);

        gpioIntf.pinReset(CPort, CPin);
    }

    return iicWaitAck(iicObj);
}

/**
 * @brief IIC发送数据
 * @note 发送IIC数据前赋值slaveAddr, 给发送缓冲区txBuffer填充数据，并指明发送长度txLen。
 * @param iicObj
 * @return IICErrCode
 */
IICErrCode iicSend(IICObjTypeDef* iicObj) {

    // 检查参数有效性
    if (iicObj == NULL || iicObj->txBuffer == NULL || iicObj->txLen == 0) {
        return IIC_ERR_PARAM;
    }
    if (iicObj->type == IIC_SOFTWARE) {

        /* 软件IIC通信 */

        // 1.发送START
        IICErrCode status = iicStart(iicObj);

        if (status != IIC_SUCCESS) {
            return status;
        }

        // 2.发送从设备地址并检查ACK
        status = iicSendByte(iicObj, iicObj->slaveAddr | 0); // 发送地址，最低位为0表示写操作
        if (status != IIC_SUCCESS) {
            iicStop(iicObj); // 发送STOP信号
            return status;   // 返回错误
        }

        // 3. 发送数据并检查ACK
        for (iicObj->txIndex = 0; iicObj->txIndex < iicObj->txLen; iicObj->txIndex++) {

            status = iicSendByte(iicObj, iicObj->txBuffer[iicObj->txIndex]);

            if (status != IIC_SUCCESS) {
                iicStop(iicObj); // 发送STOP信号
                break;
            }
        }

        // 4. 发送STOP信号
        iicStop(iicObj);

        return status;

    } else {

        /* 硬件IIC通信 */

        float startTime;
        float timeDiff;

        // 1. 检查IIC是否忙碌
        I2C_GenerateSTART(iicObj->i2c, ENABLE);

        startTime = timeServIntf.getGlobalTime(); // 获取当前时间
        while (!I2C_CheckEvent(iicObj->i2c, I2C_EVENT_MASTER_MODE_SELECT)) {
#if TIMEOUT
            timeDiff = timeServIntf.getGlobalTime() - startTime; // 计算时间差
            if (timeDiff > iicObj->timeoutUs / 1000.0f) {        // 如果超过超时时间
                I2C_GenerateSTOP(iicObj->i2c, ENABLE);           // 发送STOP信号
                return IIC_ERR_BUSY;                             // 返回总线繁忙
            }
#endif
        }

        // 2.发送从设备地址并检查ACK
        I2C_Send7bitAddress(iicObj->i2c, iicObj->slaveAddr, I2C_Direction_Transmitter);

        startTime = timeServIntf.getGlobalTime(); // 获取当前时间
        while (!I2C_CheckEvent(iicObj->i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
#if TIMEOUT
            timeDiff = timeServIntf.getGlobalTime() - startTime; // 计算时间差
            if (timeDiff > iicObj->timeoutUs / 1000.0f) {        // 如果超过超时时间
                I2C_GenerateSTOP(iicObj->i2c, ENABLE);           // 发送STOP信号
                return IIC_ERR_NACK;                             // 返回NACK错误

            }
#endif
        }


        // 3.发送数据
        for (uint16_t i = 0; i < iicObj->txLen; i++) {
            I2C_SendData(iicObj->i2c, iicObj->txBuffer[i]);

            startTime = timeServIntf.getGlobalTime(); // 获取当前时间
            while (!I2C_CheckEvent(iicObj->i2c, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
#if TIMEOUT
                timeDiff = timeServIntf.getGlobalTime() - startTime; // 计算时间差
                if (timeDiff > iicObj->timeoutUs / 1000.0f) {        // 如果超过超时时间
                    I2C_GenerateSTOP(iicObj->i2c, ENABLE);           // 发送STOP信号
                    return IIC_ERR_TIMEOUT;                          // 响应超时

                }
#endif
            }
        }

        // 4.发送STOP停止传输
        I2C_GenerateSTOP(iicObj->i2c, ENABLE);

        return IIC_SUCCESS;
    }
    return IIC_ERR_PARAM;
}


/**
 *@brief IIC配置DMA发送
 *
 * @param obj
 * @return IICErrCode
 */
IICErrCode iicTxEquipWithDMA(IICObjTypeDef* obj) {


    if ((obj->dmaObj = (DMAObjTypeDef*)malloc(sizeof(DMAObjTypeDef))) == NULL) {
        return IIC_ERR_MEM_ALLOC_FAIL; // 内存申请失败
    }

    if (obj == NULL) {
        return IIC_ERR_PARAM;
    }
    if (obj->type == IIC_SOFTWARE) {
        return IIC_ERR_PARAM; // 仅硬件IIC支持DMA
    }

    if (obj->type == IIC_HARDWARE_1) {
        obj->dmaObj->channel = DMA1_Channel6; // 硬件IIC1使用DMA1通道6
    } else if (obj->type == IIC_HARDWARE_2) {
        obj->dmaObj->channel = DMA1_Channel7; // 硬件IIC2使用DMA1通道7
    } else {
        return IIC_ERR_PARAM; // 无效的IIC类型
    }

    dmaIntf.init(obj->dmaObj, obj->dmaObj->channel, DMA_Priority_Medium);
    dmaIntf.setSorce(obj->dmaObj, (uint32_t)obj->txBuffer, DMA_SIZE_BYTE, obj->txBufferSize);
    dmaIntf.setDest(obj->dmaObj, (uint32_t)&I2C1->DR, DMA_SIZE_BYTE, 1); // 目的地址为I2C数据寄存器

    return IIC_SUCCESS;
}


/**
 *@brief IIC使用DMA发送数据
 *
 * @param iicObj
 * @return IICErrCode
 */
IICErrCode iicSendWithDMA(IICObjTypeDef* iicObj) {
    if (iicObj == NULL || iicObj->dmaObj == NULL) {
        return IIC_ERR_PARAM;
    }
    if (iicObj->type == IIC_SOFTWARE) {
        return IIC_ERR_PARAM; // 仅硬件IIC支持DMA
    }

    if (iicObj->txLen == 0 || iicObj->txBuffer == NULL) {
        return IIC_ERR_PARAM; // 发送缓冲区不能为空
    }
    if (iicObj->txLen > iicObj->txBufferSize) {
        return IIC_ERR_PARAM; // 发送长度超过缓冲区大小
    }
    while (I2C_GetFlagStatus(iicObj->i2c, I2C_FLAG_BUSY))
        ;

    I2C_GenerateSTART(iicObj->i2c, ENABLE); // 发送START信号

    while (!I2C_CheckEvent(iicObj->i2c, I2C_EVENT_MASTER_MODE_SELECT))
        ; // 等待START信号发送完成

    I2C_Send7bitAddress(iicObj->i2c, iicObj->txBuffer[0], I2C_Direction_Transmitter); // 发送从设备地址
    while (!I2C_CheckEvent(iicObj->i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ; // 等待从设备地址发送完成


    dmaIntf.start(iicObj->dmaObj); // 启动DMA传输

    I2C_DMACmd(iicObj->i2c, ENABLE); // 使能IIC的DMA传输


    if (iicObj->type == IIC_HARDWARE_1) {
        while (!DMA_GetFlagStatus(DMA1_FLAG_TC6))
            ;                         // 等待DMA传输完成
        DMA_ClearFlag(DMA1_FLAG_TC6); // 清除DMA传输完成标志
    } else if (iicObj->type == IIC_HARDWARE_2) {
        while (!DMA_GetFlagStatus(DMA1_FLAG_TC7))
            ;
        DMA_ClearFlag(DMA1_FLAG_TC7); // 清除DMA传输完成标志
    }
    I2C_GenerateSTOP(iicObj->i2c, ENABLE); // 发送STOP信号

    iicObj->dmaObj->channel->CNDTR = iicObj->txLen;

    return IIC_SUCCESS;
}
