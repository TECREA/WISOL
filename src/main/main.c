
/* 
 * File:   main.c
 * Author: julian
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "Sigfox.h"


/**************************** Data Sigfox *****************************/
typedef struct {
	union {
		uint8_t Others[4];

		/** Si solo es un canal de 12 bi*/
		struct {
			unsigned ADC_12 :12;	/*Canal AN 12 bits*/
			unsigned DUMMY	:20;	/*N/A*/
		};
		struct {
			unsigned ADC_0:12;		/*Canal AN 12 bits*/
			unsigned ADC_1:12;		/*Canal AN 12 bits*/
			unsigned DI:1; 						/* DI  LSB en el orden que lo coloque acá  DI, BattLow, Type  hacia abajo*/
			unsigned BattLow:1;
			unsigned Type:5;
			unsigned periodic:1;
		};
	}Doce_Bytes;
	float Longitud;
	float Latitud;
}DataFrame_t;

DataFrame_t DataFrame ={0};
typedef struct{
	char ID[12];
	char PAC[22];
	uint16_t VBatt;
	WSSFM1XRX_Service_Status_t WilsolService_Status;
}SigFox_Info_t;
SigFox_Info_t SigFoxData;

Channels_t Channels;
pthread_t hilo;
void *hilo_time(void *argin);
uint32_t GetTick_ms (void);
void IncTick_ms(void);
void txsigfox(void* sp,char s);
void rst(uint8_t s);
void rst2(uint8_t s);
int  loquesea(void);
unsigned char UART_SIGFOX_RX( unsigned char * Chr);
 void EmulatedReceived(unsigned char *Str);
WSSFM1XRX_Return_t waittt(WSSFM1XRXConfig_t* obj,uint32_t msec);

void *hilo_time(void *argin){
    struct timespec tm = {0,0.01*1e9}; /*0.5 s*/
        for(;;){
            /*printf("trehad hola\r\n");*/
            nanosleep(&tm, NULL);
            IncTick_ms();
        }
}


/**/

