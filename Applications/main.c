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
#include <string.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- variables -------------------------------------------------------------------------------------------------*/

uint16_t debug_errCnt;
float mainLoopTime = 0.0f; // 主循环时间
float sysTime      = 0.0f; // 系统时间
static OLEDObjTypeDef oledObj;
static uint8_t iicDMABufIdx = 0;




/* ------- function prototypes ---------------------------------------------------------------------------------------*/





/* ------- function implement ----------------------------------------------------------------------------------------*/

/**
 * @brief 主函数入口
 *
 * @return int
 */
int main(void) {

    /* ------ local variables ------------------------------------------------*/
    // TIM对象
    TIMObjTypeDef timFlashOLED;


    float mainLoopStartTime = 0.0f; // 主循环开始时间


    /* ----- drivers & service initialize ------------------------------------*/

    // 初始化时间服务
    timeServIntf.servInit();

    // 初始化OLED对象
    while (oledIntf.init(&oledObj) != OLED_SUCCESS)
        ;

    oledIntf.cmd(&oledObj);

    oledIntf.draw(&oledObj);

    timIntf.init(&timFlashOLED, TIM6);
    timIntf.setFrequency(&timFlashOLED, 30);
    timIntf.enableISR(&timFlashOLED);

    /* ----- applications initialize -----------------------------------------*/



    /* ----- main loop -------------------------------------------------------*/

    while (1) {
        sysTime = mainLoopStartTime = timeServIntf.getGlobalTime(); // 获取主循环开始时间


        mainLoopTime                = timeServIntf.getGlobalTime() - mainLoopStartTime; // 计算主循环时间
    }

    return 0;
}


/**
 * @brief DMA1通道6中断处理函数
 *
 * @return void
 */
void DMA1_Channel6_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC6)) {     // 检查DMA1通道6传输完成中断
        DMA_ClearITPendingBit(DMA1_IT_TC6); // 清除中断标志
        DMA_Cmd(DMA1_Channel6, DISABLE);    // 禁用DMA通道6
        I2C_DMACmd(I2C1, DISABLE);
    }
}


/**
 * @brief TIM6中断处理函数
 *
 * @return void
 */
void TIM6_IRQHandler(void) {
    if (TIM_GetITStatus(TIM6, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
        sysTime = timeServIntf.getGlobalTime();
        if (iicDMABufIdx == 0) {
            DMA1_Channel6->CMAR = (uint32_t)oledObj.graphicsBuffer;
        } else {
            DMA1_Channel6->CMAR = (uint32_t)oledObj.iic->txBuffer;
        }
        DMA1_Channel6->CNDTR = 1024;
        I2C_DMACmd(I2C1, ENABLE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
        TIM_Cmd(TIM6, ENABLE);
    }
}
