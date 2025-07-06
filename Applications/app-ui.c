/**
 ***********************************************************************************************************************
 * @file           : app-ui.c
 * @brief          : UI界面的业务逻辑处理
 * @author         : 李嘉豪
 * @date           : 2025-07-05
 ***********************************************************************************************************************
 * @attention
 *
 * UI界面的业务逻辑处理文件
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "../Services/graph-service.h"
#include "app-ui.h"
#include <stdio.h>
#include <string.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/

#define UI_SELECT_INDEX_QUANTITY 6

#define FREQ_STEP                0.5f
#define AMP_STEP                 0.3f
#define PHASE_STEP               15

#define FREQ_MAX                 6.0f
#define FREQ_MIN                 1.0f

#define AMP_MAX                  3.3f
#define AMP_MIN                  1.5f

#define PHASE_MAX                180
#define PHASE_MIN                0




/* ------- macro -----------------------------------------------------------------------------------------------------*/

#define UI_SELECTED_GOTO_NEXT(x)                                                                                       \
    do {                                                                                                               \
        x++;                                                                                                           \
        if (x >= UI_SELECT_INDEX_QUANTITY) {                                                                           \
            x = 0;                                                                                                     \
        }                                                                                                              \
    } while (0);

#define UI_SELECTED_GOTO_PREV(x)                                                                                       \
    do {                                                                                                               \
        if (x == 0) {                                                                                                  \
            x = UI_SELECT_INDEX_QUANTITY - 1;                                                                          \
        } else {                                                                                                       \
            x--;                                                                                                       \
        }                                                                                                              \
    } while (0);




/* ------- function prototypes ---------------------------------------------------------------------------------------*/

static void actionWhileBrowse(void* argument);
static void actionWhileEdit(void* argument);
static void actionWhileFigureView(void* argument);

static void browseAnimate(void* argument);
static void singalInfoDisplay(void* argument);

static void updateSignal(void* argument);


/* ------- variables -------------------------------------------------------------------------------------------------*/


// UI状态机列表
static UIStateTransitionTypeDef uiStateMachineList[] = {
    {UI_STATE_ADJUST_BROUWSE, UI_STATE_ADJUST_EDIT, UI_EVENT_VALUE_SELECT,
     actionWhileBrowse}, // 浏览状态下选择值进入编辑状态
    {UI_STATE_ADJUST_BROUWSE, UI_STATE_FIGURE_VIEW, UI_EVENT_FIGURE_VIEW,
     actionWhileBrowse}, // 浏览状态下查看图形进入图形查看状态
    {UI_STATE_ADJUST_BROUWSE, UI_STATE_ADJUST_BROUWSE, UI_EVENT_NONE,
     actionWhileBrowse}, // 浏览状态下无事件保持浏览状态
    {UI_STATE_ADJUST_EDIT, UI_STATE_ADJUST_BROUWSE, UI_EVENT_VALUE_UNSELECT,
     actionWhileEdit},                                                            // 编辑状态下取消选择返回浏览状态
    {UI_STATE_ADJUST_EDIT, UI_STATE_ADJUST_EDIT, UI_EVENT_NONE, actionWhileEdit}, // 编辑状态下无事件保持编辑状态

    {UI_STATE_FIGURE_VIEW, UI_STATE_ADJUST_BROUWSE, UI_EVENT_FIGURE_EXIT, actionWhileFigureView},
    {UI_STATE_FIGURE_VIEW, UI_STATE_FIGURE_VIEW, UI_EVENT_NONE,
     actionWhileFigureView}, // 图形查看状态下无事件保持图形查看状态
};

// UI选择信息显示数据
static UISelDispInfoTypeDef uiSelInfoDispList[] = {
    // 信号1频率选择数据
    {{SIGNAL_1_FREQ}, {32, 17, 76, 29, 3}},
    // 信号2频率选择数据
    {{SIGNAL_2_FREQ}, {80, 17, 126, 29, 3}},
    // 信号1幅度选择数据
    {{SIGNAL_1_AMP}, {32, 33, 76, 45, 3}},
    // 信号2幅度选择数据
    {{SIGNAL_2_AMP}, {80, 33, 126, 45, 3}},
    // 信号1相位选择数据
    {{SIGNAL_1_PHASE}, {32, 49, 76, 61, 3}},
    // 信号2相位选择数据
    {{SIGNAL_2_PHASE}, {80, 49, 126, 61, 3}},
};


