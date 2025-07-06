/**
 ***********************************************************************************************************************
 * @file           : app-ui.h
 * @brief          : UI界面业务逻辑处理应用
 * @author         : 李嘉豪
 * @date           : 2025-07-05
 ***********************************************************************************************************************
 * @attention
 *
 * UI界面的业务逻辑处理文件
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __APP_UI_H__
#define __APP_UI_H__




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include "../Services/time-service.h"
#include <stdint.h>





/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef enum {
    UI_EVENT_NONE,           // 无事件
    UI_EVENT_VALUE_ADD,      // 值增加事件
    UI_EVENT_VALUE_SUB,      // 值减少事件
    UI_EVENT_VALUE_SELECT,   // 值选择事件
    UI_EVENT_VALUE_UNSELECT, // 值取消选择事件
    UI_EVENT_SELECT_NEXT,    // 选择下一个事件
    UI_EVENT_SELECT_PREV,    // 选择上一个事件
    UI_EVENT_FIGURE_VIEW,    // 图形查看事件
    UI_EVENT_FIGURE_EXIT,    // 图形退出事件
} UIEventEnum;               // UI事件枚举类型定义

typedef enum {
    UI_STATE_ADJUST_BROUWSE, // 调节浏览状态
    UI_STATE_ADJUST_EDIT,    // 调节编辑状态
    UI_STATE_FIGURE_VIEW,    // 图形查看状态
} UIStateEnum;               // UI状态枚举类型定义

typedef enum {
    SIGNAL_1_FREQ,  // 信号1频率
    SIGNAL_2_FREQ,  // 信号2频率
    SIGNAL_1_AMP,   // 信号1幅度
    SIGNAL_2_AMP,   // 信号2幅度
    SIGNAL_1_PHASE, // 信号1相位
    SIGNAL_2_PHASE, // 信号2相位
} UISelectIndexEnum;

// UI状态机类型定义
typedef struct {
    UIStateEnum curState;      // 当前UI状态
    UIStateEnum nextState;     // 下一个UI状态
    UIEventEnum event;         // 触发事件
    void (*actionFunc)(void*); // 状态动作函数指针
} UIStateTransitionTypeDef;

typedef struct {
    uint8_t startX; // 起始X坐标
    uint8_t startY; // 起始Y坐标
    uint8_t endX;   // 结束X坐标
    uint8_t endY;   // 结束Y坐标
    uint8_t radius; // 圆弧半径
} RoundedRectangleParaTypeDef;


// UI选择信息显示数据类型定义
typedef struct {
    UISelectIndexEnum index;               // 选择索引
    RoundedRectangleParaTypeDef rectParam; // 圆角矩形参数
} UISelDispInfoTypeDef;


// UI选择显示动画
typedef struct {
    UISelDispInfoTypeDef* pCurSelInfo;  // 当前选择信息
    UISelDispInfoTypeDef* pNextSelInfo; // 下一个选择信息
    uint8_t transitionState;            // 过渡状态
    UIEventEnum event;                  // 触发事件
    float duration;                     // 状态持续时间
    float elapsed;                      // 已经经过的时间
} UISelDispAnimDataTypeDef;

typedef struct {
    float freq;      // 频率
    float amp;       // 幅度
    uint16_t phase;  // 相位
} SignalInfoTypeDef; // 信号信息类型定义


// UI应用参数类型定义
typedef struct {
    void* graphicsBuffers[2];      // 图形缓冲区
    void* dotMatrix;               // 点阵图像素
    uint8_t bufferIndex;           // 图形缓冲区索引
    UIEventEnum event;             // 当前事件
    UIStateEnum curState;          // 当前状态
    UISelectIndexEnum selectIndex; // 当前选择索引

    SignalInfoTypeDef signalInfo[2]; // 信号信息

    UISelDispInfoTypeDef selDispInfo;     // 选择信息显示数据
    UISelDispAnimDataTypeDef animateData; // 浏览选择动画数据
    SoftTimerHandle browseAnimateTimer;   // 浏览动画定时器句柄
} UIAppParamTypeDef;





/*-------- define ----------------------------------------------------------------------------------------------------*/





/*-------- macro -----------------------------------------------------------------------------------------------------*/





/*-------- variables -------------------------------------------------------------------------------------------------*/





/*-------- function prototypes ---------------------------------------------------------------------------------------*/

void uiAppInit(void* argument); // UI应用初始化函数
void uiAppLoop(void* argument); // UI应用循环函数




#endif /* __APP_UI_H__ */
