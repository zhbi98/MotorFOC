
#ifndef __MBTIMER_H__
#define __MBTIMER_H__

#include "mbrtu.h"

#define MB_TIMEOUT_INVALID (65535U)

typedef struct {
    unsigned int tick50us;
    unsigned int tick50usCount;
} MBTimerTypeDef;

extern MBTimerTypeDef mbTimer;

extern void MB_TimerInit(unsigned int baudRate);
extern void MB_TimerReload();
extern void MB_TimerEnable();
extern void MB_TimerDisable();
extern void MB_TimerUpdate();

#endif