const uint8_t img[1024] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x40, 0xF0, 0x0C, 0x10, 0x50, 0x50, 0x54, 0x58, 0x50, 0x50, 0x10, 0x00, 0x80, 0x80, 0xBC, 0xA4, 0xA4,
    0xA4, 0xA4, 0xA4, 0xBC, 0x80, 0x80, 0x00, 0x00, 0x10, 0x10, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xF0, 0x0C, 0x10, 0x50, 0x50, 0x54,
    0x58, 0x50, 0x50, 0x10, 0x00, 0x80, 0x80, 0xBC, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xBC, 0x80, 0x80, 0x00, 0x00, 0x30,
    0x08, 0x08, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xBF, 0x80,
    0x80, 0xBD, 0x95, 0x95, 0x95, 0x95, 0xBD, 0x80, 0x80, 0x80, 0x80, 0x86, 0x85, 0x84, 0x84, 0x84, 0xA4, 0xA4, 0x9C,
    0x80, 0x80, 0x80, 0x88, 0x88, 0x8F, 0x88, 0x88, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xBF, 0x80, 0x80, 0xBD, 0x95, 0x95, 0x95, 0x95, 0xBD, 0x80, 0x80,
    0x80, 0x80, 0x86, 0x85, 0x84, 0x84, 0x84, 0xA4, 0xA4, 0x9C, 0x80, 0x80, 0x80, 0x88, 0x8C, 0x8A, 0x89, 0x88, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x40, 0x78, 0x40, 0xFC, 0x48, 0x48, 0xE4,
    0x34, 0xAC, 0x24, 0xE4, 0x00, 0x08, 0x28, 0x48, 0x68, 0x58, 0xCC, 0x48, 0x28, 0x48, 0x28, 0x08, 0x00, 0x00, 0x00,
    0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0xA4, 0xA3, 0x90, 0x8B, 0x84, 0x82, 0xA7, 0x90, 0x8F, 0x90, 0xA7, 0x80,
    0x88, 0x8A, 0x89, 0x8A, 0x8B, 0xBE, 0x8B, 0x8A, 0x89, 0x8A, 0x88, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
    0x00, 0x00, 0xF0, 0x10, 0xFC, 0x10, 0xF0, 0x00, 0x74, 0x54, 0x54, 0x54, 0x74, 0x00, 0x00, 0xF8, 0x28, 0x28, 0xF8,
    0xA8, 0xAC, 0xA8, 0xF8, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x87, 0x80, 0xBF,
    0x84, 0x87, 0x80, 0xBF, 0x95, 0x9F, 0x95, 0xBF, 0x80, 0xA0, 0x9F, 0x80, 0xA2, 0xA6, 0xAA, 0x92, 0x92, 0xAA, 0xA6,
    0xA0, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x20, 0xA0, 0xFC, 0x20, 0x00, 0xF8, 0x48, 0x48,
    0x48, 0x48, 0xF8, 0x00, 0x40, 0xF0, 0x0C, 0x20, 0xA0, 0x20, 0x24, 0x28, 0x20, 0xA0, 0x20, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x3F, 0x01, 0x00, 0x3F, 0x12, 0x12, 0x12, 0x12, 0x3F, 0x00, 0x00,
    0x3F, 0x00, 0x20, 0x21, 0x2E, 0x20, 0x30, 0x2C, 0x23, 0x20, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};





// 点阵图像素数据
static uint8_t dotMatrix[HEIGHT][WIDTH] = {0};

static uint8_t strBuffer[6][8];

float debugTimeMeasure;

SoftTimerHandle debugTimer;


/* ------- function implement ----------------------------------------------------------------------------------------*/

/**
 * @brief UI应用初始化函数
 *
 * @param argument
 */
