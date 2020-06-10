
#ifndef SOURCE_TEST_WITH_UNITY_H_

#define SOURCE_TEST_WITH_UNITY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "wssfm1xrx.h"
#include "wrappers.h"
#include "unity.h"

#undef _RESPONSE_BAD  /*define if want see functions with errors controled*/

    typedef struct{
        struct timespec xdelay;
        int xcount;
        WSSFM1XRX_Return_t RetValue;
        char ResponseExpected[45];
    }TestData_t;
 
/*===================================================================================*/

void setUp (void); /* Is run before every test, put unit init calls here. */
void tearDown (void); /* Is run after every test, put unit clean-up calls here. */
/*===================================================================================*/

void test_sigfox_init();
void test_sigfox_reset_module();
void test_sigfox_wakeup();
void test_sigfox_checkModule();
void test_sigfox_getvoltage();
void test_sigfox_getid();
void test_sigfox_getpac();
void test_sigfox_changefrequencyUL();
void test_sigfox_changefrequencyDL();
void test_sigfox_saveparameters();
void test_sigfox_checkchannels();
void test_sigfox_resetchannels();
void test_sigfox_matchresponse();
void test_sigfox_rxisr();
void test_sigfox_sendmessage();
void test_sigfox_askfrequencyul();
void test_sigfox_dldiscrimination();
void test_sigfox_sleep();

#endif