/**
 ***********************************************************************************************************************
 * @file           : main.c
 * @brief          : 主函数文件
 * @author         : 李嘉豪
 * @date           : 2025-04-05
 ***********************************************************************************************************************
 * @attention
 *
 * 主函数所在文件，调用所有的应用
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
float mainLoopTime = 0.0f; // 主循环时间
float sysTime = 0.0f; // 系统时间





/* ------- function prototypes ---------------------------------------------------------------------------------------*/





/* ------- function implement ----------------------------------------------------------------------------------------*/

/**
 * @brief 主函数入口
 *
 * @return int
 */
int main(void) {

    /* ------ local variables ------------------------------------------------*/
    // OLED对象
    OLEDObjTypeDef oledObj;

    float mainLoopStartTime = 0.0f; // 主循环开始时间


    /* ----- drivers & service initialize ------------------------------------*/

    // 初始化时间服务
    timeServIntf.servInit();

    // 初始化OLED对象
    while (oledIntf.init(&oledObj) != OLED_SUCCESS)
        ;

    oledIntf.cmd(&oledObj);

    /* ----- applications initialize -----------------------------------------*/

    // 显示图形
    if (oledIntf.clear(&oledObj) != OLED_SUCCESS) {
        debug_errCnt++;
    }


    // 绘制五角星
    drawStar(oledObj.graphicsBuffer);

    if (oledIntf.draw(&oledObj) != OLED_SUCCESS) {
        debug_errCnt++;
    }


    /* ----- main loop -------------------------------------------------------*/

    while (1) {
        sysTime = mainLoopStartTime  = timeServIntf.getGlobalTime(); // 获取主循环开始时间
		
		drawStar(oledObj.graphicsBuffer);
        OLEDErrCode iicErr = oledIntf.draw(&oledObj);

        timeServIntf.delayUs(2);

        iicErr = oledIntf.clear(&oledObj);

        timeServIntf.delayUs(2);
		

        mainLoopTime = timeServIntf.getGlobalTime() - mainLoopStartTime; // 计算主循环时间
    }

    return 0;
}
