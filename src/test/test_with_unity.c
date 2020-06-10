
/*!
 * *******************************************************************************
 * @file test functions  with unity
 * @author julian bustamante
 * @version 1.0.0
 * @date Jan 10 , 2020
 * @brief test for verify driver sigfox module
 *********************************************************************************/

#include "test_with_unity.h"

/*===================================================================================*/
    WSSFM1XRXConfig_t Driver_WSS;
    TestData_t TestData;
    char InputBuffer[WSSFM1XRX_BUFF_RX_FRAME_LENGTH]; /*RXframe o Input*/
    uint16_t voltage;
    char ModuleID[32]={0}; 
    char ModulePAC[32]={0};
    uint8_t Payload[12] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C};
    uint8_t CopyPayload[WSSFM1XRX_BUFF_TX_FRAME_LENGTH];

    WSSFM1XRX_Return_t RetValue;
/*===================================================================================*/

void setUp (void){  } /* Is run before every test, put unit init calls here. */
void tearDown (void){ } /* Is run after every test, put unit clean-up calls here. */
/*===================================================================================*/

void test_sigfox_init(){
    WSSFM1XRX_Return_t RetValue;
    
    RetValue = WSSFM1XRX_Init(&Driver_WSS,NULL,
                Reset2Control,OutputFunction,WSSFM1XRX_RCZ4,
                GetTickCountMs,InputBuffer, sizeof(InputBuffer), 4);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_FAILURE,RetValue);

    RetValue = WSSFM1XRX_Init(&Driver_WSS,ResetControl,
                Reset2Control,OutputFunction,WSSFM1XRX_RCZ4,
                GetTickCountMs,InputBuffer, sizeof(InputBuffer), 4);

    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_INIT_OK,RetValue);


}
void test_sigfox_reset_module(){
   WSSFM1XRX_Return_t RetValue;

    RetValue = WSSFM1XRX_ResetModule(&Driver_WSS,WSSFM1XRX_Wait_Block );
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_TIMEOUT,RetValue);

    /*return waitting because non blocking- testing with while or WaitBlock*/
    RetValue = WSSFM1XRX_ResetModule(&Driver_WSS,WSSFM1XRX_Wait_NonBlock );
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_WAITING,RetValue);
}

void test_sigfox_wakeup(){
    WSSFM1XRX_Return_t RetValue; 
    RetValue = WSSFM1XRX_WakeUP( &Driver_WSS, WSSFM1XRX_Wait_Block );
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_TIMEOUT,RetValue);
}

void test_sigfox_checkModule(){
    WSSFM1XRX_Return_t RetValue;

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r",3); /*internamente el ya compara si es ok y devuelve ok response*/
    RetValue =  WSSFM1XRX_CheckModule( &Driver_WSS, WSSFM1XRX_Wait_Block ); /*function call OutputFunction*/
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);
}

void test_sigfox_getvoltage(){
    WSSFM1XRX_Return_t RetValue;

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "4130\r",5); /*internamente el ya compara si es ok y devuelve ok response*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_GetVoltage( &Driver_WSS, WSSFM1XRX_Wait_Block, &voltage);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);
    TEST_ASSERT_EQUAL_UINT16(4130,voltage);
    
    #ifdef _RESPONSE_BAD
        /*no envio \r , 5 bytes y no 6bytes, debe retornar failure*/
        (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
        (void)memcpy(TestData.ResponseExpected, "41300\r",5); /*internamente el ya compara si es ok y devuelve ok response*/
        /*function call OutputFunction*/
        RetValue = WSSFM1XRX_GetVoltage( &Driver_WSS, WSSFM1XRX_Wait_Block, &voltage);
        TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);
        TEST_ASSERT_EQUAL_UINT16(4130,voltage);   
    #endif
}

