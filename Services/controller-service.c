/**
 ***********************************************************************************************************************
 * @file           : controller-service.c
 * @brief          : 控制器计算服务
 * @author         : 李嘉豪
 * @date           : 2025-07-13
 ***********************************************************************************************************************
 * @attention
 *
 * 用于建造和使用控制器
 *
 ***********************************************************************************************************************
 **/




/* ------- includes --------------------------------------------------------------------------------------------------*/

#include "controller-service.h"




/* ------- typedef ---------------------------------------------------------------------------------------------------*/





/* ------- define ----------------------------------------------------------------------------------------------------*/





/* ------- macro -----------------------------------------------------------------------------------------------------*/





/* ------- function prototypes ---------------------------------------------------------------------------------------*/

void pidInit(PIDControllerTypeDef* pid, float kp, float ki, float kd);
void pidCalculate(PIDControllerTypeDef* pidData, float reference, float feedback, float dt);
void pidReset(PIDControllerTypeDef* pidData);




/* ------- variables -------------------------------------------------------------------------------------------------*/

CtrlServIntfTypeDef ctrlServIntf = {
    .pidInit      = pidInit,
    .pidCalculate = pidCalculate,
    .pidReset     = pidReset,
};




/* ------- function implement ----------------------------------------------------------------------------------------*/

void pidInit(PIDControllerTypeDef* pid, float kp, float ki, float kd) {
    pid->kp       = kp;   // 比例系数
    pid->ki       = ki;   // 积分系数
    pid->kd       = kd;   // 微分系数
    pid->integral = 0.0f; // 初始化积分值
    pid->prevErr  = 0.0f; // 初始化前一误差值
    pid->output   = 0.0f; // 初始化输出值
}

void pidCalculate(PIDControllerTypeDef* pidData, float reference, float feedback, float dt) {
    float error = reference - feedback; // 计算误差

    pidData->integral += error * dt; // 积分项

    float derivative = (error - pidData->prevErr) / dt; // 微分项

    pidData->prevErr = error; // 更新前一误差

    pidData->output  = pidData->kp * error + pidData->ki * pidData->integral + pidData->kd * derivative; // PID输出
}

void pidReset(PIDControllerTypeDef* pidData) {
    pidData->integral = 0.0f; // 重置积分值
    pidData->prevErr  = 0.0f; // 重置前一误差
    pidData->output   = 0.0f; // 重置输出值
}
