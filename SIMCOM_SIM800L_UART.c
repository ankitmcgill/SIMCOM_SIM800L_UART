#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "SIMCOM_SIM800L_UART.h"
#include "internal_uart.h"
#include "application.h"
#include "printf-stdarg.h"
#include "bsp.h"

//INTERNAL VARIABLES
static bool s_simcom_sim800l_uart_debug = true;

//INTERNAL FUNCTIONS
static bool s_simcom_sim800l_uart_process_response(s_simcom_800l_uart_response_type_t type,
                                                    void* retval);
                                                
void SIMCOM_SIM800L_UART_Init(void)
{
    //INITIALIZE SIMCOM_SIM800_UART

    //INITIALIZE UASRT2
    INTERNAL_UART_Init(2);

    //SET UP /RESET GPIO LINE
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_set_mode(SIMCOM_SIM800L_UART_RST_PORT, 
                    GPIO_MODE_OUTPUT_2_MHZ,  
                    GPIO_CNF_OUTPUT_PUSHPULL,
                    SIMCOM_SIM800L_UART_RST_PIN);
    
    gpio_set(SIMCOM_SIM800L_UART_RST_PORT, SIMCOM_SIM800L_UART_RST_PIN);

    //SET UART2 RX LINE ENDING (CR + LF) COUNT TO 2
    //DEFAULT FOR MOST SIM800 COMMAND RESPONSES
    INTERNAL_UART_UART2SetLineEndingMax(2);

    my_printf_debug("%s - OK\n", __FUNCTION__);
}

void SIMCOM_SIM800L_UART_Reset(void)
{
    //RESET MODULE

    gpio_clear(SIMCOM_SIM800L_UART_RST_PORT, SIMCOM_SIM800L_UART_RST_PIN);
    APPLICATION_DelayMsBlocking(3000);
    gpio_set(SIMCOM_SIM800L_UART_RST_PORT, SIMCOM_SIM800L_UART_RST_PIN);

    my_printf_debug("%s - OK\n", __FUNCTION__);
}

void SIMCOM_SIM800L_UART_Startup(void)
{
    //START UP THE MODULE AND EXECUTE THE
    //STARTUP AT COMMANDS TO HAVE THE MODULE CONNECTED
    //TO THE NETWORK AND READY
    
    SIMCOM_SIM800L_UART_SendATCommand("AT+IPR=9600\r", "OK", 2000);
    SIMCOM_SIM800L_UART_SendATCommand("ATE0\r", "OK", 2000);
    SIMCOM_SIM800L_UART_SendATCommand("AT\r", "OK", 2000);

    //CHECK IF NETWORK TIME ENABLED
    //IF NOT, ENABLE IT
    if(!SIMCOM_SIM800L_UART_GetNetworkTimeEnabled())
    {
        SIMCOM_SIM800L_UART_NetworkTimeEnable();
    }

    //CMEE MODE = 2
    SIMCOM_SIM800L_UART_SendATCommand("AT+CMEE=2\r", "OK", 2000);

    //SMS TEXT MODE
    SIMCOM_SIM800L_UART_SendATCommand("AT+CMGF=1\r", "OK", 2000);

    //SMS STORAGE SPACE = SIM CARD(SM)
    //SIMCOM_SIM800L_UART_SendATCommand("AT+CPMS=?\r", "OK", 2000);
    SIMCOM_SIM800L_UART_SendATCommand("AT+CPMS=\"SM\",\"SM\",\"SM\"\r", "OK", 3000);

    //DELETE EXISTING SMS
    SIMCOM_SIM800L_UART_SendATCommand("AT+CMGDA=\"DEL ALL\"\r", "OK", 2000);

    //CHECK SIGNAL QUALITY
    SIMCOM_SIM800L_UART_GetSignalQuality();

    //SET SMS COMPOSE MODE
    SIMCOM_SIM800L_UART_SendATCommand("AT+CSMP=17,167,0,0\r", "OK", 2000);

    bool sim_registered = false;
    while(!sim_registered)
    {
        SIMCOM_SIM800L_UART_NetworkRegister();
        sim_registered = SIMCOM_SIM800L_UART_GetNetworkRegisterStatus();
    }

    //MODEM SAVE SETTINGS
    SIMCOM_SIM800L_UART_SendATCommand("AT&W\r", "OK", 2000);

    my_printf_debug("%s - OK\n", __FUNCTION__);
}

bool SIMCOM_SIM800L_UART_GetNetworkRegisterStatus(void)
{
    //GET NETWORK REGISTRATION STATUS

    uint8_t retval = 0;
    if(SIMCOM_SIM800L_UART_SendATCommand("AT+CREG?\r", "OK", 5000))
    {
        if(s_simcom_sim800l_uart_process_response(MODEM_RESPONSE_PROCESS_CREG,
                                                    (void*)(&retval)))
        {
            if(retval == 1 || retval == 5 || retval == 6 || retval == 7)
            {
                return true;
            }
            return false;
        }
        return false;
    }
    return false;
}

