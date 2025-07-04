/**
 ***********************************************************************************************************************
 * @file           : drv-iic.h
 * @brief          : 软件IIC驱动文件
 * @author         : 李嘉豪
 * @date           : 2025-04-29
 ***********************************************************************************************************************
 * @attention
 *
 * 编写软件IIC类
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __DRV_IIC_H__
#define __DRV_IIC_H__




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "../Peripherals/dma.h"
#include "../Peripherals/gpio.h"
#include <stdint.h>





/*-------- typedef ---------------------------------------------------------------------------------------------------*/

/* IIC错误码 */
typedef enum {
    IIC_SUCCESS            = 0x00, // 成功
    IIC_ERR_TIMEOUT        = 0x01, // 超时
    IIC_ERR_NACK           = 0x02, // NACK
    IIC_ERR_BUSY           = 0x03, // 忙
    IIC_ERR_PARAM          = 0x04, // 参数错误
    IIC_ERR_MEM_ALLOC_FAIL = 0x05, // 内存申请失败
    IIC_ERR_NSTART         = 0x06, // START错误
    IIC_ERR_ADDR           = 0x07, // 访问地址失败
} IICErrCode;

/* 硬件或软件IIC实现 */
typedef enum {
    IIC_HARDWARE_1, // 硬件IIC
    IIC_HARDWARE_2, // 硬件IIC
    IIC_SOFTWARE,   // 软件IIC
} IICImplTypeEnum;

/* IIC类 */
typedef struct {
    IICImplTypeEnum type; // IIC实现类型
    I2C_TypeDef* i2c;     // 硬件IIC使用的I2C外设指针


    GPIOPortEnum SDAPort; // SDA端口
    GPIOPinEnum SDAPin;   // SDA引脚
    GPIOPortEnum SCLPort; // SCL端口
    GPIOPinEnum SCLPin;   // SCL引脚

    uint8_t* txBuffer;     // 发送缓冲区
    uint8_t* rxBuffer;     // 接收缓冲区
    uint16_t txBufferSize; // 发送缓冲区大小
    uint16_t rxBufferSize; // 接收缓冲区大小
    uint16_t txIndex;      // 发送索引
    uint16_t rxIndex;      // 接收索引
    uint16_t txLen;        // 发送长度
    uint16_t timeoutUs;    // 超时时间

    uint32_t speed; // 传输速度(bps)

    uint8_t slaveAddr; // 从设备地址

    DMAObjTypeDef* dmaObj; // DMA对象指针，若使用DMA传输则不为NULL

} IICObjTypeDef;

/* IIC接口 */
typedef struct {
    IICErrCode (*init)(IICObjTypeDef* iicObj, IICImplTypeEnum type, GPIOPortEnum SDA, GPIOPinEnum SDA_Pin,
                       GPIOPortEnum SCL, GPIOPinEnum SCL_Pin, uint16_t txBufferSize, uint16_t rxBufferSize,
                       uint16_t timeoutMs, uint32_t speed);
    IICErrCode (*transmit)(IICObjTypeDef* iicObj);
    IICErrCode (*equippedWithDMA)(IICObjTypeDef*);
    IICErrCode (*sendWithDMA)(IICObjTypeDef* iicObj);
} IICIntfTypeDef;




/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/

extern IICIntfTypeDef iicIntf; // IIC驱动对外接口




/*-------- function prototypes ---------------------------------------------------------------------------------------*/





#endif /* __DRV_IIC_H__ */
