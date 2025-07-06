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

static void drawLine(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
static void drawStarDot(uint8_t dotMatrix[HEIGHT][WIDTH], float centerX, float centerY, float radius);
static void bitToByte(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t graphBuffer[PAGE][WIDTH]);
static void drawRoundRect2DotMatrix(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t startX, uint8_t startY, uint8_t endX,
                                    uint8_t endY, uint8_t radius);
static void drawStar(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t graphBuffer[PAGE][WIDTH]);
static void InverBufferWithMask(uint8_t mask[HEIGHT][WIDTH], uint8_t buffer[PAGE][WIDTH]);
static void printStringOnBuffer(uint8_t buffer[PAGE][WIDTH], const char* str, uint8_t startX, uint8_t startY,
                                uint8_t endX, uint8_t endY);
static void printCharOnBuffer(uint8_t x, uint8_t y, const uint8_t font[16], uint8_t buffer[8][128]);





/* ------- variables -------------------------------------------------------------------------------------------------*/

const uint8_t font016x8[2][8]      = {{0x7F, 0x80, 0x88, 0x80, 0x7F, 0x00, 0x00, 0x00},
                                      {0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00}};
const uint8_t font116x8[2][8]      = {{0x81, 0x81, 0xFF, 0x80, 0x80, 0x00, 0x00, 0x00},
                                      {0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00}};
const uint8_t fontDot16x8[2][8]    = {{0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 小数点

GraphServIntfTypeDef graphServIntf = {
    .drawRoundRect2DotMatrix = drawRoundRect2DotMatrix,
    .bitToByte               = bitToByte,
    .drawStarDot             = drawStarDot,
    .drawLine                = drawLine,
    .InverBufferWithMask     = InverBufferWithMask,
    .printStringOnBuffer     = printStringOnBuffer,
};

// 字体
FontTypeDef font[] = {
    {'0', 9, 6, (uint8_t*)font016x8},
    {'1', 9, 6, (uint8_t*)font116x8},
    {'.', 2, 2, (uint8_t*)fontDot16x8},
};




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
void drawStar(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t graphBuffer[PAGE][WIDTH]) {

    float centerX = WIDTH / 2;
    float centerY = HEIGHT / 2;
    float radius  = 25;

    drawStarDot(dotMatrix, centerX, centerY, radius);
    bitToByte(dotMatrix, graphBuffer);
}

/**
 * @brief 点阵中绘制圆角矩形
 *
 * @param dotMatrix
 * @param startX
 * @param startY
 * @param endX
 * @param endY
 * @param radius
 */
void drawRoundRect2DotMatrix(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t startX, uint8_t startY, uint8_t endX,
                             uint8_t endY, uint8_t radius) {
    // 确保起始和结束坐标合法
    if (startX >= WIDTH || startY >= HEIGHT || endX >= WIDTH || endY >= HEIGHT || startX > endX || startY > endY) {
        return;
    }

    // 逐行绘制
    for (uint8_t y = startY; y <= endY; y++) {
        uint8_t lightUpFromX;
        uint8_t lightUpToX;

        // 计算当前行的起始和结束点
        // 1.位于上方圆角区域
        if (y < startY + radius) {
            uint8_t l       = y - startY;
            uint8_t xOffset = radius - (uint8_t)sqrt(2 * l * radius - 2 * l - l * l) - 1;
            lightUpFromX    = startX + xOffset;
            lightUpToX      = endX - xOffset;
        } else if (y < endY - radius + 1) {
            // 2.位于中间区域
            lightUpFromX = startX;
            lightUpToX   = endX;
        } else {
            // 3.位于下方圆角区域
            uint8_t l       = endY - y; // 计算当前行到下方圆角的距离
            uint8_t xOffset = radius - (uint8_t)sqrt(2 * l * radius - 2 * l - l * l) - 1;
            lightUpFromX    = startX + xOffset;
            lightUpToX      = endX - xOffset;
        }

        for (uint8_t x = lightUpFromX; x <= lightUpToX; x++) {

            dotMatrix[y][x] = 1; // 设置点阵图像素为1
        }
    }
}


/**
 * @brief 反转图形缓冲区中指定掩码的像素
 *
 * @param mask 掩码数组，指示哪些像素需要被反转
 * @param buffer 图形缓冲区
 */
void InverBufferWithMask(uint8_t mask[HEIGHT][WIDTH], uint8_t buffer[PAGE][WIDTH]) {
    for (uint8_t y = 0; y < 64; y++) {
        for (uint8_t x = 0; x < 128; x++) {
            if (mask[y][x]) {
                uint8_t page    = y / 8;
                uint8_t bit_pos = y % 8;
                buffer[page][x] ^= (1 << bit_pos);
            }
        }
    }
}

void printStringOnBuffer(uint8_t buffer[PAGE][WIDTH], const char* str, uint8_t startX, uint8_t startY, uint8_t endX,
                         uint8_t endY) {
    uint8_t totalWidth  = 0; // 当前字符串总宽度
    uint8_t totalHeight = 0; // 当前字符串总高度
    for (uint8_t i = 0; str[i] != '\0'; i++) {
        for (uint8_t j = 0; j < sizeof(font) / sizeof(FontTypeDef); j++) {
            if (str[i] == font[j].character) {
                totalWidth += font[j].width; // 累加字符宽度
                if (font[j].height > totalHeight) {
                    totalHeight = font[j].height; // 更新总高度
                }
                break;
            }
        }
    }

    uint8_t offsetX = 0;
    uint8_t offsetY = 0;


    if (totalWidth < (endX - startX)) {
        offsetX = (endX - startX - totalWidth) / 2; // 水平居中
    }


    if (totalHeight < (startY - endY)) {
        offsetY = (startY - endY - totalHeight) / 2; // 垂直居中
    }

    uint8_t cursor = startX + offsetX;
    uint8_t charY  = startY - offsetY; // 字符的起始Y坐标

    for (uint8_t i = 0; str[i] != '\0'; i++) {
        for (uint8_t j = 0; j < sizeof(font) / sizeof(FontTypeDef); j++) {
            if (str[i] == font[j].character) {
                // 计算字符在缓冲区中的起始位置
                // 打印字符到缓冲区
                printCharOnBuffer(cursor, charY, font[j].fontByte, buffer);
                cursor += font[j].width; // 更新光标位置
                break;
            }
        }
    }
}


void printCharOnBuffer(uint8_t x, uint8_t y, const uint8_t fontByte[16], uint8_t buffer[8][128]) {
    if (x > 120 || y > 63)
        return; // 越界检查

    uint8_t page      = y / 8;     // 起始页（从下往上）
    uint8_t bitOffset = 8 - y % 8; // 像素行偏移（不为0说明跨页）

    for (uint8_t i = 0; i < 8; i++) {    // 字符列数 = 8
        uint8_t lower = fontByte[i];     // 字模下半部分（低位）
        uint8_t upper = fontByte[i + 8]; // 字模上半部分（高位）

        // 合成到页 buffer 中（注意跨页处理）
        if (bitOffset == 0) {
            // 恰好对齐页边界
            buffer[page - 1][x + i] |= lower;
            if (page - 2 >= 0) {
                buffer[page - 2][x + i] |= upper;
            }
        } else {
            // 跨页，需要移位叠加
            uint8_t lowerShifted = lower >> bitOffset;
            uint8_t upperShifted = (upper >> bitOffset) | (lower << (8 - bitOffset));
            uint8_t top          = upper << (8 - bitOffset);

            buffer[page][x + i] |= lowerShifted;
            buffer[page - 1][x + i] |= upperShifted;
            if (page - 2 >= 0) {
                buffer[page - 2][x + i] |= top;
            }
        }
    }
}
