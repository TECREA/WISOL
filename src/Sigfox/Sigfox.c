/*!
 * *******************************************************************************
 * @file WSSFM1XRX.c
 * @author julian bustamante
 * @version 1.2.0
 * @date April 18, 2019
 * @brief Sigfox interface for the sigfox module. Interface
 * specific for module wisol SFM11R2D.
 *********************************************************************************/

/*NOTAS A CORREGIR-NO OLVIDAR
Cambiar vector rx y tx frame por un puntero ??
rst y rst2???
State_SigfoxChangeFrequencyDL falta
Downlink tiempo como se hace en la espera de la respuesta del ok
*/

#include "Sigfox.h"
/*#include <stdio.h>*/
/** Private Prototypes************************************************************************************************************************ */

/*Function to  wait response with delay*/
static WSSFM1XRX_Return_t WSSFM1XRX_WaitForResponse(WSSFM1XRXConfig_t *obj ,char *ExpectedResponse, WSSFM1XRX_WaitMode_t Wait, uint32_t msec );

/*Function to  transmit*/
static void WSSFM1XRX_StringTX(WSSFM1XRXConfig_t *obj, char* SigfoxString);

/*Function Limpia buffer y ready */
static void WSSFM1XRX_ResetObject(WSSFM1XRXConfig_t *obj);

/*Function arma trama deacuerdo a la estructura de datos 12 bytes*/
static void WSSFM1XRX_BuildFrame(char* str, void* data, uint8_t size);

/*Nible(4bits) decimal to hex string*/
char NibbletoX(uint8_t value);

/*Definitions Private**************************************************************************************************************************/

/*Maximo tamaño buffer char*/
#define SIGFOX_MAX_DATA_SIZE 		25

/*Maximo tamaño trama hex string*/
#define	SIGFOX_MAX_BYTE_TX_FRAME	12

/*Maximo tamaño buffer para las frecuencias*/
#define SIGFOX_MAX_BUFF_FREQ		17

/*Public Functions*/

/**
 * @brief Function initialize the Wisol module.
 * @note Example :
 * 		SigfoxModule.StatusFlag = WSSFM1XRX_Init(&SigfoxModule, RSTCtrl_Sigfox, RST2Ctrl_Sigfox, UART_SIGFOX_TX_STM, UART_SIGFOX_RX_STM ,WSSFM1XRX_UL_RCZ4,NULL,GetTick_ms,500);
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_Init(WSSFM1XRXConfig_t *obj, DigitalFcn_t Reset, DigitalFcn_t Reset2,TxFnc_t Tx_Wssfm1xrx, RxFnc_t Rx_Wssfm1xrx,WSSFM1XRX_FreqUL_t Frequency_Tx,WSSFM1XRX_DL_Return_t (*DiscrimateFrameTypeFCN)(struct WSSFM1XRXConfig* ) , TickReadFcn_t TickRead, uint16_t DelayInternalWaitBlock){
	obj->RST=Reset;
	obj->RST2=Reset2;
	obj->TX_WSSFM1XRX=Tx_Wssfm1xrx;
	obj->RX_WSSFM1XRX=Rx_Wssfm1xrx;
	obj->DiscrimateFrameTypeFcn = DiscrimateFrameTypeFCN;
	obj->TICK_READ = TickRead;
	memset( (void *) obj->RxFrame,0,sizeof(obj->RxFrame));
	memset( (void *) obj->TxFrame,0,sizeof(obj->TxFrame));
	obj->RxReady=SF_FALSE;
	obj->RxIndex=0;
	obj->Frequency=Frequency_Tx;
	obj->Delay_msInternalWaitBlock = DelayInternalWaitBlock;
	return WSSFM1XRX_INIT_OK;
}

/*WSSFM1XRX_*/



/**
 * @brief Function delay non blocking.
 * @note  the function GetTick_ms must be initialized to work
 * @note Example :
 * 			WSSFM1XRX_Wait_NonBlock(&obj,500);
 * @param obj Structure containing all data from the Wisol module.	
 * @param time in mili second.
 * @return Operation result in the form WSSFM1XRX_Return_t:
 * 			<< WSSFM1XRX_TIMEOUT >> if the time has expired
 * 			<< WSSFM1XRX_WAITING >> if the time has not expired
 */
