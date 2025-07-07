/**
 ***********************************************************************************************************************
 * @file           : graph-service.h
 * @brief          : 图形绘制驱动
 * @author         : 李嘉豪
 * @date           : 2025-04-29
 ***********************************************************************************************************************
 * @attention
 *
 * None
 *
 ***********************************************************************************************************************
 **/




/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/

#ifndef __GRAPH_SERVICE_H__
#define __GRAPH_SERVICE_H__




/*-------- includes --------------------------------------------------------------------------------------------------*/

#include <stdint.h>




/*-------- define ----------------------------------------------------------------------------------------------------*/

#ifndef WIDTH
#define WIDTH 128
#endif /* WIDTH */
#ifndef HEIGHT
#define HEIGHT 64
#endif /* HEIGHT */
#ifndef PAGE
#define PAGE (HEIGHT / 8)
#endif /* PAGE */
#ifndef PI
#define PI 3.14159265358979323846f
#endif /* PI */




/*-------- typedef ---------------------------------------------------------------------------------------------------*/

typedef struct {
    uint8_t character;
    uint8_t height;
    uint8_t width;
    uint8_t* fontByte;
} FontTypeDef;

typedef struct {
    uint8_t x0;
    uint8_t y0;
    uint8_t x1;
    uint8_t y1;
} RectParamTypeDef; // 矩形参数类型定义

typedef struct {
    uint8_t x;
    uint8_t y;
} PointTypeDef; // 点类型定义

typedef struct {
    void (*drawStar)(uint8_t graphBuffer[PAGE][WIDTH]); // 绘制五角星函数
    void (*drawRoundRect2DotMatrix)(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t startX, uint8_t startY, uint8_t endX,
                                    uint8_t endY, uint8_t radius);                         // 绘制圆角矩形到点阵图像素
    void (*bitToByte)(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t graphBuffer[PAGE][WIDTH]); // 点阵图像素转换为字节数组
    void (*drawStarDot)(uint8_t dotMatrix[HEIGHT][WIDTH], float centerX, float centerY, float radius); // 绘制五角星点阵
    void (*drawLine)(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t startX, uint8_t startY, uint8_t endX,
                     uint8_t endY);                                                        // 绘制线段
    void (*InverBufferWithMask)(uint8_t mask[HEIGHT][WIDTH], uint8_t buffer[PAGE][WIDTH]); // 使用掩码反转缓冲区
    uint8_t dotMatrix[HEIGHT][WIDTH];
    void (*printStringOnBuffer)(uint8_t buffer[PAGE][WIDTH], const char* str, uint8_t startX, uint8_t startY,
                                uint8_t endX, uint8_t endY); // 点阵图
    RectParamTypeDef (*animateMovingResizingRect)(uint8_t sx0, uint8_t sy0, uint8_t sx1, uint8_t sy1, uint8_t ex0,
                                                  uint8_t ey0, uint8_t ex1, uint8_t ey1, float progress);

    void (*insertNewPoint)(uint8_t new_x, uint8_t new_y, uint8_t pixelDrawCount[HEIGHT][WIDTH]); // 插入新点到队列

} GraphServIntfTypeDef;





/*-------- macro -----------------------------------------------------------------------------------------------------*/

#define MAP_ADC_TO_OLED_X(x) (x * 60 / 4095 + 34) // 将ADC值映射到OLED X坐标范围
#define MAP_ADC_TO_OLED_Y(y) (y * 60 / 4095 + 2)  // 将ADC值映射到OLED Y坐标范围




/*-------- variables -------------------------------------------------------------------------------------------------*/

extern GraphServIntfTypeDef graphServIntf;




/*-------- function prototypes ---------------------------------------------------------------------------------------*/





#endif /* __GRAPH_SERVICE_H__ */
