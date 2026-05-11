/*
 * @file SIM7600E.c
 * @brief SIM7600 driver implementation.
 *  Created on: May 4, 2026
 *      Author: Mohamed
 */

#include "SIM7600E.h"
#include "log.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Pointer to the UART used for SIM7600 communication.
 *
 * This UART handle is used to send and receive AT commands
 * between the mcu and the SIM7600e module.
 */
static UART_HandleTypeDef *sim_uart;

/**
 * @brief Temporary variable to store one received byte.
 *
 * This variable is filled inside the UART interrupt callback.
 * Each received byte is then copied into the main buffer.
 */
uint8_t rx_data;

/**
 * @brief Buffer to store incoming data from SIM7600.
 *
 * This array collects all received characters from the module.
 * It is used later to check if a response (like "OK") is received.
 */
char sim_buffer[RX_BUFFER_SIZE];

/**
 * @brief Index of the current position in the receive buffer.
 *
 * This variable keeps track of how many bytes have been received.
 * It is incremented every time a new byte arrives.
 */
uint16_t idx = 0;


volatile bool sim_ri_flag = false;

/** @brief Indicates whether MQTT topic subscription is active. */
bool mqtt_subscribed = false;


/**
 * @brief Powers on the SIM7600 module using the PWRKEY pin.
 *
 * This function toggles the PWRKEY line according to the SIM7600
 * power-on timing requirements.
 *
 * @note Uses blocking delays.
 * @note Ensure GPIO is initialized before calling.
 */
bool SIM_EnsurePowered() {
	// Check if SIM already responding
	if (SendAtCommandRetry("AT", "OK", RETRIES)) {
		LOG_INFO("SIM already ON");
		return true;
	}

	// Power ON sequence
	HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_PORT,
	SIM_PWRKEY_PIN, GPIO_PIN_RESET);
	HAL_Delay(500);

	HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_PORT,
	SIM_PWRKEY_PIN, GPIO_PIN_SET);
	HAL_Delay(1200);

	HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_PORT,
	SIM_PWRKEY_PIN, GPIO_PIN_RESET);

	HAL_Delay(20000);

	// Verify SIM response after power ON
	if (SendAtCommandRetry("AT", "OK", RETRIES)) {
		LOG_INFO("SIM power ON");
		return true;
	}

	LOG_ERROR("SIM not responding");

	return false;
}

void SIM_PowerOff(void)
{
    HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_PORT, SIM_PWRKEY_PIN, GPIO_PIN_RESET);
    HAL_Delay(1000);

    HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_PORT, SIM_PWRKEY_PIN, GPIO_PIN_SET);
    HAL_Delay(2500);

    HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_PORT, SIM_PWRKEY_PIN, GPIO_PIN_RESET);
    HAL_Delay(8000);

    LOG_INFO("SIM power OFF");
}

void SIM_UART_Init(UART_HandleTypeDef *huart)
{
    sim_uart = huart;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == sim_uart)
    {
    	 if (idx < RX_BUFFER_SIZE - 1)
    	        {
    	            sim_buffer[idx++] = rx_data;
    	            sim_buffer[idx] = '\0';
    	        }
    	 HAL_UART_Receive_IT(sim_uart, &rx_data, 1);
    	 //LOG_RX(sim_buffer);
    }
}


bool bufferContains( char *haystack,const char *needle)
{
	int hay_len = idx;                 // number of bytes received
	int needle_len = strlen(needle);

	if (needle_len == 0 || hay_len < needle_len)
		return false;

	// Raw byte search
	for (int i = 0; i <= hay_len - needle_len; i++) {
		if (memcmp(&haystack[i], needle, needle_len) == 0) {
			return true;}
	}
	return false;
}


bool waitForResponse( char *expected, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while (1)
    {
		// Check if expected text is inside received buffer
    	if (bufferContains(sim_buffer,expected))
			return true;

		// Timeout
		if (HAL_GetTick() - start > timeout_ms)
			return false;
    }

}

