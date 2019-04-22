/*!
 * *******************************************************************************
 * @file WSSFM1XRX.h
 * @author julian bustamante
 * @version 1.2.0
 * @date April 18, 2019
 * @brief Sigfox interface for the sigfox module. Interface
 * specific for module wisol SFM11R2D.
 *********************************************************************************/

#ifndef SOURCES_WSSFM1XRX_H_
#define SOURCES_WSSFM1XRX_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*BOOL VALUES*/
#define SF_TRUE						1
#define SF_FALSE 					0

/*Down link definitions ---------------------------------------------------------*/
/** Each byte represented in hex */
#define WSSFM1XRX_DL_BYTE_SIZE 2

#define WSSFM1XRX_DL_PAYLOAD_SYZE 8

/** Header(3) + Payload(16) + Spaces(7) */
#define WSSFM1XRX_DL_PAYLOAD_LENGTH 26

/** Offset between bytes within the string frame */
#define WSSFM1XRX_DL_BYTES_OFFSET 3

/** Downlink frame timeout */
#define WSSFM1XRX_DL_TIMEOUT 45000 /*45s*/

/** Minimum report time --> 10.285 min*/
#define WSSFM1XRX_DL_MIN_REPORT_TIME 617

/** Downlink request period maximun*/
#define WSSFM1XRX_DL_REQ_PERIOD_TRANSMISSION_MAX_DAY 4

/** Downlink request period horas */
#define WSSFM1XRX_DL_REQ_PERIOD_H 6

/** Downlink request period horas --> cada 6 horas*/
#define WSSFM1XRX_DL_REQ_PERIOD_S  (WSSFM1XRX_DL_REQ_PERIOD_H*3600) /* 360 21600*/

/** X time base in seconds for wakeup */
#define WSSFM1XRX_DL_TIMEREQUEST(X)	(uint8_t)(WSSFM1XRX_DL_REQ_PERIOD_S/X)

#define WSSFM1XRX_DL_IF_ANY_ERROR(x)		((x == WSSFM1XRX_DL_HEAD_ERROR) || (x == WSSFM1XRX_DL_TAIL_ERROR) || (x == WSSFM1XRX_DL_LENGTH_ERROR) )


/* Frame types -------------------------------------------------------------------*/
#define WSSFM1XRX_DL_FRAME_REPORT_TIME  4
#define WSSFM1XRX_DL_FRAME_REPORT_TIME_AND_TURN_OFF_MOTO  5

/* Numeric frame offsets ---------------------------------------------------------*/
#define WSSFM1XRX_DL_CTRLREG 0
#define WSSFM1XRX_DL_TREP 2
#define WSSFM1XRX_DL_P_ON_OFF  4 /*turn on off moto*/

/*Delays for expected response wisol module------------------------------------------------*/

/*Delay Time for WSSFM1XRX_SendMessage wisol module [ms]*/
#define WSSFM1XRX_SEND_MESSAGE_TIME_DELAY_RESP	    1000

/*GENERAL DELAY TIME FOR COMMANDS [ms]*/
#define WSSFM1XRX_GENERAL_TIME_DELAY_RESP	500

/*Delay Time for WSSFM1XRX_WakeUP wisol module [ms]*/
#define WSSFM1XRX_WAKEUP_TIME_DELAY_RESP	1000

/*Delay Time for WSSFM1XRX_SLEEP wisol module [ms]*/
#define WSSFM1XRX_SLEEP_TIME_DELAY_RESP	    1000

/*GENERAL DELAY TIME FOR COMMANDS [ms]*/
#define WSSFM1XRX_SEND_RAW_MESSAGE_TIME_DELAY_RESP	500

/**
 * @brief Pointer to Function type TickReadFcn_t : function Get Tick in ms.
 * 
 *@note In the default implementation, this variable "tick" is incremented each 1ms
 *       in ISR.
 * Example :
 * 	    uint32_t ReadTick_ms(void){
 *			tick++;	}
 * @param none.
 * @return uint32_t.
 */
typedef uint32_t (*TickReadFcn_t)(void);


/**
 * @brief Pointer to Function type DigitalFcn_t : enable or disable pin reset.
 * @note Example :
 * 	    void RSTCtrl_Sigfox(uint8_t sValue){
 *		     if(sValue) HAL_GPIO_WritePin(GPIOA, RST_SIGFOX_Pin, GPIO_PIN_SET);
 *		     else HAL_GPIO_WritePin(GPIOA, RST_SIGFOX_Pin, GPIO_PIN_RESET); 
 *		}
 * @param uint8_t.
 * @return none.
 */
typedef void (*DigitalFcn_t)(uint8_t);


