#ifndef WRAPPERS_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <sys/time.h>
    #include <time.h>
    #include <unistd.h>

    #include "wssfm1xrx.h"

    #define WRAPPERS_H
    extern char ret_str[20][100];

    typedef struct{
        struct timespec xdelay;
        int xcount;
        WSSFM1XRX_Return_t RetValue;
    }TestData_t;
 
    extern TestData_t TestData;

    #define TEST_WSSFM_WAITUNTIL( Response, chk, LibCall, xresp, expected)  puts(#LibCall); \
                                                                            TestData.xdelay.tv_sec = 0; \
                                                                            TestData.xdelay.tv_nsec = 0.1*1e9; \
                                                                            TestData.xcount = 0; \
                                                                            while( (Response) chk (TestData.RetValue = LibCall) ) { \
                                                                                nanosleep(&TestData.xdelay, NULL); \
                                                                                if(TestData.xcount++ == xresp){ \
                                                                                    puts("\r\n"); \
                                                                                    puts(expected); \
                                                                                    strcpy((char*)Driver_WSS.RxFrame, expected); \
                                                                                    Driver_WSS.RxReady = SF_TRUE; \
                                                                                } \
                                                                            }printf("RetValue = %s\r\n", ret_str[TestData.RetValue])


    void ResetControl(uint8_t c);
    void Reset2Control(uint8_t c);
    void OutputFunction(void *sp, char c);
    uint32_t GetTickCountMs(void);


#endif /*WRAPPERS_H*/