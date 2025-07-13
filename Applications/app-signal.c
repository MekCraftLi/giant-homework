/**
 ***********************************************************************************************************************
 * @file           : app-signal.c
 * @brief          : 信号处理和生成
 * @author         : 李嘉豪
 * @date           : 2025-07-07
 ***********************************************************************************************************************
 * @attention
 *
 * 信号处理和生成应用
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "../Peripherals/adc.h"
#include "../Peripherals/dac.h"
#include "../Peripherals/dma.h"
#include "../Peripherals/gpio.h"
#include "../Peripherals/tim.h"
#include "app-signal.h"
#include <math.h>





/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/

#define PI             3.1415926535897932384626433832795f
#define DAC_RESOLUTION 4095
#define SAMPLING_FREQ  500 // 采样频率为500Hz



/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

void generateSineWave512(uint16_t* buf, float freq, float amplitude, float phase_deg);




/* ------- variables -------------------------------------------------------------------------------------------------*/

static DMAObjTypeDef dacChannel1DMA; // DAC通道1的DMA对象
static DMAObjTypeDef dacChannel2DMA; // DAC通道2的DMA对象
static DMAObjTypeDef adcChannel1DMA; // ADC通道1的DMA对象

static TIMObjTypeDef adcSamplingTimer; // ADC采样定时器对象
static TIMObjTypeDef dacTimer;         // 主定时器对象
static TIMObjTypeDef debugTimer;       // 调试用




/* ------- function implement ----------------------------------------------------------------------------------------*/

void signalAppInit(void* argument) {
    SignalAppParamTypeDef* pSignalParam = (SignalAppParamTypeDef*)argument;

    generateSineWave512(pSignalParam->sign[0], pSignalParam->signalInfo[0].freq, pSignalParam->signalInfo[0].amp,
                        pSignalParam->signalInfo[0].phase);
    generateSineWave512(pSignalParam->sign[1], pSignalParam->signalInfo[1].freq, pSignalParam->signalInfo[1].amp,
                        pSignalParam->signalInfo[1].phase);

    // 1. 初始化GPIO
    gpioIntf.pinInit(PORT_A, PIN_4, INPUT_ANALOG);
    gpioIntf.pinInit(PORT_A, PIN_5, INPUT_ANALOG);
    gpioIntf.pinInit(PORT_A, PIN_0, INPUT_ANALOG); // ADC1通道0
    gpioIntf.pinInit(PORT_A, PIN_1, INPUT_ANALOG); // ADC2通道1




    // 2. 初始化DMA
    dmaIntf.init(&dacChannel1DMA, DMA2_Channel3, DMA_Priority_Medium);
    dmaIntf.init(&dacChannel2DMA, DMA2_Channel4, DMA_Priority_Medium);
    dmaIntf.init(&adcChannel1DMA, DMA1_Channel1, DMA_Priority_Medium);

    dmaIntf.setSorceCycle(&dacChannel1DMA, (uint32_t)pSignalParam->sign[0], DMA_SIZE_HALF_WORD, WAVE_LEN * 2);
    dmaIntf.setSorceCycle(&dacChannel2DMA, (uint32_t)pSignalParam->sign[1], DMA_SIZE_HALF_WORD, WAVE_LEN * 2);
    dmaIntf.setSorce(&adcChannel1DMA, (uint32_t)&ADC1->DR, DMA_SIZE_WORD, 1); // ADC采样数据

    dmaIntf.setDest(&dacChannel1DMA, (uint32_t)&DAC->DHR12R1, DMA_SIZE_HALF_WORD, 1);
    dmaIntf.setDest(&dacChannel2DMA, (uint32_t)&DAC->DHR12R2, DMA_SIZE_HALF_WORD, 1);
    dmaIntf.setDest(&adcChannel1DMA, (uint32_t)&pSignalParam->adcData.adcBuf, DMA_SIZE_WORD, 1); // ADC采样数据存储

    dmaIntf.start(&dacChannel1DMA); // 启动DAC通道1的DMA
    dmaIntf.start(&dacChannel2DMA); // 启动DAC通道2的DMA
    dmaIntf.start(&adcChannel1DMA);

    // 3. 初始化DAC
    dacInit();

    // 4. 初始化ADC
    adcInit();

    // 5. 配置定时器
    timIntf.init(&dacTimer, TIM2);
    timIntf.init(&adcSamplingTimer, TIM7);

    // 设定频率

    timIntf.countConfig(&dacTimer, dacTimer.clkFreq, 120);

    // 设定触发源
    TIM_SelectOutputTrigger(dacTimer.tim, TIM_TRGOSource_Update);

    // 设定频率
    timIntf.setFrequency(&adcSamplingTimer, SAMPLING_FREQ); // 配置ADC采样定时器为500Hz
    timIntf.enableISR(&adcSamplingTimer);                   // 使能ADC采样定时器中断

    // 设定触发源
    TIM_SelectOutputTrigger(dacTimer.tim, TIM_TRGOSource_Update);

    timIntf.start(&dacTimer); // 启动DAC通道1定时器
    timIntf.start(&debugTimer);

    TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 触发ADC1开始转换
}

