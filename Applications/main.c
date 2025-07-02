/**
 ***********************************************************************************************************************
 * @file           : main.c
 * @brief          : 主函数文件
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

#include "../Devices/drv-oled.h"
#include "../Protocols/drv-usart.h"
#include "../Services/graph-service.h"
#include "../Services/time-service.h"
#include "main.h"





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- variables -------------------------------------------------------------------------------------------------*/

uint16_t debug_errCnt;




/* ------- function prototypes ---------------------------------------------------------------------------------------*/





/* ------- function implement ----------------------------------------------------------------------------------------*/

int main(void) {
    // OLED对象
    OLEDObjTypeDef oledObj;

    // USART对象
    USARTObjTypeDef usartObj;
    // 图形缓冲区

    // 初始化OLED对象
    while (oledIntf.init(&oledObj) != OLED_SUCCESS)
        ;
	
	oledIntf.cmd(&oledObj);


    // 显示图形
    if (oledIntf.clear(&oledObj) != OLED_SUCCESS) {
        debug_errCnt++;
    }


    // 绘制五角星
    drawStar(oledObj.graphicsBuffer);

    if (oledIntf.draw(&oledObj) != OLED_SUCCESS) {
        debug_errCnt++;
    }



    while (1) {
		IICErrCode iicErr = oledIntf.fill(&oledObj);
		
		timeServIntf.delayMs(300);

		iicErr = oledIntf.clear(&oledObj);
		
		timeServIntf.delayMs(300);

    }

    return 0;
}