WSSFM1XRXConfig_t SigfoxConfig;
volatile uint32_t tick_count = 0 ;
char respuesta[37];
char respuesta2[37];
struct timespec tmm = {0,0.5*1e9}; /*0.5 s*/
char ChrG;
int x;
WSSFM1XRX_Return_t retvalue;
#define WAIT WSSFM1XRX_Wait_NonBlock
#define	MAX_SIZE_IBUTTON_DATA	12
/*Main*/
int main(int argc, char** argv) {
    uint32_t x = 0;
    char* str = NULL;
    char ID[10]; 
    char PAC[18];
    uint16_t voltaje;
    x = WSSFM1XRX_Init( &SigfoxConfig,rst,rst2,txsigfox ,UART_SIGFOX_RX ,WSSFM1XRX_UL_RCZ4,NULL,GetTick_ms,respuesta,sizeof(respuesta),respuesta2,sizeof(respuesta2));
    pthread_create(&hilo, NULL, hilo_time, NULL);
    retvalue = 255;
    WSSFM1XRX_FreqUL_t Freq;
   

    DataFrame.Latitud= 6.19;
    DataFrame.Longitud = -75.2;

    for(;;){
        /*nanosleep(&tmm, NULL);*/ /*100ms*/          

        /*TEST DE FUNCIONES, PENDIENTE PROBAR CON*/
 

		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_CHK_MODULE;
		printf( "status = %d", SigFoxData.WilsolService_Status);
		while(1 ){
            retvalue = WSSFM1XRX_CheckModule(&SigfoxConfig, WAIT);
            if(WSSFM1XRX_OK_RESPONSE == retvalue ){

                printf( "Buff %s\n", SigfoxConfig.RxFrame);
                x = loquesea();
            }

        }
        



        SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_WKUP;
		printf("status = %d ", SigFoxData.WilsolService_Status);
	    while(  WSSFM1XRX_WAITING == (retvalue = WSSFM1XRX_WakeUP(&SigfoxConfig, WAIT) )){}
        printf( "retvalue %d\n", retvalue);




        SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_SLEEP;
		printf("status = %d", SigFoxData.WilsolService_Status);
	    while(  WSSFM1XRX_WAITING == (retvalue = WSSFM1XRX_Sleep(&SigfoxConfig, WAIT) ) || retvalue == WSSFM1XRX_FAILURE || retvalue == WSSFM1XRX_TIMEOUT){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);

		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_CHK_MODULE;
		printf( "status = %d", SigFoxData.WilsolService_Status);
		while( WSSFM1XRX_WAITING == (retvalue = WSSFM1XRX_CheckModule(&SigfoxConfig, WAIT) ) || retvalue == WSSFM1XRX_FAILURE ){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);

		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_GET_VOLTAGE;
		printf("status = %d ", SigFoxData.WilsolService_Status);
		while( WSSFM1XRX_WAITING == WSSFM1XRX_GetVoltage(&SigfoxConfig,WAIT,&SigFoxData.VBatt) ){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);

		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_CHANGE_FREQ_UL;
		printf( "status = %d ", SigFoxData.WilsolService_Status);
		while( WSSFM1XRX_WAITING == WSSFM1XRX_ChangeFrequencyUL(&SigfoxConfig,WAIT,WSSFM1XRX_UL_RCZ4)   ){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);

		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_SAVE_PARM;
		printf("status = %d ", SigFoxData.WilsolService_Status);
		while( WSSFM1XRX_WAITING == WSSFM1XRX_SaveParameters(&SigfoxConfig,WAIT) ){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);

		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_GET_ID;
		printf("status = %d ", SigFoxData.WilsolService_Status);
		while( WSSFM1XRX_WAITING == WSSFM1XRX_GetID(&SigfoxConfig,WAIT,SigFoxData.ID) ){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);

		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_GET_PAC;
		printf( "status = %d ", SigFoxData.WilsolService_Status);
		while(WSSFM1XRX_WAITING == WSSFM1XRX_GetPAC(&SigfoxConfig,WAIT,SigFoxData.PAC) ){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);



		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_CHK_CHANNELS;
		printf( "status = %d ", SigFoxData.WilsolService_Status);
		while( (retvalue = WSSFM1XRX_CheckChannels(&SigfoxConfig, WAIT ))  == WSSFM1XRX_WAITING || retvalue == WSSFM1XRX_WAITING ){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);

		if(  retvalue == WSSFM1XRX_CHANN_NO_OK   ){
            SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_RST_CHANNELS;
			printf("status = %d ", SigFoxData.WilsolService_Status);
			while( WSSFM1XRX_WAITING ==  WSSFM1XRX_ResetChannels(&SigfoxConfig,WAIT) ){}  /*Se daña con 0.01 en task pero solo con el at$rc*/ /*WSSFM1XRX_Wait_Block*/
		    printf( "Buff %s\n", SigfoxConfig.RxFrame);
        }

		SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_SEND_MESSAGE;
		printf("status = %d ", SigFoxData.WilsolService_Status);
        SigfoxConfig.DownLink =0;
		while( WSSFM1XRX_WAITING == WSSFM1XRX_SendMessage(&SigfoxConfig,WAIT,&DataFrame,MAX_SIZE_IBUTTON_DATA,SigfoxConfig.DownLink) );
		printf( "Buff %s\n", SigfoxConfig.RxFrame);

        SigFoxData.WilsolService_Status = WSSFM1XRX_STATUS_SENT_MESSAGE;
		printf( "status = %d ", SigFoxData.WilsolService_Status);
		


        while( 1  ){}
        printf( "Buff %s\n", SigfoxConfig.RxFrame);
    }
    return (EXIT_SUCCESS);

}
/*Devuelve Tick para que funcionen delay block or nonblock internos*/
uint32_t GetTick_ms (void){
    return tick_count;    
}
/*Tick para que funcionen delay block or nonblock internos*/
void IncTick_ms(void){
   tick_count++;
   if( (tick_count % 1) == 0 ){
      switch (SigFoxData.WilsolService_Status)
      {
          case WSSFM1XRX_STATUS_WKUP:
              /* code */
              
              break;
          case WSSFM1XRX_STATUS_SLEEP:
              /* code */
              EmulatedReceived((unsigned char*)"OK\r");
              break;
          case WSSFM1XRX_STATUS_CHK_MODULE:
              /* code */
              EmulatedReceived((unsigned char*)"OK\r");
              break;
          case WSSFM1XRX_STATUS_GET_VOLTAGE:
              /* code */
              EmulatedReceived((unsigned char*)"3200\r");
              break;
          case WSSFM1XRX_STATUS_CHANGE_FREQ_UL:
              /* code */
              EmulatedReceived((unsigned char*)"OK\r");
              break;
          case WSSFM1XRX_STATUS_SAVE_PARM:
              /* code */
              EmulatedReceived((unsigned char*)"OK\r");
              break;
          case WSSFM1XRX_STATUS_GET_ID:
              /* code */
              EmulatedReceived((unsigned char*)"00445566\r");
              break;                                                                      
          case WSSFM1XRX_STATUS_GET_PAC:
              /* code */
              EmulatedReceived((unsigned char*)"132456789aabbee\r");
              break;
          case WSSFM1XRX_STATUS_CHK_CHANNELS:
              /* code */
              EmulatedReceived((unsigned char*)"1,6\r");
              break;
          case WSSFM1XRX_STATUS_RST_CHANNELS:
              /* code */
              EmulatedReceived((unsigned char*)"OK\r");
              break;                                          
          case WSSFM1XRX_STATUS_SEND_MESSAGE:
              /* code */
              EmulatedReceived((unsigned char*)"OK\r");
              break;        
          default:
              break;
      }
       /*EmulatedReceived((unsigned char*)"OK\r");*/
   }
  /* if(tick_count >= 200) EmulatedReceived((unsigned char*)"444444444444444444444\r");*/
}

/*Wrappers a funciones para inicializar modulo*/
void txsigfox(void* sp,char s){
    static char buf[32];
    static uint8_t index=0, ready = 0;
/*    if(ready == 1) return;*/
    buf[index++] = s;
    if(index > sizeof(buf) -1) index =0;
    buf[index] = 0;
    if(s == '\r'){
        ready = 1;
        index = 0;
       printf("%s\r\n",buf);  /*print frame transmited*/
    }
}
/*sIMULAR FUNCIÓN DE RETARDO por usuario*/
WSSFM1XRX_Return_t waittt(WSSFM1XRXConfig_t* obj,uint32_t msec){ 
     volatile int i =0;
      for(i =0 ; i < 500000000; i++){ }
      /*nanosleep(&tmm, NULL);*/
      return 0;
}

unsigned char UART_SIGFOX_RX( unsigned char *Chr){
    *Chr = (unsigned char)ChrG;       
 	return 1;
 }
 /*Emular/simular interrupción*/
 void EmulatedReceived(unsigned char *Str){
    while(*Str) {
        ChrG = (char)*Str;
        WSSFM1XRX_ISRRX(&SigfoxConfig,ChrG);
        Str++;
    }
 }

void rst(uint8_t s){ }
void rst2(uint8_t s){ }
int  loquesea(void){

    	printf( "loqueasea = %d ", SigFoxData.WilsolService_Status);
    return 0;
}