/*
 * timer.h
 *
 *  Created on: May 9, 2026
 *      Author: Mohamed
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"
#include <stdbool.h>
#include <stdint.h>


extern volatile bool tracking_timer_flag;


void Tracking_Timer_Init(TIM_HandleTypeDef *htim);

void Tracking_Timer_Start();

void Tracking_Timer_Stop();

bool Tracking_Timer_HasElapsed();

void sysHealth_Timer_Init(TIM_HandleTypeDef *htim);

void sysHealth_Timer_Start();

void sysHealth_Timer_Stop();

bool sysHealth_Timer_HasElapsed();


#endif /* INC_TIMER_H_ */
