/*!
 * *******************************************************************************
 * @file WSSFM1XRX.c
 * @author julian bustamante
 * @version 1.2.0
 * @date April 18, 2019
 * @brief Sigfox interface for the sigfox module. Interface
 * specific for module wisol SFM11R2D.
 *********************************************************************************/

/*NOTAS A CORREGIR-SOLO FORMATO-FUNCIONANDO MUY BIEN-NO OLVIDAR

-rsty rst2 cambiar nombres  
-State_SigfoxChangeFrequencyDL falta
-Revisar wakeup block o non block
-revisar d enuevo todas las funciones ya que s emodifica si no se manda frame en sendrawmessage
-hacer función reiniciar status leer linea abajo, la meto en ResetObject Revisar??
Si por algun motivo no responde el gettick, la no bloqueante se queda en waitting cambiar variables y agregarlas a una estructura para reiniciar la estructura en la plaicación en su momento??
Puede que pase...
-RX_Sigfox puntero a función Si se necesita , ya que se modifico ISR
-Revisar ISR de nuevo se modifica char input
-Caracteres no imprimibles en ISR revisar, esta pendiente
-Nombre de check module , ask frequency verificar
-Documentar todas las funciones , se modificaron todas.
-hacer test con unity
-Hacer funcion para reiniciar el modulo

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
#define WSSFM1XRX_MAX_DATA_SIZE 		25 /*25*/

/*Maximo tamaño trama hex string*/
#define	WSSFM1XRX_MAX_BYTE_TX_FRAME	12

/*Maximo tamaño buffer para las frecuencias*/
#define WSSFM1XRX_MAX_BUFF_FREQ		17

/*Public Functions*/

/**
 * @brief Function initialize the Wisol module.
 * @note Example :
 * 		SigfoxModule.StatusFlag = WSSFM1XRX_Init(&SigfoxModule, RSTCtrl_Sigfox, RST2Ctrl_Sigfox, UART_SIGFOX_TX_STM, UART_SIGFOX_RX_STM ,WSSFM1XRX_UL_RCZ4,NULL,GetTick_ms);
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_Init(WSSFM1XRXConfig_t *obj, DigitalFcn_t Reset, DigitalFcn_t Reset2, TxFnc_t Tx_Wssfm1xrx, RxFnc_t Rx_Wssfm1xrx,WSSFM1XRX_FreqUL_t Frequency_Tx, WSSFM1XRX_DL_Return_t (*DiscrimateFrameTypeFCN)(struct WSSFM1XRXConfig* ) ,TickReadFcn_t TickRead,char* BuffRxframe , uint8_t SizeBuffRx, char* BuffTxframe, uint8_t SizeBuffTx){
	obj->RST=Reset;
	obj->RST2=Reset2;
	obj->TX_WSSFM1XRX=Tx_Wssfm1xrx;
	obj->RX_WSSFM1XRX=Rx_Wssfm1xrx; /*No se necesita con char input isr*/
	obj->DiscrimateFrameTypeFcn = DiscrimateFrameTypeFCN;
	obj->TICK_READ = TickRead;
	obj->RxReady=SF_FALSE;
	obj->RxIndex=0;
	obj->Frequency=Frequency_Tx;
	obj->RxFrame = BuffRxframe; 
	obj->SizeBuffRx = SizeBuffRx;
	obj->TxFrame = BuffTxframe;
	obj->SizeBuffTx = SizeBuffTx;
	obj->State_Api = WSSFM1XRX_IDLE; /**/
	obj->State_W = WSSFM1XRX_W_IDLE; /*State Idle functión Wait non blocking*/
	memset( (void *) obj->RxFrame,0,obj->SizeBuffRx);
	memset( (void *) obj->TxFrame,0,obj->SizeBuffTx);
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
	static uint8_t RetValue;
	static uint32_t WSSFM1XRX_StartTick = 0;
	if(obj->State_W == WSSFM1XRX_W_IDLE ){
		RetValue = WSSFM1XRX_WAITING ;
		WSSFM1XRX_StartTick = 0;
		WSSFM1XRX_StartTick = obj->TICK_READ() ;/*tickRead_ms();*/
		obj->State_W = WSSFM1XRX_W_RUNNING ;
	}
	if( ( obj->TICK_READ() - WSSFM1XRX_StartTick) > msec ){ 
		obj->State_W = WSSFM1XRX_W_IDLE;
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
	return WSSFM1XRX_SendRawMessage(obj,"AT$P=2\r","OK",NULL,Wait,WSSFM1XRX_SLEEP_TIME_DELAY_RESP); 
}


