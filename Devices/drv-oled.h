/**
 ***********************************************************************************************************************
 * @file           : drv-oled.h
 * @brief          : OLED的驱动文件
 * @author         : 李嘉豪
 * @date           : 2025-04-29
 ***********************************************************************************************************************
 * @attention
 *
 * OLED对象具有IIC对象和图形缓冲区两个属性
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __DRV_OLED_H__
#define __DRV_OLED_H__




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "../Protocols/drv-iic.h"




/*-------- define ----------------------------------------------------------------------------------------------------*/

#define OLED_HEIGHT 8
#define OLED_WIDTH  128




/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef enum {
    OLED_SUCCESS, // OLED驱动函数运行无问题
    OLED_ERR,     // OLED驱动函数运行有问题
} OLEDErrCode;

typedef struct {
    IICObjTypeDef* iic;
    uint8_t graphicsBuffer[OLED_HEIGHT][OLED_WIDTH];
    uint8_t graphicsBufferSub[OLED_HEIGHT][OLED_WIDTH]; // 用于DMA传输时的辅助缓冲区
} OLEDObjTypeDef;

typedef struct {
    OLEDErrCode (*clear)(OLEDObjTypeDef*);
    OLEDErrCode (*draw)(OLEDObjTypeDef*);
    OLEDErrCode (*init)(OLEDObjTypeDef*);
    OLEDErrCode (*fill)(OLEDObjTypeDef*);
    OLEDErrCode (*cmd)(OLEDObjTypeDef*);
} OLEDIntfTypeDef;





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/

extern OLEDIntfTypeDef oledIntf;




/*-------- function prototypes ---------------------------------------------------------------------------------------*/





#endif /* __DRV_OLED_H__ */