void signalAppLoop(void* argument) {
    SignalAppParamTypeDef* pSignalParam = (SignalAppParamTypeDef*)argument;
    if (pSignalParam->updateFlag) {

        DMA_Cmd(DMA2_Channel3, DISABLE);    // 禁用DAC通道1的DMA
        DMA_Cmd(DMA2_Channel4, DISABLE);    // 禁用DAC通道2的DMA
        DAC_DMACmd(DAC_Channel_1, DISABLE); // 禁用DAC通道1
        DAC_DMACmd(DAC_Channel_2, DISABLE); // 禁用DAC通道2

        // 重新生成波形数据
        generateSineWave512(pSignalParam->sign[0], pSignalParam->signalInfo[0].freq, pSignalParam->signalInfo[0].amp,
                            pSignalParam->signalInfo[0].phase);
        generateSineWave512(pSignalParam->sign[1], pSignalParam->signalInfo[1].freq, pSignalParam->signalInfo[1].amp,
                            pSignalParam->signalInfo[1].phase);

        DMA2_Channel3->CNDTR = WAVE_LEN * 2; // 设置DMA传输数据长度
        DMA2_Channel4->CNDTR = WAVE_LEN * 2; // 设置DMA传输数据长度

        DAC_DMACmd(DAC_Channel_1, ENABLE); // 使能DAC通道1的DMA
        DAC_DMACmd(DAC_Channel_2, ENABLE); // 使能DAC通道2的DMA
        DMA_Cmd(DMA2_Channel3, ENABLE);    // 启动DAC通道1的DMA
        DMA_Cmd(DMA2_Channel4, ENABLE);    // 启动DAC通道2的DMA


        pSignalParam->updateFlag = 0; // 清除更新标志
    }
}


void generateSineWave512(uint16_t* buf, float freq, float amplitude, float phase_deg) {
    float phase_rad = phase_deg * PI / 180.0f;

    for (uint16_t i = 0; i < WAVE_LEN * 2; ++i) {

        float angle  = 2.0f * PI * i * freq / WAVE_LEN + phase_rad;
        float value  = sinf(angle) + 1; // 范围：-1 ~ +1

        // 幅度调整为 0~amplitude，然后加上 offset
        float scaled = value * (amplitude / 3.3f) * DAC_RESOLUTION / 2 * 3.3f / 3.3f;

        // 限制范围到 0~4095
        if (scaled < 0)
            scaled = 0;
        if (scaled > DAC_RESOLUTION)
            scaled = DAC_RESOLUTION;

        buf[i] = (uint16_t)(scaled);
    }
}
