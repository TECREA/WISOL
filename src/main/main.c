
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

Channels_t Channels;
pthread_t hilo;
void *hilo_time(void *argin);
uint32_t GetTick_ms (void);
void IncTick_ms(void);
void txsigfox(void* sp,char s);
void rst(uint8_t s);
void rst2(uint8_t s);
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
char respuesta[32];
struct timespec tmm = {0,0.5*1e9}; /*0.5 s*/
char ChrG;
WSSFM1XRX_Return_t retvalue;

/*Main*/
int main(int argc, char** argv) {
    uint32_t x = 0;
    char* str = NULL;
    char ID[10]; 
    char PAC[12];
    uint16_t voltaje;
    x = WSSFM1XRX_Init( &SigfoxConfig,rst,rst2,txsigfox ,UART_SIGFOX_RX ,WSSFM1XRX_UL_RCZ4,NULL,GetTick_ms,respuesta,sizeof(respuesta),respuesta,sizeof(respuesta));
    pthread_create(&hilo, NULL, hilo_time, NULL);
    retvalue = 255;
    WSSFM1XRX_FreqUL_t Freq;
   

    DataFrame.Latitud= 6.19;
    DataFrame.Longitud = -75.2;

    for(;;){
        /*nanosleep(&tmm, NULL);*/ /*100ms*/          

        /*TEST DE FUNCIONES, PENDIENTE PROBAR CON*/
        /*x     = WSSFM1XRX_Sleep(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block);*/     

        /* if(retvalue == 255) */
        /*retvalue = WSSFM1XRX_GetID(&SigfoxConfig, (WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block, ID);*/
        retvalue = WSSFM1XRX_GetPAC(&SigfoxConfig, (WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_NonBlock, PAC);
        /*retvalue = WSSFM1XRX_GetVoltage(&SigfoxConfig, (WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block, &voltaje);*/
        /* retvalue   = WSSFM1XRX_AskChannels(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_NonBlock,PAC);*/
        /* retvalue = WSSFM1XRX_SaveParameters(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block);*/
        /* retvalue   = WSSFM1XRX_CheckChannels(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block);*/
        /* retvalue  = WSSFM1XRX_SendMessage(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block,&DataFrame,12,0);*/
        /* retvalue= WSSFM1XRX_ChangeFrequencyUL(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block, WSSFM1XRX_UL_RCZ4);*/
        /*retvalue     = WSSFM1XRX_ResetChannels(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block);*/
        /*retvalue = WSSFM1XRX_AskFrequencyUL(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block, &Freq );*/

        if(WSSFM1XRX_OK_RESPONSE == retvalue){
            /*printf("ok response %d\r\n",Freq);*/
            printf("==========================================================STRING  %s\r\n", PAC);
            /*printf("channels  %d %d\r\n", Channels.x,Channels.y);*/
        printf("WSSFM1XRX_OK_RESPONSE %d\r\n",retvalue);
            retvalue = 0;
        } /*else printf("return %d\r\n",retvalue);*/
        if(WSSFM1XRX_CHANN_OK == retvalue){
            printf("channels  %d %d\r\n", Channels.x,Channels.y);
        }
        if(WSSFM1XRX_CHANN_NO_OK == retvalue){
            printf("channels no ok  %d %d\r\n", Channels.x,Channels.y);
        }
        /*printf("frame = %s\r\n",SigfoxConfig.TxFrame);*/
        /*printf("hola \r\n");*/
        /*nanosleep(&tmm, NULL);*/
        if(x == WSSFM1XRX_OK_RESPONSE){
            printf("RetValue %d\r\n",x);
            memset((void *)SigfoxConfig.RxFrame,0,sizeof(SigfoxConfig.RxFrame));
            x = 0;
        } 
        /*printf("frame = %s\r\n",SigfoxConfig.RxFrame);*/
        if(x == WSSFM1XRX_RSP_NOMATCH){
            printf("RetValue %d\r\n",x);
        }
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
   if( (tick_count % 40) == 0 ){
       EmulatedReceived((unsigned char*)"123456\r");
   }
}

/*Wrappers a funciones para inicializar modulo*/
void txsigfox(void* sp,char s){
    static char buf[32];
    static uint8_t index=0, ready = 0;
    
    if(ready == 1) return;
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