WSSFM1XRX_Return_t WSSFM1XRX_Wait_NonBlock(WSSFM1XRXConfig_t *obj, uint32_t msec){
	static uint8_t DelayRunning = 0;
	uint8_t RetValue = WSSFM1XRX_WAITING ;
	static uint32_t WSSFM1XRX_START = 0;

	if( !DelayRunning){
		DelayRunning = 1;
		WSSFM1XRX_START = obj->TICK_READ() ;/*tickRead_ms();*/
	}
	
	if( ( obj->TICK_READ() - WSSFM1XRX_START) > msec ){ 
		DelayRunning = 0; 
		RetValue = WSSFM1XRX_TIMEOUT ;
	}else RetValue = WSSFM1XRX_WAITING;
	return RetValue;
}

/**
 * @brief Function delay blocking.
 * @note  the function GetTick_ms must be initialized to work
 * @note Example :
 * 			WSSFM1XRX_Wait_Block(&obj,500);
 * @param obj Structure containing all data from the Wisol module.	
 * @param time in mili second.
 * @return Operation result in the form WSSFM1XRX_Return_t:
 * 			<< WSSFM1XRX_TIMEOUT >> if the time has expired
 */
WSSFM1XRX_Return_t WSSFM1XRX_Wait_Block(WSSFM1XRXConfig_t *obj, uint32_t msec){
    while( WSSFM1XRX_WAITING == WSSFM1XRX_Wait_NonBlock(obj, msec) ){ 
	}
    return WSSFM1XRX_TIMEOUT ;
}

/**
 * @brief Function set low power the Wisol module.
 * @note Example :
 *		WSSFM1XRX_Sleep(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block);
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 * 			<< WSSFM1XRX_OK_RESPONSE >> If response expected is the correct
 * 			<< WSSFM1XRX_RSP_NOMATCH >> If response expected is not correct
 */
WSSFM1XRX_Return_t WSSFM1XRX_Sleep(WSSFM1XRXConfig_t *obj ,WSSFM1XRX_WaitMode_t Wait ){
	obj->RST(1);
	obj->RST2(1);
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT$P=2\r");
	return WSSFM1XRX_WaitForResponse(obj,"OK",Wait,WSSFM1XRX_SLEEP_TIME_DELAY_RESP);
}


/**
 * @brief Function wakeup from pin extern the Wisol module.
 * @note Example :
 * 		WSSFM1XRX_WakeUP(&SigfoxModule,Wait);
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return void.
 */
void WSSFM1XRX_WakeUP(WSSFM1XRXConfig_t *obj ,WSSFM1XRX_WaitMode_t Wait  ){
	obj->RST(SF_FALSE);
	if(WSSFM1XRX_TIMEOUT == Wait(obj,WSSFM1XRX_WAKEUP_TIME_DELAY_RESP)) {
		obj->RST(SF_TRUE);
		obj->RST2(SF_TRUE);
	}
}


/**
 * @brief Function check module sending AT command to the Wisol module.
 * @note Example :
 * 		WSSFM1XRX_CheckModule(&SigfoxModule,Wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 * 			<< WSSFM1XRX_OK_RESPONSE >> If response expected is the correct
 * 			<< WSSFM1XRX_RSP_NOMATCH >> If response expected is not correct
 */
WSSFM1XRX_Return_t WSSFM1XRX_CheckModule(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait ){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT\r");
	return WSSFM1XRX_WaitForResponse(obj,"OK",Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP);
}

/**
 * @brief Function get ID unique from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_GetID(&SigfoxModule,wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_GetID(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait ){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT$I=10\r");
	if(WSSFM1XRX_TIMEOUT ==Wait(obj,WSSFM1XRX_GENERAL_TIME_DELAY_RESP)){}
	if( obj->RxReady ){  
		return (char*)obj->RxFrame ;
	}else return NULL;
	/*return NULL;*/
}