void test_sigfox_getid(){
    WSSFM1XRX_Return_t RetValue;

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "0044BC37\r",9); /*internamente el ya compara si es ok y devuelve ok response*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_GetID( &Driver_WSS, WSSFM1XRX_Wait_Block, ModuleID);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);
    TEST_ASSERT_EQUAL_STRING("0044BC37\r",ModuleID);
}

void test_sigfox_getpac(){
    WSSFM1XRX_Return_t RetValue;

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "132456789AABBEE\r",16); /*internamente el ya compara si es ok y devuelve ok response*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_GetPAC( &Driver_WSS, WSSFM1XRX_Wait_Block, ModulePAC);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);
    TEST_ASSERT_EQUAL_STRING("132456789AABBEE\r",ModulePAC);
}


void test_sigfox_changefrequencyUL(){
    WSSFM1XRX_Return_t RetValue;
    
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r",3); /*internamente el ya compara si es ok y devuelve ok response*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_ChangeFrequencyUL( &Driver_WSS, WSSFM1XRX_Wait_Block, WSSFM1XRX_RCZ4);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);

    /*response bad*/
    #ifdef _RESPONSE_BAD
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "O\r",2); /*internamente el ya compara si es ok y devuelve ok response*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_ChangeFrequencyUL( &Driver_WSS, WSSFM1XRX_Wait_Block, WSSFM1XRX_RCZ4);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_TIMEOUT,RetValue);
    #endif

}


void test_sigfox_changefrequencyDL(){
    WSSFM1XRX_Return_t RetValue;
    
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r",3); /*internamente el ya compara si es ok y devuelve ok response*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_ChangeFrequencyDL( &Driver_WSS, WSSFM1XRX_Wait_Block, WSSFM1XRX_RCZ4);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);

    /*response bad*/
    #ifdef _RESPONSE_BAD
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "O\r",2); /*internamente el ya compara si es ok y devuelve ok response*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_ChangeFrequencyDL( &Driver_WSS, WSSFM1XRX_Wait_Block, WSSFM1XRX_RCZ4);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_TIMEOUT,RetValue);
    #endif

}

void test_sigfox_saveparameters(){
    WSSFM1XRX_Return_t RetValue;
    
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r",3); /*internamente el ya compara si es ok y devuelve ok response*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_SaveParameters( &Driver_WSS, WSSFM1XRX_Wait_Block);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);

    /*response bad*/
    #ifdef _RESPONSE_BAD

    #endif

}

void test_sigfox_checkchannels(){
    WSSFM1XRX_Return_t RetValue;
    /* 		WSSFM1XRX_CHANN_NO_OK   : se debe resetear canales
 			WSSFM1XRX_CHANN_OK      : No se resetea canales*/

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "1,3\r",4);  /*X==0 | Y<3 -> RESET*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_CheckChannels( &Driver_WSS, WSSFM1XRX_Wait_Block);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_CHANN_OK,RetValue);

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "2,6\r",4);  /*X==0 | Y<3 -> RESET*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_CheckChannels( &Driver_WSS, WSSFM1XRX_Wait_Block);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_CHANN_OK,RetValue);
    
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "1,2\r",4);  /*X==0 | Y<3 -> RESET*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_CheckChannels( &Driver_WSS, WSSFM1XRX_Wait_Block);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_CHANN_NO_OK,RetValue);

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "0,3\r",4);  /*X==0 | Y<3 -> RESET*/
     /*function call OutputFunction*/
    RetValue = WSSFM1XRX_CheckChannels( &Driver_WSS, WSSFM1XRX_Wait_Block);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_CHANN_NO_OK,RetValue);

    /*response bad*/

    #ifdef _RESPONSE_BAD

    #endif

}

void test_sigfox_resetchannels(){
    WSSFM1XRX_Return_t RetValue;
    /* 		WSSFM1XRX_CHANN_NO_OK   : se debe resetear canales
 			WSSFM1XRX_CHANN_OK      : No se resetea canales*/

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r",3);  /*X==0 | Y<3 -> RESET*/
    RetValue = WSSFM1XRX_ResetChannels( &Driver_WSS, WSSFM1XRX_Wait_Block);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);
}


void test_sigfox_sendmessage(){
    WSSFM1XRX_Return_t RetValue;
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r",3);  
    RetValue = WSSFM1XRX_SendMessage( &Driver_WSS, WSSFM1XRX_Wait_Block, Payload,CopyPayload, 12, SF_FALSE);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r",3);  
    RetValue = WSSFM1XRX_SendMessage( &Driver_WSS, WSSFM1XRX_Wait_Block, Payload,CopyPayload, 12, SF_FALSE);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);


    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r\nRX=00 01 02 03 04 05 06 07\r",35); 
    RetValue = WSSFM1XRX_SendMessage( &Driver_WSS, WSSFM1XRX_Wait_Block, Payload,CopyPayload, 12, SF_TRUE);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);

    /*if response is bad, return timeout because non set flag-ready reception*/
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "ERR_SFX_ERR_SEND_FRAME_WAIT_TIMEOUT\r\n",39);  
    RetValue = WSSFM1XRX_SendMessage( &Driver_WSS, WSSFM1XRX_Wait_Block, Payload,CopyPayload, 12, SF_TRUE);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_TIMEOUT,RetValue);
    

    /*response bad*/
    #ifdef _RESPONSE_BAD
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r\nRX =00 01 02 03 04 05 06 07\r",20);  
    RetValue = WSSFM1XRX_SendMessage( &Driver_WSS, WSSFM1XRX_Wait_Block, Payload,CopyPayload, 12, SF_TRUE);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);  /*if response is bad, return WSSFM1XRX_TIMEOUT because non set flag-ready reception*/

    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "ERR_SFX_ERR_SEND_FRAME_WAIT_TIMEOUT\r\n",39);  
    RetValue = WSSFM1XRX_SendMessage( &Driver_WSS, WSSFM1XRX_Wait_Block, Payload,CopyPayload, 12, SF_TRUE);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);  /*if response is bad, return timeout because non set flag-ready reception*/
    #endif
}

