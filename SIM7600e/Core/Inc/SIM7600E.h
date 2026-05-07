/*
 * @file SIM7600E.h
 * @brief SIM7600 driver public interface.
 *  Created on: May 4, 2026
 *      Author: Mohamed
 */

#ifndef INC_SIM7600E_H_
#define INC_SIM7600E_H_


#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define SIM_PWRKEY_GPIO_PORT   GPIOC
#define SIM_PWRKEY_PIN         GPIO_PIN_7
#define SIM_RI_PIN			   GPIO_PIN_6

#define RX_BUFFER_SIZE 512
#define TIMEOUT 10000
#define RETRIES 3
#define PHONE_NUMBER "+49017647327703"

#define MQTT_APN        "iotsim.melita.io"
#define MQTT_CLIENT_ID   "sim7600_client_01"
#define MQTT_BROKER      "broker.emqx.io"
#define MQTT_PORT        1883
#define MQTT_PUB_TOPIC   "gps/location"
#define MQTT_SUB_TOPIC   "test/node-red"


typedef struct {
    char latitude[16];
    char longitude[16];
} GPSData;



/**
 * @brief Ensures that the SIM7600 module is powered ON.
 *
 * Checks communication with AT command first.
 * If no response is received, the module is powered ON.
 */
bool SIM_EnsurePowered();

/**
 * @brief Powers OFF the SIM7600 module using PWRKEY.
 *
 */
void SIM_PowerOff(void);

/**
 * @brief Initializes the SIM7600E UART.
 *
 * @param huart Pointer to UART used by the SIM7600E.
 */
void SIM_UART_Init(UART_HandleTypeDef *huart);


/**
 * @brief Check whether a buffer is contained in another buffer.
 *
 * @param haystack The buffer which will be checked.
 * @param needle The buffer that should be contained.
 *
 * @return true if haystack contains needle.
 */
bool bufferContains( char *haystack,const char *needle);


/**
 * @brief Wait until the expected text comes.
 *
 * @param expected The ecpected buffer.
 * @param timeout_ms The waiting time.
 *
 * @return true if the expected buffer has came.
 */
bool waitForResponse( char *expected, uint32_t timeout_ms);


/**
 * @brief Sends an AT command and waits for a response.
 *
 * @param cmd Command string.
 * @param expected Expected response.
 *
 * @return true if successful, false otherwise.
 */
bool SendAtCommand( char *cmd ,char *expected);


/** @brief Sends AT command and retries until expected response or retries exhausted.
 *
 *
 * */
bool SendAtCommandRetry(char *cmd, char *expected, uint8_t retries);

/**
 * @brief Sends raw text over UART without waiting for response.
 *
 * This function is used when only transmitting data (no AT response needed),
 * such as sending SMS body or MQTT payload.
 *
 * @param cmd Pointer to the text to send.
 */
void SendRawText(char *cmd);



/**
 * @brief Checks if GPS is enabled and enables it if needed.
 *
 * @return true if GPS is ready.
 * @return false if GPS enabling fails.
 */
bool GPS_EnsureReady();


/**
 * @brief Waits until GPS fix is acquired or timeout occurs.
 *
 * @param timeout_ms Timeout in milliseconds.
 * @return true if GPS fix acquired.
 * @return false if timeout occurs.
 */
bool WaitForGPSFix(uint32_t timeout_ms);

/**
 * @brief Parses GPS data from SIM7600 buffer (CGPSINFO format).
 *
 * Converts latitude and longitude from degrees+minutes to decimal.
 *
 * @param buffer SIM7600 response buffer.
 * @return GPSData struct with latitude and longitude.
 */
GPSData parseGPS(char *buffer);

/**
 * @brief Creates a Google Maps link from GPS data.
 *
 * @param gps gps data exctracted.
 * @param linkBuffer where the link should be saved.
 * @param bufferSize size of the link buffer.
 */
void formatGPSLink(GPSData gps, char *linkBuffer, uint16_t bufferSize);


/**
 * @brief Waits for a specific prompt character from the SIM module.
 *
 * This function scans the receive buffer until the given prompt
 * character (e.g. '>') is found or timeout occurs.
 *
 * @param prompt The character to wait for.
 * @param timeout_ms Timeout in milliseconds.
 *
 * @return true if prompt is received.
 * @return false if timeout occurs.
 */
bool waitForPrompt(char prompt, uint32_t timeout_ms);


/**
 * @brief Extracts SMS text body from CMGL response.
 *
 * The message body is the line after the +CMGL header.
 *
 * @param buffer SIM7600 receive buffer.
 * @param smsText Buffer to store extracted SMS text.
 * @param size Size of smsText buffer.
 */
void extractSMSMessage(char *buffer, char *smsText, uint16_t size);

/**
 * @brief Sends current GPS location via SMS.
 *
 * Parses GPS data, creates a Google Maps link,
 * and sends it as an SMS using SIM7600.
 */
void sendGPS_over_SMS();


/**
 * @brief Checks RI flag and reads unread SMS.
 *
 * When RI is triggered, it reads unread SMS,
 * extracts the message text, and logs it.
 *
 * @return true if SMS received and parsed.
 * @return false if no event or timeout.
 */
bool checkRI();

/**
 * @brief Checks and prepares MQTT connection.
 *
 * @return true if MQTT is connected.
 * @return false if connection fails.
 */
bool MQTT_EnsureConnected(void);


/**
 * @brief Subscribes to the MQTT topic if not already subscribed.
 *
 * @return true if subscription is active.
 * @return false if subscription fails.
 */
bool MQTT_EnsureSubscribed();

/**
 * @brief Publishes current GPS position over MQTT.
 *
 * Reads GPS coordinates, formats them as "lat,long",
 * and publishes the payload to the MQTT topic.
 */
void publishGPS_over_MQTT();


#endif /* INC_SIM7600E_H_ */
