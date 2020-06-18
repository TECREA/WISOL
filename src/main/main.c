/*!
 * *******************************************************************************
 * @file Test with unity
 * @author julian bustamante
 * @version 1.0.0
 * @date Jan 10 , 2020
 * @brief test for verify driver sigfox module
 * @note for higher speed change WSSFM1XRX_DL_TIMEOUT 600  WSSFM1XRX_SEND_MESSAGE_TIME_DELAY_RESP 600 WSSFM1XRX_GENERAL_TIME_DELAY_RESP 400
 *********************************************************************************/
#include "test_with_unity.h"
#include "wrappers.h"
/*===================================================================================*/
WSSFM1XRX_DL_Return_t DownlinkCallback(WSSFM1XRXConfig_t* Instance){
    return WSSFM1XRX_DL_SUCCESS;
}
/*===================================================================================*/


int main(int argc,  char *argv[]){     

    UNITY_BEGIN();
    RUN_TEST(test_sigfox_init);
    
    RUN_TEST(test_sigfox_reset_module);
    RUN_TEST(test_sigfox_wakeup);
    RUN_TEST(test_sigfox_checkModule);
    RUN_TEST(test_sigfox_getvoltage);
    RUN_TEST(test_sigfox_getid);
    RUN_TEST(test_sigfox_getpac);
    RUN_TEST(test_sigfox_changefrequencyUL);
    RUN_TEST(test_sigfox_changefrequencyDL);
    RUN_TEST(test_sigfox_saveparameters);
    RUN_TEST(test_sigfox_checkchannels);
    RUN_TEST(test_sigfox_resetchannels);
    RUN_TEST(test_sigfox_askfrequencyul);
    RUN_TEST(test_sigfox_matchresponse);
    RUN_TEST(test_sigfox_rxisr);
    RUN_TEST(test_sigfox_sendmessage);
    RUN_TEST(test_sigfox_dldiscrimination);
    RUN_TEST(test_sigfox_sleep);
    
    
    return UNITY_END();

/*    return EXIT_SUCCESS;*/
}
/*===================================================================================*/

