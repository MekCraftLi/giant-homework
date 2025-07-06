/**
 ***********************************************************************************************************************
 * @file           : adc.c
 * @brief          : ADC配置文件
 * @author         : 李嘉豪
 * @date           : 2025-07-07
 ***********************************************************************************************************************
 * @attention
 *
 * ADC配置
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "adc.h"




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/





/* ------- variables -------------------------------------------------------------------------------------------------*/





/* ------- function implement ----------------------------------------------------------------------------------------*/


void adcInit(void) {

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // ADC时钟为72MHz/6=12MHz


    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); // ADC1通道0
    ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5); // ADC2通道1

    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode               = ADC_Mode_RegSimult;
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;       // ADC1数据右对齐
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None; // 转换由软件触发启动
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                   // 单次转换
    ADC_InitStructure.ADC_ScanConvMode       = ENABLE;                    // 扫描模式
    ADC_InitStructure.ADC_NbrOfChannel       = 1;                         // 转换2个通道
    ADC_Init(ADC1, &ADC_InitStructure);                                   // 配置ADC1
    ADC_Init(ADC2, &ADC_InitStructure);

    ADC_ExternalTrigConvCmd(ADC2, ENABLE);


    ADC_DMACmd(ADC1, ENABLE); // 使能ADC1的DMA功能
    ADC_Cmd(ADC1, ENABLE);    // 使能ADC1
    ADC_Cmd(ADC2, ENABLE);

    ADC_ResetCalibration(ADC1); // ADC1的校准复位


    while (ADC_GetResetCalibrationStatus(ADC1) == SET)
        ; // 等待复位完成

    ADC_StartCalibration(ADC1); // 开始ADC1的校准

    while (ADC_GetCalibrationStatus(ADC1) == SET)
        ; // 等待ADC1的校准完成

    ADC_ResetCalibration(ADC2); // ADC1的校准复位


    while (ADC_GetResetCalibrationStatus(ADC2) == SET)
        ; // 等待复位完成

    ADC_StartCalibration(ADC2); // 开始ADC1的校准

    while (ADC_GetCalibrationStatus(ADC2) == SET)
        ; // 等待ADC1的校准完成
}
