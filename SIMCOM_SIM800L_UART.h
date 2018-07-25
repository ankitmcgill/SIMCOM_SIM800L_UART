#ifndef _SIMCOM_SIM800L_UART_
#define _SIMCOM_SIM800L_UART_  

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define SIMCOM_SIM800L_UART_AT_RETRY_COUNT          (2)
#define SIMCOM_SIM800L_UART_AT_RESPONSE_ENDING      ('\r')

typedef enum
{
    MODEM_RESPONSE_PROCESS_CREG = 0,
    MODEM_RESPONSE_PROCESS_CSQ,
    MODEM_RESPONSE_PROCESS_CLTS
}s_simcom_800l_uart_response_type_t;

void SIMCOM_SIM800L_UART_Init(void);
void SIMCOM_SIM800L_UART_Reset(void);

void SIMCOM_SIM800L_UART_Startup(void);

bool SIMCOM_SIM800L_UART_GetNetworkRegisterStatus(void);
void SIMCOM_SIM800L_UART_NetworkRegister(void);
uint8_t SIMCOM_SIM800L_UART_GetSignalQuality(void);

bool SIMCOM_SIM800L_UART_GetNetworkTimeEnabled(void);
void SIMCOM_SIM800L_UART_NetworkTimeEnable(void);
void SIMCOM_SIM800L_UART_GetNetworkTime(void);

void SIMCOM_SIM800L_UART_SendSms(void);
void SIMCOM_SIM800L_UART_DeleteSms(void);

bool SIMCOM_SIM800L_UART_SendATCommand(char* command, char* response_match_token, uint32_t timeout_ms);

#endif