/**
 * @brief Function get PAC unique from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_GetID(&SigfoxModule,wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_GetPAC(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait ){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT$I=11\r");
	if(WSSFM1XRX_TIMEOUT ==Wait(obj,WSSFM1XRX_GENERAL_TIME_DELAY_RESP)){}
	if( obj->RxReady ){
		return (char*)obj->RxFrame ;
	}else return NULL;
}


/**
 * @brief Function get supply Voltage  from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_GetVolts(&SigfoxModule,Wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_GetVoltage(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait ){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT$V?\r");
	if(WSSFM1XRX_TIMEOUT ==Wait(obj,WSSFM1XRX_GENERAL_TIME_DELAY_RESP)){}
	if( obj->RxReady ){
		return (char*)obj->RxFrame ;
	}else return NULL;
}

/**
 * @brief Function Send command to the Wisol module.
 * @note  Example : WSSFM1XRX_SendRawMessageWaitResponse("AT$SF=aabbcc1122","OK",Wait)
 * @param obj Structure containing all data from the Sigfox module.
 * @param Payload containing string to transmitions at Wisol module.
 * @param ExpectedResponse expectedResponse expected Response from module Wisol .
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_SendRawMessage(WSSFM1XRXConfig_t *obj,char* Payload,char* ExpectedResponse,WSSFM1XRX_WaitMode_t Wait){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,Payload);
	if(WSSFM1XRX_TIMEOUT ==Wait(obj,WSSFM1XRX_SEND_RAW_MESSAGE_TIME_DELAY_RESP)){}
	if( obj->RxReady ){
		return (char*)obj->RxFrame ;
	}else return NULL;
}


/**
 * @brief Function ask channels of the transceiver.
 * @note Example :
 * 		WSSFM1XRX_AskChannels(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_AskChannels(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait ){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT$GI?\r");
	if(WSSFM1XRX_TIMEOUT ==Wait(obj,WSSFM1XRX_GENERAL_TIME_DELAY_RESP)){}
	if( obj->RxReady ){
		return (char*)obj->RxFrame ;
	}else return NULL;
}

/**  Revisar DOC--------
 * @brief Function verificate channels of the transceiver.
 * @note Example :
 * 		WSSFM1XRX_CheckChannels(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form WSSFM1XRX_Return_t.
 * 			WSSFM1XRX_CHANN_NO_OK   : se debe resetear canales
 *			WSSFM1XRX_CHANN_OK      : No se resetea canales
 */
WSSFM1XRX_Return_t WSSFM1XRX_CheckChannels(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait ){
	uint8_t x,y;
	char* rspPtr;
	char *x_y;
		x_y = WSSFM1XRX_AskChannels(obj,Wait);
	    rspPtr=strstr( ((const char *)x_y) , ",");
	/*rspPtr=strstr((const char *)obj->RxFrame , ",");*/
	if(rspPtr != NULL){
		x = (*(rspPtr-1))-'0';
		y = (*(rspPtr+1))-'0';
		return (x == 0 || y < 3) ? WSSFM1XRX_CHANN_NO_OK : WSSFM1XRX_CHANN_OK;
	}
	return WSSFM1XRX_PROCESS_FAILED;
}

/**
 * @brief Function reset channels from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_ResetChannels(&SigfoxModule, Wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 * 			<< WSSFM1XRX_OK_RESPONSE >> If response expected is the correct
 * 			<< WSSFM1XRX_RSP_NOMATCH >> If response expected is not correct 
 **/
WSSFM1XRX_Return_t WSSFM1XRX_ResetChannels(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait ){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT$RC\r");
	return WSSFM1XRX_WaitForResponse(obj,"OK",Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP);
}

/**
 * @brief Function change frequency uplink from Sigfox module.
 * @note Example :
 * 		WSSFM1XRX_ChangeFrequencyUL(&SigfoxModule,);
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 * 			<< WSSFM1XRX_OK_RESPONSE >> If response expected is the correct
 * 			<< WSSFM1XRX_RSP_NOMATCH >> If response expected is not correct 
 * */
WSSFM1XRX_Return_t WSSFM1XRX_ChangeFrequencyUL(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait ){
	char Frequency[SIGFOX_MAX_BUFF_FREQ];
	memset(Frequency,0,sizeof(Frequency));
	sprintf(Frequency,"AT$IF=%u\r",obj->Frequency); 
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,Frequency);
	return WSSFM1XRX_WaitForResponse(obj,"OK",Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP);
}

/**
 * @brief Function ask frequency uplink from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_AskFrequencyUL(&SigfoxModule, Wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 * */
char* WSSFM1XRX_AskFrequencyUL(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait ){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT$IF?\r");
	if(WSSFM1XRX_TIMEOUT == Wait(obj,WSSFM1XRX_GENERAL_TIME_DELAY_RESP)){}
	if( obj->RxReady ){
		return (char*)obj->RxFrame ;
	}else return NULL;
}

/**
 * @brief Function save parameter in the flash memory from Sigfox module.
 * @note Example :
 * 		WSSFM1XRX_SaveParameters(&SigfoxModule,Wait);
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 * 			<< WSSFM1XRX_OK_RESPONSE >> If response expected is the correct
 * 			<< WSSFM1XRX_RSP_NOMATCH >> If response expected is not correct 
 * */
WSSFM1XRX_Return_t WSSFM1XRX_SaveParameters(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait ){
	WSSFM1XRX_ResetObject(obj);
	WSSFM1XRX_StringTX(obj,"AT$WR\r");
	return WSSFM1XRX_WaitForResponse(obj,"OK",Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP);
}


