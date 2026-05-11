/*
 * system_fsm.h
 *
 *  Created on: May 7, 2026
 *      Author: Mohamed
 */

#ifndef INC_SYSTEM_FSM_H_
#define INC_SYSTEM_FSM_H_

#include "main.h"
#include "SIM7600E.h"
#include "log.h"
#include "timer.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Represents the different system states.
 */
typedef enum {
    STATE_CONNECT,        /**<  */
	STATE_IDLE,		   /**< System is idle */
    STATE_TRACKING,    /**< GPS tracking mode */
    STATE_PARK,        /**< Park monitoring mode */

} State_t;


/**
 * @brief Represents events handled by the FSM.
 */
typedef enum {
    EVENT_NONE,              /**< No event */

    /* MQTT commands */
    EVENT_CMD_WHERE,         /**< Request current GPS position */
    EVENT_CMD_TRACK_ON,      /**< Enable tracking mode */
    EVENT_CMD_PARK_ON,       /**< Enable park mode */
    EVENT_CMD_STOP,          /**< Stop current mode */
    EVENT_CONNECTION_LOST,   /**< MQTT/network connection lost */
    EVENT_CONNECTION_CLOSE,  /**< Connection closed */

    /* Buttons */
    EVENT_BTN_TRACK,         /**< Tracking button pressed */
    EVENT_BTN_PARK,          /**< Park button pressed */

    /* Timers */
    EVENT_TRACKING_TIMER,       /**< Tracking timer event */

    /* Sensors */
    EVENT_GEOFENCE_EXIT      /**< Vehicle left geofence */
} Event_t;


/** @brief Current active system state. */
extern State_t current_state;

/** @brief Previous system state. */
extern State_t previous_state;

/** @brief Called when entering Connect state. */
void State_Connect_Enter();

/** @brief Called when entering Idle state. */
void State_Idle_Enter();

/** @brief Called when entering Tracking state. */
void State_Tracking_Enter();

/** @brief Called when entering Park state. */
void State_Park_Enter();



/** @brief Called when leaving Connect state. */
void State_Connect_Exit();

/** @brief Called when leaving Idle state. */
void State_Idle_Exit();

/** @brief Called when leaving Tracking state. */
void State_Tracking_Exit();

/** @brief Called when leaving Park state. */
void State_Park_Exit();



/** @brief Handles events while in Connect state. */
void State_Connect_HandleEvent(Event_t ev);

/** @brief Handles events while in Idle state. */
void State_Idle_HandleEvent(Event_t ev);

/** @brief Handles events while in Tracking state. */
void State_Tracking_HandleEvent(Event_t ev);

/** @brief Handles events while in Park state. */
void State_Park_HandleEvent(Event_t ev);


/** @brief Changes the system from current state to a new state. */
void Transition_To(State_t new_state);

/**
 * @brief Waits for and returns the next system event.
 *
 * @return Detected event type.
 */
Event_t Event_Wait();

#endif /* INC_SYSTEM_FSM_H_ */
