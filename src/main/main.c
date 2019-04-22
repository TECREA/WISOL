
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

#ifdef Doce_Bytes
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
			unsigned DI:1; 						// DI  LSB en el orden que lo coloque acá  DI, BattLow, Type  hacia abajo
			unsigned BattLow:1;
			unsigned Type:5;
			unsigned periodic:1;
		};
	}Doce_Bytes;
	float Longitud;
	float Latitud;

#endif
}DataFrame_t;

DataFrame_t DataFrame ={0};

pthread_t hilo;
void *hilo_time(void *argin);
uint32_t GetTick_ms (void);
void IncTick_ms(void);
void txsigfox(void* sp,char s);
void rst(uint8_t s);
void rst2(uint8_t s);

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
uint8_t respuesta[32]="OK\r\n";
struct timespec tmm = {0,0.5*1e9}; /*0.5 s*/
int main(int argc, char** argv) {
    uint32_t x = 0;
    char* str = NULL;
    x = WSSFM1XRX_Init( &SigfoxConfig,rst,rst2,txsigfox ,NULL,WSSFM1XRX_UL_RCZ4,NULL,GetTick_ms,500);
    pthread_create(&hilo, NULL, hilo_time, NULL);
    
    for(;;){

        /*nanosleep(&tmm, NULL);*/ /*100ms*/          
         
         /*TEST DE FUNCIONES, PENDIENTE PROBAR CON*/
        /*x     = WSSFM1XRX_Sleep(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block);*/
        /*str   = WSSFM1XRX_GetID(&SigfoxConfig);*/
        /*str   = WSSFM1XRX_GetPAC(&SigfoxConfig);*/
        /*str   = WSSFM1XRX_AskChannels(&SigfoxConfig);*/
        /*x     = WSSFM1XRX_CheckChannels(&SigfoxConfig);*/
        /*x     = WSSFM1XRX_ResetChannels(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block);*/
        /*x     = WSSFM1XRX_ChangeFrequencyUL(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block);*/
        /*str   = WSSFM1XRX_AskFrequencyUL(&SigfoxConfig);*/
        /*x     = WSSFM1XRX_SendMessage(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)WSSFM1XRX_Wait_Block,&DataFrame,12,0);*/
        x       = WSSFM1XRX_CheckModule(&SigfoxConfig,(WSSFM1XRX_WaitMode_t)waittt);

        /*printf("STRING %s\r\n",str);*/
        /*printf("frame = %s\r\n",SigfoxConfig.TxFrame);*/
   
   
        /* printf("hola \r\n");
        nanosleep(&tmm, NULL);*/
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
       strcpy((char*)SigfoxConfig.RxFrame,"OK\r");  /*100ms*/
       SigfoxConfig.RxReady = 1;
   }
   /*   printf("tick count = %s\r\n",SigfoxConfig.RX_SIGFOX);*/
}

/*Wrappers a funciones para inicializar modulo*/
void txsigfox(void* sp,char s){

}
void rst(uint8_t s){

}
void rst2(uint8_t s){

}
/*sIMULAR FUNCIÓN DE RETARDO*/
WSSFM1XRX_Return_t waittt(WSSFM1XRXConfig_t* obj,uint32_t msec){ 
     volatile int i =0;
      for(i =0 ; i < 500000000; i++){

      }
      /*nanosleep(&tmm, NULL);*/
      return 0;
}