/**
 * @brief Function send message frame in string hexadecimal to sigfox module.
 * 	@note Example :
 * 		x = WSSFM1XRX_SendMessage(&SigfoxModule,Wait,&iButton_Data,MAX_SIZE_IBUTTON_DATA,SigfoxModule.DownLink);
 * 
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @param data Structure containing frame to send the Sigfox module.
 * @param size number of bytes in the payload. less o equal to 12 bytes.
 * @param eDownlink downlink enable o disable (0/1)
 * 
 * @return WSSFM1XRX_Return_t.
 * 			<< WSSFM1XRX_OK_RESPONSE >> If response expected is the correct
 * 			<< WSSFM1XRX_RSP_NOMATCH >> If response expected is not correct 
 *
 */
WSSFM1XRX_Return_t WSSFM1XRX_SendMessage(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait, void* data, uint8_t size, uint8_t eDownlink){
	char str[SIGFOX_MAX_DATA_SIZE] = {0};
	uint32_t timeWait = WSSFM1XRX_SEND_MESSAGE_TIME_DELAY_RESP;
	WSSFM1XRX_ResetObject(obj);

	WSSFM1XRX_BuildFrame(str, data, size);
	WSSFM1XRX_StringTX(obj,"AT$SF=");
	WSSFM1XRX_StringTX(obj, str);
	
	WSSFM1XRX_StringTX(obj, ( obj->DownLink = eDownlink )?  ",1\r" : "\r");
	strcpy((char *)obj->TxFrame,(const char *)str);
	timeWait = eDownlink ? WSSFM1XRX_DL_TIMEOUT : WSSFM1XRX_SEND_MESSAGE_TIME_DELAY_RESP;

	return WSSFM1XRX_WaitForResponse(obj,"OK",Wait,timeWait);
}

/**
 * @brief Function ISR UART receive incoming frame to Wisol module.
 * @note Example :
 * 		SigfoxISRRX(&SigfoxModule);    //call in the  interrup serial
 * the buffer is stored in the structure obj->RxFrame.
 * 
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void WSSFM1XRX_ISRRX(WSSFM1XRXConfig_t *obj){
	unsigned char rxChar_Sigfox;

	obj->RX_WSSFM1XRX(&rxChar_Sigfox);
	if(obj->RxReady) return; /* B_uffer reveived*/
	obj->RxFrame[obj->RxIndex++] = rxChar_Sigfox;
	if (obj->RxIndex>=sizeof(obj->RxFrame)-1) obj->RxIndex=0;
	obj->RxFrame[obj->RxIndex] = 0;
	if (rxChar_Sigfox=='\r'){
		/*  Check if there is a downlink request */
		if(!obj->DownLink){
		    obj->RxIndex = 0;
		    obj->RxReady = SF_TRUE; /* Framed completed*/
		}else
			obj->DownLink = 0; /* Clear the downlink request */
	}
}


/**
 * @brief Function verificate response received from Wisol module.
 * @note Example :
 * 		if(WSSFM1XRX_MatchResponse(&SigfoxModule,"OK") == WSSFM1XRX_OK_RESPONSE) {}
 * @param obj Structure containing all data from the Sigfox module.
 * @param expectedResponse expected Response from module sigfox .
 * 
 * @return Operation result in the form WSSFM1XRX_Return_t.
 * 			<< WSSFM1XRX_OK_RESPONSE >> If response expected is the correct
 * 			<< WSSFM1XRX_RSP_NOMATCH >> If response expected is not correct 
 */
WSSFM1XRX_Return_t WSSFM1XRX_MatchResponse(WSSFM1XRXConfig_t *obj, char *expectedResponse){
	uint8_t Rprocess = WSSFM1XRX_RSP_NOMATCH;
	if(strstr((const char *) obj->RxFrame,(char *)expectedResponse) != NULL) Rprocess = WSSFM1XRX_OK_RESPONSE;
	else Rprocess = WSSFM1XRX_RSP_NOMATCH;
	return Rprocess;
}

/** Revisar DOC---------
 * @brief Function to discriminate downlink frames.
 * @param obj Structure containing the incoming frame from the Sigfox module.
 * @param retVal Pointer to return a value.
 * @return Operation result in the form WSSFM1XRX_DL_Return_t.
 */