bool SendAtCommand( char *cmd ,  char *expected){

	memset(sim_buffer, 0, RX_BUFFER_SIZE); //reset the buffer
	idx = 0;							   //reset the index

	LOG_TX(cmd);

	HAL_UART_Receive_IT(sim_uart, &rx_data, 1);

    HAL_UART_Transmit(sim_uart, (uint8_t *)cmd, strlen(cmd), HAL_MAX_DELAY);
    HAL_UART_Transmit(sim_uart, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY); 		  //Append the "\r\n"

	if (waitForResponse(expected, TIMEOUT)) {
		LOG_RX(sim_buffer);
		return true;
	} else{
		LOG_RX("Timeout");
		return false;
	}
}


bool SendAtCommandRetry(char *cmd, char *expected, uint8_t retries)
{
    for (uint8_t i = 0; i < retries; i++)
    {
        if (SendAtCommand(cmd, expected))
        {
            return true;
        }

        HAL_Delay(500); // small delay before
    }

    return false;
}

void SendRawText(char *cmd)
{
    memset(sim_buffer, 0, RX_BUFFER_SIZE); // reset buffer
    idx = 0;                               // reset index

    LOG_TX(cmd);

    HAL_UART_Receive_IT(sim_uart, &rx_data, 1);
    HAL_UART_Transmit(sim_uart, (uint8_t *)cmd, strlen(cmd), HAL_MAX_DELAY);
}

bool SIM_IsAlive(void)
{
    return SendAtCommandRetry("AT\r\n", "OK", RETRIES);
}

bool SIM_IsSimReady(void)
{
    return SendAtCommandRetry("AT+CPIN?\r\n",
                              "+CPIN: READY",
                              3);
}

bool SIM_IsNetworkRegistered(void) {
	// Home network
	if (SendAtCommandRetry("AT+CGREG?\r\n", "+CGREG: 0,1", RETRIES)) {
		return true;
	}

	// Roaming network
	if (SendAtCommandRetry("AT+CGREG?\r\n", "+CGREG: 0,5", RETRIES)) {
		return true;
	}

	return false;
}

bool SIM_IsDataAttached(void) {
	return SendAtCommandRetry("AT+CGATT?\r\n", "+CGATT: 1", RETRIES);
}

bool MQTT_IsConnected(void) {
	return SendAtCommandRetry("AT+CMQTTCONNECT?\r\n", "+CMQTTCONNECT: 0,0", RETRIES);
}

bool System_IsReady(void) {
	return SIM_IsAlive() && SIM_IsSimReady() && SIM_IsNetworkRegistered()
			&& SIM_IsDataAttached() && MQTT_IsConnected();
}


bool GPS_EnsureReady() {

	// Check if GPS already enabled
	if (SendAtCommandRetry("AT+CGPS?", "+CGPS: 1,1", RETRIES)) {
		LOG_INFO("GPS already ON");
	} else {
		LOG_INFO("GPS OFF, turning ON");

		if (!SendAtCommandRetry("AT+CGPS=1,1", "OK", RETRIES)) {
			LOG_ERROR("Failed to turn GPS ON");
			return false;
		}
	}
	//GPS is ON, wait for fix
	if (WaitForGPSFix(120000)) {
		LOG_INFO("GPS READY");
		return true;
	}

	LOG_ERROR("GPS ON but no fix");
	return false;
}



bool WaitForGPSFix(uint32_t timeout_ms) {

	uint32_t start = HAL_GetTick();

	while ((HAL_GetTick() - start) < timeout_ms) {
		if (SendAtCommandRetry("AT+CGPSINFO", "+CGPSINFO", RETRIES)) {
			if (!bufferContains(sim_buffer, "+CGPSINFO: ,")) {
				LOG_INFO("GPS Fix acquired");
				return true;
			}
		}
		LOG_INFO("Waiting for GPS fix...");
		HAL_Delay(5000);
	}

	LOG_ERROR("GPS fix timeout");

	return false;
}




