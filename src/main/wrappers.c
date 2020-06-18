#include "wrappers.h"
#include "test_with_unity.h"

extern TestData_t TestData;
extern WSSFM1XRXConfig_t Driver_WSS;

/*====================================================================================*/
void ResetControl(uint8_t c){
    puts(c? "RESET=ON": "RESET=OFF");
}
/*====================================================================================*/
void Reset2Control(uint8_t c){
    puts(c? "RESET2=ON": "RESET2=OFF");
}
/*====================================================================================*/
void OutputFunction(void *sp, char c){
	int i = 0; 
	if(c != '\r')
		printf("%c",c); /*no print \r in console */
	
	if(c == '\r') {	 /*if send command to module*/
		while(TestData.ResponseExpected[i] != '\0') { /*then the module response is pass to isr char by char */
		 	WSSFM1XRX_ISRRX(&Driver_WSS, TestData.ResponseExpected[i]);
			i++;
		}
		printf(" --> %s\r\n",TestData.ResponseExpected);  /*print command and response*/

	}
}
/*====================================================================================*/
uint32_t GetTickCountMs(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_nsec / 1000000) + ((uint32_t)ts.tv_sec * 1000ull);
}
/*====================================================================================*/