/**
 * @brief Pointer to Function type TxFnc_t : Transmission by UART.
 * @note Example :
 *  	void UART_SIGFOX_TX_STM(void * Sp, char c){
 *  		HAL_UART_Transmit(&huart1,(uint8_t*)&c,USART_TX_AMOUNT_BYTES,USART_TIMEOUT);
 *		}
 * @param void *
 * @param char
 * @return none.
 */
typedef void (*TxFnc_t)(void*,char);


/**
 * @brief Pointer to Function type RxFnc_t : Reception  by UART.
 * @note Example :
 *		unsigned char UART_SIGFOX_RX_STM( unsigned char * Chr){
 *			*Chr = UART_RX.Data;
 *			return WRAPER_ERR_OK;
 *		}
 * @param unsigned char*
 * @return unsigned char.
 */
typedef unsigned char (*RxFnc_t)(unsigned char*);


/**
 * @brief Return codes for downlink operation.
 */
typedef enum{
   WSSFM1XRX_DL_SUCCESS,      /* Downlink successfully processed */
   WSSFM1XRX_DL_HEAD_ERROR,   /* Could not find RX */
   WSSFM1XRX_DL_TAIL_ERROR,   /* Could not find \r */
   WSSFM1XRX_DL_LENGTH_ERROR, /* Wrong length */
   WSSFM1XRX_DL_TIME_OK,		/*Time success*/
   WSSFM1XRX_DL_UNKNOWN,
   WSSFM1XRX_DL_DISCRIMINATE_ERROR
} WSSFM1XRX_DL_Return_t;

/**Frequency  Hz - Uplink********************************************************
 * */
typedef enum{
	WSSFM1XRX_UL_RCZ1 = 868130000,
	WSSFM1XRX_UL_RCZ2 = 902200000,
	WSSFM1XRX_UL_RCZ3 = 923200000,
	WSSFM1XRX_UL_RCZ4 = 920800000,
	WSSFM1XRX_UL_RCZ5 = 923300000,
	WSSFM1XRX_UL_RCZ6 = 865200000
}WSSFM1XRX_FreqUL_t;

/** Frequency Hz- Downlink******************************************************
 * */
typedef enum{
	WSSFM1XRX_DL_RCZ1 = 869525000,
	WSSFM1XRX_DL_RCZ2 = 905200000,
	WSSFM1XRX_DL_RCZ3 = 922200000,
	WSSFM1XRX_DL_RCZ4 = 922300000,
	WSSFM1XRX_DL_RCZ5 = 922300000,
	WSSFM1XRX_DL_RCZ6 = 866300000
}WSSFM1XRX_FreqDL_t;

/**
 * @brief Return codes for TIME and Expected Response operation.
 */
typedef enum{
	WSSFM1XRX_TIMEOUT   = 0,
	WSSFM1XRX_WAITING,
	WSSFM1XRX_RSP_NOMATCH,
	WSSFM1XRX_OK_RESPONSE,
	WSSFM1XRX_NONE,
	WSSFM1XRX_INIT_OK,
	WSSFM1XRX_PROCESS_FAILED, 		
	WSSFM1XRX_CHANN_OK, 			
	WSSFM1XRX_CHANN_NO_OK, 		
	WSSFM1XRX_DEFAULT
}WSSFM1XRX_Return_t;

/*Struct  containing all data*/
typedef struct WSSFM1XRXConfig{
	DigitalFcn_t RST;
	DigitalFcn_t RST2;
	TxFnc_t TX_WSSFM1XRX;
	RxFnc_t RX_WSSFM1XRX;
	TickReadFcn_t TICK_READ;
	/*Decodificar trama numerica return*/
	WSSFM1XRX_DL_Return_t (*DiscrimateFrameTypeFcn)(struct WSSFM1XRXConfig* );	 /*as� por que depende de la misma estructura*/
	volatile char RxFrame[100];
	volatile char TxFrame[100];
	volatile unsigned char RxReady;
	volatile uint8_t RxIndex;
	uint8_t StatusFlag;
	WSSFM1XRX_FreqUL_t Frequency;
	uint8_t DownLink;
	uint16_t Delay_msInternalWaitBlock;
	uint32_t UL_ReportTimeS;
	uint8_t DL_NumericFrame[WSSFM1XRX_DL_PAYLOAD_SYZE];
}WSSFM1XRXConfig_t;


/**
 * @brief  Pointer to function type WSSFM1XRX_WaitMode_t
 * @param obj Structure containing all data from the Wisol module.	
 * @param time in mili second.
 * @return WSSFM1XRX_Return_t 
 */
typedef WSSFM1XRX_Return_t (*WSSFM1XRX_WaitMode_t)(WSSFM1XRXConfig_t* ,uint32_t);

/*================================================================================
 *   prototypes
 *================================================================================*/



