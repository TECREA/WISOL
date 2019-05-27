#include "wrappers.h"


TestData_t TestData;

char ret_str[20][100]=
{
	"WSSFM1XRX_TIMEOUT",
	"WSSFM1XRX_WAITING",
	"WSSFM1XRX_RSP_NOMATCH",
	"WSSFM1XRX_OK_RESPONSE",
	"WSSFM1XRX_NONE",
	"WSSFM1XRX_INIT_OK",
	"WSSFM1XRX_PROCESS_FAILED", 		
	"WSSFM1XRX_CHANN_OK", 			
	"WSSFM1XRX_CHANN_NO_OK", 		
	"WSSFM1XRX_DEFAULT",
	"WSSFM1XRX_FAILURE",
	"WSSFM1XRX_MAX_RETRIES_REACHED"
};

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
	printf("%c",c);
}
/*====================================================================================*/
uint32_t GetTickCountMs(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_nsec / 1000000) + ((uint32_t)ts.tv_sec * 1000ull);
}
/*====================================================================================*/