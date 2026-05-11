/*
 * log.h
 *
 *  Created on: May 4, 2026
 *      Author: Mohamed
 */

#ifndef INC_LOG_H_
#define INC_LOG_H_

#include <stdio.h>

#define LOG_ENABLE       1
#define LOG_UART_ENABLE  0

#if LOG_ENABLE

  #define LOG_INFO(fmt, ...)   printf("[INFO] " fmt "\r\n", ##__VA_ARGS__)
  #define LOG_ERROR(fmt, ...)  printf("[ERR ] " fmt "\r\n", ##__VA_ARGS__)
  #define LOG_DEBUG(fmt, ...)  printf("[DBG ] " fmt "\r\n", ##__VA_ARGS__)

  #if LOG_UART_ENABLE
    #define LOG_TX(cmd)        printf("MCU >> %s\r\n", cmd)
    #define LOG_RX(resp)       printf("SIM << %s\r\n", resp)
  #else
    #define LOG_TX(cmd)
    #define LOG_RX(resp)
  #endif

#else

  #define LOG_INFO(fmt, ...)
  #define LOG_ERROR(fmt, ...)
  #define LOG_DEBUG(fmt, ...)
  #define LOG_TX(cmd)
  #define LOG_RX(resp)

#endif

#endif /* INC_LOG_H_ */
