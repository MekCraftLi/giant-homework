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
#include "app-signal.h"
#include "app-ui.h"
#include "main.h"
#include <string.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/

struct {
    struct {
        float systemTime;     // 系统时间
        float mainLoopTime;   // 主循环时间
        float uiLoopTime;     // UI循环时间
        float inputLoopTime;  // 输入循环时间
        float signalLoopTime; // 信号循环时间
    } timeInfo;               // 时间信息

    SoftTimerHandle mainLoopTimer; // 主循环定时器句柄

    uint8_t errCnt; // 错误计数
} debugInfo;        // 调试信息结构体




/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- variables -------------------------------------------------------------------------------------------------*/

static OLEDObjTypeDef oledObj;
UIAppParamTypeDef uiAppParam;         // UI应用参数
InputAppParamTypeDef inputAppParam;   // 输入应用参数
SignalAppParamTypeDef signalAppParam; // 信号应用参数


/* ------- function prototypes ---------------------------------------------------------------------------------------*/

static void uiParamUpdate(UIAppParamTypeDef*);
inline static void signalParamUpdate(SignalAppParamTypeDef* pSignalAppParam);




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




    /* ----- drivers & service initialize ------------------------------------*/

    // 初始化时间服务
    timeServIntf.servInit();

    debugInfo.mainLoopTimer = timeServIntf.softTimerRegister(); // 注册主循环定时器

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
    signalParamUpdate(&signalAppParam);
    signalAppInit(&signalAppParam); // 初始化信号应用




    /* ----- main loop -------------------------------------------------------*/

    while (1) {
        debugInfo.timeInfo.systemTime = timeServIntf.getGlobalTime();


        // 调用应用
        inputAppLoop(&inputAppParam); // 输入应用循环



        // 参数更新
        uiParamUpdate(&uiAppParam);

        uiAppLoop(&uiAppParam);



        signalParamUpdate(&signalAppParam); // 更新信号参数

        signalAppLoop(&signalAppParam); // 信号应用循环

        // 切换DMA缓冲区索引
        debugInfo.timeInfo.mainLoopTime = timeServIntf.getElapsedTime(debugInfo.mainLoopTimer); // 获取主循环时间
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

/**
 * @brief 信号参数更新函数
 *
 * @param pSignalAppParam
 */
inline static void signalParamUpdate(SignalAppParamTypeDef* pSignalAppParam) {
    pSignalAppParam->signalInfo[0].freq  = uiAppParam.signalInfo[0].freq;  // 更新信号1频率
    pSignalAppParam->signalInfo[1].freq  = uiAppParam.signalInfo[1].freq;  // 更新信号2频率
    pSignalAppParam->signalInfo[0].amp   = uiAppParam.signalInfo[0].amp;   // 更新信号1幅度
    pSignalAppParam->signalInfo[1].amp   = uiAppParam.signalInfo[1].amp;   // 更新信号2幅度
    pSignalAppParam->signalInfo[0].phase = uiAppParam.signalInfo[0].phase; // 更新信号1相位
    pSignalAppParam->signalInfo[1].phase = uiAppParam.signalInfo[1].phase; // 更新信号2相位
    static UIStateEnum lastUIState;

    if (uiAppParam.curState == UI_STATE_ADJUST_BROUWSE && lastUIState == UI_STATE_ADJUST_EDIT) {
        // 如果当前状态不是图形查看状态，并且触发了图形查看事件
        pSignalAppParam->updateFlag = 1; // 设置更新标志位
    } else {
        pSignalAppParam->updateFlag = 0; // 清除更新标志位
    }

    lastUIState = uiAppParam.curState;
}



/**
 * @brief DMA1通道6中断处理函数, 用于在发送完一帧数据后停止DMA
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
 * @brief TIM6中断处理函数, 固定30Hz频率刷新OLED屏幕
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

/**
 * @brief TIM7中断处理函数, 500Hz采样ADC数据
 *
 * @return void
 */
void TIM7_IRQHandler(void) {
    if (TIM_GetITStatus(TIM7, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        DMA_Cmd(DMA1_Channel1, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel1, 2);
        DMA_Cmd(DMA1_Channel1, ENABLE);

        ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 启动 ADC（ADC2 自动同步）

#if 1
        graphServIntf.insertNewPoint(MAP_ADC_TO_OLED_Y(signalAppParam.adcData.adcValues.signal1),
                                     MAP_ADC_TO_OLED_X(signalAppParam.adcData.adcValues.signal2), uiAppParam.dotMatrix);
#else
        ((uint8_t(*)[128])uiAppParam.dotMatrix)[MAP_ADC_TO_OLED_Y(signalAppParam.adcData.adcValues.signal2)]
                                               [MAP_ADC_TO_OLED_X(signalAppParam.adcData.adcValues.signal1)] +=
            1; // 在点阵图上设置点
#endif // DELETE_OLD
    }
}

/**
 * @brief KEY0中断处理函数
 *
 */
void EXTI9_5_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line5)) {     // 检查外部中断线5
        EXTI_ClearITPendingBit(EXTI_Line5); // 清除中断标志

        float keyUpdateTime = timeServIntf.getElapsedTime(inputAppParam.key0Timer);
        if (keyUpdateTime > 0.3f) {
            inputAppParam.key0Pressed = 1;
        }
    }
}

/**
 *@brief KEY1中断处理函数
 *
 */
void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line15)) {     // 检查外部中断线15
        EXTI_ClearITPendingBit(EXTI_Line15); // 清除中断标志

        float keyUpdateTime = timeServIntf.getElapsedTime(inputAppParam.key1Timer);
        if (keyUpdateTime > 0.3f) {
            inputAppParam.key1Pressed = 1;
        }
    }
}