void SIMCOM_SIM800L_UART_NetworkRegister(void)
{
    //REGISTER ON THE NETWORK

    SIMCOM_SIM800L_UART_SendATCommand("AT+COPS=0\r", "OK", 2000);
    SIMCOM_SIM800L_UART_SendATCommand("AT+COPS=2\r", "OK", 2000);
}

uint8_t SIMCOM_SIM800L_UART_GetSignalQuality(void)
{
    //GET CELL SIGNAL QUALITY

    uint8_t retval= 0;
    if(SIMCOM_SIM800L_UART_SendATCommand("AT+CSQ\r", "OK", 2000))
    {
        if(s_simcom_sim800l_uart_process_response(MODEM_RESPONSE_PROCESS_CSQ,
                                                    (void*)(&retval)))
        {
            return retval;
        }
        return 0;
    }
    return 0;
}

bool SIMCOM_SIM800L_UART_GetNetworkTimeEnabled(void)
{
    //CHECK IF NETWORK TIME FEATURE ENABLED
    
    bool retval = true;
    if(SIMCOM_SIM800L_UART_SendATCommand("AT+CLTS?\r", "OK", 2000))
    {
        if(s_simcom_sim800l_uart_process_response(MODEM_RESPONSE_PROCESS_CLTS,
                                                    (void*)(&retval)))
        {
            return retval;
        }
        return false;
    }
    return false;
}

void SIMCOM_SIM800L_UART_NetworkTimeEnable(void)
{
    //ENABLE NETWORK TIME FEATURE

    SIMCOM_SIM800L_UART_SendATCommand("AT+CLTS=1\r", "OK", 2000);
    SIMCOM_SIM800L_UART_SendATCommand("AT&W\r", "OK", 2000);
}

void SIMCOM_SIM800L_UART_GetNetworkTime(void)
{
    //GET NETWORK TIME


}

void SIMCOM_SIM800L_UART_SendSms(void)
{
    //SEND SMS
}

void SIMCOM_SIM800L_UART_DeleteSms(void)
{
    //DELETE SMS
}

bool SIMCOM_SIM800L_UART_SendATCommand(char* command, char* response_match_token, uint32_t timeout_ms)
{
    //SEND AT COMMAND TO MODEM AND GET REPLY
    //LOOK FOR SPECIFIED RESPONSE MATCH TOKEN IN RESPONSE
    //WAIT TIMEOUT_MS FOR REPLY TO THE COMMAND

    //PRINT COMMAND ON DEBUG TERMINAL IF DEBUG MODE ON
    if(s_simcom_sim800l_uart_debug)
    {
        my_printf_debug("%s", command);
    }

    //SEND COMMAND TO MODEM
    INTERNAL_UART_UART2ClearCounters();
    my_printf_modem("%s", command);

    //START TIMEOUT
    APPLICATION_DelayMsNonBlocking(timeout_ms);

    //WAIT TILL TIMEOUT OR REPLY LINE RECEIVED
    while(INTERNAL_UART_UART2GetLineEndStatus() == false &&
            APPLICATION_GetNonBlockingDelayStatus() == false){};
    
    //PRINT RESPONSE ON DEBUG TERMINAL IF DEBUG MODE ON
    if(s_simcom_sim800l_uart_debug)
    {
        my_printf_debug("%s\n", INTERNAL_UART_UART2GetResponseBuffer());
    }

    //CHECK FOR RESPONSE MATCH TOKEN IN THE RECEIVED LINE
    if(strstr(INTERNAL_UART_UART2GetResponseBuffer(), response_match_token) != NULL)
    {
        //RESPONSE MATCH FOUND
        return true;
    }
    return false;
}

static bool s_simcom_sim800l_uart_process_response(s_simcom_800l_uart_response_type_t type,
                                                    void* retval)
{
    //PROCESS MODEM RESPONSE ACCORDING TO THE GIVEN RESPONSE TYPE

    int val1;
    int val2;

    switch(type)
    {
        case MODEM_RESPONSE_PROCESS_CREG:
            printf("response = %s", INTERNAL_UART_UART2GetResponseBuffer());
            sscanf((const char*)INTERNAL_UART_UART2GetResponseBuffer(),
                        "%*c%*c+CREG:%*c%u,%u\r",
                        &val1,
                        &val2);
            *((uint8_t*)retval) = val2;
            return true;
            break;
        
        case MODEM_RESPONSE_PROCESS_CSQ:
            printf("response = %s", INTERNAL_UART_UART2GetResponseBuffer());
            sscanf((const char*)INTERNAL_UART_UART2GetResponseBuffer(),
                        "%*c%*c+CSQ:%*c%u,%u\r",
                        &val1,
                        &val2);
            *((uint8_t*)retval) = val1;
            return true;
            break;

        default:
            return false;
    }
    return true;
}