void test_sigfox_matchresponse(){
    WSSFM1XRX_Return_t RetValue;
    (void)memset(Driver_WSS.RxFrame,0,WSSFM1XRX_BUFF_RX_FRAME_LENGTH);
    /*memcpy(Driver_WSS.RxFrame, "OK\r\nRX=00 01 02 03 04 05 06 07\r",40); */
    Driver_WSS.RxReady = 1;
    (void)memcpy(Driver_WSS.RxFrame, "OK\r\nRX=00 01 02 03 04 05 06 07\r",34);
    RetValue = WSSFM1XRX_MatchResponse( &Driver_WSS, "OK\r"); /*find first occurrence*/
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);  /*WSSFM1XRX_RSP_NOMATCH*/
    (void)memset(Driver_WSS.RxFrame,0,WSSFM1XRX_BUFF_RX_FRAME_LENGTH);
    Driver_WSS.RxReady = 0;
}

void test_sigfox_rxisr(){
    int i=0;
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r\nRX=00 01 02 03 04 05 06 07\r",34); 
    Driver_WSS.DownLink = 1;

    while(TestData.ResponseExpected[i] != '\0') {
		 	WSSFM1XRX_ISRRX(&Driver_WSS, TestData.ResponseExpected[i]);
			i++;
	}
    TEST_ASSERT_EQUAL_STRING(TestData.ResponseExpected,Driver_WSS.RxFrame);
}
/*Convierte downlink payload a array con los datos*/
void test_sigfox_dldiscrimination(){
    WSSFM1XRX_DL_Return_t RetValue;
    
    (void)memset(Driver_WSS.RxFrame,0,WSSFM1XRX_BUFF_RX_FRAME_LENGTH);
    (void)memcpy(Driver_WSS.RxFrame, "OK\r\nRX=00 01 02 03 04 05 06 07\r",40); 
    RetValue = DL_DiscriminateDownLink( &Driver_WSS);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_DL_SUCCESS,RetValue);

    (void)memset(Driver_WSS.RxFrame,0,WSSFM1XRX_BUFF_RX_FRAME_LENGTH);
    (void)memcpy(Driver_WSS.RxFrame, "ERR_SFX_ERR_SEND_FRAME_WAIT_TIMEOUT\r\n",39); 
    RetValue = DL_DiscriminateDownLink( &Driver_WSS);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_DL_HEAD_ERROR,RetValue);

      /*response bad*/
    #ifdef _RESPONSE_BAD
    (void)memset(Driver_WSS.RxFrame,0,WSSFM1XRX_BUFF_RX_FRAME_LENGTH);
    (void)memcpy(Driver_WSS.RxFrame, "ERR_SFX_ERR_SEND_FRAME_WAIT_TIMEOUT\r\n",39); 
    RetValue = DL_DiscriminateDownLink( &Driver_WSS);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_DL_SUCCESS,RetValue);
    #endif
}

void test_sigfox_askfrequencyul(){
    WSSFM1XRX_FreqUL_t frequencyul[50];
    WSSFM1XRX_Return_t RetValue;
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "920800000\r",11);  
    RetValue = WSSFM1XRX_AskFrequencyUL( &Driver_WSS, WSSFM1XRX_Wait_Block,frequencyul);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);
    
}

void test_sigfox_sleep(){
    WSSFM1XRX_Return_t RetValue;
    (void)memset(TestData.ResponseExpected,0,sizeof(TestData.ResponseExpected));
    (void)memcpy(TestData.ResponseExpected, "OK\r",11);  
    RetValue = WSSFM1XRX_Sleep( &Driver_WSS, WSSFM1XRX_Wait_Block);
    TEST_ASSERT_EQUAL_INT32(WSSFM1XRX_OK_RESPONSE,RetValue);
}