void uiAppInit(void* argument) {
    UIAppParamTypeDef* pParam = (UIAppParamTypeDef*)argument;


    memcpy(pParam->graphicsBuffers[0], img, sizeof(img)); // 初始化图形缓冲区


    debugTimer                       = timeServIntf.softTimerRegister();

    pParam->browseAnimateTimer       = timeServIntf.softTimerRegister();
    pParam->eventGroup               = 0;                                       // 初始化事件为无
    pParam->curState                 = UI_STATE_ADJUST_BROUWSE;                 // 初始状态为浏览状态
    pParam->selectIndex              = SIGNAL_1_FREQ;                           // 初始选择索引为信号1频率
    pParam->animateData.pCurSelInfo  = &uiSelInfoDispList[pParam->selectIndex]; // 设置当前选择信息
    pParam->animateData.pNextSelInfo = &uiSelInfoDispList[pParam->selectIndex + 1];
    pParam->animateData.duration     = 0.3f;      // 动画持续时间
    pParam->dotMatrix                = dotMatrix; // 设置点阵图像素
    graphServIntf.drawRoundRect2DotMatrix(pParam->dotMatrix, 30, 16, 79, 32, 5);

    pParam->signalInfo[0].freq  = 1.0f; // 初始化信号1频率
    pParam->signalInfo[1].freq  = 1.0f; // 初始化信号2频率
    pParam->signalInfo[0].amp   = 3.3f; // 初始化信号1幅度
    pParam->signalInfo[1].amp   = 3.3f; // 初始化信号2幅度
    pParam->signalInfo[0].phase = 0;    // 初始化信号1相位
    pParam->signalInfo[1].phase = 0;    // 初始化信号2相位
}

/**
 * @brief UI应用循环函数
 *
 * @param argument
 */
void uiAppLoop(void* argument) {
    // 遍历状态机列表，检查每个状态机的条件函数
    UIAppParamTypeDef* pParam = (UIAppParamTypeDef*)argument;

    // 处理前切换了缓冲区
    pParam->bufferIndex       = !pParam->bufferIndex; // 切换图形缓冲区索引

    for (int i = 0; i < sizeof(uiStateMachineList) / sizeof(UIStateTransitionTypeDef); i++) {
        if (uiStateMachineList[i].curState == pParam->curState) {
            if (pParam->eventGroup & (1 << uiStateMachineList[i].event) ||
                uiStateMachineList[i].event == UI_EVENT_NONE) {
                pParam->curState = uiStateMachineList[i].nextState;
                uiStateMachineList[i].actionFunc(argument);
                break;
            }
        }
    }

    pParam->eventGroup = 0;
}

/**
 * @brief 浏览状态下的动作函数
 *
 * @param argument
 */
static void actionWhileBrowse(void* argument) {
    UIAppParamTypeDef* pParam = (UIAppParamTypeDef*)argument;
    memset(pParam->dotMatrix, 0, HEIGHT * WIDTH);


    memcpy(pParam->graphicsBuffers[pParam->bufferIndex], img, sizeof(img)); // 恢复图形缓冲区

    sprintf((char*)(strBuffer[0]), "%.1fkHz", pParam->signalInfo[0].freq);
    sprintf((char*)(strBuffer[1]), "%.1fkHz", pParam->signalInfo[1].freq);
    sprintf((char*)(strBuffer[2]), "%.1f V", pParam->signalInfo[0].amp);
    sprintf((char*)(strBuffer[3]), "%.1f V", pParam->signalInfo[1].amp);
    sprintf((char*)(strBuffer[4]), "%d °", pParam->signalInfo[0].phase);
    sprintf((char*)(strBuffer[5]), "%d °", pParam->signalInfo[1].phase);


    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[0], 30, 29,
                                      79, 16);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[1], 76, 29,
                                      128, 16);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[2], 30, 46,
                                      79, 32);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[3], 76, 46,
                                      128, 32);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[4], 30, 62,
                                      79, 48);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[5], 76, 62,
                                      128, 48);



    browseAnimate(argument); // 浏览动画处理

    // 绘制当前选择信息的圆角矩形
    graphServIntf.drawRoundRect2DotMatrix(pParam->dotMatrix, pParam->selDispInfo.rectParam.startX,
                                          pParam->selDispInfo.rectParam.startY, pParam->selDispInfo.rectParam.endX,
                                          pParam->selDispInfo.rectParam.endY, pParam->selDispInfo.rectParam.radius);

    // 将圆角矩形区域颜色反转
    graphServIntf.InverBufferWithMask(pParam->dotMatrix, pParam->graphicsBuffers[pParam->bufferIndex]);
}

