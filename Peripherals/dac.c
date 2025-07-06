/**
 ***********************************************************************************************************************
 * @file           : dac.c
 * @brief          : DAC配置文件
 * @author         : 李嘉豪
 * @date           :  2025-07-07
 ***********************************************************************************************************************
 * @attention
 *
 * DAC配置
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "dac.h"




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/





/* ------- variables -------------------------------------------------------------------------------------------------*/





/* ------- function implement ----------------------------------------------------------------------------------------*/

void dacInit(void) {
    DAC_InitTypeDef DAC_InitStructure;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
    DAC_InitStructure.DAC_OutputBuffer                 = DAC_OutputBuffer_Enable;
    DAC_InitStructure.DAC_Trigger                      = DAC_Trigger_T4_TRGO;
    DAC_InitStructure.DAC_WaveGeneration               = DAC_WaveGeneration_None;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);

    DAC_InitStructure.DAC_Trigger = DAC_Trigger_T8_TRGO;
    DAC_Init(DAC_Channel_2, &DAC_InitStructure);

    DAC_Cmd(DAC_Channel_1, ENABLE);
    DAC_Cmd(DAC_Channel_2, ENABLE);
    DAC_DMACmd(DAC_Channel_1, ENABLE);
    DAC_DMACmd(DAC_Channel_2, ENABLE);
}
