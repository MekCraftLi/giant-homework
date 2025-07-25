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
#include <string.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/

#define MAX_POINTS 1000




/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

static void drawLine(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
static void drawStarDot(uint8_t dotMatrix[HEIGHT][WIDTH], float centerX, float centerY, float radius);
static void bitToByte(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t graphBuffer[PAGE][WIDTH]);
static void drawRoundRect2DotMatrix(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t startX, uint8_t startY, uint8_t endX,
                                    uint8_t endY, uint8_t radius, uint8_t);
#if 0
static void drawStar(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t graphBuffer[PAGE][WIDTH]);
#endif
static void InverBufferWithMask(uint8_t mask[HEIGHT][WIDTH], uint8_t buffer[PAGE][WIDTH]);
static void printStringOnBuffer(uint8_t buffer[PAGE][WIDTH], const char* str, uint8_t startX, uint8_t startY,
                                uint8_t endX, uint8_t endY);
static void printCharOnBuffer(uint8_t x, uint8_t y, const uint8_t font[16], uint8_t buffer[8][128]);
RectParamTypeDef animateMovingResizingRect(uint8_t sx0, uint8_t sy0, uint8_t sx1, uint8_t sy1, uint8_t ex0, uint8_t ey0,
                                           uint8_t ex1, uint8_t ey1, float progress);
static void insertNewPoint(uint8_t new_x, uint8_t new_y, uint8_t pixelDrawCount[HEIGHT][WIDTH]);
static void blendImagesWithSineScroll(uint8_t imageA[PAGE][WIDTH], uint8_t imageB[PAGE][WIDTH], uint8_t shift,
                                      uint8_t direction, uint8_t result[PAGE][WIDTH]);



/* ------- variables -------------------------------------------------------------------------------------------------*/

#if 0
const uint8_t font016x8[2][8] = { {0x7F, 0x80, 0x88, 0x80, 0x7F, 0x00, 0x00, 0x00},
                                   {0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00} };
const uint8_t font116x8[2][8]      = {{0x81, 0x81, 0xFF, 0x80, 0x80, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00}};
const uint8_t font516x8[2][8] = { {0x67, 0x84, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00},
                                      {0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00} };
const uint8_t font216x8[2][8]      = {{0x83, 0xC0, 0xA0, 0x90, 0x8F, 0x00, 0x00, 0x00},
                                      {0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00}};
const uint8_t font316x8[2][8]      = {{0x60, 0x80, 0x86, 0x85, 0x78, 0x00, 0x00, 0x00},
                                      {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80}};
const uint8_t font416x8[2][8]      = {{0x30, 0x2C, 0x20, 0x03, 0x20, 0x00, 0x00, 0x00},
                                      {0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00}};

const uint8_t font616x8[2][8]      = {{0x78, 0x8C, 0x85, 0x88, 0x78, 0x00, 0x00, 0x00},
                                      {0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00}};
const uint8_t font716x8[2][8]      = {{0x00, 0x80, 0xE0, 0x1C, 0x03, 0x00, 0x00, 0x00},
                                      {0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00}};
const uint8_t font816x8[2][8]      = {{0x77, 0x88, 0x88, 0x88, 0x77, 0x00, 0x00, 0x00},
                                      {0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00}};
const uint8_t font916x8[2][8]      = {{0x0F, 0x10, 0xD0, 0x30, 0x1F, 0x00, 0x00, 0x00},
                                      {0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00}};
#else
const uint8_t font016x8[2][8] = {{0x7C, 0xC2, 0x81, 0x80, 0x40, 0x20, 0x1F, 0x00},
                                 {0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00}};
const uint8_t font116x8[2][8] = {{0x80, 0x81, 0xF9, 0x87, 0x80, 0x00, 0x00, 0x00},
                                 {0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00}};
const uint8_t font216x8[2][8] = {{0xC1, 0xA0, 0x90, 0x8C, 0x83, 0x00, 0x00, 0x00},
                                 {0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00}};
const uint8_t font316x8[2][8] = {{0x80, 0x80, 0x88, 0x88, 0xCC, 0x73, 0x00, 0x00},
                                 {0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00}};
const uint8_t font416x8[2][8] = {{0x18, 0x14, 0x13, 0xD0, 0x3C, 0x17, 0x10, 0x00},
                                 {0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00}};
const uint8_t font516x8[2][8] = {{0x80, 0x87, 0x84, 0x4C, 0x28, 0x00, 0x00, 0x00},
                                 {0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00}};
const uint8_t font616x8[2][8] = {{0x7C, 0xCB, 0x85, 0x84, 0x44, 0x38, 0x00, 0x00},
                                 {0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00}};
const uint8_t font716x8[2][8] = {{0xC0, 0x30, 0x0C, 0x02, 0x01, 0x00, 0x00, 0x00},
                                 {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00}};
const uint8_t font816x8[2][8] = {{0x70, 0xCB, 0x84, 0x88, 0x9C, 0x73, 0x00, 0x00},
                                 {0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00}};
const uint8_t font916x8[2][8] = {{0x8E, 0x91, 0x90, 0xD0, 0x69, 0x1F, 0x00, 0x00},
                                 {0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00}};


#endif

const uint8_t fontDot16x8[2][8]    = {{0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // 小数点

const uint8_t fontk16x8[2][8]      = {{0xE0, 0x3C, 0x13, 0xE8, 0x04, 0x00, 0x00, 0x00},
                                      {0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00}};
const uint8_t fontH16x8[2][8]      = {{0xC0, 0x3C, 0x27, 0x10, 0x10, 0xF0, 0x1C, 0x03},
                                      {0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80}};
const uint8_t fontz16x8[2][8]      = {{0xC0, 0xB8, 0xA8, 0x98, 0x48, 0x00, 0x00, 0x00},
                                      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
const uint8_t fontV16x8[2][8]      = {{0x7F, 0x80, 0x40, 0x20, 0x18, 0x07, 0x01, 0x00},
                                      {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00}};
const uint8_t fontDeg16x8[2][8]    = {{0x07, 0x05, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00},
                                      {0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00}};
const uint8_t fontStar16x8[2][8]   = {{0x05, 0x0F, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00},
                                      {0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};




GraphServIntfTypeDef graphServIntf = {
    .drawRoundRect2DotMatrix   = drawRoundRect2DotMatrix,
    .bitToByte                 = bitToByte,
    .drawStarDot               = drawStarDot,
    .drawLine                  = drawLine,
    .InverBufferWithMask       = InverBufferWithMask,
    .printStringOnBuffer       = printStringOnBuffer,
    .animateMovingResizingRect = animateMovingResizingRect,
    .insertNewPoint            = insertNewPoint,
    .blendImagesWithSineScroll = blendImagesWithSineScroll,
};

// 字体
FontTypeDef font[] = {
    {'0', 9, 6, (uint8_t*)font016x8},    {'1', 9, 6, (uint8_t*)font116x8},
    {'.', 2, 4, (uint8_t*)fontDot16x8},  {'2', 9, 6, (uint8_t*)font216x8},
    {'3', 9, 6, (uint8_t*)font316x8},    {'4', 9, 6, (uint8_t*)font416x8},
    {'5', 9, 7, (uint8_t*)font516x8},    {'6', 9, 6, (uint8_t*)font616x8},
    {'7', 9, 6, (uint8_t*)font716x8},    {'8', 9, 6, (uint8_t*)font816x8},
    {'9', 9, 6, (uint8_t*)font916x8},    {'k', 9, 5, (uint8_t*)fontk16x8},
    {'H', 16, 7, (uint8_t*)fontH16x8},   {'z', 5, 5, (uint8_t*)fontz16x8},
    {'V', 9, 8, (uint8_t*)fontV16x8},    {(uint16_t)"°"[0], 8, 5, (uint8_t*)fontDeg16x8}, // 度符号
    {'*', 8, 5, (uint8_t*)fontStar16x8},                                                  // 星号
};

static PointTypeDef points[MAX_POINTS]; // 点阵图点存储



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
    uint8_t dx  = (x1 - x0) > 0 ? (x1 - x0) : (x0 - x1);
    uint8_t dy  = (y1 - y0) > 0 ? (y0 - y1) : (y1 - y0);
    uint8_t sx  = x0 < x1 ? 1 : -1;
    uint8_t sy  = y0 < y1 ? 1 : -1;
    uint8_t err = dx + dy;

    while (1) {
        // 在合法范围内设置像素点
        if (x0 < WIDTH && y0 < HEIGHT) {
            dotMatrix[y0][x0] = 1;
        }

        if (x0 == x1 && y0 == y1)
            break;

        uint8_t e2 = 2 * err;
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
            for (uint8_t bit = 0; bit < 8; bit++) {
                uint8_t y = page * 8 + bit;
                if (y < HEIGHT && dotMatrix[y][col]) {
                    byte |= (1 << bit); // bit0对应页顶部像素
                }
            }
            graphBuffer[page][col] = byte;
        }
    }
}

#if 0
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
#endif

/**
 * @brief 点阵中绘制圆角矩形
 *
 * @param dotMatrix 点阵图像素数组
 * @param startX 起始X坐标
 * @param startY 起始Y坐标
 * @param endX 结束X坐标
 * @param endY 结束Y坐标
 * @param radius 圆角半径
 */
void drawRoundRect2DotMatrix(uint8_t dotMatrix[HEIGHT][WIDTH], uint8_t startX, uint8_t startY, uint8_t endX,
                             uint8_t endY, uint8_t radius, uint8_t padding) {


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

        if (padding) {

            for (uint8_t x = lightUpFromX; x <= lightUpToX; x++) {

                dotMatrix[y][x] = 1; // 设置点阵图像素为1
            }
        } else {
            // 如果不需要填充，则只绘制边框
            if (y == startY || y == endY) {
                for (uint8_t x = lightUpFromX; x <= lightUpToX; x++) {
                    dotMatrix[y][x] = 1; // 设置点阵图像素为1
                }
            } else {
                dotMatrix[y][lightUpFromX] = 1; // 左边框
                dotMatrix[y][lightUpToX]   = 1; // 右边框
            }
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

/**
 * @brief 在图形缓冲区中在一定范围内居中打印字符串
 *
 * @param buffer 图形缓冲区
 * @param str 要打印的字符串
 * @param startX // 打印起始X坐标
 * @param startY // 打印起始Y坐标
 * @param endX // 打印结束X坐标
 * @param endY // 打印结束Y坐标
 * @note 字符串会在指定范围内水平和垂直居中对齐
 */
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
        if (str[i] == ' ') {
            cursor += 4; // 空格宽度为4
            continue;    // 跳过空格
        }
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


/**
 * @brief 在图形缓冲区中打印单个字符
 *
 * @param x 左下角X坐标
 * @param y 左下角Y坐标
 * @param fontByte 字符对应的字模数据
 * @param buffer 图形缓冲区
 */
void printCharOnBuffer(uint8_t x, uint8_t y, const uint8_t fontByte[16], uint8_t buffer[8][128]) {
    if (x > 120 || y > 63) {
        return; // 越界检查
	}

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


/**
 * @brief 平滑动画过渡矩形位置和大小
 *
 * @param sx0 起始矩形左上角X坐标
 * @param sy0 起始矩形左上角Y坐标
 * @param sx1 起始矩形右下角X坐标
 * @param sy1 起始矩形右下角Y坐标
 * @param ex0 结束矩形左上角X坐标
 * @param ey0 结束矩形左上角Y坐标
 * @param ex1 结束矩形右下角X坐标
 * @param ey1 结束矩形右下角Y坐标
 * @param progress 动画进度 [0, 1]，0表示起始状态，1表示结束状态
 * @return RectParamTypeDef
 */
RectParamTypeDef animateMovingResizingRect(uint8_t sx0, uint8_t sy0, uint8_t sx1, uint8_t sy1, uint8_t ex0, uint8_t ey0,
                                           uint8_t ex1, uint8_t ey1, float progress) {
    RectParamTypeDef r;

    // 限制进度在 [0,1]
    if (progress < 0.0f)
        progress = 0.0f;
    if (progress > 1.0f)
        progress = 1.0f;

    // 平滑位置过渡（sine in-out）
    float posFactor   = 0.5f * (1 - cosf(PI * progress));

    // 尺寸缩放：1 → 0.3 → 1，最大压缩出现在 progress=0.5
    float scaleFactor = 0.8f + 0.2f * cosf(2 * PI * progress);
    // 范围 [0.3, 1.0]

    // 起始中心和大小
    float sx          = (sx0 + sx1) / 2.0f;
    float sy          = (sy0 + sy1) / 2.0f;
    float sw          = sx1 - sx0;
    float sh          = sy1 - sy0;

    // 目标中心和大小
    float ex          = (ex0 + ex1) / 2.0f;
    float ey          = (ey0 + ey1) / 2.0f;
    float ew          = ex1 - ex0;
    float eh          = ey1 - ey0;

    // 当前中心位置
    float cx          = sx + (ex - sx) * posFactor;
    float cy          = sy + (ey - sy) * posFactor;

    // 当前宽高
    float w           = (sw + (ew - sw) * posFactor) * scaleFactor;
    float h           = (sh + (eh - sh) * posFactor) * scaleFactor;

    // 当前矩形
    r.x0              = (uint8_t)(cx - w / 2.0f);
    r.y0              = (uint8_t)(cy - h / 2.0f);
    r.x1              = (uint8_t)(cx + w / 2.0f);
    r.y1              = (uint8_t)(cy + h / 2.0f);

    return r;
}


/**
 * @brief 点阵队列中插入新点
 *
 * @param new_y
 * @param new_x
 * @param pixelDrawCount
 */
void insertNewPoint(uint8_t new_y, uint8_t new_x, uint8_t pixelDrawCount[HEIGHT][WIDTH]) {
    static uint16_t count;
    static uint16_t head = 0; // 队列头指针

    // 如果队列满，删除最旧的点（从 head 出队）
    if (count == MAX_POINTS) {
        PointTypeDef old = points[head];

        if (pixelDrawCount[old.y][old.x] > 0) {
            pixelDrawCount[old.y][old.x]--;
        }

        head = (head + 1) % MAX_POINTS;
        count--;
    }

    // 插入新点到 tail
    uint16_t tail  = (head + count) % MAX_POINTS;
    points[tail].x = new_x;
    points[tail].y = new_y;
    count++;

    // 增加新点计数

    pixelDrawCount[new_y][new_x]++;
}

/**
 * @brief 使用正弦滚动效果混合两张图像
 *
 * @param imageA 第一张图像
 * @param imageB 第二张图像
 * @param ratio 混合比例，范围从 0.0 到 1.0
 * @param direction 滚动方向，0 表示向左，1 表示向右
 * @param result 混合后的结果图像
 */
static void blendImagesWithSineScroll(uint8_t imageA[PAGE][WIDTH], uint8_t imageB[PAGE][WIDTH], uint8_t shift,
                                      uint8_t direction, uint8_t result[PAGE][WIDTH]) {

    for (uint8_t row = 0; row < PAGE; row++) {
        if (direction == 0) // 向左滑动：A 向左退，B 从右入
        {
            uint8_t partA = WIDTH - shift;
            uint8_t partB = shift;

            if (partA > 0) {
                memcpy(&result[row][0], &imageA[row][shift], partA);
            }
            if (partB > 0) {
                memcpy(&result[row][partA], &imageB[row][0], partB);
            }
        } else // 向右滑动：A 向右退，B 从左入
        {
            uint8_t partB = shift;
            uint8_t partA = WIDTH - shift;

            if (partB > 0) {
                memcpy(&result[row][0], &imageB[row][WIDTH - shift], partB);
            }
            if (partA > 0) {
                memcpy(&result[row][partB], &imageA[row][0], partA);
            }
        }
    }
}