/**
 * @brief 编辑状态下的动作函数
 *
 * @param argument
 */
static void actionWhileEdit(void* argument) {
    UIAppParamTypeDef* pParam = (UIAppParamTypeDef*)argument;
    memset(pParam->dotMatrix, 0, HEIGHT * WIDTH);


    memcpy(pParam->graphicsBuffers[pParam->bufferIndex], img, sizeof(img)); // 恢复图形缓冲区

    updateSignal(argument);


    sprintf((char*)(strBuffer[0]), "%.1fkHz", pParam->signalInfo[0].freq);
    sprintf((char*)(strBuffer[1]), "%.1fkHz", pParam->signalInfo[1].freq);
    sprintf((char*)(strBuffer[2]), "%.1f V", pParam->signalInfo[0].amp);
    sprintf((char*)(strBuffer[3]), "%.1f V", pParam->signalInfo[1].amp);
    sprintf((char*)(strBuffer[4]), "%d °", pParam->signalInfo[0].phase);
    sprintf((char*)(strBuffer[5]), "%d °", pParam->signalInfo[1].phase);

    for (uint8_t i = sizeof(strBuffer) / sizeof(strBuffer[0]); i > 0; i--) {
        if (strBuffer[pParam->selectIndex][i - 1] != '\0') {
            strBuffer[pParam->selectIndex][i] = '*';
            break;
        }
    }

    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[0], 30, 29,
                                      79, 16);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[1], 76, 29,
                                      128, 16);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[2], 30, 46,
                                      79, 32);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[3], 76, 46,
                                      128, 32);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[4], 30, 62,
                                      79, 48);
    graphServIntf.printStringOnBuffer(pParam->graphicsBuffers[pParam->bufferIndex], (const char*)strBuffer[5], 76, 62,
                                      128, 48);



    // 绘制当前选择信息的圆角矩形
    graphServIntf.drawRoundRect2DotMatrix(pParam->dotMatrix, pParam->selDispInfo.rectParam.startX,
                                          pParam->selDispInfo.rectParam.startY, pParam->selDispInfo.rectParam.endX,
                                          pParam->selDispInfo.rectParam.endY, pParam->selDispInfo.rectParam.radius);

    // 将圆角矩形区域颜色反转
    graphServIntf.InverBufferWithMask(pParam->dotMatrix, pParam->graphicsBuffers[pParam->bufferIndex]);
}


/**
 * @brief 图形查看状态下的动作函数
 *
 * @param argument
 */
static void actionWhileFigureView(void* argument) {}


/**
 * @brief 浏览动画处理函数
 *
 * @param argument
 */
