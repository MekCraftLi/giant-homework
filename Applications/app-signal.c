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
#define SAMPLING_FREQ  50000 // 采样频率为50kHz



/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

void generateSineWave512(uint16_t* buf, float amplitude, float phase_deg);




/* ------- variables -------------------------------------------------------------------------------------------------*/

static DMAObjTypeDef dacChannel1DMA; // DAC通道1的DMA对象
static DMAObjTypeDef dacChannel2DMA; // DAC通道2的DMA对象
static DMAObjTypeDef adcChannel1DMA; // ADC通道1的DMA对象

static TIMObjTypeDef dacChannel1Timer; // DAC通道1的定时器对象
static TIMObjTypeDef dacChannel2Timer; // DAC通道2的定时器对象
static TIMObjTypeDef adcSamplingTimer; // ADC采样定时器对象
static TIMObjTypeDef masterTimer;      // 主定时器对象




/* ------- function implement ----------------------------------------------------------------------------------------*/

void signalAppInit(void* argument) {
    SignalAppParamTypeDef* pSignalParam = (SignalAppParamTypeDef*)argument;

    generateSineWave512(pSignalParam->sign[0], pSignalParam->signalInfo[0].amp, pSignalParam->signalInfo[0].phase);
    generateSineWave512(pSignalParam->sign[1], pSignalParam->signalInfo[1].amp, pSignalParam->signalInfo[1].phase);

    // 1. 初始化GPIO
    gpioIntf.pinInit(PORT_A, PIN_4, INPUT_ANALOG);
    gpioIntf.pinInit(PORT_A, PIN_5, INPUT_ANALOG);
    gpioIntf.pinInit(PORT_A, PIN_0, INPUT_ANALOG); // ADC1通道0
    gpioIntf.pinInit(PORT_A, PIN_1, INPUT_ANALOG); // ADC2通道1




    // 2. 初始化DMA
    dmaIntf.init(&dacChannel1DMA, DMA2_Channel3, DMA_Priority_High);
    dmaIntf.init(&dacChannel2DMA, DMA2_Channel4, DMA_Priority_High);
    dmaIntf.init(&adcChannel1DMA, DMA1_Channel1, DMA_Priority_High);

    dmaIntf.setSorceCycle(&dacChannel1DMA, (uint32_t)pSignalParam->sign[0], DMA_SIZE_HALF_WORD, WAVE_LEN);
    dmaIntf.setSorceCycle(&dacChannel2DMA, (uint32_t)pSignalParam->sign[1], DMA_SIZE_HALF_WORD, WAVE_LEN);
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
    timIntf.init(&masterTimer, TIM1);
    timIntf.init(&dacChannel1Timer, TIM4);
    timIntf.init(&dacChannel2Timer, TIM8);
    timIntf.init(&adcSamplingTimer, TIM7);

    // 设定频率
    timIntf.countConfig(&masterTimer, 6000 * WAVE_LEN * 2, 0x01); // 配置定时器为6k x WAVE_LEN x 2 计数频率

    // 设定触发源
    TIM_SelectOutputTrigger(masterTimer.tim, TIM_TRGOSource_Update);

    // 设定频率
    timIntf.setFrequency(&adcSamplingTimer, SAMPLING_FREQ); // 配置ADC采样定时器为50kHz
    timIntf.enableISR(&adcSamplingTimer);                   // 使能ADC采样定时器中断


    timIntf.countConfig(&dacChannel1Timer, dacChannel1Timer.clkFreq, 12); // 配置DAC通道定时器为1k*512Hz
    timIntf.countConfig(&dacChannel2Timer, dacChannel2Timer.clkFreq, 12);


    // 设定触发源
    TIM_SelectOutputTrigger(dacChannel1Timer.tim, TIM_TRGOSource_Update);
    TIM_SelectOutputTrigger(dacChannel2Timer.tim, TIM_TRGOSource_Update);

    TIM_SelectInputTrigger(dacChannel1Timer.tim, TIM_TS_ITR0); // 设置DAC通道1定时器为主定时器的输入触发
    TIM_SelectInputTrigger(dacChannel2Timer.tim, TIM_TS_ITR0); // 设置DAC通道2定时器为主定时器的输入触发

    timIntf.start(&masterTimer);      // 启动主定时器
    timIntf.start(&dacChannel1Timer); // 启动DAC通道1定时器
    timIntf.start(&dacChannel2Timer); // 启动DAC通道2定时器
    timIntf.start(&adcSamplingTimer);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 触发ADC1开始转换
}

void signalAppLoop(void* argument) { SignalAppParamTypeDef* pSignalParam = (SignalAppParamTypeDef*)argument; }


void generateSineWave512(uint16_t* buf, float amplitude, float phase_deg) {
    float phase_rad = phase_deg * PI / 180.0f;

    for (int i = 0; i < WAVE_LEN; ++i) {
        float angle  = 2.0f * PI * i / WAVE_LEN + phase_rad;
        float value  = sinf(angle) + 1; // 范围：-1 ~ +1

        // 幅度调整为 0~amplitude，然后加上 offset
        float scaled = value * (amplitude / 3.3f) * DAC_RESOLUTION / 2;

        // 限制范围到 0~4095
        if (scaled < 0)
            scaled = 0;
        if (scaled > DAC_RESOLUTION)
            scaled = DAC_RESOLUTION;

        buf[i] = (uint16_t)(scaled);
    }
}