GPSData parseGPS(char *buffer)
{
	    GPSData gps;
	    gps.latitude[0] = '\0';
	    gps.longitude[0] = '\0';

	    char *token = strtok(buffer, ",");
	    int i = 0;

	    char latRaw[20] = {0};
	    char lonRaw[20] = {0};
	    char latDir = 0;
	    char lonDir = 0;

	    while (token != NULL)
	    {
	        if (i == 0)
	        {
	            char *p = strchr(token, ':');   // find ':'
	            if (p != NULL)
	            {
	                p++;                        // move after ':'
	                while (*p == ' ') p++;      // skip spaces
	                strncpy(latRaw, p, sizeof(latRaw) - 1);
	            }
	        }

	        if (i == 1) latDir = token[0];
	        if (i == 2) strncpy(lonRaw, token, sizeof(lonRaw) - 1);
	        if (i == 3) lonDir = token[0];

	        token = strtok(NULL, ",");
	        i++;
	    }

	    double latValue = atof(latRaw);
	    double lonValue = atof(lonRaw);

	    int latDeg = (int)(latValue / 100);
	    double latMin = latValue - (latDeg * 100);
	    double latitude = latDeg + (latMin / 60.0);

	    int lonDeg = (int)(lonValue / 100);
	    double lonMin = lonValue - (lonDeg * 100);
	    double longitude = lonDeg + (lonMin / 60.0);

	    if (latDir == 'S') latitude = -latitude;
	    if (lonDir == 'W') longitude = -longitude;

	snprintf(gps.latitude, sizeof(gps.latitude), "%.6f", latitude);
	snprintf(gps.longitude, sizeof(gps.longitude), "%.6f", longitude);

	    return gps;
	}



void formatGPSLink(GPSData gps, char *linkBuffer, uint16_t bufferSize)
{
    snprintf(linkBuffer,
             bufferSize,
             "https://maps.google.com/?q=%s,%s",
             gps.latitude,
             gps.longitude);
}

bool waitForPrompt(char prompt, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();

    while (1)
    {
        // Check all received bytes
        for (uint16_t i = 0; i < idx; i++)
        {
            if (sim_buffer[i] == prompt)
                return true;
        }

        // Timeout check
        if (HAL_GetTick() - start > timeout_ms)
            return false;
    }
}
void sendGPS_over_SMS()
{

	GPSData gps = parseGPS(sim_buffer);

	char gpsLink[80];
	formatGPSLink(gps, gpsLink, sizeof(gpsLink));

	LOG_INFO("%s", gpsLink);


	SendAtCommandRetry("AT+CMGF=1", "OK", 3);  // Text mode

	SendRawText("AT+CMGS=\"+4917647327703\"\r\n");
	if (waitForPrompt('>', TIMEOUT)) {
		// Send SMS text
		SendRawText(gpsLink);

		// End with Ctrl+Z
		uint8_t ctrlZ = 0x1A;
		HAL_UART_Transmit(sim_uart, &ctrlZ, 1, HAL_MAX_DELAY);

		// Wait for message confirmation
		if (waitForResponse("OK",TIMEOUT)) {
			LOG_RX(sim_buffer);
		}

	} else
		LOG_ERROR("No Prompt Received.");
}