static void browseAnimate(void* argument) {

    UIAppParamTypeDef* pParam = (UIAppParamTypeDef*)argument;

    // 如果触发了切换选择事件
    if (pParam->eventGroup & (1 << UI_EVENT_SELECT_NEXT) || pParam->eventGroup & (1 << UI_EVENT_SELECT_PREV)) {

        if (pParam->animateData.transitionState) {
            // 如果上一次的动画没有加载完成，直接跳过
            pParam->animateData.transitionState = 0;                                // 过渡状态结束
            pParam->animateData.elapsed         = 0.0f;                             // 重置已经经过的时间
            pParam->animateData.pCurSelInfo     = pParam->animateData.pNextSelInfo; // 更新当前选择信息
        }

        // 进入过渡状态
        // 开始软定时器计时
        pParam->animateData.transitionState = 1;
        pParam->animateData.elapsed         = timeServIntf.getElapsedTime(pParam->browseAnimateTimer);


        // 设置下一个选择信息
        if (pParam->eventGroup & (1 << UI_EVENT_SELECT_NEXT)) {


            // 如果是下一个选择事件，更新选择索引并获取下一个选择信息
            uint8_t nextIndex = pParam->selectIndex;
            UI_SELECTED_GOTO_NEXT(nextIndex);
            pParam->animateData.pNextSelInfo = &uiSelInfoDispList[nextIndex];
            pParam->selectIndex              = (UISelectIndexEnum)nextIndex;

        } else if (pParam->eventGroup & (1 << UI_EVENT_SELECT_PREV)) {


            uint8_t prevIndex = pParam->selectIndex;
            UI_SELECTED_GOTO_PREV(prevIndex);
            pParam->animateData.pNextSelInfo = &uiSelInfoDispList[prevIndex];
            pParam->selectIndex              = (UISelectIndexEnum)prevIndex;
        }
        pParam->animateData.elapsed = 0;

    } else if (pParam->animateData.transitionState == 1) {

        // 如果处于过渡状态，更新动画数据

        pParam->animateData.elapsed += timeServIntf.getElapsedTime(pParam->browseAnimateTimer);

        // 计算过渡比例
        RectParamTypeDef rectParam = graphServIntf.animateMovingResizingRect(
            pParam->animateData.pCurSelInfo->rectParam.startX, pParam->animateData.pCurSelInfo->rectParam.startY,
            pParam->animateData.pCurSelInfo->rectParam.endX, pParam->animateData.pCurSelInfo->rectParam.endY,
            pParam->animateData.pNextSelInfo->rectParam.startX, pParam->animateData.pNextSelInfo->rectParam.startY,
            pParam->animateData.pNextSelInfo->rectParam.endX, pParam->animateData.pNextSelInfo->rectParam.endY,
            pParam->animateData.elapsed / pParam->animateData.duration);

        pParam->selDispInfo.rectParam.startX = rectParam.x0;
        pParam->selDispInfo.rectParam.startY = rectParam.y0;
        pParam->selDispInfo.rectParam.endX   = rectParam.x1;
        pParam->selDispInfo.rectParam.endY   = rectParam.y1;

        if (pParam->animateData.elapsed >= pParam->animateData.duration) {
            // 如果已经经过的时间超过了持续时间，结束过渡状态
            pParam->animateData.transitionState = 0;                                // 过渡状态结束
            pParam->animateData.elapsed         = 0.0f;                             // 重置已经经过的时间
            pParam->animateData.pCurSelInfo     = pParam->animateData.pNextSelInfo; // 更新当前选择信息
        }

    } else if (pParam->animateData.transitionState == 0) {

        // 如果不处于过渡状态，直接显示当前选择信息
        pParam->selDispInfo = *pParam->animateData.pCurSelInfo;
    }
}

