
#include "mbtimer.h"

MBTimerTypeDef mbTimer;

void MB_TimerInit(unsigned int baudRate)
{
    mbTimer.tick50usCount = (baudRate > 19200) ? 35 : ((7UL * 220000UL) / (2UL * baudRate));
}

void MB_TimerReload()
{
    mbTimer.tick50us = mbTimer.tick50usCount;
}

void MB_TimerEnable()
{
    mbTimer.tick50us = mbTimer.tick50usCount;
}

void MB_TimerDisable()
{
    mbTimer.tick50us = MB_TIMEOUT_INVALID;
}

void MB_TimerUpdate()
{
    if (mbTimer.tick50us == MB_TIMEOUT_INVALID)
        return;

    mbTimer.tick50us -= 1;
    if (mbTimer.tick50us > 0)
        return;

    mbTimer.tick50us = MB_TIMEOUT_INVALID;

    if (mbState == MB_STATE_INIT) {
        mbState = MB_STATE_IDLE;
    } else if (mbState == MB_STATE_RECV) {
        mbState = MB_STATE_RECEIVEED;
    }
}