/**
 * @brief External interrupt callback for RI pin.
 *
 * Sets a flag when RI (Ring Indicator) is triggered.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if (GPIO_Pin == SIM_RI_PIN)
	    {
	        sim_ri_flag = true;
	    }
}


void extractSMSMessage(char *buffer, char *smsText, uint16_t size)
{
    smsText[0] = '\0';

    char *line = strtok(buffer, "\r\n");
    uint8_t foundHeader = 0;

    while (line != NULL)
    {
        if (foundHeader)
        {
            strncpy(smsText, line, size - 1);
            smsText[size - 1] = '\0';
            return;
        }

        if (strncmp(line, "+CMGL:", 6) == 0)
        {
            foundHeader = 1;
        }

        line = strtok(NULL, "\r\n");
    }
}

bool checkRI(){
	if (sim_ri_flag)
	{
	    sim_ri_flag = false;
	    LOG_INFO("RI event detected");

	    if(SendAtCommandRetry("AT+CMGL=\"REC UNREAD\"", "OK", RETRIES)){

			char smsText[64];
			extractSMSMessage(sim_buffer, smsText, sizeof(smsText));
			LOG_INFO("SMS: %s", smsText);
			return true;
	    }else{
	    	LOG_ERROR("TIMEOUT Occurred. No SMS Received.");
	    	return false;
	    }

	}else {
		LOG_INFO("waiting for RI event...");
		return false;
	}
}

bool MQTT_EnsureConnected(void) {
	// Check APN, if not set then configure it
	if (!SendAtCommandRetry("AT+CGDCONT?", MQTT_APN, 1)) {
		if (!SendAtCommandRetry("AT+CGDCONT=1,\"IP\",\"" MQTT_APN "\"", "OK",
				RETRIES))
			return false;
	}

	// Check network open, if not open then open it
	if (!SendAtCommandRetry("AT+NETOPEN?", "+NETOPEN: 1", 1)) {
		if (!SendAtCommandRetry("AT+NETOPEN", "OK", RETRIES))
			return false;
	}

	// Check MQTT connection
	if (SendAtCommandRetry("AT+CMQTTCONNECT?", "+CMQTTCONNECT: 0,\"tcp://", 1)) {
		LOG_INFO("MQTT already connected");
		return true;
	}

	// Start MQTT service
	SendAtCommandRetry("AT+CMQTTSTART", "OK", RETRIES);

	// Set client ID
	if (!SendAtCommandRetry("AT+CMQTTACCQ=0,\"" MQTT_CLIENT_ID "\"", "OK",
			RETRIES))
		return false;

	// Connect to broker
	if (!SendAtCommandRetry(
			"AT+CMQTTCONNECT=0,\"tcp://" MQTT_BROKER ":1883\",60,1",
			"+CMQTTCONNECT: 0,0",
			RETRIES))
		return false;

	return true;
}


void publishGPS_over_MQTT(){

	SendAtCommandRetry("AT+CGPSINFO", "OK", 3);
	GPSData gps = parseGPS(sim_buffer);
	char payload[40];
	snprintf(payload, sizeof(payload), "%s,%s", gps.latitude, gps.longitude);

	//Set topic
	char topicCmd[64];
	sprintf(topicCmd, "AT+CMQTTTOPIC=0,%d \r\n", strlen(MQTT_PUB_TOPIC));


	if (!SendAtCommandRetry(topicCmd, ">", RETRIES)) {
		LOG_ERROR("No Prompt Received.");
		//return false;
	}
	// Send topic text
	SendRawText(MQTT_PUB_TOPIC);
	HAL_Delay(100);
	LOG_RX(sim_buffer);


	//Set payload length
	char payloadCmd[64];
	sprintf(payloadCmd, "AT+CMQTTPAYLOAD=0,%d \r\n", strlen(payload));
	if (!SendAtCommandRetry(payloadCmd, ">", RETRIES)) {
		LOG_ERROR("No Prompt Received.");
		//return false;
	}
	// Send topic text
	SendRawText(payload);
	HAL_Delay(100);
	LOG_RX(sim_buffer);

	//Publish
	if (SendAtCommandRetry("AT+CMQTTPUB=0,1,60", "OK", 3)) {
		LOG_INFO("Latitude: %s", gps.latitude);
		LOG_INFO("Longitude: %s", gps.longitude);
	}
}



bool MQTT_EnsureSubscribed() {

	char subCmd[64];

	// Check if already subscribed
	/*if (mqtt_subscribed) {
		LOG_INFO("Already subscribed");
		return true;
	}*/

	// Set topic
	sprintf(subCmd, "AT+CMQTTSUBTOPIC=0,%d,1 \r\n", strlen(MQTT_SUB_TOPIC));

	if (!SendAtCommandRetry(subCmd, ">", RETRIES)) {
		LOG_ERROR("No Prompt Received.");
		return false;
	}

	// Send topic text
	//SendRawText(MQTT_SUB_TOPIC);
	SendAtCommand(MQTT_SUB_TOPIC, "OK");

	// Subscribe
	if (!SendAtCommandRetry("AT+CMQTTSUB=0", "OK", RETRIES)) {
		LOG_ERROR("Not subscribed");
		return false;
	}

	// Enable subscription flag
	mqtt_subscribed = true;
	LOG_INFO("Subscribed to topic: %s", MQTT_SUB_TOPIC);

	return true;
}

bool checkCommand(char *text){
	if (bufferContains(sim_buffer,"+CMQTTRX")) {
		if (bufferContains( sim_buffer, text)){
			 LOG_INFO("%s Received!\r\n", text);
			 memset(sim_buffer, 0, RX_BUFFER_SIZE); //reset the buffer
			 idx = 0;							   //reset the index
			return true;
		}else{
			return false;
		}
	}else {
		return false;
	}

}
