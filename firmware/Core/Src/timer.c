/*
 * timer.c
 *
 *  Created on: May 9, 2026
 *      Author: Mohamed
 */


#include "timer.h"
#include "stm32l4xx_hal.h"

static TIM_HandleTypeDef *tracking_timer;
volatile bool tracking_timer_flag = false;

static TIM_HandleTypeDef *sysHealth_timer;
volatile bool sysHealth_timer_flag = false;


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
    	tracking_timer_flag = true;
    }
    if (htim->Instance == TIM3) {
    	//watchdog_timer_flag = true; // signal event to FSM
        }
}


void Tracking_Timer_Init(TIM_HandleTypeDef *htim) {
	tracking_timer = htim;
}


void sysHealth_Timer_Init(TIM_HandleTypeDef *htim) {
	sysHealth_timer = htim;
}


void Tracking_Timer_Start()
{
    HAL_TIM_Base_Start_IT(tracking_timer);
    tracking_timer_flag = false;
}


void sysHealth_Timer_Start()
{
    HAL_TIM_Base_Start_IT(sysHealth_timer);
    sysHealth_timer_flag = false;
}

/**
 * @brief Stop the timer
 */
void Tracking_Timer_Stop()
{
    HAL_TIM_Base_Stop_IT(tracking_timer);
    tracking_timer_flag = false;
}

void sysHealth_Timer_Stop()
{
    HAL_TIM_Base_Stop_IT(sysHealth_timer);
    sysHealth_timer_flag = false;
}

bool Tracking_Timer_HasElapsed()
{
    if (tracking_timer_flag) {
    	tracking_timer_flag = false;
        return true;
    }
    return false;
}

bool sysHealth_Timer_HasElapsed()
{
    if (sysHealth_timer_flag) {
    	sysHealth_timer_flag = false;
        return true;
    }
    return false;
}