/**
 * @brief Function initialize the Wisol module.
 * @param obj Structure containing all data from the Wisol module.
 * @return Operation result in the form WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_Init(WSSFM1XRXConfig_t *obj, DigitalFcn_t Reset, DigitalFcn_t Reset2, TxFnc_t Tx_Wssfm1xrx, RxFnc_t Rx_Wssfm1xrx,WSSFM1XRX_FreqUL_t Frequency_Tx, WSSFM1XRX_DL_Return_t (*DiscrimateFrameTypeFCN)(struct WSSFM1XRXConfig* ) ,TickReadFcn_t TickRead, uint16_t DelayInternalWaitBlock);


/**
 * @brief Function delay non blocking.
 * @note  the function GetTick_ms must be initialized to work
 * @param obj Structure containing all data from the Wisol module.	
 * @param time in mili second.
 * @return Operation result in the form WSSFM1XRX_Return_t:
 */
WSSFM1XRX_Return_t WSSFM1XRX_Wait_NonBlock(WSSFM1XRXConfig_t *obj, uint32_t msec);

/**
 * @brief Function delay blocking.
 * @note  the function GetTick_ms must be initialized to work
 * @param obj Structure containing all data from the Wisol module.	
 * @param time in mili second.
 * @return Operation result in the form WSSFM1XRX_Return_t:
 */
WSSFM1XRX_Return_t WSSFM1XRX_Wait_Block(WSSFM1XRXConfig_t *obj , uint32_t msec);

/**
 * @brief Function set low power the Wisol module.
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_Sleep(WSSFM1XRXConfig_t *obj ,WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function wakeup from pin extern the Wisol module.
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return void.
 */
void WSSFM1XRX_WakeUP(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait  );

/**
 * @brief Function check module sending AT command to the Wisol module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
*/
WSSFM1XRX_Return_t WSSFM1XRX_CheckModule(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait);

/**
 * @brief Function get ID unique from Wisol module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_GetID(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function get PAC unique from Wisol module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_GetPAC(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function get supply Voltage  from Wisol module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_GetVoltage(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function Send command to the Wisol module.
* @param obj Structure containing all data from the Sigfox module.
 * @param Payload containing string to transmitions at Wisol module.
 * @param ExpectedResponse expectedResponse expected Response from module Wisol .
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_SendRawMessage(WSSFM1XRXConfig_t *obj,char* Payload,char* ExpectedResponse,WSSFM1XRX_WaitMode_t Wait);

/**
 * @brief Function ask channels of the transceiver.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 */
char* WSSFM1XRX_AskChannels(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait );

/**  Revisar DOC--------
 * @brief Function verificate channels of the transceiver.
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form WSSFM1XRX_Return_t.
*/
WSSFM1XRX_Return_t WSSFM1XRX_CheckChannels(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function reset channels from Wisol module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_ResetChannels(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function change frequency uplink from Sigfox module.
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_ChangeFrequencyUL(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function ask frequency uplink from Wisol module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return Pointer to reception buffer
 * */
char* WSSFM1XRX_AskFrequencyUL(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function save parameter in the flash memory from Sigfox module.
 * @param obj Structure containing all data from the Wisol module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @return WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_SaveParameters(WSSFM1XRXConfig_t *obj, WSSFM1XRX_WaitMode_t Wait );

/**
 * @brief Function send message frame in string hexadecimal to sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param Pointer to function delay blocking or non blocking, of type WSSFM1XRX_WaitMode_t
 * @param data Structure containing frame to send the Sigfox module.
 * @param size number of bytes in the payload. less o equal to 12 bytes.
 * @param eDownlink downlink enable o disable (0/1)
 * 
 * @return WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_SendMessage(WSSFM1XRXConfig_t *obj,WSSFM1XRX_WaitMode_t Wait, void* data, uint8_t size, uint8_t eDownlink);

/**
 * @brief Function ISR UART receive incoming frame to Sigfox module.
 * the buffer is stored in the structure obj->RxFrame.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void WSSFM1XRX_ISRRX(WSSFM1XRXConfig_t *obj);

/**
 * @brief Function verificate response received from Wisol module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param expectedResponse expected Response from module sigfox .
 * @return Operation result in the form WSSFM1XRX_Return_t.
 */
WSSFM1XRX_Return_t WSSFM1XRX_MatchResponse(WSSFM1XRXConfig_t *obj, char *expectedResponse);
/**
 * @brief Function to discriminate downlink frames.
 * @param obj Structure containing the incoming frame from the Sigfox module.
 * @param retVal Pointer to return a value.
 * @return Operation result in the form WSSFM1XRX_DL_Return_t.
 */
WSSFM1XRX_DL_Return_t DL_DiscriminateDownLink(WSSFM1XRXConfig_t* buff);

#endif /* SOURCES_WSSFM1XRX_H_ */
