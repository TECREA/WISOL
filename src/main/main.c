#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <wssfm1xrx.h>
#include <unity.h>
#include "wrappers.h"

WSSFM1XRXConfig_t Driver_WSS;
/*===================================================================================*/
WSSFM1XRX_DL_Return_t DownlinkCallback(WSSFM1XRXConfig_t* Instance){
    return WSSFM1XRX_DL_SUCCESS;
}
/*===================================================================================*/
int main(int argc,  char *argv[]){     
    char InputBuffer[32];
    uint16_t voltage;
    char ModuleID[32]={0}; 
    char ModulePAC[32]={0};
    uint8_t Payload[12] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C};
    uint8_t CopyPayload[37];

    WSSFM1XRX_Return_t RetValue;


    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_INIT_OK,      != , WSSFM1XRX_Init( &Driver_WSS, ResetControl, Reset2Control, OutputFunction, WSSFM1XRX_RCZ4, GetTickCountMs, InputBuffer, sizeof(InputBuffer), 4), 1E4, "");
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_WAITING,      == , WSSFM1XRX_ResetModule( &Driver_WSS, WSSFM1XRX_Wait_NonBlock ), 1E4, "" ); 
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_WAITING,      == , WSSFM1XRX_WakeUP( &Driver_WSS, WSSFM1XRX_Wait_NonBlock ), 1E4, "" );        
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_CheckModule( &Driver_WSS, WSSFM1XRX_Wait_NonBlock ), 10, "OK\r" );
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_GetVoltage( &Driver_WSS, WSSFM1XRX_Wait_NonBlock, &voltage), 10, "1254\r" );
    printf("Voltage = %d\r\n", voltage);
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_GetID( &Driver_WSS, WSSFM1XRX_Wait_NonBlock, ModuleID), 10, "00445566\r" );
    printf("ID = %s\r\n", ModuleID);
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_GetPAC( &Driver_WSS, WSSFM1XRX_Wait_NonBlock, ModulePAC), 10, "132456789AABBEE\r" );
    printf("PAC = %s\r\n", ModulePAC);   
    
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_ChangeFrequencyUL( &Driver_WSS, WSSFM1XRX_Wait_NonBlock, WSSFM1XRX_RCZ4), 10, "OK\r" );

    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_SaveParameters( &Driver_WSS, WSSFM1XRX_Wait_NonBlock), 10, "OK\r" );/* pROBAR CON Parser error*/
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_CHANN_NO_OK ,    != , RetValue = WSSFM1XRX_CheckChannels( &Driver_WSS, WSSFM1XRX_Wait_NonBlock), 10, "0,3\r" );
   if(RetValue == WSSFM1XRX_CHANN_NO_OK) {TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_ResetChannels( &Driver_WSS, WSSFM1XRX_Wait_NonBlock), 10, "OK\r" );}
    else printf("No reset channeld\r\n");
    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_SendMessage( &Driver_WSS, WSSFM1XRX_Wait_NonBlock, Payload,CopyPayload, 12, SF_FALSE), 20, "OK\r" );

    TEST_WSSFM_WAITUNTIL(  WSSFM1XRX_OK_RESPONSE , != , WSSFM1XRX_SendMessage( &Driver_WSS, WSSFM1XRX_Wait_NonBlock, Payload,CopyPayload, 12, SF_TRUE), 20, "OK\r" );

    return EXIT_SUCCESS;
}
/*===================================================================================*/