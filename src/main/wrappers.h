#ifndef WRAPPERS_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <sys/time.h>
    #include <time.h>
    #include <unistd.h>

    #include "wssfm1xrx.h"

    #define WRAPPERS_H

    void ResetControl(uint8_t c);
    void Reset2Control(uint8_t c);
    void OutputFunction(void *sp, char c);
    uint32_t GetTickCountMs(void);


#endif /*WRAPPERS_H*/