/**
 * @brief Function wakeup from pin extern the Wisol module.
 * @note Example :
 * 		WSSFM1XRX_WakeUP(&SigfoxModule,Wait);
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return void.
 */
WSSFM1XRX_Return_t WSSFM1XRX_WakeUP(WSSFM1XRXConfig_t *obj ,WSSFM1XRX_WaitMode_t Wait  ){
	WSSFM1XRX_Return_t RetValue;  
	obj->RST(SF_FALSE);
	RetValue = WSSFM1XRX_SendRawMessage(obj,NULL,NULL,NULL,Wait,WSSFM1XRX_WAKEUP_TIME_DELAY_PULSE);
	if(WSSFM1XRX_TIMEOUT == RetValue) {
		obj->RST(SF_TRUE);
		obj->RST2(SF_TRUE);
	}
	/*Wait despues de salir del modo de bajo consumo*/
	return  RetValue == WSSFM1XRX_TIMEOUT ? WSSFM1XRX_SendRawMessage(obj,NULL,NULL,NULL,Wait,WSSFM1XRX_WAKEUP_WAIT_TIME_DELAY_RESP):  RetValue;
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
	return WSSFM1XRX_SendRawMessage(obj,"AT\r","OK",NULL,Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP); 
}

/**
 * @brief Function get ID unique from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_GetID(&SigfoxModule,wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @param Pointer to buffer for store ID
 * @return WSSFM1XRX_Return_t
 * 			WSSFM1XRX_OK_RESPONSE
 * 			WSSFM1XRX_FAILURE
 * 			WSSFM1XRX_WAITING
 */
WSSFM1XRX_Return_t WSSFM1XRX_GetID(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait,char *IDStr){
	return 	WSSFM1XRX_GetRespNoexpected(obj,Wait,"AT$I=10\r",IDStr);

	/*return NULL;*/
}

/**
 * @brief Function get PAC unique from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_GetID(&SigfoxModule,wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @param Pointer to buffer for store PAC
 * @return WSSFM1XRX_Return_t
 * 			WSSFM1XRX_OK_RESPONSE
 * 			WSSFM1XRX_FAILURE
 * 			WSSFM1XRX_WAITING
 */
WSSFM1XRX_Return_t WSSFM1XRX_GetPAC(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait,char *PACStr ){
	return 	WSSFM1XRX_GetRespNoexpected(obj,Wait,"AT$I=11\r",PACStr);
}


/**
 * @brief Function get supply Voltage  from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_GetVolts(&SigfoxModule,Wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @param Pointer to reception buffer
 * @return WSSFM1XRX_Return_t
 * 			WSSFM1XRX_OK_RESPONSE
 * 			WSSFM1XRX_FAILURE
 * 			WSSFM1XRX_WAITING
 */
