/**
 ***********************************************************************************************************************
 * @file           : drv-oled.c
 * @brief          : 定义OLED类
 * @author         : 李嘉豪
 * @date           : 2025-04-29
 ***********************************************************************************************************************
 * @attention
 *
 * OLED的驱动文件
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "../Protocols/drv-iic.h"
#include "../Services/time-service.h"
#include "drv-oled.h"
#include <stdlib.h>
#include <string.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/

#define OLED_HEIGHT 8
#define OLED_WIDTH  128




/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

OLEDErrCode oledClear(OLEDObjTypeDef*);
OLEDErrCode oledDraw(OLEDObjTypeDef*);
OLEDErrCode oledInit(OLEDObjTypeDef*);
OLEDErrCode oledCmd(OLEDObjTypeDef*);
OLEDErrCode oledFill(OLEDObjTypeDef*);
OLEDErrCode oledDrawLoop(OLEDObjTypeDef*);




/* ------- variables -------------------------------------------------------------------------------------------------*/

OLEDIntfTypeDef oledIntf = {
    .clear = oledClear,
    .draw  = oledDrawLoop,
    .cmd   = oledCmd,
    .init  = oledInit,
    .fill  = oledFill,
};

static IICObjTypeDef oledIIC;

static uint8_t cmd[] = {
    0XAE, 0XD5, 0X80, 0XA8, 0X3F, 0XD3, 0X00, 0X40, 0X8D, 0X14, 0X20, 0X00, 0XA1, 0XC8, 0XDA, 0X12,
    0X81, 0XEF, 0XD9, 0XF1, 0XDB, 0X30, 0XA4, 0XA6, 0XAF, 0X21, 0X00, 0X7F, 0X22, 0X00, 0X07,
};



/* ------- function implement ----------------------------------------------------------------------------------------*/

/**
 *@brief oledInit 初始化OLED对象
 *
 * @param oledObj
 * @return OLEDErrCode
 */
OLEDErrCode oledInit(OLEDObjTypeDef* oledObj) {

    // 初始化IIC对象
    // 硬件IIC1
    // 数据线SDA连接到PB7
    // 时钟线SCL连接到PB6
    // 发送缓冲区大小1025(更新一帧图像需要的最大字节数)
    // 接收缓冲区大小1(不需要接收数据)
    // 超时时间1000ms
    // 传输速度400kHz(快速IIC)
    if (iicIntf.init(&oledIIC, IIC_HARDWARE_1, PORT_B, PIN_7, PORT_B, PIN_6, OLED_WIDTH * OLED_HEIGHT + 2, 1, 1000,
                     400000) != IIC_SUCCESS) {
        return OLED_ERR;
    }

    if (iicIntf.equippedWithDMA(&oledIIC) != IIC_SUCCESS) {
        return OLED_ERR;
    }

    timeServIntf.delayMs(200);

    oledObj->iic            = &oledIIC;
    oledObj->iic->slaveAddr = 0x78; // OLED的IIC地址

    return OLED_SUCCESS;
}

OLEDErrCode oledCmd(OLEDObjTypeDef* oledObj) {
    /* 往IIC对象的发送缓冲区写入数据*/
    oledIIC.txBuffer[0] = 0x00;
    memcpy(oledIIC.txBuffer + 1, cmd, sizeof(cmd));
    oledIIC.txLen = sizeof(cmd) + 1;

    if (iicIntf.transmit(&oledIIC) != IIC_SUCCESS) {
        return OLED_ERR;
    }
    return OLED_SUCCESS;
}

/**
 * @brief oledDraw OLED对象绘制方法
 *
 * @param oledObj
 * @return OLEDErrCode
 */
OLEDErrCode oledDraw(OLEDObjTypeDef* oledObj) {
    oledIIC.txBuffer[0] = 0x40;
    memcpy(oledIIC.txBuffer + 1, oledObj->graphicsBuffer, OLED_HEIGHT * OLED_WIDTH);
    oledIIC.txLen = OLED_HEIGHT * OLED_WIDTH + 1;
    return iicIntf.transmit(&oledIIC) == IIC_SUCCESS ? OLED_SUCCESS : OLED_ERR;
}

/**
 * @brief oledClear 清除OLED对象的显示内容
 *
 * @param oledObj
 * @return OLEDErrCode
 */
OLEDErrCode oledClear(OLEDObjTypeDef* oledObj) {
    memset(oledObj->graphicsBuffer, 0, OLED_HEIGHT * OLED_WIDTH);
    return oledDraw(oledObj);
}

/**
 * @brief oledFill 填充OLED对象的显示内容
 *
 * @param oledObj
 * @return OLEDErrCode
 */
OLEDErrCode oledFill(OLEDObjTypeDef* oledObj) {
    memset(oledObj->graphicsBuffer, 0xFF, OLED_HEIGHT * OLED_WIDTH);
    return oledDraw(oledObj);
}

/**
 * @brief oledDrawLoop 开启OLED发送循环
 *
 * @param oledObj
 * @return OLEDErrCode
 */
OLEDErrCode oledDrawLoop(OLEDObjTypeDef* oledObj) {
    oledIIC.txBuffer[0] = 0x40;
    memcpy(oledIIC.txBuffer + 1, oledObj->graphicsBuffer, OLED_HEIGHT * OLED_WIDTH);
    oledIIC.txLen = OLED_HEIGHT * OLED_WIDTH + 1;
    return iicIntf.transmitWithDMA(&oledIIC) == IIC_SUCCESS ? OLED_SUCCESS : OLED_ERR;
}