void updateSignal(void* argument) {
    UIAppParamTypeDef* pParam = (UIAppParamTypeDef*)argument;

    // 更新信号信息
    switch (pParam->selectIndex) {
        case SIGNAL_1_FREQ: {
            if (pParam->eventGroup & (1 << UI_EVENT_VALUE_ADD)) {
                if (pParam->signalInfo[0].freq + FREQ_STEP <= FREQ_MAX) {
                    pParam->signalInfo[0].freq += FREQ_STEP; // 增加频率
                } else {
                    pParam->signalInfo[0].freq = FREQ_MAX; // 限制最大频率
                }
            } else if (pParam->eventGroup & (1 << UI_EVENT_VALUE_SUB)) {
                if (pParam->signalInfo[0].freq - FREQ_STEP >= FREQ_MIN) {
                    pParam->signalInfo[0].freq -= FREQ_STEP; // 减少频率
                } else {
                    pParam->signalInfo[0].freq = FREQ_MIN; // 限制最小频率
                }
            }

        } break;
        case SIGNAL_2_FREQ: {
            if (pParam->eventGroup & (1 << UI_EVENT_VALUE_ADD)) {
                if (pParam->signalInfo[1].freq + FREQ_STEP <= FREQ_MAX) {
                    pParam->signalInfo[1].freq += FREQ_STEP; // 增加频率
                } else {
                    pParam->signalInfo[1].freq = FREQ_MAX; // 限制最大频率
                }
            } else if (pParam->eventGroup & (1 << UI_EVENT_VALUE_SUB)) {
                if (pParam->signalInfo[1].freq - FREQ_STEP >= FREQ_MIN) {
                    pParam->signalInfo[1].freq -= FREQ_STEP; // 减少频率
                } else {
                    pParam->signalInfo[1].freq = FREQ_MIN; // 限制最小频率
                }
            }

        } break;
        case SIGNAL_1_AMP: {
            if (pParam->eventGroup & (1 << UI_EVENT_VALUE_ADD)) {
                if (pParam->signalInfo[0].amp + AMP_STEP <= AMP_MAX) {
                    pParam->signalInfo[0].amp += AMP_STEP; // 增加幅度
                } else {
                    pParam->signalInfo[0].amp = AMP_MAX; // 限制最大幅度
                }
            } else if (pParam->eventGroup & (1 << UI_EVENT_VALUE_SUB)) {
                if (pParam->signalInfo[0].amp - AMP_STEP >= AMP_MIN) {
                    pParam->signalInfo[0].amp -= AMP_STEP; // 减少幅度
                } else {
                    pParam->signalInfo[0].amp = AMP_MIN; // 限制最小幅度
                }
            }

        } break;
        case SIGNAL_2_AMP: {
            if (pParam->eventGroup & (1 << UI_EVENT_VALUE_ADD)) {
                if (pParam->signalInfo[1].amp + AMP_STEP <= AMP_MAX) {
                    pParam->signalInfo[1].amp += AMP_STEP; // 增加幅度
                } else {
                    pParam->signalInfo[1].amp = AMP_MAX; // 限制最大幅度
                }
            } else if (pParam->eventGroup & (1 << UI_EVENT_VALUE_SUB)) {
                if (pParam->signalInfo[1].amp - AMP_STEP >= AMP_MIN) {
                    pParam->signalInfo[1].amp -= AMP_STEP; // 减少幅度
                } else {
                    pParam->signalInfo[1].amp = AMP_MIN; // 限制最小幅度
                }
            }

        } break;
        case SIGNAL_1_PHASE: {
            if (pParam->eventGroup & (1 << UI_EVENT_VALUE_ADD)) {
                if (pParam->signalInfo[0].phase + PHASE_STEP <= PHASE_MAX) {
                    pParam->signalInfo[0].phase += PHASE_STEP; // 增加相位
                } else {
                    pParam->signalInfo[0].phase = PHASE_MAX; // 限制最大相位
                }
            } else if (pParam->eventGroup & (1 << UI_EVENT_VALUE_SUB)) {
                if (pParam->signalInfo[0].phase - PHASE_STEP >= PHASE_MIN) {
                    pParam->signalInfo[0].phase -= PHASE_STEP; // 减少相位
                } else {
                    pParam->signalInfo[0].phase = PHASE_MIN; // 限制最小相位
                }
            }

        } break;
        case SIGNAL_2_PHASE: {
            if (pParam->eventGroup & (1 << UI_EVENT_VALUE_ADD)) {
                if (pParam->signalInfo[1].phase + PHASE_STEP <= PHASE_MAX) {
                    pParam->signalInfo[1].phase += PHASE_STEP; // 增加相位
                } else {
                    pParam->signalInfo[1].phase = PHASE_MAX; // 限制最大相位
                }
            } else if (pParam->eventGroup & (1 << UI_EVENT_VALUE_SUB)) {
                if (pParam->signalInfo[1].phase - PHASE_STEP >= PHASE_MIN) {
                    pParam->signalInfo[1].phase -= PHASE_STEP; // 减少相位
                } else {
                    pParam->signalInfo[1].phase = PHASE_MIN; // 限制最小相位
                }
            }

        } break;
        default: {
            // 如果选择索引不在范围内，什么都不做
            break;
        }
    }
}
