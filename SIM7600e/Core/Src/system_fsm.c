/*
 * system_fsm.c
 *
 *  Created on: May 7, 2026
 *      Author: Mohamed
 */

#include "system_fsm.h"


State_t current_state = STATE_CONNECT;
State_t previous_state = STATE_CONNECT;

/* =============== WAIT state Entry / Exit =============== */
void State_Connect_Enter() {
	LOG_INFO("Enter Connect State");

	sysHealth_Timer_Stop();

	// Ensure SIM module is powered
	if (!SIM_EnsurePowered()) {
		Transition_To(STATE_CONNECT);
		return;
	}

	// Ensure GPS is ready
	if (!GPS_EnsureReady()) {
		Transition_To(STATE_CONNECT);
		return;
	}

	// Ensure MQTT connection
	if (!MQTT_EnsureConnected()) {
		Transition_To(STATE_CONNECT);
		return;
	}

	// Ensure MQTT subscription
	if (!MQTT_EnsureSubscribed()) {
		Transition_To(STATE_CONNECT);
		return;
	}

	LOG_INFO("System Ready");

	// Everything ready -> go idle
	Transition_To(STATE_IDLE);

}

void State_Connect_Exit() {
	sysHealth_Timer_Start();
	LOG_INFO("EXIT Connect State");
}


/* =============== IDLE state Entry / Exit =============== */
void State_Idle_Enter() {
	//GPS_Wakeup();
	//MQTT_Connect();
	//MQTT_SubscribeCommands();
	//TrackTimer_Start(1);
	LOG_INFO("Enter Idle State");
}

void State_Idle_Exit() {
	//TrackTimer_Stop();
	LOG_INFO("EXIT Idle State");

}

/* =============== TRACKING state Entry / Exit =============== */
void State_Tracking_Enter() {
	//GPS_Wakeup();
	//MQTT_Connect();
	//MQTT_SubscribeCommands();
	Tracking_Timer_Start();
	LOG_INFO("Enter Tracking State");
}

void State_Tracking_Exit() {
	Tracking_Timer_Stop();
	LOG_INFO("EXIT Tracking State");

}
/* =============== Park state Entry / Exit =============== */
void State_Park_Enter() {
	LOG_INFO("Enter Park State");
	publishGPS_over_MQTT();

}

void State_Park_Exit() {
	/* Nothing special for now */
	LOG_INFO("EXIT Park State");
}

/* ================ Transition function =================== */
 void Transition_To(State_t new_state)
{
    if (current_state == new_state && current_state != STATE_CONNECT ) {
        return;   // prevent re-entering same state
    }
	// If we are moving into RECONNECT, save previous state
	if (new_state == STATE_CONNECT) {
		previous_state = current_state;
	}
    /* ---- Exit old state ---- */
    switch (current_state) {

    case STATE_CONNECT:
        State_Connect_Exit();
        break;

    case STATE_IDLE:
           State_Idle_Exit();
        break;

    case STATE_TRACKING:
        State_Tracking_Exit();
        break;

    case STATE_PARK:
        State_Park_Exit();
        break;

    default:
        break;
    }

    current_state = new_state;

    /* ---- Enter new state ---- */
    switch (new_state) {

    case STATE_CONNECT:
        State_Connect_Enter();
        break;

    case STATE_IDLE:
         State_Idle_Enter();
         break;

    case STATE_TRACKING:
        State_Tracking_Enter();
        break;

    case STATE_PARK:
        State_Park_Enter();
        break;

    default:
        break;
    }
}

 /* ================ WAIT Handler =================== */
  void State_Connect_HandleEvent(Event_t ev)
 {

 }

 /* ================ WAIT Handler =================== */
void State_Idle_HandleEvent(Event_t ev) {
	switch (ev) {

	case EVENT_CMD_WHERE:
		publishGPS_over_MQTT();
		break;
	case EVENT_CONNECTION_LOST:
		Transition_To(STATE_CONNECT);
		break;

	case EVENT_CMD_TRACK_ON:
		Transition_To(STATE_TRACKING);
		break;

	case EVENT_CMD_PARK_ON:
		Transition_To(STATE_PARK);
		break;

	default:
		break;
	}
}
 /* ================ Track Handler =================== */
void State_Tracking_HandleEvent(Event_t ev) {
	switch (ev) {
	case EVENT_CMD_STOP:
		Transition_To(STATE_IDLE);
		break;

	case EVENT_TRACKING_TIMER:
		publishGPS_over_MQTT();
		break;

	case EVENT_CONNECTION_LOST:
		Transition_To(STATE_CONNECT);
		break;

	default:
		break;
	}
}
 /* ================ PARK Handler =================== */
  void State_Park_HandleEvent(Event_t ev)
 {
     switch (ev) {

     case EVENT_GEOFENCE_EXIT:

         break;

 	case EVENT_CONNECTION_LOST:
 		Transition_To(STATE_CONNECT);
 		break;

     case EVENT_CMD_STOP:
    	 Transition_To(STATE_IDLE);
         break;

     default:
         break;
     }
 }

 /* ================ Methods =================== */

Event_t Event_Wait() {

	if (checkCommand("Where")) {
		return EVENT_CMD_WHERE;
	}
	if (checkCommand("track")) {
		return EVENT_CMD_TRACK_ON;
	}
	if (checkCommand("stop")) {
		return EVENT_CMD_STOP;
	}

	if (checkCommand("park")) {
		return EVENT_CMD_PARK_ON;
	}
	if (Tracking_Timer_HasElapsed()) {
		return EVENT_TRACKING_TIMER;
	}

	if (sysHealth_Timer_HasElapsed()) {
		if(!System_IsReady)
		return EVENT_CONNECTION_LOST;
	}


 	return EVENT_NONE;
 }