WSSFM1XRX_Return_t WSSFM1XRX_GetVoltage(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait,uint16_t *mVolt ){
	char mVolStr[10];
	char *ptr = NULL;
	WSSFM1XRX_Return_t RetValue;

	RetValue =	WSSFM1XRX_GetRespNoexpected(obj,Wait,"AT$V?\r",mVolStr);
	*mVolt = strtol((const char*)mVolStr , &ptr ,BASE_DECIMAL);
	return RetValue;
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
WSSFM1XRX_Return_t WSSFM1XRX_SendRawMessage(WSSFM1XRXConfig_t *obj,char* Payload,char* ExpectedResponse,char * BuffStr,WSSFM1XRX_WaitMode_t Wait,uint32_t msec){
	WSSFM1XRX_Return_t RetValue;
	if( ( obj->State_Api == WSSFM1XRX_IDLE) ) { /*Firstentry bloqueante o no bloqueante*/
		WSSFM1XRX_ResetObject(obj);
		if(Payload != NULL) WSSFM1XRX_StringTX(obj,Payload);
		obj->State_Api = WSSFM1XRX_RUNNING;
	}
	if(ExpectedResponse != NULL){  /*Si la respuesta no es la esperada se queda esperando pero no envia hasta que reinicie obj->State_Api */
		RetValue = WSSFM1XRX_WaitForResponse(obj,ExpectedResponse,Wait,msec);

	}else RetValue = Wait(obj,WSSFM1XRX_GENERAL_TIME_DELAY_RESP) ;

	if(WSSFM1XRX_TIMEOUT == RetValue || WSSFM1XRX_OK_RESPONSE == RetValue){  /*para que funcione block or non block*/
		if( obj->RxReady ){ 
			if(BuffStr != NULL) strcpy((char*)BuffStr, (char*)obj->RxFrame) ;
			RetValue = WSSFM1XRX_OK_RESPONSE;
		}else {
			if(Payload != NULL)	RetValue = WSSFM1XRX_FAILURE;
		}
		obj->State_Api = WSSFM1XRX_IDLE;
	}
	return RetValue;
}


/**
 * @brief Function ask channels of the transceiver.
 * @note Example :
 * 		WSSFM1XRX_AskChannels(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
WSSFM1XRX_Return_t WSSFM1XRX_AskChannels(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait,Channels_t *Channels ){
	WSSFM1XRX_Return_t RetVal ;
	char* rspPtr;
	RetVal = WSSFM1XRX_SendRawMessage(obj,"AT$GI?\r",NULL,NULL,Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP);
	if(WSSFM1XRX_OK_RESPONSE == RetVal){
		rspPtr=strchr( ((const char *)obj->RxFrame) , ',');
		if(rspPtr != NULL){
			Channels->x = (*(rspPtr-1))-'0';
			Channels->y = (*(rspPtr+1))-'0';
		}
	}
	return RetVal;
}

/**  Revisar DOC-------- 
 * @brief Function verificate c hannels of the transceiver.
 * @note Example :
 * 		WSSFM1XRX_CheckChannels(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form WSSFM1XRX_Return_t.
 * 			WSSFM1XRX_CHANN_NO_OK   : se debe resetear canales
 *			WSSFM1XRX_CHANN_OK      : No se resetea canales
 */
/*hacer getchannels */
WSSFM1XRX_Return_t WSSFM1XRX_CheckChannels(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait ){
	Channels_t Channels ={0,0};
	WSSFM1XRX_Return_t retval;
	retval = WSSFM1XRX_AskChannels(obj,Wait,&Channels );
	if(WSSFM1XRX_OK_RESPONSE == retval){
		retval = (Channels.x == 0 || Channels.y < 3) ? WSSFM1XRX_CHANN_NO_OK : WSSFM1XRX_CHANN_OK;
	}
	return retval;
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
	return WSSFM1XRX_SendRawMessage(obj,"AT$RC\r","OK",NULL,Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP); 
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
WSSFM1XRX_Return_t WSSFM1XRX_ChangeFrequencyUL(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait , WSSFM1XRX_FreqUL_t Frequency){	
	char BFrequency[WSSFM1XRX_MAX_BUFF_FREQ];
	memset(BFrequency,0,sizeof(BFrequency));
	sprintf(BFrequency,"AT$IF=%u\r",Frequency);  /*Modificar despues*/
	return WSSFM1XRX_SendRawMessage(obj,BFrequency,"OK",NULL,Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP); 
}

/**
 * @brief Function ask frequency uplink from Wisol module.
 * @note Example :
 * 		WSSFM1XRX_AskFrequencyUL(&SigfoxModule, Wait);
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return   WSSFM1XRX_Return_t  WSSFM1XRX_OK_RESPONSE
 * */
WSSFM1XRX_Return_t WSSFM1XRX_AskFrequencyUL(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait, WSSFM1XRX_FreqUL_t *Frequency ){
	/*No tiene control de los digitos tamañp de la freq*/
	char FreqStr[10];
	char *ptr = NULL;
	WSSFM1XRX_Return_t RetValue;

	RetValue =	WSSFM1XRX_GetRespNoexpected(obj,Wait,"AT$IF?\r",FreqStr);
	*Frequency = strtol((const char*)FreqStr , &ptr ,BASE_DECIMAL);
	return RetValue;
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
	return WSSFM1XRX_SendRawMessage(obj,"AT$WR\r","OK",NULL,Wait,WSSFM1XRX_GENERAL_TIME_DELAY_RESP); 
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
	char str[WSSFM1XRX_MAX_DATA_SIZE] = {0};
	char Frame[37] = {0};

	uint32_t timeWait = WSSFM1XRX_SEND_MESSAGE_TIME_DELAY_RESP;
	WSSFM1XRX_BuildFrame(str, data, size);
	memset(Frame,0,sizeof(Frame));
	/*con Downlink no transmite bien EN CONSOLA*/
	sprintf(Frame,"AT$SF=%s%s",str,(  obj->DownLink = eDownlink )?  ",1\r" : "\r");  /*Modificar despues, revisar salida de mensaje*/
	strcpy(obj->TxFrame,Frame);
	timeWait = eDownlink ? WSSFM1XRX_DL_TIMEOUT : WSSFM1XRX_SEND_MESSAGE_TIME_DELAY_RESP; /*WSSFM1XRX_DL_TIMEOUT*/
	return WSSFM1XRX_SendRawMessage(obj,Frame,"OK",NULL,Wait,timeWait);
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
void WSSFM1XRX_ISRRX(WSSFM1XRXConfig_t *obj, const char RxChar){
	if(RxChar < CHAR_PRINT_BELOW  || RxChar > CHAR_PRINT_ABOVE) return ;  /*Char no print*/
	if(obj->RxReady) return; /* B_uffer reveived*/
	obj->RxFrame[obj->RxIndex++] = RxChar;
	if (obj->RxIndex>= obj->SizeBuffRx -1) obj->RxIndex=0;
	obj->RxFrame[obj->RxIndex] = 0;
	if (RxChar =='\r'){
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
	while(*WSSFM1XRX_String) {
		obj->TX_WSSFM1XRX(NULL,*WSSFM1XRX_String);
		WSSFM1XRX_String++;
	}
}

/*Private Functions ***********************************************************************************************************************************************/
static void WSSFM1XRX_ResetObject(WSSFM1XRXConfig_t *obj){
	memset((void *)obj->RxFrame,0,sizeof(obj->RxFrame));
	memset((void *)obj->TxFrame,0,sizeof(obj->TxFrame));
	obj->RxReady=SF_FALSE;
	obj->RxIndex=0;
	obj->StatusFlag=WSSFM1XRX_DEFAULT;
	/*obj->State_Api = WSSFM1XRX_IDLE;*/ /*verificar*/
}


/*Private Functions*/

/**
 * @brief Function order the frame.
 */
static void WSSFM1XRX_BuildFrame(char* str, void* data, uint8_t size){
	int8_t  i,j;   
	uint8_t xbyte, finalsize;
	uint8_t *bdata = (uint8_t*)data;
	size = (size > WSSFM1XRX_MAX_BYTE_TX_FRAME )? WSSFM1XRX_MAX_BYTE_TX_FRAME : size;
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

		if( (Wait != WSSFM1XRX_Wait_Block) && (Wait != WSSFM1XRX_Wait_NonBlock) ){
			retvalue = Wait(obj,msec);
			retvalue = WSSFM1XRX_MatchResponse(obj, ExpectedResponse);
		}else  retvalue =  Wait(obj,msec);
		if(retvalue == WSSFM1XRX_TIMEOUT){  /*Delay blocking*/
			return  WSSFM1XRX_MatchResponse(obj, ExpectedResponse);
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


/**
 * @brief function aux send command and wait  to sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @param Pointer to char *  containing Command AT to send the Sigfox module.
 * @param Pointer to char *  to store the response of the sigfox module.
 * @param eDownlink downlink enable o disable (0/1)
 * 
 * @return WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_GetRespNoexpected(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait, char * CommandStr, char *BuffStr){
	WSSFM1XRX_Return_t RetValue;
	if( ( obj->State_Api == WSSFM1XRX_IDLE) ) { /*Firstentry block or Nonblocking*/
		WSSFM1XRX_ResetObject(obj);
		WSSFM1XRX_StringTX(obj,CommandStr);
		obj->State_Api = WSSFM1XRX_RUNNING;
	}
	RetValue = Wait(obj,WSSFM1XRX_GENERAL_TIME_DELAY_RESP) ;
	if(WSSFM1XRX_TIMEOUT == RetValue){
		if( obj->RxReady ){  
			if(BuffStr != NULL ) strcpy((char*)BuffStr, (char*)obj->RxFrame) ; /*frame stored in RxFrame*/
			RetValue = WSSFM1XRX_OK_RESPONSE;
		}else RetValue = WSSFM1XRX_FAILURE;
		obj->State_Api = WSSFM1XRX_IDLE;
	}
	return RetValue;
}