WSSFM1XRX_DL_Return_t DL_DiscriminateDownLink(WSSFM1XRXConfig_t* obj){
    uint8_t* payLoadHead;
    uint8_t* payLoadTail;
    uint8_t byteIndex = WSSFM1XRX_DL_BYTES_OFFSET;
   /* uint8_t numericFrame[WSSFM1XRX_DL_PAYLOAD_SYZE];*/
    uint8_t byteStr[WSSFM1XRX_DL_BYTE_SIZE + 1] = {0};

    /* Get payload offset */
    payLoadHead = (uint8_t *)strstr((const char*)obj->RxFrame, "RX");

    if(!payLoadHead)
        return WSSFM1XRX_DL_HEAD_ERROR;

    /* Check payload length */
    payLoadTail = (uint8_t *)strstr((const char*)payLoadHead, "\r");

    if(!payLoadTail)
        return WSSFM1XRX_DL_TAIL_ERROR;

    if((payLoadTail - payLoadHead) != WSSFM1XRX_DL_PAYLOAD_LENGTH)
        return WSSFM1XRX_DL_LENGTH_ERROR;

    /* Convert frame to numeric values */
	uint8_t i;
    for( i = 0; i < WSSFM1XRX_DL_PAYLOAD_SYZE; i++){

        /* Copy byte strings an convert them to numbers */
        strncpy((char *)byteStr, (const char *)(payLoadHead + byteIndex), WSSFM1XRX_DL_BYTE_SIZE);
        obj->DL_NumericFrame[i] = (uint8_t)strtol((const char *)byteStr, NULL, 16);
        byteIndex += WSSFM1XRX_DL_BYTES_OFFSET;
    }


    return ( NULL != obj->DiscrimateFrameTypeFcn )? obj->DiscrimateFrameTypeFcn(obj) : WSSFM1XRX_DL_DISCRIMINATE_ERROR;
}

/*Private Functions ********************************************************************************************************************************/
static void WSSFM1XRX_StringTX(WSSFM1XRXConfig_t *obj, char* WSSFM1XRX_String){
	while(*WSSFM1XRX_String) {obj->TX_WSSFM1XRX(NULL,*WSSFM1XRX_String);WSSFM1XRX_String++;}
}

/*Private Functions ***********************************************************************************************************************************************/
static void WSSFM1XRX_ResetObject(WSSFM1XRXConfig_t *obj){
	memset((void *)obj->RxFrame,0,sizeof(obj->RxFrame));
	memset((void *)obj->TxFrame,0,sizeof(obj->TxFrame));
	obj->RxReady=SF_FALSE;
	obj->RxIndex=0;
	obj->StatusFlag=WSSFM1XRX_DEFAULT;
}


/*Private Functions*/

/**
 * @brief Function order the frame.
 */
static void WSSFM1XRX_BuildFrame(char* str, void* data, uint8_t size){
	int8_t  i,j;   
	uint8_t xbyte, finalsize;
	uint8_t *bdata = (uint8_t*)data;
	size = (size > SIGFOX_MAX_BYTE_TX_FRAME )? SIGFOX_MAX_BYTE_TX_FRAME : size;
	finalsize = size*2;
	str[finalsize]='\0';

	for(i = (size-1) , j = 0; i >= 0; i--){
		xbyte = bdata[i];
		str[j++]=NibbletoX(xbyte>>4);
		str[j++]=NibbletoX(xbyte);
	}
}

/**
 * @brief Function wait for response expected.
 */
static WSSFM1XRX_Return_t WSSFM1XRX_WaitForResponse(WSSFM1XRXConfig_t *obj , char *ExpectedResponse, WSSFM1XRX_WaitMode_t Wait ,uint32_t msec){
   uint8_t retvalue = WSSFM1XRX_NONE;
    if( WSSFM1XRX_MatchResponse(obj,ExpectedResponse)  == WSSFM1XRX_RSP_NOMATCH ){  /*mientras no sea resp = */
       
	   if( (Wait != WSSFM1XRX_Wait_Block) || (Wait != WSSFM1XRX_Wait_Block) ){
		   retvalue = Wait(obj,msec);
		   retvalue = WSSFM1XRX_MatchResponse(obj, ExpectedResponse);
	   }
	   else retvalue =  Wait(obj,msec);
	  	 /*retvalue =  Wait(obj,500);*/
       if(retvalue == WSSFM1XRX_TIMEOUT){  /*Delay blocking*/
           return WSSFM1XRX_MatchResponse(obj, ExpectedResponse);
       }
       return retvalue;   /*Delay NonBlocking*/
    }
    return WSSFM1XRX_OK_RESPONSE;
}

/*Nible(4bits) decimal to hex string***/

char NibbletoX(uint8_t value){
    char ch;
    ch = (char)(value & 0x0F) + '0';
    return (ch > '9')? ch+7u : ch;
}