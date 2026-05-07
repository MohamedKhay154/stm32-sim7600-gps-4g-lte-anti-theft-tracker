/*
 * system_fsm.c
 *
 *  Created on: May 7, 2026
 *      Author: Mohamed
 */

#include "system_fsm.h"
#include "log.h"
#include "SIM7600E.h"
#include <string.h>
#include <stdio.h>

State_t current_state = STATE_INIT;
State_t previous_state = STATE_INIT;

/* =============== WAIT state Entry / Exit =============== */
void State_Init_Enter() {
	LOG_INFO("Enter Init State");

	// Ensure SIM module is powered
	if (!SIM_EnsurePowered()) {
		Transition_To(STATE_RECONNECT);
		return;
	}

	// Ensure GPS is ready
	if (!GPS_EnsureReady()) {
		Transition_To(STATE_RECONNECT);
		return;
	}

	// Ensure MQTT connection
	if (!MQTT_EnsureConnected()) {
		Transition_To(STATE_RECONNECT);
		return;
	}

	// Ensure MQTT subscription
	if (!MQTT_EnsureSubscribed()) {
		Transition_To(STATE_RECONNECT);
		return;
	}

	LOG_INFO("System Ready");

	// Everything ready -> go idle
	Transition_To(STATE_TRACKING);

}

void State_Init_Exit() {
	//Power_ExitSleep();
	//GPS_Wakeup();
	LOG_INFO("EXIT Init State\r\n");
}

/* =============== TRACK state Entry / Exit =============== */
void State_Tracking_Enter() {
	//GPS_Wakeup();
	//MQTT_Connect();
	//MQTT_SubscribeCommands();
	//TrackTimer_Start(1);
	LOG_INFO("Enter Tracking State\r\n");
}

void State_Tracking_Exit() {
	//TrackTimer_Stop();
	LOG_INFO("EXIT Tracking State\r\n");

}
/* =============== Park state Entry / Exit =============== */
void State_Park_Enter() {
	//GPS_Wakeup();
	/*if (GPS_GetPosition(&last_lat, &last_lon)) {
	 Geofence_SetCenter(last_lat, last_lon, park_radius_m);
	 }*/
	LOG_INFO("Enter Park State\r\n");
}

void State_Park_Exit() {
	/* Nothing special for now */
	LOG_INFO("EXIT Park State\r\n");
}
/* =============== Park state Entry / Exit =============== */
void State_Reconnect_Enter() {
	LOG_INFO("Enter RECONNECT State\r\n");
	//WatchdogTimer_Stop();
}
void State_Reconnect_Exit() {
	LOG_INFO("EXIT RECONNECT State\r\n");
	//WatchdogTimer_Start();
}

/* ================ Transition function =================== */
 void Transition_To(State_t new_state)
{
    if (current_state == new_state) {
        return;   // prevent re-entering same state
    }
	// If we are moving into RECONNECT, save previous state
	if (new_state == STATE_RECONNECT) {
		previous_state = current_state;
	}
    /* ---- Exit old state ---- */
    switch (current_state) {

    case STATE_INIT:
        State_Init_Exit();
        break;

    case STATE_TRACKING:
        State_Tracking_Exit();
        break;

    case STATE_PARK:
        State_Park_Exit();
        break;
    case STATE_RECONNECT:
    	State_Reconnect_Exit();
    	break;

    default:
        break;
    }

    current_state = new_state;

    /* ---- Enter new state ---- */
    switch (new_state) {

    case STATE_INIT:
        State_Init_Enter();
        break;

    case STATE_TRACKING:
        State_Tracking_Enter();
        break;

    case STATE_PARK:
        State_Park_Enter();
        break;
    case STATE_RECONNECT:
    	State_Reconnect_Enter();
    	break;
    default:
        break;
    }
}

 /* ================ WAIT Handler =================== */
  void State_Init_HandleEvent(Event_t ev)
 {
     switch (ev) {

     case EVENT_CMD_WHERE:

         break;
     case EVENT_CONNECTION_LOST:

          break;

     case EVENT_CMD_TRACK_ON:

         break;

     case EVENT_CMD_PARK_ON:

 		break;

     default:
         break;
     }
 }
 /* ================ Track Handler =================== */
  void State_Tracking_HandleEvent(Event_t ev)
 {
     switch (ev) {

     case EVENT_TRACK_TIMER:

         break;

     case EVENT_CMD_WHERE:

         break;

     case EVENT_CMD_STOP:

 		break;
 	case EVENT_CONNECTION_LOST:

 		break;

 	case EVENT_CMD_PARK_ON:

 		break;


     default:
         break;
     }
 }
 /* ================ PARK Handler =================== */
  void State_Park_HandleEvent(Event_t ev)
 {
     switch (ev) {

     case EVENT_CMD_WHERE:

         break;

     case EVENT_GEOFENCE_EXIT:


         break;

 	case EVENT_CONNECTION_LOST:

 		break;

 	 case EVENT_CMD_TRACK_ON:

 		break;

     case EVENT_CMD_STOP:

         break;

     default:
         break;
     }
 }
  /* ================ RECONNECT Handler =================== */
  void State_Reconnect_HandleEvent(Event_t ev)
  {
 	switch (ev) {

 	case EVENT_CONNECTION_LOST:

 		break;

 	default:
 		break;  // ignore other events
 	}
 }
 /* ================ Methods =================== */

 Event_t Event_Wait(){

	 if(checkCommand("Where")){
		 return EVENT_CMD_WHERE;
	 }


 	/* Check connection first */
 	/*if (sim_connection_lost) {

 		return EVENT_CONNECTION_LOST;
 	}

 	if (TrackTimer_EventPending()) {
 	        return EVENT_TRACK_TIMER;
 	    }
 	if (WatchdogTimer_EventPending()) {
 		if(!checkConnection()){
 			return EVENT_CONNECTION_LOST;
 		}
 	}*/
 	/* --- SIM commands --- */
 	/*char* msg = CheckReceivedBuffer();

 	if (msg != NULL) {
 	    if (bufferContains(msg, "Where")) {
 	        return EVENT_CMD_WHERE;
 	    }
 	    if (bufferContains(msg, "close")) {
 	    	sim_connection_lost = true;
 	        return EVENT_NONE;
 	    }
 		if (bufferContains(msg, "track")) {
 			return EVENT_CMD_TRACK_ON;
 		}
 		if (bufferContains(msg, "stop")) {
 			return EVENT_CMD_STOP;
 		}
 		if (bufferContains(msg, "park")) {
 			return EVENT_CMD_PARK_ON;
 		}
 	}*/

 	/* --- Buttons (optional, polling example) --- */
 	/*
 	 if (ButtonTrack_Pressed()) {
 	 return EVENT_BTN_TRACK;
 	 }

 	 if (ButtonPark_Pressed()) {
 	 return EVENT_BTN_PARK;
 	 }
 	 */

 	return EVENT_NONE;
 }

