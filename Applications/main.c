/**
 ***********************************************************************************************************************
 * @file           : main.c
 * @brief          : 主函数文件
 * @author         : 李嘉豪
 * @date           : 2025-04-05
 ***********************************************************************************************************************
 * @attention
 *
 * 主函数所在文件，调用所有的应用以及中断处理函数
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "../Devices/drv-oled.h"
#include "../Protocols/drv-usart.h"
#include "../Services/graph-service.h"
#include "../Services/time-service.h"
#include "app-input.h"
#include "app-ui.h"
#include "main.h"
#include <string.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- variables -------------------------------------------------------------------------------------------------*/

uint16_t debug_errCnt;
float mainLoopTime = 0.0f; // 主循环时间
static OLEDObjTypeDef oledObj;
UIAppParamTypeDef uiAppParam;       // UI应用参数
InputAppParamTypeDef inputAppParam; // 输入应用参数



/* ------- function prototypes ---------------------------------------------------------------------------------------*/

static void uiParamUpdate(UIAppParamTypeDef*);




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

    uiAppParam.graphicsBuffers[0] = oledObj.graphicsBuffer;    // 设置图形缓冲区
    uiAppParam.graphicsBuffers[1] = oledObj.graphicsBufferSub; // 设置辅助图形缓冲区
    inputAppInit(&inputAppParam);
    uiAppInit(&uiAppParam);




    /* ----- main loop -------------------------------------------------------*/

    while (1) {
        mainLoopStartTime = timeServIntf.getGlobalTime(); // 获取主循环开始时间
		
		
		// 调用应用
        inputAppLoop(&inputAppParam); // 输入应用循环

        // 参数更新
        uiParamUpdate(&uiAppParam);


        uiAppLoop(&uiAppParam);

        // 切换DMA缓冲区索引
        mainLoopTime = timeServIntf.getGlobalTime() - mainLoopStartTime; // 计算主循环时间
    }
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

        // 将准备好的数据转运到缓冲区
        if (uiAppParam.bufferIndex == 0) {
            memcpy(oledObj.iic->txBuffer, uiAppParam.graphicsBuffers[1], OLED_WIDTH * OLED_HEIGHT);
        } else if (uiAppParam.bufferIndex == 1) {
            memcpy(oledObj.iic->txBuffer, uiAppParam.graphicsBuffers[0], OLED_WIDTH * OLED_HEIGHT);
        }

        DMA1_Channel6->CNDTR = 1024;

        I2C_DMACmd(I2C1, ENABLE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
        TIM_Cmd(TIM6, ENABLE);
    }
}

void EXTI9_5_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line5)) {     // 检查外部中断线5
        EXTI_ClearITPendingBit(EXTI_Line5); // 清除中断标志

        float keyUpdateTime = timeServIntf.getElapsedTime(inputAppParam.key0Timer);
        if (keyUpdateTime > 0.7f) {
            inputAppParam.key0Pressed = 1;
        }
    }
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line15)) {     // 检查外部中断线15
        EXTI_ClearITPendingBit(EXTI_Line15); // 清除中断标志

        float keyUpdateTime = timeServIntf.getElapsedTime(inputAppParam.key1Timer);
        if (keyUpdateTime > 0.7f) {
            inputAppParam.key1Pressed = 1;
        }
    }
}


/**
 * @brief UI参数更新函数
 *
 * @param pUIAppParam
 */
inline static void uiParamUpdate(UIAppParamTypeDef* pUIAppParam) {

    // 事件映射

    if (inputAppParam.event == INPUT_EVENT_KEY_0_CLICK) {
        // 按下0进入或退出图形查看状态
        pUIAppParam->eventGroup |= (1 << UI_EVENT_FIGURE_EXIT);
        pUIAppParam->eventGroup |= (1 << UI_EVENT_FIGURE_VIEW);
    } else if (inputAppParam.event == INPUT_EVENT_KEY_1_CLICK) {
        // 按下1进入或退出编辑状态
        pUIAppParam->eventGroup |= (1 << UI_EVENT_VALUE_SELECT);
        pUIAppParam->eventGroup |= (1 << UI_EVENT_VALUE_UNSELECT);

    } else if (inputAppParam.event == INPUT_EVENT_ENCODER_MOVE_POSITIVE) {
        // 编码器正向移动选择下一个或者增加值
        pUIAppParam->eventGroup |= (1 << UI_EVENT_SELECT_NEXT); // 设置选择下一个事件
        pUIAppParam->eventGroup |= (1 << UI_EVENT_VALUE_ADD);   // 设置值增加事件

    } else if (inputAppParam.event == INPUT_EVENT_ENCODER_MOVE_NEGATIVE) {
        // 编码器反向移动选择上一个或者减少值
        pUIAppParam->eventGroup |= (1 << UI_EVENT_SELECT_PREV); // 设置选择上一个事件
        pUIAppParam->eventGroup |= (1 << UI_EVENT_VALUE_SUB);   // 设置值减少事件
        
    } else {
        pUIAppParam->eventGroup = 0; // 无事件
    }
}
