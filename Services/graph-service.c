/**
 ***********************************************************************************************************************
 * @file           : graph-service.c
 * @brief          : 图形绘制驱动
 * @author         : 李嘉豪
 * @date           : 2025-04-29
 ***********************************************************************************************************************
 * @attention
 *
 * 屏幕绘制图形函数封装
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "graph-service.h"
#include <math.h>




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/






/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/





/* ------- variables -------------------------------------------------------------------------------------------------*/

uint8_t dotMatrix[HEIGHT][WIDTH] = {0}; // 点阵图




/* ------- function implement ----------------------------------------------------------------------------------------*/

/**
 * @brief Bresenham算法绘制线段
 *
 * @param dotMatrix
 * @param x0
 * @param y0
 * @param x1
 * @param y1
 */
static void drawLine(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    int dx  = (x1 - x0) > 0 ? (x1 - x0) : (x0 - x1);
    int dy  = (y1 - y0) > 0 ? (y0 - y1) : (y1 - y0);
    int sx  = x0 < x1 ? 1 : -1;
    int sy  = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        // 在合法范围内设置像素点
        if (x0 < WIDTH && y0 < HEIGHT) {
            dotMatrix[y0][x0] = 1;
        }

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

/**
 * @brief 绘制五角星
 *
 * @param dotMatrix
 * @param centerX
 * @param centerY
 * @param radius
 */
static void drawStarDot(uint8_t dotMatrix[HEIGHT][WIDTH], float centerX, float centerY, float radius) {
    float theta = PI * 2 / 5;
    uint8_t vertex[5][2];
    for (uint8_t i = 0; i < 5; i++) {
        vertex[i][0] = (uint8_t)(radius * cos(theta * i + PI / 10) + centerX);
        vertex[i][1] = (uint8_t)(radius * sin(theta * i + PI / 10) + centerY);
    }
    for (uint8_t i = 0; i < 5; i++) {
        drawLine(dotMatrix, vertex[i][0], vertex[i][1], vertex[(i + 2) % 5][0], vertex[(i + 2) % 5][1]);
        drawLine(dotMatrix, vertex[i][0], vertex[i][1], vertex[(i + 2) % 5][0], vertex[(i + 2) % 5][1]);
    }
}

/**
 * @brief 将点阵图转换为字节数组
 *
 * @param dotMatrix 点阵图
 * @param graphBuffer 字节数组
 */
static void bitToByte(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t graphBuffer[PAGE][WIDTH]) {
    for (uint16_t col = 0; col < WIDTH; col++) {
        for (uint16_t page = 0; page < PAGE; page++) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                int y = page * 8 + bit;
                if (y < HEIGHT && dotMatrix[y][col]) {
                    byte |= (1 << bit); // bit0对应页顶部像素
                }
                if (page >= 7) {
                    byte = 0;
                }
            }
            graphBuffer[page][col] = byte;
        }
    }
}

/**
 * @brief 绘制五角星
 *
 * @param graphBuffer 字节数组
 */
void drawStar(uint8_t graphBuffer[PAGE][WIDTH]) {

    float centerX                    = WIDTH / 2;
    float centerY                    = HEIGHT / 2;
    float radius                     = 25;

    drawStarDot(dotMatrix, centerX, centerY, radius);
    bitToByte(dotMatrix, graphBuffer);
}
